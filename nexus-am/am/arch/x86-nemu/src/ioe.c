#include <am.h>
#include <x86.h>

#define RTC_PORT 0x48   // Note that this is not standard
static unsigned long boot_time;

//用于进行IOE相关的初始化操作，调用后程序才能正常使用上述IOE相关的API
void _ioe_init() {
  boot_time = inl(RTC_PORT);//inl(RTC_PORT)记录了当前时间
}

//返回系统启动后经过的毫秒数
unsigned long _uptime() {
  //return 0;
  return inl(RTC_PORT)-boot_time;//毫秒数就是当前时间和启动时间的差值
}

uint32_t* const fb = (uint32_t *)0x40000;

//指示屏幕的大小
_Screen _screen = {
  .width  = 400,
  .height = 300,
};

extern void* memcpy(void *, const void *, int);

//用于将pixels指定的矩形像素绘制到屏幕中以(x,y)和(x+w,y+h)两点连线为对角线的矩形区域
void _draw_rect(const uint32_t *pixels, int x, int y, int w, int h) {
  int i;
  for (i = 0; i < _screen.width * _screen.height; i++) {
    fb[i] = i;
  }
  // int i;
  // for(i=0;i<h;i++)
  //     memcpy(fb+(y+i)*_screen.width+x,pixels+i*w,w*4);
}

//用于将之前的绘制内容同步到屏幕上（在NEMU中绘制内容总是会同步到屏幕上，因而无需实现此API）
void _draw_sync() {
}

//返回按键的键盘码，若无按键，则返回_KEY_NONE
int _read_key() {
  // uint32_t keyCode=_KEY_NONE;
  // if(inl(0x64)){//状态寄存器生效
  //   keyCode=inl(0x60);
  // }
  // return keyCode;
      uint32_t code = _KEY_NONE;
    if(inb(0x64) & 0x1)
        code = inl(0x60);
    //if(inb(I8042_STATUS_PORT) & I8042_STATUS_HASKEY_MASK)
    //    code = inl(I8042_DATA_PORT);
    // return _KEY_NONE;
    return code;
}
