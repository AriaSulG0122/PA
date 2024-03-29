#include "cpu/exec.h"

make_EHelper(test) {
  //TODO();
  rtl_and(&t2, &id_dest->val, &id_src->val);//利用rtl基本操作进行运算
  //更新各个标志位
  rtl_update_ZFSF(&t2, id_dest->width);
  rtl_set_OF(&tzero);
  rtl_set_CF(&tzero);
  print_asm_template2(test);
}

make_EHelper(and) {
  //TODO();
  rtl_and(&t2, &id_dest->val, &id_src->val);//利用rtl基本操作进行运算
  operand_write(id_dest, &t2);//完成计算，写入结果
  //更新各个标志位
  rtl_update_ZFSF(&t2, id_dest->width);
  rtl_set_OF(&tzero);
  rtl_set_CF(&tzero);
  print_asm_template2(and);
}

make_EHelper(xor) {
  //TODO();
  rtl_xor(&t2, &id_dest->val, &id_src->val);//利用rtl基本操作进行运算
  operand_write(id_dest, &t2);//完成计算，写入结果
  //更新各个标志位
  rtl_update_ZFSF(&t2, id_dest->width);
  rtl_set_OF(&tzero);
  rtl_set_CF(&tzero);
  print_asm_template2(xor);
}

make_EHelper(or) {
  //TODO();
  rtl_or(&t2, &id_dest->val, &id_src->val);//利用rtl基本操作进行运算
  operand_write(id_dest, &t2);//完成计算，写入结果
  //更新各个标志位
  rtl_update_ZFSF(&t2, id_dest->width);
  rtl_set_OF(&tzero);
  rtl_set_CF(&tzero);
  print_asm_template2(or);
}

make_EHelper(sar) {
  //TODO();
  // unnecessary to update CF and OF in NEMU
  rtl_sar(&t2, &id_dest->val, &id_src->val);//利用rtl基本操作进行运算
  operand_write(id_dest, &t2);//完成计算，写入结果
  //更新ZF和SF
  rtl_update_ZFSF(&t2, id_dest->width);
  print_asm_template2(sar);
}

make_EHelper(shl) {
  //TODO();
  // unnecessary to update CF and OF in NEMU
  rtl_shl(&t2, &id_dest->val, &id_src->val);//利用rtl基本操作进行运算
  operand_write(id_dest, &t2);//完成计算，写入结果
  //更新ZF和SF
  rtl_update_ZFSF(&t2, id_dest->width);
  print_asm_template2(shl);
}

make_EHelper(shr) {
  //TODO();
  // unnecessary to update CF and OF in NEMU
  rtl_shr(&t2, &id_dest->val, &id_src->val);//利用rtl基本操作进行运算
  operand_write(id_dest, &t2);//完成计算，写入结果
  //更新ZF和SF
  rtl_update_ZFSF(&t2, id_dest->width);
  print_asm_template2(shr);
}

make_EHelper(setcc) {
  uint8_t subcode = decoding.opcode & 0xf;
  rtl_setcc(&t2, subcode);
  operand_write(id_dest, &t2);

  print_asm("set%s %s", get_cc_name(subcode), id_dest->str);
}

make_EHelper(not) {
  //TODO();
  rtl_mv(&t2,&id_dest->val);
  rtl_not(&t2);
  operand_write(id_dest,&t2);
  print_asm_template1(not);
}

//原框架中没有该指令，自己添加
make_EHelper(rol) {
  rtl_shl(&t0,&id_dest->val,&id_src->val);//左移n
  rtl_shri(&t1,&id_dest->val,id_dest->width*8-id_src->val);//对于最左边的位，左移n相当于右移width-n
  rtl_or(&t0,&t1,&t0);
  operand_write(id_dest, &t0);

  //注意，下面的部分按照手册P372来
  rtl_get_CF(&t2);//得到CF位
  t0 = id_src->val; //count
  if(t0 == 1)
  {
      rtl_msb(&t1,&id_dest->val,id_dest->width);//Get high-order bit of r/m
      if(t2!=t1)
      {
          rtl_set_OF(&t0);
      }
      else{
          rtl_set_OF(&tzero);
      }
  }
  print_asm_template2(rol);
}