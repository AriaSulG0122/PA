#include "cpu/exec.h"

void diff_test_skip_qemu();
void diff_test_skip_nemu();

make_EHelper(lidt) {
  //TODO();

  /*cpu.idtr.limit=vaddr_read(id_dest->addr,2);
  if(decoding.is_operand_size_16)
    cpu.idtr.base=vaddr_read(id_dest->addr+2,3);
  else
    cpu.idtr.base=vaddr_read(id_dest->addr+2,4);*/

  t1=id_dest->val;
  rtl_lm(&t0,&t1,2);
  cpu.idtr.limit=t0;
  t1=id_dest->val+2;
  rtl_lm(&t0,&t1,4);
  cpu.idtr.base=t0;


  print_asm_template1(lidt);
}
make_EHelper(mov_r2cr) {
  //TODO();
  if(id_dest->reg==0){
    cpu.cr0.val=reg_l(id_src->reg);
  }else if(id_dest->reg==3){
    cpu.cr3.val=reg_l(id_src->reg);
  }else{
    assert(0);
  }

  print_asm("movl %%%s,%%cr%d", reg_name(id_src->reg, 4), id_dest->reg);
}

make_EHelper(mov_cr2r) {
  //TODO();
  if(id_src->reg==0){
    reg_l(id_dest->reg)=cpu.cr0.val;
  }else if(id_src->reg==3){
    reg_l(id_dest->reg)=cpu.cr3.val;
  }else{
    assert(0);
  }
  
  print_asm("movl %%cr%d,%%%s", id_src->reg, reg_name(id_dest->reg, 4));

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}
make_EHelper(int) {
  //TODO();
  raise_intr(id_dest->val,decoding.seq_eip);

  print_asm("int %s", id_dest->str);

#ifdef DIFF_TEST
  diff_test_skip_nemu();
#endif
}

make_EHelper(iret) {
  //TODO();

  /*rtl_pop(&t0);
  decoding.is_jmp=1;
  decoding.jmp_eip=t0;
  rtl_pop(&t0);
  cpu.cs=(uint16_t)t0;
  rtl_pop(&t0);
  cpu.eflags=t0;*/
  /*
  rtl_pop(&cpu.eip);
  rtl_pop(&cpu.cs);
  rtl_pop(&t0);
  memcpy(&cpu.eflags,&t0,sizeof(cpu.eflags));

  decoding.jmp_eip=1;
  decoding.seq_eip=cpu.eip;
  */
  rtl_pop(&decoding.jmp_eip);
  rtl_pop(&t0);
  cpu.CS=t0;
  rtl_pop(&cpu.eflags);
  decoding.is_jmp=1;
  print_asm("iret");
}

uint32_t pio_read(ioaddr_t, int);
void pio_write(ioaddr_t, int, uint32_t);

make_EHelper(in) {
  //TODO();
  t0=pio_read(id_src->val,id_dest->width);
  operand_write(id_dest,&t0);

  print_asm_template2(in);

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}

make_EHelper(out) {
  //TODO();
  rtl_li(&t0,id_dest->val);
  pio_write(t0,id_src->width,id_src->val);
  print_asm_template2(out);

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}
