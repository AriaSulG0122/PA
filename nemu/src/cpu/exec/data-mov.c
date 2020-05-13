#include "cpu/exec.h"

make_EHelper(mov) {
  operand_write(id_dest, &id_src->val);
  print_asm_template2(mov);
}

make_EHelper(push) {
  //TODO();
  rtl_push(&id_dest->val);//把value值push到栈顶
  print_asm_template1(push);
}

make_EHelper(pop) {
  //TODO();
  //rtl_pop(&id_dest->val);//不能这么写！
  /* 调用rtl_pop()取出一个32位值到临时寄存器 */
  rtl_pop(&t2);
  /* 使用operand_write()将取出的临时值写入目标寄存器 */
  operand_write(id_dest,&t2);
  print_asm_template1(pop);
}

//把通用寄存器的值压入堆栈
make_EHelper(pusha) {
  //TODO();
  t0=cpu.esp;
  rtl_push(&cpu.eax);
  rtl_push(&cpu.ecx);
  rtl_push(&cpu.edx);
  rtl_push(&cpu.ebx);
  rtl_push(&t0);
  rtl_push(&cpu.ebp);
  rtl_push(&cpu.esi);
  rtl_push(&cpu.edi);
  print_asm("pusha");
}

make_EHelper(popa) {
  //TODO();
  rtl_pop(&cpu.edi);
  rtl_pop(&cpu.esi);
  rtl_pop(&cpu.ebp);
  rtl_pop(&t0);//Skip ESP
  rtl_pop(&cpu.ebx);
  rtl_pop(&cpu.edx);
  rtl_pop(&cpu.ecx);
  rtl_pop(&cpu.eax);
  print_asm("popa");
}

make_EHelper(leave) {
  //TODO();
  rtl_mv(&cpu.esp,&cpu.ebp);//Set ESP to EBP
  rtl_pop(&cpu.ebp);//pop EBP
  print_asm("leave");
}

make_EHelper(cltd) {
  if (decoding.is_operand_size_16) {//CWD instruction，DX:AX(sign-extend of AX)
    //TODO();
    rtl_msb(&t0,&cpu.eax,2);//获取16位数的最高位，看看是否<0，即获取AX的最高位
    if(t0 == 1)cpu.edx = cpu.edx | 0xffff;//AX<0，则以AX的符号位拓展DX
    else cpu.edx = 0;
  }
  else {//CDQ instruction,EDX:EAX(sign-extend of EAX)
    //TODO();
    rtl_msb(&t0,&cpu.eax,4);
    if(t0 == 1)cpu.edx = cpu.edx | 0xffffffff;
    else cpu.edx = 0;
  }
  print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {//Convert Byte to Word/Convert Word to Doubleword
  if (decoding.is_operand_size_16) {//AX<-SignExtend(AL),Convert Byte to Word
    //TODO();
    rtl_sext(&t0,&cpu.eax,1);//进行符号拓展
    cpu.eax = (cpu.eax & 0xffff0000) | (t0 & 0xffff);
  }
  else {//EAX<-SignExtend(AX),Convert Word to Doubleword
    //TODO();
    rtl_sext(&t0,&cpu.eax,2);
    cpu.eax = t0;
  }

  print_asm(decoding.is_operand_size_16 ? "cbtw" : "cwtl");
}

make_EHelper(movsx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  rtl_sext(&t2, &id_src->val, id_src->width);
  operand_write(id_dest, &t2);
  print_asm_template2(movsx);
}

make_EHelper(movzx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  operand_write(id_dest, &id_src->val);
  print_asm_template2(movzx);
}

make_EHelper(lea) {
  rtl_li(&t2, id_src->addr);
  operand_write(id_dest, &t2);
  print_asm_template2(lea);
}
