#include "common.h"

#define DEFAULT_ENTRY ((void *)0x4000000)

// 从ramdisk中`offset`偏移处的`len`字节读入到`buf`中
extern size_t ramdisk_read(void *buf, size_t offset, size_t len);

// 把`buf`中的`len`字节写入到ramdisk中`offset`偏移处
extern size_t ramdisk_write(const void *buf, size_t offset, size_t len);

// 返回ramdisk的大小, 单位为字节
extern size_t get_ramdisk_size();

extern int fs_open(const char *pathname,int flags,int mode);
extern ssize_t fs_read(int fd,void *buf,size_t len);
extern int fs_close(int fd);
extern size_t fs_filesz(int fd);
uintptr_t loader(_Protect *as, const char *filename) {
  //TODO();
  //ramdisk_read(DEFAULT_ENTRY,0,get_ramdisk_size());
  int fd=fs_open(filename,0,0);
  fs_read(fd,DEFAULT_ENTRY,fs_filesz(fd));
  printf("%d",fd);
  fs_close(fd);
  return (uintptr_t)DEFAULT_ENTRY;
}
