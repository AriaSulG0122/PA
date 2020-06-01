#include "nemu.h"
//*** Analog memory
#define PMEM_SIZE (128 * 1024 * 1024)

//控制寄存器标志
#define CR0_PG 0x80000000 //Paging位

//页表或页目录的present标志位
#define PTE_P 0x001 //Present位

//页表大小
#define PGSIZE 4096

//地址分段
typedef uint32_t PTE;
typedef uint32_t PDE;
//前十位是页目录项
#define PDX(va) (((uint32_t)(va) >> 22) & 0x3ff)
//中间十位是页表项
#define PTX(va) (((uint32_t)(va) >> 12) & 0x3ff)
//最后十二位是页内偏移
#define OFF(va) ((uint32_t)(va)&0xfff)

//在页表和页目录中的地址，取高20位
#define PTE_ADDR(pte) ((uint32_t)(pte) & ~0xfff)

#define pmem_rw(addr, type) *(type *)({                                       \
  Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
  guest_to_host(addr);                                                        \
})

uint8_t pmem[PMEM_SIZE];

/* Memory accessing interfaces */

//***Get the last 8|16|24|32 bits of pmem_rw(addr,uint32_t)
uint32_t paddr_read(paddr_t addr, int len)
{
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

paddr_t page_translate(vaddr_t vaddr)
{
  
  //获取页目录的基地址
  paddr_t dir = PTE_ADDR(cpu.CR3);
  //检查页目录项的present位，如发现无效表项，则终止
  assert(paddr_read(dir + sizeof(paddr_t) * PDX(vaddr), sizeof(paddr_t)) & PTE_P);
  //读取页表项的基地址
  paddr_t pg = PTE_ADDR(paddr_read(dir + sizeof(paddr_t) * PDX(vaddr), sizeof(paddr_t)));
  //检查页表项的present位，如发现无效表项，则终止
  assert(paddr_read(pg + sizeof(paddr_t) * PTX(vaddr), sizeof(paddr_t)) & PTE_P);
  //返回的物理地址需要先读取对应页表项所记录的物理地址，再加上偏移量
  return (PTE_ADDR(paddr_read(pg + sizeof(paddr_t) * PTX(vaddr), sizeof(paddr_t))) | OFF(vaddr)); 
  
 /*
 //页目录
 PDE pde *pgdir;
 //页表
 PTE pte,*pgtable;
 paddr_t paddr=addr;
 if(cpu.cr0.protect_enable*)*/
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

// ***x86 is small end.
uint32_t vaddr_read(vaddr_t addr, int len)
{
  //如果发现 CR0 的 PG 位为 1,则开启分页机制
  if (cpu.CR0 & CR0_PG)
  {
    //数据跨越了边界，则要进行两次转换
    if (OFF(addr) + len > PGSIZE)
    {
      int firstLen = PGSIZE - OFF(addr);
      int secondLen = len - firstLen;
      uint32_t first = paddr_read(page_translate(addr), firstLen);
      uint32_t second = paddr_read(page_translate(addr + firstLen), secondLen);
      //对两次转换结果进行拼接
      return (second << (8 * firstLen)) | first;
    }
    else
    { //否则直接转换就行了
      Log("paddr_read:0x%08x",page_translate(addr));
      return paddr_read(page_translate(addr), len);
    }
  }
  return paddr_read(addr, len);
}

void vaddr_write(vaddr_t addr, int len, uint32_t data)
{
  // 当CR0的PG位为1则开启分页模式
  if (cpu.CR0 & CR0_PG)
  {
    if (OFF(addr) + len > PGSIZE)
    {
      assert(0); //跨越边界，报错
    }
    else
    {
      paddr_write(page_translate(addr), len, data);
    }
  }
  else
  {
    paddr_write(addr, len, data);
  }
}
