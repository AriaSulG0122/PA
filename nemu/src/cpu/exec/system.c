#include "cpu/exec.h"

void diff_test_skip_qemu();
void diff_test_skip_nemu();

//在IDTR寄存器中设置IDT的首地址和长度
make_EHelper(lidt) {
  //TODO();
  cpu.idtr.limit=vaddr_read(id_dest->addr,2);
  if(decoding.is_operand_size_16){
    cpu.idtr.base=vaddr_read(id_dest->addr+2,3);
  }
  else{
    cpu.idtr.base=vaddr_read(id_dest->addr+2,4);
  }
  print_asm_template1(lidt);
}

make_EHelper(mov_r2cr) {
  TODO();

  print_asm("movl %%%s,%%cr%d", reg_name(id_src->reg, 4), id_dest->reg);
}

make_EHelper(mov_cr2r) {
  TODO();

  print_asm("movl %%cr%d,%%%s", id_src->reg, reg_name(id_dest->reg, 4));

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}

make_EHelper(int) {
  //TODO();
  void raise_intr(uint8_t NO, vaddr_t ret_addr);
  print_asm("int %s", id_dest->str);

#ifdef DIFF_TEST
  diff_test_skip_nemu();
#endif
}

make_EHelper(iret) {
  TODO();

  print_asm("iret");
}

uint32_t pio_read(ioaddr_t, int);
void pio_write(ioaddr_t, int, uint32_t);

make_EHelper(in) {//DEST<-[SRC](Reads from I/O address space),来自手册
  //TODO();
  t0=pio_read(id_src->val,id_dest->width);
  operand_write(id_dest,&t0);
  print_asm_template2(in);

#ifdef DIFF_TEST
  diff_test_skip_qemu();//跳过与QEMU的检查
#endif
}

make_EHelper(out) {//[DEST]<-SRC(I/O address space used),来自手册
  //TODO();
  pio_write(id_dest->val,id_dest->width,id_src->val);
  print_asm_template2(out);

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}
