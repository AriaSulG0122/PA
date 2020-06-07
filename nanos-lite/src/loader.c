#include "common.h"

//#define DEFAULT_ENTRY ((void *)0x4000000)
#define DEFAULT_ENTRY ((void *)0x8048000)

uintptr_t loader(_Protect *as, const char *filename) {
  //3.1
  //size_t size = get_ramdisk_size();
  //ramdisk_read(DEFAULT_ENTRY, 0, size);

  //3.2
  //int fd = fs_open(filename, 0, 0);
  //int size = fs_filesz(fd);
  //fs_read(fd, DEFAULT_ENTRY, size);
  
  int fd = fs_open(filename, 0, 0);
  int size = fs_filesz(fd);
  void *page;

  for(int i=0;i<size;i+=PGSIZE){
	page = (void*)new_page();
	_map(as, DEFAULT_ENTRY + i, page);
	fs_read(fd, page, PGSIZE);
  }
  fs_close(fd);

  return (uintptr_t)DEFAULT_ENTRY;
}
