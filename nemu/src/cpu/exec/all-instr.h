#include "cpu/exec.h"

make_EHelper(mov);

make_EHelper(operand_size);

make_EHelper(inv);
make_EHelper(nemu_trap);

make_EHelper(call);//control  call为跳转控制类指令
make_EHelper(push);//data-mov.c   将某个值放入到栈顶
make_EHelper(sub);//arith.c   减法
make_EHelper(xor);//logic.c   异或
make_EHelper(pop);//data-mov.c   将某个值从栈顶取出
make_EHelper(ret);//control.c  过程回归