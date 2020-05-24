#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
    [_KEY_NONE] = "NONE",
    _KEYS(NAME)};

//把事件写入到buf中，然后返回写入的实际长度
size_t events_read(void *buf, size_t len)
{
  char str[100];
  int key=_read_key();
  Log("ReadKey:%s\n",keyname[key]);
  bool down=false;
  if(key&0x8000){
    key^=0x8000;//获取该通码表示的按键位置
    down=true;
  }
  
  if(key!=_KEY_NONE){//没有按键，则表明为时钟事件
    sprintf(str,"%s %s\n",down?"kd":"ku",keyname[key]);
    Log("Key:%s\n",keyname[key]);
  }else{
    sprintf(str,"t %d\n",_uptime());
    Log("Time\n");
  }
  if(strlen(str)<=len){
    strncpy((char*)buf,str,strlen(str));
    return strlen(str);
  }
  Log("Strlen(event)>len,return 0");
  return 0;
}

static char dispinfo[128] __attribute__((used));
//把字符串dispinfo中offset开始的len字节写到buf中
void dispinfo_read(void *buf, off_t offset, size_t len)
{
  strncpy(buf,dispinfo+offset,len);
}
//把buf中的len字节写到屏幕上的offset处
void fb_write(const void *buf, off_t offset, size_t len)
{
  //VGA像素单位为uint32_t，因此先除4
  offset = offset >> 2;
  len = len >> 2;
  int x, y;
  //x轴坐标
  x = offset % _screen.width;
  //y轴坐标
  y = offset / _screen.width;
  int firstlen, middlelen, lastlen;
  firstlen = middlelen = lastlen = 0;
  //绘制第一行
  firstlen = len <= _screen.width - x ? len : _screen.width - x;
  _draw_rect((uint32_t *)buf, x, y, firstlen, 1);

  if (len > firstlen) //除了第一行以外还有
  {
    middlelen = (len - firstlen) / _screen.width; //中间行数
    lastlen = (len - firstlen) % _screen.width;   //最后一行偏移
    if (middlelen > 0)
    { //绘制中间行
      _draw_rect((uint32_t *)buf + firstlen, 0, y + 1, _screen.width, middlelen);
    }
    if (lastlen > 0)
    { //绘制最后一行
      _draw_rect((uint32_t *)buf + len - lastlen, 0, y + middlelen + 1, lastlen, 1);
    }
  }
}

//将/proc/dispinfo的内容提前写入到字符串dispinfo中
void init_device()
{
  _ioe_init();

  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
  sprintf(dispinfo, "WIDTH:%d\nHEIGHT:%d\n", _screen.width, _screen.height);
}
