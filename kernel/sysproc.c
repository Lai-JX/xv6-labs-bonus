#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
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
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  /*lab traps ð*/
  backtrace();
  /*lab traps ð*/
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
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

  if(argint(0, &pid) < 0)
    return -1;
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

/* lab traps ð */
uint64
sys_sigalarm(void)
{
  // å¨ç³»ç»è°ç¨æ¶ï¼è¿è¡åå§å
  int ticks;
  uint64 handler;
  struct proc *p = myproc();
  // è·ååæ°
  if (argint(0,&ticks) < 0)
    return -1;
  if (argaddr(1,&handler) < 0)
    return -1;
  acquire(&p->lock);
  p->ticks = ticks;       // ä¸¤æ¬¡alarmé´éçæ¶éå¨ææ°
  p->handler = handler;   // handlerå¤çå½æ°çå°å
  p->cur_ticks = 0;       // åå§åä¸º0
  release(&p->lock);
  return 0;
}

uint64
sys_sigreturn(void)
{
  struct proc *p = myproc();
  acquire(&p->lock);
  if (p->save_trapframe)
  {
    // æ¢å¤ä¹åä¿å­çå¯å­å¨
    memmove(p->trapframe, p->save_trapframe, PGSIZE);
    kfree(p->save_trapframe);
    p->save_trapframe = 0;
  }
  release(&p->lock);
  return 0;
}
