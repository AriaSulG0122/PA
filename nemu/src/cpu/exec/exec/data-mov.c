#include "cpu/exec.h"

make_EHelper(mov) {
  operand_write(id_dest, &id_src->val);
  print_asm_template2(mov);
}

make_EHelper(push) {
  //TODO();
  rtl_push(&id_dest->val);

  print_asm_template1(push);
}

make_EHelper(pop) {
  //TODO();
  rtl_pop(&t2);
  operand_write(id_dest,&t2);
  print_asm_template1(pop);
}

make_EHelper(pusha) {
  //TODO();
  /*if(decoding.is_operand_size_16){
    t0=cpu.esp;
    rtl_lr_w(&t1,R_AX);
    rtl_push(&t1);
    rtl_lr_w(&t1,R_CX);
    rtl_push(&t1);
    rtl_lr_w(&t1,R_DX);
    rtl_push(&t1);
    rtl_lr_w(&t1,R_BX);
    rtl_push(&t1);
    rtl_push(&t0);
    rtl_lr_w(&t1,R_BP);
    rtl_push(&t1);
    rtl_lr_w(&t1,R_SI);
    rtl_push(&t1);
    rtl_lr_w(&t1,R_DI);
    rtl_push(&t1);
  }
  else{
    t0=cpu.esp;
    rtl_lr_w(&t1,R_EAX);
    rtl_push(&t1);
    rtl_lr_w(&t1,R_ECX);
    rtl_push(&t1);
    rtl_lr_w(&t1,R_EDX);
    rtl_push(&t1);
    rtl_lr_w(&t1,R_EBX);
    rtl_push(&t1);
    rtl_push(&t0);
    rtl_lr_w(&t1,R_EBP);
    rtl_push(&t1);
    rtl_lr_w(&t1,R_ESI);
    rtl_push(&t1);
    rtl_lr_w(&t1,R_EDI);
    rtl_push(&t1);
  }*/

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
/*
  if(decoding.is_operand_size_16){
    rtl_pop(&t1);
    rtl_sr_w(R_DI,&t1);
    rtl_pop(&t1);
    rtl_sr_w(R_SI,&t1);
    rtl_pop(&t1);
    rtl_sr_w(R_BP,&t1);
    cpu.esp+=4;
    rtl_pop(&t1);
    rtl_sr_w(R_BX,&t1);
    rtl_pop(&t1);
    rtl_sr_w(R_DX,&t1);
    rtl_pop(&t1);
    rtl_sr_w(R_CX,&t1);
    rtl_pop(&t1);
    rtl_sr_w(R_AX,&t1);
  }
  else{
    rtl_pop(&t1);
    rtl_sr_w(R_EDI,&t1);
    rtl_pop(&t1);
    rtl_sr_w(R_ESI,&t1);
    rtl_pop(&t1);
    rtl_sr_w(R_EBP,&t1);
    cpu.esp+=4;
    rtl_pop(&t1);
    rtl_sr_w(R_EBX,&t1);
    rtl_pop(&t1);
    rtl_sr_w(R_EDX,&t1);
    rtl_pop(&t1);
    rtl_sr_w(R_ECX,&t1);
    rtl_pop(&t1);
    rtl_sr_w(R_EAX,&t1);
  }
*/

  rtl_pop(&cpu.edi);
  rtl_pop(&cpu.esi);
  rtl_pop(&cpu.ebp);
  rtl_pop(&t0);
  rtl_pop(&cpu.ebx);
  rtl_pop(&cpu.edx);
  rtl_pop(&cpu.ecx);
  rtl_pop(&cpu.eax);



  print_asm("popa");
}

make_EHelper(leave) {
  //TODO();
  rtl_mv(&cpu.esp,&cpu.ebp);
  rtl_pop(&cpu.ebp);
  print_asm("leave");
}

make_EHelper(cltd) {
/*
  if (decoding.is_operand_size_16) {//ax
    //TODO();
    rtl_lr_w(&t0,R_AX);
    rtl_sext(&t0,&t0,2);
    if(t0<0)
    {
	reg_w(R_DX)=0xffff;
    }
    else
    {
	reg_w(R_DX)=0;
    }
  }
  else {//eax
    //TODO();
    rtl_lr_l(&t0,R_EAX);
    if((int32_t)t0<0)
    {
	reg_l(R_EDX)=0xffffffff;
    }
    else
    {
	reg_l(R_EDX)=0;
    }
  }
*/
  if(decoding.is_operand_size_16)
  {
    rtl_lr_w(&t0,R_AX);
    rtl_sext(&t0,&t0,2);
    rtl_sari(&t0,&t0,31);
    rtl_sr_w(R_DX,&t0);
  }
  else
  {
    rtl_sari(&cpu.edx,&cpu.eax,31);
  }


  print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {
/*
  if (decoding.is_operand_size_16) {
    //TODO();
    rtl_lr_b(&t0,R_AL);
    rtl_sext(&t0,&t0,1);//al->ax
    reg_w(R_AX)=t0;
  }
  else {
    //TODO();
    rtl_lr_w(&t0,R_AX);
    rtl_sext(&t0,&t0,2);//ax->aex
    reg_l(R_EAX)=t0;
  }*/

  if(decoding.is_operand_size_16)
  {
    panic("operand size shoule be 32");
  }
  else
  {
    rtl_lr_w(&t0,R_AX);
    rtl_sext(&t0,&t0,2);
    rtl_sr_l(R_EAX,&t0);
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
