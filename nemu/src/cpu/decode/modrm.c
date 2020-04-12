#include "cpu/exec.h"
#include "cpu/rtl.h"

void load_addr(vaddr_t *eip, ModR_M *m, Operand *rm) {
  assert(m->mod != 3);//断言，m的mod字段必然不等于3

  //进行初始化
  int32_t disp = 0;//记录偏移
  int disp_size = 4;
  int base_reg = -1, index_reg = -1, scale = 0;//SIB
  rtl_li(&rm->addr, 0);

  if (m->R_M == R_ESP) {//如果R_M字段对应寄存器ESP
    SIB s;
    s.val = instr_fetch(eip, 1);
    base_reg = s.base;
    scale = s.ss;

    if (s.index != R_ESP) { index_reg = s.index; }
  }
  else {
    /* no SIB */
    base_reg = m->R_M;
  }

  if (m->mod == 0) {
    if (base_reg == R_EBP) { base_reg = -1; }
    else { disp_size = 0; }
  }
  else if (m->mod == 1) { disp_size = 1; }

  if (disp_size != 0) {
    /* has disp */
    disp = instr_fetch(eip, disp_size);
    if (disp_size == 1) { disp = (int8_t)disp; }

    rtl_addi(&rm->addr, &rm->addr, disp);
  }

  if (base_reg != -1) {
    rtl_add(&rm->addr, &rm->addr, &reg_l(base_reg));
  }

  if (index_reg != -1) {
    rtl_shli(&t0, &reg_l(index_reg), scale);
    rtl_add(&rm->addr, &rm->addr, &t0);
  }

#ifdef DEBUG
  char disp_buf[16];
  char base_buf[8];
  char index_buf[8];

  if (disp_size != 0) {
    /* has disp */
    sprintf(disp_buf, "%s%#x", (disp < 0 ? "-" : ""), (disp < 0 ? -disp : disp));
  }
  else { disp_buf[0] = '\0'; }

  if (base_reg == -1) { base_buf[0] = '\0'; }
  else { 
    sprintf(base_buf, "%%%s", reg_name(base_reg, 4));
  }

  if (index_reg == -1) { index_buf[0] = '\0'; }
  else { 
    sprintf(index_buf, ",%%%s,%d", reg_name(index_reg, 4), 1 << scale);
  }

  if (base_reg == -1 && index_reg == -1) {
    sprintf(rm->str, "%s", disp_buf);
  }
  else {
    sprintf(rm->str, "%s(%s%s)", disp_buf, base_buf, index_buf);
  }
#endif

  rm->type = OP_TYPE_MEM;
}

//解析ModR_M字段
void read_ModR_M(vaddr_t *eip, Operand *rm, bool load_rm_val, Operand *reg, bool load_reg_val) {
  ModR_M m;
  m.val = instr_fetch(eip, 1);//先获取ModR_M字段值，一个字节
  decoding.ext_opcode = m.opcode;//获取拓展指令字段
  if (reg != NULL) {//如果Operand reg不为空，则表明中间三比特字段为寄存器的编号
    reg->type = OP_TYPE_REG;//操作数类型为OP_TYPE_REG
    reg->reg = m.reg;//记录操作数的寄存器编号
    if (load_reg_val) {//如果需要加载操作数值，则加载到reg->val中
      rtl_lr(&reg->val, reg->reg, reg->width);
    }

#ifdef DEBUG
    snprintf(reg->str, OP_STR_SIZE, "%%%s", reg_name(reg->reg, reg->width));
#endif
  }

  if (m.mod == 3) {//如果ModR_M前两比特的mod字段是11，则表明末三比特的R_M字段是寄存器编号
    rm->type = OP_TYPE_REG;
    rm->reg = m.R_M;
    if (load_rm_val) {
      rtl_lr(&rm->val, m.R_M, rm->width);
    }

#ifdef DEBUG
    sprintf(rm->str, "%%%s", reg_name(m.R_M, rm->width));
#endif
  }
  else {//否则对Operand rm进行加载
    load_addr(eip, &m, rm);
    if (load_rm_val) {
      rtl_lm(&rm->val, &rm->addr, rm->width);
    }
  }
}
