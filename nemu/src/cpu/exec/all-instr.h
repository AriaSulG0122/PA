#include "cpu/exec.h"

make_EHelper(mov);

make_EHelper(operand_size);

make_EHelper(inv);
make_EHelper(nemu_trap);

//control.c
make_EHelper(call);//call为跳转控制类指令
make_EHelper(ret);//过程回归
make_EHelper(jcc);//Jump if Condition is met
//data-mov.c
make_EHelper(pop);//将某个值从栈顶取出
make_EHelper(push);//将某个值放入到栈顶
make_EHelper(lea);//装载有效地址
make_EHelper(movzx);//Move with zero-extend
//arith.c
make_EHelper(sub); //减法
make_EHelper(add); //加法
make_EHelper(adc); //带CF加法
//logic.c   
make_EHelper(xor);//异或
make_EHelper(and);//与
make_EHelper(or);//或
make_EHelper(setcc);//Byte Set on condition
make_EHelper(test);
//special.c
make_EHelper(nop);//空指令