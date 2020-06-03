#include "proc.h"
#include "memory.h"

static void *pf = NULL;

void *new_page(void)
{
  assert(pf < (void *)_heap.end);
  void *p = pf;
  pf += PGSIZE;
  return p;
}

void free_page(void *p)
{
  panic("not implement yet");
}

/* The brk() system call handler. */
int mm_brk(uint32_t new_brk)
{

  if (current->cur_brk == 0)
  {
    current->cur_brk = current->max_brk = new_brk;
  }
  else
  {
    if (new_brk > current->max_brk)
    {//��[current->max_brk,new_brk)�Ŀռ�ӳ�䵽��ַ�ռ�current->as
      /*
      uintptr_t page_start = PGROUNDUP(current->max_brk);
      uintptr_t page_end = PGROUNDUP(new_brk);
      for (; page_start <= page_end; page_start += PGSIZE) {
        _map(&current->as, (void *)page_start, new_page());
        */
       int size = new_brk - current->max_brk;
      void *page;
      void* va = (void*)PGROUNDUP(current->max_brk); 
      for(int i=0;i<size;i+=PGSIZE){
        page = (void*)new_page();
        _map(&current->as, va + i, page);
      }
      current->max_brk = new_brk;
    }
    current->cur_brk = new_brk;
  }
  return 0;
}

//��ʼ��MM��MM�Ǵ洢������(Memory Manager)ģ�飬ר�Ÿ����ҳ��صĴ洢����
void init_mm()
{
  //��TRM�ṩ�Ķ�����ʼ��ַ��Ϊ��������ҳ���׵�ַ
  //������ͨ��new_page()������������е�����ҳ
  pf = (void *)PGROUNDUP((uintptr_t)_heap.start);
  Log("free physical pages starting from %p", pf);
  //����AM��_pte_init����
  _pte_init(new_page, free_page);
}
