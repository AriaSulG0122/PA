#include "cpu/exec.h"

void diff_test_skip_qemu();
void diff_test_skip_nemu();

make_EHelper(lidt) {
  cpu.idtr.size=vaddr_read(id_dest->addr, 2);
  cpu.idtr.address = vaddr_read(id_dest->addr+2, 4);
  if(decoding.is_operand_size_16){
  	cpu.idtr.address &=0xffffff;
  }

  print_asm_template1(lidt);
}

make_EHelper(mov_r2cr) {
  assert(id_dest->reg==0 || id_dest->reg==3);

  if(id_dest->reg==0){
	cpu.cr0 = id_src->val;
  }
  else{
	cpu.cr3 = id_src->val;
  }

  print_asm("movl %%%s,%%cr%d", reg_name(id_src->reg, 4), id_dest->reg);
}

make_EHelper(mov_cr2r) {
  assert(id_src->reg==0 || id_src->reg==3);

  id_dest->val = id_src->reg==0?cpu.cr0:cpu.cr3;

  print_asm("movl %%cr%d,%%%s", id_src->reg, reg_name(id_dest->reg, 4));

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}

make_EHelper(int) {
  raise_intr(id_dest->val, decoding.seq_eip);

  print_asm("int %s", id_dest->str);

#ifdef DIFF_TEST
  diff_test_skip_nemu();
#endif
}

make_EHelper(iret) {
  rtl_pop(&t2);
  decoding.jmp_eip=t2;
  decoding.is_jmp = 1;

  rtl_pop((rtlreg_t*)&cpu.cs);
  rtl_pop((rtlreg_t*)&cpu.eflags);

  print_asm("iret");
}

uint32_t pio_read(ioaddr_t, int);
void pio_write(ioaddr_t, int, uint32_t);

make_EHelper(in) {
  t0 = pio_read(id_src->val, id_dest->width);
  operand_write(id_dest, &t0);
  print_asm_template2(in);

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}

make_EHelper(out) {
  pio_write(id_dest->val, id_src->width, id_src->val);

  print_asm_template2(out);

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}
