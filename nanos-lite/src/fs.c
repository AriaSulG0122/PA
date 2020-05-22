#include "fs.h"

// 从ramdisk中`offset`偏移处的`len`字节读入到`buf`中
extern size_t ramdisk_read(void *buf, size_t offset, size_t len);
// 把`buf`中的`len`字节写入到ramdisk中`offset`偏移处
extern size_t ramdisk_write(const void *buf, size_t offset, size_t len);
// 返回ramdisk的大小, 单位为字节
extern size_t get_ramdisk_size();
//把buf中的len字节写到屏幕上的offset处
extern void fb_write(const void *buf, off_t offset, size_t len);
//把字符串dispinfo中offset开始的len字节写到buf中
extern void dispinfo_read(void *buf, off_t offset, size_t len);

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
//对文件记录表中/dev/fb的大小进行初始化
void init_fs() {
  // TODO: initialize the size of /dev/fb
  //利用ioe.c中的API来获取屏幕大小
  file_table[FD_FB].size=_screen.height*_screen.width*sizeof(int);
}
//返回一个文件的大小
size_t fs_filesz(int fd){
  //printf("fd:%d\n",fd);
  return file_table[fd].size;
}
//打开文件并返回fd
int fs_open(const char *pathname,int flags,int mode){
  for(int i=0;i<NR_FILES;i++){
    if(strcmp(file_table[i].name,pathname)==0){
      return i;//返回对应的文件描述符
    }
  }
  assert(0);//没找到目标文件，则报错
  return -1;
}
//读取文件
ssize_t fs_read(int fd,void *buf,size_t len){
  ssize_t fs_size=fs_filesz(fd);
   //printf("Read: fd:%d len:%d,size:%d,openoffset:%d\n",fd,len,fs_size,file_table[fd].open_offset); 
  //处理越界
  if(file_table[fd].open_offset>fs_size||len==0){
    return 0;
  }
  if((file_table[fd].open_offset+len)>fs_size){
    len=fs_size-file_table[fd].open_offset;
  }
  if(fd==FD_DISPINFO){
    dispinfo_read(buf,file_table[fd].open_offset,len);
  }
  else{//默认写文件
      ramdisk_read(buf,file_table[fd].disk_offset+file_table[fd].open_offset,len);
  }
  file_table[fd].open_offset+=len;
  return len;
}
//关闭文件
int fs_close(int fd){
  return 0;//直接返回0，表示总是关闭成功
}


//写入文件
ssize_t fs_write(int fd,const void *buf,size_t len){
  if(fd==1||fd==2){//stdout或stderr
    for(int i=0;i<len;i++){
      //将buf为首地址的len字节输出到串口
      _putc(((char*)buf)[i]);
    }
    return 0;
  }
  if (fd==FD_FB){
    fb_write(buf,file_table[fd].open_offset,len);
    file_table[fd].open_offset+=len;
    return len;
  }
  ssize_t fs_size=fs_filesz(fd);
  //printf("Write: fd:%d len:%d,size:%d,openoffset:%d\n",fd,len,fs_size,file_table[fd].open_offset);
  //处理越界
  if(file_table[fd].open_offset>fs_size){
    return 0;
  }
  if((file_table[fd].open_offset+len)>fs_size){
    len=fs_size-file_table[fd].open_offset;
  }
  //printf("len:%d\n",len);
  ramdisk_write(buf,file_table[fd].disk_offset+file_table[fd].open_offset,len);
  file_table[fd].open_offset+=len;
  return len;
}
//调整偏移量
off_t fs_lseek(int fd,off_t offset,int whence){
  off_t result=-1;//默认-1，为错误返回值
  switch(whence){
    case SEEK_SET://直接重设偏移
      if(offset>=0 && offset<=file_table[fd].size){
        file_table[fd].open_offset=offset;
        result=file_table[fd].open_offset;
      }
      break;
    case SEEK_CUR://原偏移基础上新增偏移
      if( (offset+file_table[fd].open_offset)>=0 && (offset+file_table[fd].open_offset)<=file_table[fd].size){
        file_table[fd].open_offset+=offset;
        result=file_table[fd].open_offset;
      }
      break;
    case SEEK_END://文件末尾基础上新增偏移
      file_table[fd].open_offset=file_table[fd].size+offset;
      result=file_table[fd].open_offset;
  }
  return result;
}

