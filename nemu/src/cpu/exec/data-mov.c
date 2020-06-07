#include "cpu/exec.h"

make_EHelper(mov) {
  operand_write(id_dest, &id_src->val);
  print_asm_template2(mov);
}

make_EHelper(movs) {
  Log("movs");
  rtl_lr(&t0, 4, R_ESI);
  rtl_lm(&t1, &t0, 1);
  t0 += 1;
  rtl_sr(R_ESI, 4, &t0);

  rtl_lr(&t0, 4, R_EDI);
  rtl_sm(&t0, 1, &t1);
  t0 += 1;
  rtl_sr(R_EDI, 4, &t0);

  print_asm_template1(movs);
}


make_EHelper(push){
  if(id_dest->type==OP_TYPE_IMM && id_dest->width==1){
	rtl_sext(&id_dest->val, &id_dest->val, 1);
  }
  else if(id_dest->type==OP_TYPE_IMM && id_dest->width==2){
	rtl_sext(&id_dest->val, &id_dest->val, 2);
  }
  rtl_push(&id_dest->val);
  print_asm_template1(push);
}

make_EHelper(pop) {
  rtl_pop(&id_src->val);
  operand_write(id_dest, &id_src->val);
  print_asm_template1(pop);
}

make_EHelper(pusha) {
  rtl_li(&t2, cpu.esp);
  rtl_push(&cpu.eax);
  rtl_push(&cpu.ecx);
  rtl_push(&cpu.edx);
  rtl_push(&cpu.ebx);
  rtl_push(&t2);
  rtl_push(&cpu.ebp);
  rtl_push(&cpu.esi);
  rtl_push(&cpu.edi);
  print_asm("pusha");
}

make_EHelper(popa) {
  if(decoding.is_operand_size_16){
	rtl_pop(&t1);
	t1 &= 0xffff;
	rtl_sr_w(R_DI, &t1);

	rtl_pop(&t1);
	t1 &= 0xffff;
	rtl_sr_w(R_SI, &t1);

	rtl_pop(&t1);
	t1 &= 0xffff;
	rtl_sr_w(R_BP, &t1);
 	
	rtl_pop(&t1);

	rtl_pop(&t1);
	t1 &= 0xffff;
	rtl_sr_w(R_BX, &t1);

	rtl_pop(&t1);
	t1 &= 0xffff;
	rtl_sr_w(R_DX, &t1);

	rtl_pop(&t1);
	t1 &= 0xffff;
	rtl_sr_w(R_CX, &t1);

	rtl_pop(&t1);
	t1 &= 0xffff;
	rtl_sr_w(R_AX, &t1);

  }
  else{
	rtl_pop(&cpu.edi);
	rtl_pop(&cpu.esi);
	rtl_pop(&cpu.ebp);
	rtl_pop(&t1);
	rtl_pop(&cpu.ebx);
	rtl_pop(&cpu.edx);
	rtl_pop(&cpu.ecx);
	rtl_pop(&cpu.eax);
  }

  print_asm("popa");
}

make_EHelper(leave) {
  if(decoding.is_operand_size_16){
	rtl_lr_w(&t0, R_BP);
	rtl_sr_w(R_SP, &t0);
	rtl_pop(&t0);
	t0 = t0 & 0xffff;
	rtl_sr_w(R_BP, &t0);
  }
  else{
	cpu.esp = cpu.ebp;
	rtl_pop(&cpu.ebp);
  }

  print_asm("leave");
}

make_EHelper(cltd) {
  // DX:AX = sign-extend of AX
  if (decoding.is_operand_size_16) {
	rtl_lr_w(&t0, R_AX);
	rtl_msb(&t0, &t0, 2);
	if(t0 == 0){
		t0 = 0x0000;
		rtl_sr_w(R_DX, &t0);	
	}
	else{
		t0 = 0xffff;
		rtl_sr_w(R_DX, &t0);	
	}
  }
  else {
	// bug rtl_lr_w(&t0, R_EAX);
	rtl_lr_l(&t0, R_EAX);
	rtl_msb(&t1, &t0, 4);
	if(t1 == 0){
		t0 = 0x00000000;
		rtl_sr_l(R_EDX, &t0);	
	}
	else{
		t0 = 0xffffffff;
		rtl_sr_l(R_EDX, &t0);	
	}
  }

  print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {
  if (decoding.is_operand_size_16) {
	rtl_lr_b(&t0, R_AL);
	rtl_msb(&t0, &t0, 1);
	if(t0 == 0){
		t0 = 0x00;
		rtl_sr_b(R_AH, &t0);	
	}
	else{
		t0 = 0xff;
		rtl_sr_b(R_AH, &t0);	
	}
  }
  else {
	rtl_lr_w(&t0, R_AX);
	rtl_msb(&t1, &t0, 2);
	if(t1 == 0){
		t2 = 0x00000000;
		rtl_sr_l(R_EAX, &t2);
		rtl_sr_w(R_AX, &t0);	
	}
	else{
		t2 = 0xffffffff;
		rtl_sr_l(R_EAX, &t2);
		rtl_sr_w(R_AX, &t0);	
	}
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
