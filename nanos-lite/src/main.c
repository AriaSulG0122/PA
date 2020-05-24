#include "common.h"

/* Uncomment these macros to enable corresponding functionality. */
#define HAS_ASYE  //Asynchronous Extension，异步处理拓展
//#define HAS_PTE

void init_mm(void);
void init_ramdisk(void);
void init_device(void);
void init_irq(void);
void init_fs(void);
uint32_t loader(_Protect *, const char *);

int main() {
#ifdef HAS_PTE
  init_mm();
#endif

  Log("'Hello World!' from Nanos-lite");//输出hello信息
  Log("Build time: %s, %s", __TIME__, __DATE__);//输出编译时间

  init_ramdisk();//初始化ramdisk

  init_device();//初始化设备

#ifdef HAS_ASYE//如果定义了异步处理拓展
  Log("Initializing interrupt/exception handler...");
  init_irq();//初始化IDT并注册一个事件处理函数
#endif

  init_fs();
  //实现文件系统后，更换用户程序只需要修改传入loader()函数的文件名即可
  uint32_t entry = loader(NULL, "/bin/text");//调用loader来加载用户程序，函数返回用户程序的入口地址
  ((void (*)(void))entry)();//跳转至入口地址执行

  panic("Should not reach here");
}
