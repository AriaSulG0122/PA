#ifndef __CPU_EXEC_H__
#define __CPU_EXEC_H__

#include "nemu.h"

//定义一个执行阶段相关的helper函数，concat用于拼接，在这里拼接为exec_[name]，作为函数名
//最后的效果就是make_EHelper(name) 等效于函数 void exec_[name] (vaddr_t *eip)
#define make_EHelper(name) void concat(exec_, name) (vaddr_t *eip)
typedef void (*EHelper) (vaddr_t *);//typedef为复杂的声明定义了一个简单的别名，为EHelper
void raise_intr(uint8_t NO, vaddr_t ret_addr);
#include "cpu/decode.h"
    
//读取指令的len字节长度，然后eip前进len字节
static inline uint32_t instr_fetch(vaddr_t *eip, int len) {
  uint32_t instr = vaddr_read(*eip, len);
#ifdef DEBUG
  uint8_t *p_instr = (void *)&instr;
  int i;
  for (i = 0; i < len; i ++) {
    decoding.p += sprintf(decoding.p, "%02x ", p_instr[i]);
  }
#endif
  (*eip) += len;//eip前进len字节
  return instr;
}

void rtl_setcc(rtlreg_t*, uint8_t);

static inline const char* get_cc_name(int subcode) {
  static const char *cc_name[] = {
    "o", "no", "b", "nb",
    "e", "ne", "be", "nbe",
    "s", "ns", "p", "np",
    "l", "nl", "le", "nle"
  };
  return cc_name[subcode];
}

//print_asm将反汇编结果的字符串打印到缓冲区decoding.assembly中
#ifdef DEBUG
#define print_asm(...) Assert(snprintf(decoding.assembly, 80, __VA_ARGS__) < 80, "buffer overflow!")
#else
#define print_asm(...)
#endif
//suffix_char(width)为操作数宽度width对应的后缀字符
#define suffix_char(width) ((width) == 4 ? 'l' : ((width) == 1 ? 'b' : ((width) == 2 ? 'w' : '?')))
//打印单目操作数指令instr的反汇编结果
#define print_asm_template1(instr) \
  print_asm(str(instr) "%c %s", suffix_char(id_dest->width), id_dest->str)
//打印双目操作数指令instr的反汇编结果
#define print_asm_template2(instr) \
  print_asm(str(instr) "%c %s,%s", suffix_char(id_dest->width), id_src->str, id_dest->str)
//打印三目操作数指令instr的反汇编结果
#define print_asm_template3(instr) \
  print_asm(str(instr) "%c %s,%s,%s", suffix_char(id_dest->width), id_src->str, id_src2->str, id_dest->str)

#endif
