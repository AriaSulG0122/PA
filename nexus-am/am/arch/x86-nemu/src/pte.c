#include <x86.h>

#define PG_ALIGN __attribute((aligned(PGSIZE)))

static PDE kpdirs[NR_PDE] PG_ALIGN;
static PTE kptabs[PMEM_SIZE / PGSIZE] PG_ALIGN;
static void* (*palloc_f)();
static void (*pfree_f)(void*);

_Area segments[] = {      // Kernel memory mappings
  {.start = (void*)0,          .end = (void*)PMEM_SIZE}
};

#define NR_KSEG_MAP (sizeof(segments) / sizeof(segments[0]))

void _pte_init(void* (*palloc)(), void (*pfree)(void*)) {
  //需要提供物理页的分配和回收两个回调函数，用于从AM中获取/释放物理页
  palloc_f = palloc;
  pfree_f = pfree;

  int i;

  // make all PDEs invalid
  for (i = 0; i < NR_PDE; i ++) {
    kpdirs[i] = 0;
  }
  //填写内核的页目录和页表
  PTE *ptab = kptabs;
  for (i = 0; i < NR_KSEG_MAP; i ++) {
    uint32_t pdir_idx = (uintptr_t)segments[i].start / (PGSIZE * NR_PTE);
    uint32_t pdir_idx_end = (uintptr_t)segments[i].end / (PGSIZE * NR_PTE);
    for (; pdir_idx < pdir_idx_end; pdir_idx ++) {
      // fill PDE
      kpdirs[pdir_idx] = (uintptr_t)ptab | PTE_P;

      // fill PTE
      PTE pte = PGADDR(pdir_idx, 0, 0) | PTE_P;
      PTE pte_end = PGADDR(pdir_idx + 1, 0, 0) | PTE_P;
      for (; pte < pte_end; pte += PGSIZE) {
        *ptab = pte;
        ptab ++;
      }
    }
  }
  //设置CR3寄存器
  set_cr3(kpdirs);
  //通过设置CR0寄存器来开启分页机制
  set_cr0(get_cr0() | CR0_PG);
}

void _protect(_Protect *p) {
  PDE *updir = (PDE*)(palloc_f());
  p->ptr = updir;
  // map kernel space
  for (int i = 0; i < NR_PDE; i ++) {
    updir[i] = kpdirs[i];
  }

  p->area.start = (void*)0x8000000;
  p->area.end = (void*)0xc0000000;
}

void _release(_Protect *p) {
}

void _switch(_Protect *p) {
  set_cr3(p->ptr);
}

//将虚拟地址空间p中的虚拟地址va映射到物理地址pa
void _map(_Protect *p, void *va, void *pa,void* mydata) {
  
  //获取页目录的基地址pgdir
  PDE* pde,*pgdir=p->ptr;
  PTE *pgtable;
  //获取va对应的页目录项地址pde
  pde=&pgdir[PDX(va)];
  if(*pde&PTE_P){//若页目录项存在，则获取对应的页表基地址
    pgtable=(PTE*)PTE_ADDR(*pde);
  }else{//若页目录项不存在
    //申请空闲物理页
    pgtable=(PTE*)palloc_f();
    //将该物理页清零，表明目前的每一个页表项都不存在映射
    for(int i=0;i<NR_PTE;i++){
      pgtable[i]=0;
    }
    //设置该页目录项及其P位，下一次访问就存在了
    *pde=PTE_ADDR(pgtable)|PTE_P;
  }
  //设置页表项中物理页的映射关系，同时设置P位
  pgtable[PTX(va)]=PTE_ADDR(pa)|PTE_P;
  mydata=pa;
}

void _unmap(_Protect *p, void *va) {
}

_RegSet *_umake(_Protect *p, _Area ustack, _Area kstack, void *entry, char *const argv[], char *const envp[]) {
  return NULL;
}
