#include "proc.h"
#include "memory.h"

static void *pf = NULL;

void* new_page(void) {
  assert(pf < (void *)_heap.end);
  void *p = pf;
  pf += PGSIZE;
  return p;
}

void free_page(void *p) {
  panic("not implement yet");
}

/* The brk() system call handler. */
int mm_brk(uint32_t new_brk) {
  /*
  if(current->cur_brk==0){
    current->cur_brk=current->max_brk=new_brk;
  }else{
    if(new_brk>current->max_brk){
      uintptr_t pa,va;
      for(va=(current->max_brk+0xfff)&~0xfff;va<new_brk;va+=PGSIZE){
        pa=(uintptr_t)new_page();
        _map(&current->as,(void*)va,(void*)pa);
      }
      current->max_brk=va;
    }
    current->cur_brk=new_brk;
  }*/
  return 0;
}

//初始化MM，MM是存储管理器(Memory Manager)模块，专门负责分页相关的存储管理
void init_mm() {
  //将TRM提供的堆区起始地址作为空闲物理页的首地址
  //将来会通过new_page()函数来分配空闲的物理页
  pf = (void *)PGROUNDUP((uintptr_t)_heap.start);
  Log("free physical pages starting from %p", pf);
  //调用AM的_pte_init函数
  _pte_init(new_page, free_page);
}
