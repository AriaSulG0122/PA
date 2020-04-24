#include "common.h"
#include "device/port-io.h"

#define PORT_IO_SPACE_MAX 65536
#define NR_MAP 8

/* "+ 3" is for hacking, see pio_read() below */
static uint8_t pio_space[PORT_IO_SPACE_MAX + 3];

//记录一个端口I/O映射的关系
typedef struct {
  ioaddr_t low;
  ioaddr_t high;
  pio_callback_t callback;
} PIO_t;

static PIO_t maps[NR_MAP];
static int nr_map = 0;

static void pio_callback(ioaddr_t addr, int len, bool is_write) {
  int i;
  for (i = 0; i < nr_map; i ++) {
    if (addr >= maps[i].low && addr + len - 1 <= maps[i].high) {
      maps[i].callback(addr, len, is_write);
      return;
    }
  }
}

/* device interface */
//设备在初始化时调用该函数来注册一个端口I/O映射关系，返回该映射关系的I/O映射空间首地址
void* add_pio_map(ioaddr_t addr, int len, pio_callback_t callback) {
  assert(nr_map < NR_MAP);
  assert(addr + len <= PORT_IO_SPACE_MAX);
  maps[nr_map].low = addr;
  maps[nr_map].high = addr + len - 1;
  maps[nr_map].callback = callback;
  nr_map ++;
  return pio_space + addr;
}


/* CPU interface */
//面向CPU的端口I/O读接口，从地址addr读取len字节长度
uint32_t pio_read(ioaddr_t addr, int len) {
  assert(len == 1 || len == 2 || len == 4);
  assert(addr + len - 1 < PORT_IO_SPACE_MAX);
  pio_callback(addr, len, false);		// prepare data to read
  uint32_t data = *(uint32_t *)(pio_space + addr) & (~0u >> ((4 - len) << 3));
  return data;
}

//面向CPU的端口I/O写接口，从地址addr写入len长度的数据data
void pio_write(ioaddr_t addr, int len, uint32_t data) {
  assert(len == 1 || len == 2 || len == 4);
  assert(addr + len - 1 < PORT_IO_SPACE_MAX);
  memcpy(pio_space + addr, &data, len);
  pio_callback(addr, len, true);
}

