#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  rtl_push((rtlreg_t*)&cpu.eflags);
  rtl_push((rtlreg_t*)&cpu.cs);
  rtl_push(&ret_addr);
  
  decoding.is_jmp=1;

  uint32_t low = vaddr_read(NO * sizeof(GateDesc) + cpu.idtr.address, 4);
  uint32_t high = vaddr_read(NO * sizeof(GateDesc)+ cpu.idtr.address+4, 4);
  decoding.jmp_eip = (low & 0xffff) | (high & 0xffff0000);

  cpu.eflags.IF=0;
}

void dev_raise_intr() {
	cpu.INTR=true;
}
