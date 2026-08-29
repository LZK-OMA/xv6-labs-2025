// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);
void incref(uint64 pa);
void decref(uint64 pa);


extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

// 引用计数相关
struct spinlock ref_lock;
int refcount[(PHYSTOP - KERNBASE) / PGSIZE];  // 物理页引用计数数组

// 获取物理页索引
#define PA2IDX(pa) (((uint64)pa - KERNBASE) / PGSIZE)

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  initlock(&ref_lock, "refcount"); 
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  acquire(&ref_lock); 
  for (; p + PGSIZE <= (char*)pa_end; p += PGSIZE) {
    refcount[PA2IDX(p)] = 1;  // 初始化引用计数为1
  }  

  release(&ref_lock);
  // 调用 kfree，此时 kfree 会获取锁并减少计数
  p = (char*)PGROUNDUP((uint64)pa_start);
  for (; p + PGSIZE <= (char*)pa_end; p += PGSIZE) {
    kfree(p);  // kfree 内部会获取 ref_lock，减少计数，然后释放
  }
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // 减少引用计数，为0释放
  acquire(&ref_lock);
  int idx = PA2IDX((uint64)pa);
  if (refcount[idx] > 0) {
    refcount[idx]--;
  }
  if (refcount[idx] > 0) {
    release(&ref_lock);
    return;  // 不释放
  }
  release(&ref_lock);

  // Fill with junk to catch dangling refs.
  //memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if (r) {
    // 初始化引用计数为1
    acquire(&ref_lock);
    refcount[PA2IDX((uint64)r)] = 1;
    release(&ref_lock);
  }
    //memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}

void
incref(uint64 pa)
{
  if ((uint64)pa < KERNBASE || (uint64)pa >= PHYSTOP)
    panic("incref");
  acquire(&ref_lock);
  refcount[PA2IDX(pa)]++;
  release(&ref_lock);
}

// 减少引用计数（其实就是调用kfree）
void
decref(uint64 pa)
{
  if ((uint64)pa < KERNBASE || (uint64)pa >= PHYSTOP)
    panic("decref");
  kfree((void*)pa);
}