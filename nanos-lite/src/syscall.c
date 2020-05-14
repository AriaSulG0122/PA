#include "common.h"
#include "syscall.h"

static inline _RegSet* sys_write(_RegSet *r){
  int fd=(int)SYSCALL_ARG2(r);
  char *buf=(char*)SYSCALL_ARG3(r);
  int len=(int)SYSCALL_ARG4(r);
  Log("fd:%d,len:%d",fd,len);
  if(fd==1||fd==2){//stdout或stderr
    for(int i=0;i<len;i++){
      //将buf为首地址的len字节输出到串口
      _putc(buf[i]);
    }
    //设置返回值
    SYSCALL_ARG1(r)=SYSCALL_ARG4(r);
  }
  return NULL;
}

_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);
  a[1] = SYSCALL_ARG2(r);
  a[2] = SYSCALL_ARG3(r);
  a[3] = SYSCALL_ARG4(r);
  switch (a[0]) {
    case SYS_none:
      r->eax = 1;
      break;
    case SYS_exit:
      _halt(a[1]);
      break;
    case SYS_write:
      return sys_write(r);
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
  return NULL;
}
