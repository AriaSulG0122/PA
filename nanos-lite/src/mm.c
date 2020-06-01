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
  return 0;
}

//��ʼ��MM��MM�Ǵ洢������(Memory Manager)ģ�飬ר�Ÿ����ҳ��صĴ洢����
void init_mm() {
  //��TRM�ṩ�Ķ�����ʼ��ַ��Ϊ��������ҳ���׵�ַ
  //������ͨ��new_page()������������е�����ҳ
  pf = (void *)PGROUNDUP((uintptr_t)_heap.start);
  Log("free physical pages starting from %p", pf);
  //����AM��_pte_init����
  _pte_init(new_page, free_page);
}
