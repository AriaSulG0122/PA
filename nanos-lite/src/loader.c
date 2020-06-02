#include "common.h"

#define DEFAULT_ENTRY ((void *)0x8048000)

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
extern void* new_page(void);

uintptr_t loader(_Protect *as, const char *filename) {
  //TODO();
  //ramdisk_read(DEFAULT_ENTRY,0,get_ramdisk_size());
  
  int fd=fs_open(filename,0,0);
  /*
  fs_read(fd,DEFAULT_ENTRY,fs_filesz(fd));
  printf("Load file:%d",fd);
  */
  
  //读取文件长度
  size_t len=fs_filesz(fd);
  //读取文件末尾位置
  void* end=DEFAULT_ENTRY+len;
  //输出日志信息
  Log("load file:%s,fd:%d,size:%d\n",filename,fd,len);
  for(void* va=DEFAULT_ENTRY;va<end;va+=PGSIZE){
    //从堆区获取新的物理页
    void* pa=new_page();
    //将虚拟页映射到获取到的物理页
    Log("Map va:0x%08x to pa:0x%08x",va,pa);
    uint32_t mydata=0;
    _map(as,va,pa,mydata);
    Log("MyData:0x%08x",mydata);
    Log("PDT_Base:0x%08x",as->ptr);
    //读取文件，读取长度不能超过页的大小
    fs_read(fd,pa,(pa-va)<PGSIZE?(end-va):PGSIZE);
  }
  

  fs_close(fd);
  return (uintptr_t)DEFAULT_ENTRY;
}
