#include "fs.h"

// 从ramdisk中`offset`偏移处的`len`字节读入到`buf`中
extern size_t ramdisk_read(void *buf, size_t offset, size_t len);
// 把`buf`中的`len`字节写入到ramdisk中`offset`偏移处
extern size_t ramdisk_write(const void *buf, size_t offset, size_t len);
// 返回ramdisk的大小, 单位为字节
extern size_t get_ramdisk_size();

typedef struct {
  char *name;// 文件名
  size_t size;// 文件大小
  off_t disk_offset;// 文件在磁盘中的偏移 
  off_t open_offset;// 文件在打开之后的读写指针
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO, FD_NORMAL};

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin (note that this is not the actual stdin)", 0, 0},
  {"stdout (note that this is not the actual stdout)", 0, 0},
  {"stderr (note that this is not the actual stderr)", 0, 0},
  [FD_FB] = {"/dev/fb", 0, 0},
  [FD_EVENTS] = {"/dev/events", 0, 0},
  [FD_DISPINFO] = {"/proc/dispinfo", 128, 0},
#include "files.h"
};

//文件数目
#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

void init_fs() {
  // TODO: initialize the size of /dev/fb
}

size_t fs_filesz(int fd){
  return file_table[fd].size;
}

int fs_open(const char *pathname,int flags,int mode){
  for(int i=0;i<NR_FILES;i++){
    if(strcmp(file_table[i].name,pathname)==0){
      return i;//返回对应的文件描述符
    }
  }
  assert(0);//没找到目标文件，则报错
  return -1;
}

ssize_t fs_read(int fd,void *buf,size_t len){
  ssize_t fs_size=fs_filesz(fd);
  if((file_table[fd].open_offset + len)>fs_size||len==0){//读取越界
    return 0;
  }
  ramdisk_read(buf,file_table[fd].disk_offset+file_table[fd].open_offset,len);
  file_table[fd].open_offset+=len;
  return len;
}


ssize_t fs_write(int fd,const void *buf,size_t len){
  ssize_t fs_size=fs_filesz(fd);
  if((file_table[fd].open_offset + len)>fs_size||len==0){//写入越界
    return 0;
  }
  ramdisk_write(buf,file_table[fd].disk_offset+file_table[fd].open_offset,len);
  file_table[fd].open_offset+=len;
  return len;
}

int fs_close(int fd){
  return 0;//直接返回0，表示总是关闭成功
}

