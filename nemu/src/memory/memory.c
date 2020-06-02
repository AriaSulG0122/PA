#include "nemu.h"
#include "memory/mmu.h"

#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];

paddr_t page_translate(paddr_t addr) {
  if (!cpu.cr0.paging) return addr;
  // Log("page_translate: addr: 0x%x\n", addr);
  paddr_t dir = (addr >> 22) & 0x3ff;
  paddr_t page = (addr >> 12) & 0x3ff;
  paddr_t offset = addr & 0xfff;
  paddr_t PDT_base = cpu.cr3.page_directory_base;
  // Log("page_translate: dir: 0x%x page: 0x%x offset: 0x%x PDT_base: 0x%x\n", dir, page, offset, PDT_base);
  PDE pde;
  pde.val = paddr_read((PDT_base << 12) + (dir << 2), 4);
  if (!pde.present) {
    Log("page_translate: addr: 0x%x\n", addr);
    Log("page_translate: dir: 0x%x page: 0x%x offset: 0x%x PDT_base: 0x%x\n", dir, page, offset, PDT_base);
    assert(pde.present);
  }
  PTE pte;
  // Log("page_translate: page_frame: 0x%x\n", pde.page_frame);
  pte.val = paddr_read((pde.page_frame << 12) + (page << 2), 4);
  if (!pte.present) {
    Log("page_translate: addr: 0x%x\n", addr);
    assert(pte.present);
  }
  paddr_t paddr = (pte.page_frame << 12) | offset;
  // Log("page_translate: paddr: 0x%x\n", paddr);
  return paddr;
}


/* Memory accessing interfaces */

//***Get the last 8|16|24|32 bits of pmem_rw(addr,uint32_t)
uint32_t paddr_read(paddr_t addr, int len)
{
  //Log("paddr_read:0x%08x",addr);
  if (is_mmio(addr) == -1)
  { //为-1，则不是内存映射I/O的访问
    return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
  }
  else
  {
    return mmio_read(addr, len, is_mmio(addr)); //根据映射号访问内存映射I/O
  }
  //***(4-len)<<3 = (4-len)*2^3,     ~ = take inverse
}

void paddr_write(paddr_t addr, int len, uint32_t data)
{
  if (is_mmio(addr) == -1)
  {
    memcpy(guest_to_host(addr), &data, len);
  }
  else
  {
    mmio_write(addr, len, data, is_mmio(addr));
  }
}

uint32_t vaddr_read(vaddr_t addr, int len) {
  // Log("vaddr_read: 0x%x", addr);
  int data_cross = (addr % PAGE_SIZE + len) > PAGE_SIZE;
  if (data_cross) {
    
    int prev = PAGE_SIZE - addr % PAGE_SIZE;
    int last = len - prev;
    uint32_t p = paddr_read(page_translate(addr), prev);
    uint32_t l = paddr_read(page_translate(addr + prev), last);
    // Log("vaddr_read: addr: 0x%x prev %d last %d", addr, prev, last);
    uint32_t ret = (l << (8 * prev)) | p; 
    // Log("vaddr_read: addr: 0x%x p 0x%x l 0x%x res 0x%x", addr, p, l, ret);
    return ret;
    assert(0);
  } else {
    paddr_t paddr = page_translate(addr);
    // Log("vaddr_read: 0x%x", paddr);
    return paddr_read(paddr, len);
  }
}

void vaddr_write(vaddr_t addr, int len,uint32_t data) {
  // Log("vaddr_write: 0x%x", addr);
  int data_cross = (addr % PAGE_SIZE + len) > PAGE_SIZE;
  if (data_cross) {
    assert(0);
  } else {
    paddr_t paddr = page_translate(addr);
    paddr_write(paddr, len, data);
  }
}
