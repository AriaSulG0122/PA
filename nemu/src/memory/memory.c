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
  int mmioFlag=is_mmio(addr);
  if(mmioFlag != -1){
    return mmio_read(addr,len,mmioFlag);
  }
  else{
    return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
  }

}

void paddr_write(paddr_t addr, int len, uint32_t data) {
  int mmioFlag=is_mmio(addr);
  if(mmioFlag != -1){
    mmio_write(addr,len,data,mmioFlag);
  }
  else{
     memcpy(guest_to_host(addr), &data, len); 
  }
}

paddr_t page_translate(paddr_t addr, bool iswrite){
  PDE pde;
  PTE pte;

  paddr_t dir = (addr >> 22) & 0x3ff;
  paddr_t page = (addr >> 12) & 0x3ff;
  paddr_t offset = addr & 0xfff;
  if(cpu.cr0.paging){

    uint32_t pdb = cpu.cr3.page_directory_base;
    pde.val = paddr_read((pdb << 12) | (dir << 2), 4);
    if (!pde.present) panic("Invalid page directory entry at address %#x\n", addr);
    if (!pde.accessed)
    {
      pde.accessed = 1;
      paddr_write((pdb << 12) | (dir << 2), 4, pde.val);
    }
    
    pte.val = paddr_read((pde.val & 0xfffff000) | (page << 2), 4);
    if (! pte.present) panic("Invalid page table entry at address %#x\n", addr);
    if (! pte.accessed) pte.accessed = 1;
    if (iswrite) pte.dirty = 1;
    paddr_write((pde.val & 0xfffff000) | (page << 2), 4, pte.val);
    return (pte.val & 0xfffff000) | offset;
  }
  return addr;
}



uint32_t vaddr_read(vaddr_t addr, int len) {
  if ((addr & PAGE_MASK) + len > PAGE_SIZE) {
    //assert(0);
    //Log("read");
    int len1,len2;
    uint32_t data;
    paddr_t paddr;
    len1 = PAGE_SIZE - (addr & PAGE_MASK);
    len2 = len - len1;
    paddr = page_translate(addr,false);
    data = paddr_read(paddr,len1);
    paddr = page_translate(addr + len1,false);
    data = (paddr_read(paddr,len2) << (len1 << 3)) | data;
    return data;
  }
  if(cpu.cr0.paging) {
    //Log("vaddr:0x%08x,paddr:0x%08x",addr,page_translate(addr,false));
    return paddr_read(page_translate(addr,false), len);
    }
  else return paddr_read(addr, len);
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  if ((addr & PAGE_MASK) + len > PAGE_SIZE) {
    //Log("write");
    int len1,len2;
    paddr_t paddr;
    len1 = PAGE_SIZE - (addr & PAGE_MASK);
    len2 = len - len1;
    paddr = page_translate(addr,true);
    paddr_write(paddr,len1,data);
    paddr = page_translate(addr + len1,true);
    paddr_write(paddr,len2,data >> (len1<<3));
   // assert(0);
  }
  if(cpu.cr0.paging) return paddr_write(page_translate(addr,true), len, data);
  else return paddr_write(addr, len, data);
}
