#ifndef __CPU_DECODE_H__
#define __CPU_DECODE_H__

#include "common.h"

#include "rtl.h"

enum { OP_TYPE_REG, OP_TYPE_MEM, OP_TYPE_IMM };//三种操作数类型，分别为寄存器、内存、立即数

#define OP_STR_SIZE 40

//记录操作数的信息，如操作数类型、操作数宽度、操作数值等
typedef struct {
  uint32_t type;
  int width;
  union {
    uint32_t reg;//记录寄存器编号
    rtlreg_t addr;//属于rtl寄存器，记录访存地址
    uint32_t imm;//记录非负立即数
    int32_t simm;//记录立即数
  };
  rtlreg_t val;//属于rtl寄存器，记录操作数内容
  char str[OP_STR_SIZE];
} Operand;

//记录一些全局译码信息供后续使用,包括操作码、操作数、指令地址等
typedef struct {
  uint32_t opcode;  // 记录操作码
  vaddr_t seq_eip;  // sequential eip，记录顺序指令
  bool is_operand_size_16;  
  uint8_t ext_opcode;//记录拓展指令字段，位于ModR/M中间三位
  bool is_jmp;
  vaddr_t jmp_eip;  // 记录jmp地址
  Operand src, dest, src2;//two source operands and one destination operand
#ifdef DEBUG
  char assembly[80];
  char asm_buf[128];
  char *p;
#endif
} DecodeInfo;

typedef union {
  struct {
    uint8_t R_M		:3;
    uint8_t reg		:3;
    uint8_t mod		:2;
  };
  struct {
    uint8_t dont_care	:3;
    uint8_t opcode		:3;
  };
  //中间三位是reg/opcode域，指明寄存器编号或者作为操作码的延长位
  uint8_t val;
} ModR_M;

typedef union {
  struct {
    uint8_t base	:3;
    uint8_t index	:3;
    uint8_t ss		:2;
  };
  uint8_t val;
} SIB;

void load_addr(vaddr_t *, ModR_M *, Operand *);
void read_ModR_M(vaddr_t *, Operand *, bool, Operand *, bool);

void operand_write(Operand *, rtlreg_t *);

/* shared by all helper functions */
extern DecodeInfo decoding;//decoding中记录了全局译码信息

//定义三个宏，方便访问两个源操作数(src,src2)和一个目的操作数(dest)
#define id_src (&decoding.src)
#define id_src2 (&decoding.src2)
#define id_dest (&decoding.dest)
//定义一个译码阶段相关的helper函数
#define make_DHelper(name) void concat(decode_, name) (vaddr_t *eip)
typedef void (*DHelper) (vaddr_t *);//后续利用DHelper就能代表整个函数类型{void (*DHelper) (vaddr_t *);}

make_DHelper(I2E);
make_DHelper(I2a);
make_DHelper(I2r);
make_DHelper(SI2E);
make_DHelper(SI_E2G);
make_DHelper(I_E2G);
make_DHelper(I_G2E);
make_DHelper(I);
make_DHelper(r);
make_DHelper(E);
make_DHelper(gp7_E);
make_DHelper(test_I);
make_DHelper(SI);
make_DHelper(G2E);
make_DHelper(E2G);

make_DHelper(mov_I2r);
make_DHelper(mov_I2E);
make_DHelper(mov_G2E);
make_DHelper(mov_E2G);
make_DHelper(lea_M2G);

make_DHelper(gp2_1_E);
make_DHelper(gp2_cl2E);
make_DHelper(gp2_Ib2E);

make_DHelper(O2a);
make_DHelper(a2O);

make_DHelper(J);

make_DHelper(push_SI);

make_DHelper(in_I2a);
make_DHelper(in_dx2a);
make_DHelper(out_a2I);
make_DHelper(out_a2dx);

#endif
