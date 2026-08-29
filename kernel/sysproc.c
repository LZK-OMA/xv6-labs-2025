#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  kexit(n);
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
  return kfork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return kwait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int t;
  int n;

  argint(0, &n);
  argint(1, &t);
  addr = myproc()->sz;

  if(t == SBRK_EAGER || n < 0) {
    if(growproc(n) < 0) {
      return -1;
    }
  } else {
    // Lazily allocate memory for this process: increase its memory
    // size but don't allocate memory. If the processes uses the
    // memory, vmfault() will allocate it.
    if(addr + n < addr)
      return -1;
    myproc()->sz += n;
  }
  return addr;
}

uint64
sys_pause(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  backtrace();
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kkill(pid);
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
sys_sigalarm(void)
{
  int ticks;
  uint64 handler;

  argint(0, &ticks);
  argaddr(1, &handler);

  struct proc *p = myproc();
  p->alarmticks = ticks;
  p->alarmtickscount = 0;

  // 如果参数是 (0, 0)，则关闭闹钟功能
  if(ticks == 0 && handler == 0){
    p->alarmhandler = -1;
  } else {
    p->alarmhandler = handler;
  }

  return 0;
}

uint64
sys_sigreturn(void)
{
  struct proc *p = myproc();

  // 1. 从备份中恢复原来的所有寄存器状态
  memmove(p->trapframe, &p->alarm_tf, sizeof(struct trapframe));

  // 2. 解除正在执行处理函数的标记
  p->inhandler = 0;
  
  // 3. 重置计数器
  p->alarmtickscount = 0;

  // 必须返回被中断处的 a0，否则原程序获取的返回值会出错
  return p->trapframe->a0;
}