#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */

  //TODO();
  //保存跳转前的状态
  rtl_push(&cpu.eflags);//将EFLAGS压入堆栈
  rtl_li(&t0,cpu.CS);
  rtl_push(&t0);//将CS压入堆栈
  rtl_li(&t0,ret_addr);
  rtl_push(&t0);//将EIP压入堆栈，作为返回地址
  //找到对应门描述符
  uint32_t buf[2];
  vaddr_t base = cpu.idtr.base+(NO<<3);//从IDTR中读取首地址并进行索引，从<<3可见，一个门描述符占8字节
  buf[0]=vaddr_read(base,4);//前四字节
  buf[1]=vaddr_read(base+4,4);//后四字节
  //获取对应门描述符
  GateDesc *g=(void *) buf;//得到门描述符
  assert(g->present);//断言有效位为1
  //跳转到目标地址
  decoding.jmp_eip=g->offset_15_0|g->offset_31_16<<16;//根据门描述符对应位置设置跳转地址
  decoding.is_jmp=1;
}

void dev_raise_intr() {
}
