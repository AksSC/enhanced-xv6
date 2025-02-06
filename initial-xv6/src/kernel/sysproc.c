#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0; // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if (growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n)
  {
    if (killed(myproc()))
    {
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_waitx(void)
{
  uint64 addr, addr1, addr2;
  uint wtime, rtime;
  argaddr(0, &addr);
  argaddr(1, &addr1); // user virtual memory
  argaddr(2, &addr2);
  int ret = waitx(addr, &wtime, &rtime);
  struct proc *p = myproc();
  if (copyout(p->pagetable, addr1, (char *)&wtime, sizeof(int)) < 0)
    return -1;
  if (copyout(p->pagetable, addr2, (char *)&rtime, sizeof(int)) < 0)
    return -1;
  return ret;
}

uint64
sys_getSysCount(void)
{
  int sysc;
  argint(0, &sysc);
  if(sysc < 0 || sysc > 31) return -1;
  return myproc()->child_syscall_count[sysc];
}

uint64
sys_sigalarm(void)
{
  int t;
  uint64 handler;
  argint(0, &t);
  argaddr(1, &handler);
  struct proc *p = myproc();
  p->alarm_t = t;
  p->alarm = 1;
  p->alarm_handler = handler;
  p->handle = 1;
  return 0;
}

uint64 
sys_sigreturn(void)
{
  struct proc *p = myproc();
  p->handle = 1;
  memmove(p->trapframe, p->alarm_trapframe, PGSIZE);
  kfree(p->alarm_trapframe);
  return p->trapframe->a0;
}

uint64
sys_settickets(void){
  int num;
  argint(0, &num);
  if(num < 1) return -1;
  myproc()->tickets = num;
  return num;
}

extern uint page_faults;
int sys_get_page_faults(void) {
    int ret = myproc()->page_faults;
    return ret;
}