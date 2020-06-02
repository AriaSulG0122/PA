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
  //��Ҫ�ṩ����ҳ�ķ���ͻ��������ص����������ڴ�AM�л�ȡ/�ͷ�����ҳ
  palloc_f = palloc;
  pfree_f = pfree;

  int i;

  // make all PDEs invalid
  for (i = 0; i < NR_PDE; i ++) {
    kpdirs[i] = 0;
  }
  //��д�ں˵�ҳĿ¼��ҳ��
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
  //����CR3�Ĵ���
  set_cr3(kpdirs);
  //ͨ������CR0�Ĵ�����������ҳ����
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

//�������ַ�ռ�p�е������ַvaӳ�䵽�����ַpa
void _map(_Protect *p, void *va, void *pa,void* mydata) {
  
  //��ȡҳĿ¼�Ļ���ַpgdir
  PDE* pde,*pgdir=p->ptr;
  PTE *pgtable;
  //��ȡva��Ӧ��ҳĿ¼���ַpde
  pde=&pgdir[PDX(va)];
  if(*pde&PTE_P){//��ҳĿ¼����ڣ����ȡ��Ӧ��ҳ�����ַ
    pgtable=(PTE*)PTE_ADDR(*pde);
  }else{//��ҳĿ¼�����
    //�����������ҳ
    pgtable=(PTE*)palloc_f();
    //��������ҳ���㣬����Ŀǰ��ÿһ��ҳ���������ӳ��
    for(int i=0;i<NR_PTE;i++){
      pgtable[i]=0;
    }
    //���ø�ҳĿ¼���Pλ����һ�η��ʾʹ�����
    *pde=PTE_ADDR(pgtable)|PTE_P;
  }
  //����ҳ����������ҳ��ӳ���ϵ��ͬʱ����Pλ
  pgtable[PTX(va)]=PTE_ADDR(pa)|PTE_P;
  mydata=pa;
}

void _unmap(_Protect *p, void *va) {
}

_RegSet *_umake(_Protect *p, _Area ustack, _Area kstack, void *entry, char *const argv[], char *const envp[]) {
  return NULL;
}
