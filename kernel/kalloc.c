// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end, int pgsize);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock4kb;
  struct spinlock lock2mb;
  struct run *freelist4kb;
  struct run *freelist2mb;
} kmem;

void
kinit()
{
  initlock(&kmem.lock4kb, "kmem4kb");
  initlock(&kmem.lock2mb, "kmem2mb");
  freerange(end, (void*)SUPERPGPHYSTOP, SUPERPGSIZE);
  freerange((void*)SUPERPGPHYSTOP, (void*)PHYSTOP, PGSIZE);
}

void
freerange(void *pa_start, void *pa_end, int pgsize)
{
  char *p;

  if(pgsize == PGSIZE){
    p = (char*)PGROUNDUP((uint64)pa_start);
    for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
      kfree(p);
  }
  else {
    p = (char*)SUPERPGROUNDUP((uint64)pa_start);
    for(; p + SUPERPGSIZE <= (char*)pa_end; p += SUPERPGSIZE)
      superfree(p);
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

  if(((uint64)pa % PGSIZE) != 0 || (uint64)pa < SUPERPGPHYSTOP || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock4kb);
  r->next = kmem.freelist4kb;
  kmem.freelist4kb = r;
  release(&kmem.lock4kb);
}

void superfree(void* pa) {
  struct run *r;

  if(((uint64)pa % SUPERPGSIZE) != 0 || (char*)pa < end || (uint64)pa >= SUPERPGPHYSTOP)
    panic("superfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, SUPERPGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock2mb);
  r->next = kmem.freelist2mb;
  kmem.freelist2mb = r;
  release(&kmem.lock2mb);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock4kb);
  r = kmem.freelist4kb;
  if(r)
    kmem.freelist4kb = r->next;
  release(&kmem.lock4kb);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}

void* superalloc() { 
  struct run *r;

  acquire(&kmem.lock2mb);
  r = kmem.freelist2mb;
  if(r)
    kmem.freelist2mb = r->next;
  release(&kmem.lock2mb);

  if(r)
    memset((char*)r, 5, SUPERPGSIZE); // fill with junk
  return (void*)r;
}
