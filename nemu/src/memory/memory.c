#include "nemu.h"
#include "device/mmio.h"
#include "memory/mmu.h"

#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];

/* Memory accessing interfaces */

uint32_t paddr_read(paddr_t addr, int len) {
  int mmio= is_mmio(addr);
  if(mmio != -1){
	return mmio_read(addr, len, mmio);
  }
  return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
  int mmio= is_mmio(addr);
  if(mmio != -1){
	return mmio_write(addr, len, data, mmio);
  }
  memcpy(guest_to_host(addr), &data, len);
}

paddr_t page_translate(vaddr_t vaddr, bool dirty){
	if(cpu.cr0.paging!=1){
		return vaddr;
	}
	else{
		paddr_t basePDE = cpu.cr3.val;
		paddr_t addrPDE = basePDE + (vaddr>>22) * 4;
		paddr_t pde = paddr_read(addrPDE, 4);
		assert(pde & 0x1);

		paddr_t basePTE = pde & 0xfffff000;
		paddr_t addrPTE = basePTE + ((vaddr>>12) & 0x3ff) * 4;
		paddr_t pte = paddr_read(addrPTE, 4);
		assert(pte & 0x1);

		paddr_t basePage = pte & 0xfffff000;
		paddr_t page = basePage + (vaddr & 0xfff);
		
		if(dirty){
			pde |= 0x60;
			pte |= 0x60;
		}
		else{
			pde |= 0x20;
			pte |= 0x20;	
		}
		paddr_write(addrPDE, 4, pde);
		paddr_write(addrPTE, 4, pte);

		return page;
	}
}

uint32_t vaddr_read(vaddr_t addr, int len) {
	if((addr & 0xfff) + len > 0x1000){
		paddr_t paddr = page_translate(addr, false);
		uint32_t low = paddr_read(
			paddr, 
			(int)(0x1000-(addr & 0xfff)));

		paddr = page_translate(
			addr + (int)(0x1000-(addr & 0xfff)), 
			false);
		uint32_t high = paddr_read(
			paddr, 
			len - (int)(0x1000-(addr & 0xfff)));
		
		return (high << ((0x1000 - (addr & 0xfff)) * 8)) + low;
	}
	else{
		paddr_t paddr = page_translate(addr, false);
		return paddr_read(paddr, len);
	}
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
	if((addr & 0xfff) + len > 0x1000){
		uint32_t high = data >> ((0x1000 - (addr & 0xfff)) * 8);
		uint32_t low = 
			data & ((1<<((0x1000-(addr & 0xfff))*8))-1);		
	
		paddr_t paddr = page_translate(addr, true);
		paddr_write(
			paddr,
			0x1000-(addr & 0xfff), 
			low);

		paddr = page_translate(
			addr + 0x1000-(addr & 0xfff), 
			true);
		paddr_write(
			paddr,
			len - (int)(0x1000-(addr & 0xfff)), 
			high);
	}
	else{
		paddr_t paddr = page_translate(addr, true);
		paddr_write(paddr, len, data);
	}
}
