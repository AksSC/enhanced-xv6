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

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
  int refcount[(PHYSTOP - KERNBASE) / PGSIZE]; // counts references to each page
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE){
    kmem.refcount[((uint64)p - KERNBASE)/PGSIZE] = 1; // initialize refcount to 1
    kfree(p);
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

  acquire(&kmem.lock);

  int idx = ((uint64)pa - KERNBASE) / PGSIZE;
  if(kmem.refcount[idx] < 1)
    panic("kfree: refcount underflow");
  kmem.refcount[idx]--;
  if(kmem.refcount[idx] == 0){
    // Fill with junk to catch dangling refs.
    memset(pa, 1, PGSIZE);
    r = (struct run*)pa;
    r->next = kmem.freelist;
    kmem.freelist = r;
  }

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
  if(r){
    kmem.freelist = r->next;
    int idx = ((uint64)r - KERNBASE) / PGSIZE;
    if(kmem.refcount[idx] != 0)
      panic("kalloc refcount error");
    kmem.refcount[idx] = 1;
  }
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}

void
inc_ref(void *pa){
  acquire(&kmem.lock);
  int idx = ((uint64)pa - KERNBASE) / PGSIZE;
  if(kmem.refcount[idx] < 1)
    panic("inc_ref: refcount underflow");
  kmem.refcount[idx]++;
  // printf("inc_ref: refcount[%d] = %d\n", idx, kmem.refcount[idx]);
  release(&kmem.lock);
}
