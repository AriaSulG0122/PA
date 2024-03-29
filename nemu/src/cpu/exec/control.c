#include "cpu/exec.h"

make_EHelper(jmp) {
  // the target address is calculated at the decode stage
  decoding.is_jmp = 1;

  print_asm("jmp %x", decoding.jmp_eip);
}

make_EHelper(jcc) {
  // the target address is calculated at the decode stage
  uint8_t subcode = decoding.opcode & 0xf;
  rtl_setcc(&t2, subcode);
  decoding.is_jmp = t2;

  print_asm("j%s %x", get_cc_name(subcode), decoding.jmp_eip);
}

make_EHelper(jmp_rm) {
  decoding.jmp_eip = id_dest->val;
  decoding.is_jmp = 1;

  print_asm("jmp *%s", id_dest->str);
}

make_EHelper(call) {
  // the target address is calculated at the decode stage
  //TODO();
  decoding.is_jmp=1;//This is jump instruction
  rtl_push(eip);

  print_asm("call %x", decoding.jmp_eip);
}

make_EHelper(ret) {
  //TODO();
  rtl_pop(&decoding.jmp_eip);//取出栈顶保存的eip值，然后将其设置为跳转的eip值
  decoding.is_jmp=1;//指令跳转为真
  print_asm("ret");
}

//参考jmp_rm和call
make_EHelper(call_rm) {
  //TODO();
  decoding.is_jmp=1;//This is jump instruction
  rtl_push(eip);
  decoding.jmp_eip = id_dest->val;
  print_asm("call *%s", id_dest->str);
}
