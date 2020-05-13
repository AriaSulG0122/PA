#include "cpu/exec.h"

make_EHelper(mov);

make_EHelper(operand_size);

make_EHelper(inv);
make_EHelper(nemu_trap);

//control.c
make_EHelper(call);//call为跳转控制类指令
make_EHelper(call_rm);//基于数值的跳转
make_EHelper(ret);//过程回归
make_EHelper(jcc);//Jump if Condition is met
make_EHelper(jmp);//基于当前指令的立即数偏移的跳转
make_EHelper(jmp_rm);//基于数值的跳转
//data-mov.c
make_EHelper(pop);//将某个值从栈顶取出
make_EHelper(push);//将某个值放入到栈顶
make_EHelper(pusha);//Push All General Registers
make_EHelper(lea);//装载有效地址
make_EHelper(movzx);//Move with Zero-Extend
make_EHelper(movsx);//Move with Sign-Extend
make_EHelper(cltd);//Convert Word to Doubleword / Conver Doubleword to Quadword  
make_EHelper(cwtl);//Convert Byte to Word/Convert Word to Doubleword
make_EHelper(leave);//Hign Level Procedure Exit  
//arith.c
make_EHelper(sub); //减法
make_EHelper(sbb); //Interger Subtraction with Borrow
make_EHelper(add); //加法
make_EHelper(adc); //带CF加法
make_EHelper(cmp); //Compare Two Operand  
make_EHelper(inc); //+1
make_EHelper(dec); //-1
make_EHelper(mul); // Unsigned multiplication of AL or AX
make_EHelper(imul1);// Signed multiply, imul with one operand
make_EHelper(imul2);// Signed multiply, imul with two operands
make_EHelper(idiv);//Unsigned divide
make_EHelper(div);//Signed Divide
make_EHelper(neg);//Two's Complement Negation
//logic.c   
make_EHelper(xor);//异或
make_EHelper(and);//与
make_EHelper(or);//或
make_EHelper(not);//非
make_EHelper(setcc);//Byte Set on condition
make_EHelper(test);//Logical Compare  
make_EHelper(sar);//Shift Right with extension
make_EHelper(shl);//Shift Left
make_EHelper(shr);//Shift right
make_EHelper(rol);//Rotate left
//special.c
make_EHelper(nop);//空指令
//system.c
make_EHelper(in);//Input from port
make_EHelper(out);//Output to port
make_EHelper(lidt);//Interrput Descriptor Table Register
make_EHelper(int);//Call to Interrupt Procedure