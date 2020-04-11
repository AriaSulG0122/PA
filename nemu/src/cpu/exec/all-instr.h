#include "cpu/exec.h"

make_EHelper(mov);

make_EHelper(operand_size);

make_EHelper(inv);
make_EHelper(nemu_trap);

make_EHelper(call);//call为跳转控制类指令，在control.c中实现
make_EHelper(push);//将某个值放入到栈顶
make_EHelper(sub);//arith.c
make_EHelper(xor);//logic.c
make_EHelper(pop);//将某个值从栈顶取出