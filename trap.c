#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"

// Debug flag for page fault messages is controlled by Makefile
// Compile with: make DEBUG=1 to enable debug output
// Compile with: make DEBUG=0 (or just make) to disable debug output

// Interrupt descriptor table (shared by all CPUs).
struct gatedesc idt[256];
extern uint vectors[];  // in vectors.S: array of 256 entry pointers
struct spinlock tickslock;
uint ticks;

void
tvinit(void)
{
  int i;

  for(i = 0; i < 256; i++)
    SETGATE(idt[i], 0, SEG_KCODE<<3, vectors[i], 0);
  SETGATE(idt[T_SYSCALL], 1, SEG_KCODE<<3, vectors[T_SYSCALL], DPL_USER);

  initlock(&tickslock, "time");
}

void
idtinit(void)
{
  lidt(idt, sizeof(idt));
}

//PAGEBREAK: 41
void
trap(struct trapframe *tf)
{
  if(tf->trapno == T_SYSCALL){
    if(myproc()->killed)
      exit();
    myproc()->tf = tf;
    syscall();
    if(myproc()->killed)
      exit();
    return;
  }

  switch(tf->trapno){
  case T_IRQ0 + IRQ_TIMER:
    if(cpuid() == 0){
      acquire(&tickslock);
      ticks++;
      wakeup(&ticks);
      release(&tickslock);
    }
    // Increment ticks_running for the currently running process
    if(myproc() != 0 && myproc()->state == RUNNING){
      myproc()->ticks_running++;
      // Decrement time slice for PRIORITYRR scheduler
      #ifdef PRIORITYRR
      myproc()->time_slice--;
      #endif
    }
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE:
    ideintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE+1:
    // Bochs generates spurious IDE1 interrupts.
    break;
  case T_IRQ0 + IRQ_KBD:
    kbdintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_COM1:
    uartintr();
    lapiceoi();
    break;
  case T_IRQ0 + 7:
  case T_IRQ0 + IRQ_SPURIOUS:
    cprintf("cpu%d: spurious interrupt at %x:%x\n",
            cpuid(), tf->cs, tf->eip);
    lapiceoi();
    break;

  case T_PGFLT:
  {
      // Lazy page allocation: allocate page on first access
      uint faulting_address = rcr2();
      struct proc *curproc = myproc();
      char *mem;
      uint page_addr;
      
      // Round to page boundary
      page_addr = PGROUNDDOWN(faulting_address);
      
      #if DEBUG_PAGEFAULT
      cprintf("[PID %d] Page fault at address 0x%x (page 0x%x)\n", 
              curproc->pid, faulting_address, page_addr);
      #endif
      
      // Validate the faulting address:
      // 1. Must be below process size (within allocated region)
      // 2. Must be above first page (guard page for null pointer protection)
      if(faulting_address >= PGSIZE && faulting_address < curproc->sz){
          #ifdef LOCALITY
          // Locality-aware allocation: allocate 3 pages at once
          // (faulting page + 2 subsequent pages)
          #if DEBUG_PAGEFAULT
          cprintf("[PID %d] LOCALITY-AWARE: Attempting to allocate 3 pages starting at 0x%x\n",
                  curproc->pid, page_addr);
          #endif
          int pages_allocated = 0;
          int i;
          
          for(i = 0; i < 3; i++){
              uint current_page = page_addr + (i * PGSIZE);
              pte_t *pte;
              
              // Check if page is within process size
              if(current_page >= curproc->sz)
                  break;
              
              // Check if page is already allocated
              pte = walkpgdir(curproc->pgdir, (void*)current_page, 0);
              if(pte != 0 && (*pte & PTE_P))
                  continue; // Page already allocated, skip it
              
              // Allocate physical page
              mem = kalloc();
              if(mem == 0){
                  if(pages_allocated == 0){
                      // Failed to allocate even the faulting page
                      cprintf("locality_alloc: out of memory\n");
                      curproc->killed = 1;
                  }
                  // Partial allocation is acceptable for locality-aware
                  break;
              }
              
              // Zero the page for security
              memset(mem, 0, PGSIZE);
              
              // Map the page with user and write permissions
              if(mappages(curproc->pgdir, (char*)current_page, PGSIZE, V2P(mem), PTE_W|PTE_U) < 0){
                  kfree(mem);
                  if(pages_allocated == 0){
                      // Failed to map even the faulting page
                      cprintf("locality_alloc: mappages failed\n");
                      curproc->killed = 1;
                  }
                  // Partial allocation is acceptable
                  break;
              }
              #if DEBUG_PAGEFAULT
              cprintf("[PID %d] LOCALITY-AWARE: Allocated page %d/3 at 0x%x\n",
                      curproc->pid, i+1, current_page);
              #endif
              pages_allocated++;
          }
          #if DEBUG_PAGEFAULT
          cprintf("[PID %d] LOCALITY-AWARE: Total pages allocated = %d\n",
                  curproc->pid, pages_allocated);
          #endif
          #else
          // Pure lazy allocation: allocate only the faulting page
          #if DEBUG_PAGEFAULT
          cprintf("[PID %d] LAZY: Allocating single page at 0x%x\n",
                  curproc->pid, page_addr);
          #endif
          mem = kalloc();
          if(mem == 0){
              cprintf("lazy_alloc: out of memory\n");
              curproc->killed = 1;
              break;
          }
          // Zero the page for security (prevent data leaks)
          memset(mem, 0, PGSIZE);
          
          // Map the page with user and write permissions
          if(mappages(curproc->pgdir, (char*)page_addr, PGSIZE, V2P(mem), PTE_W|PTE_U) < 0){
              cprintf("lazy_alloc: mappages failed\n");
              kfree(mem);
              curproc->killed = 1;
              break;
          }
          #if DEBUG_PAGEFAULT
          cprintf("[PID %d] LAZY: Successfully allocated page at 0x%x\n",
                  curproc->pid, page_addr);
          #endif
          #endif
      } else {
          #if DEBUG_PAGEFAULT
          cprintf("[PID %d] SEGFAULT: Invalid address 0x%x (process size: 0x%x)\n",
                  curproc->pid, faulting_address, curproc->sz);
          #endif
          curproc->killed = 1;
      }
  }
  break;

  //PAGEBREAK: 13
  default:
    if(myproc() == 0 || (tf->cs&3) == 0){
      // In kernel, it must be our mistake.
      cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x)\n",
              tf->trapno, cpuid(), tf->eip, rcr2());
      panic("trap");
    }
    // In user space, assume process misbehaved.
    cprintf("pid %d %s: trap %d err %d on cpu %d "
            "eip 0x%x addr 0x%x--kill proc\n",
            myproc()->pid, myproc()->name, tf->trapno,
            tf->err, cpuid(), tf->eip, rcr2());
    myproc()->killed = 1;
  }

  // Force process exit if it has been killed and is in user space.
  // (If it is still executing in the kernel, let it keep running
  // until it gets to the regular system call return.)
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();

  // Force process to give up CPU on clock tick.
  // If interrupts were on while locks held, would need to check nlock.
  #ifdef SJF
  // For SJF, don't yield on timer (non-preemptive)
  // Process runs until it blocks or exits
  #elif defined(PRIORITYRR)
  // For PRIORITYRR, yield when time slice expires OR when preempted by higher priority
  if(myproc() && myproc()->state == RUNNING && tf->trapno == T_IRQ0+IRQ_TIMER){
    // Always yield on timer to allow priority preemption
    if(myproc()->time_slice <= 0){
      myproc()->time_slice = TIME_QUANTUM;
    }
    yield();
  }
  #else
  // For DEFAULT, yield on every timer tick (round-robin)
  if(myproc() && myproc()->state == RUNNING &&
     tf->trapno == T_IRQ0+IRQ_TIMER)
    yield();
  #endif

  // Check if the process has been killed since we yielded
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();
}
