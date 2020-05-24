#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
    [_KEY_NONE] = "NONE",
    _KEYS(NAME)};

//���¼�д�뵽buf�У�Ȼ�󷵻�д���ʵ�ʳ���
size_t events_read(void *buf, size_t len)
{
  char str[100];
  int key=_read_key();
  Log("ReadKey:%s\n",keyname[key]);
  bool down=false;
  if(key&0x8000){
    key^=0x8000;//��ȡ��ͨ���ʾ�İ���λ��
    down=true;
  }
  
  if(key!=_KEY_NONE){//û�а����������Ϊʱ���¼�
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
//���ַ���dispinfo��offset��ʼ��len�ֽ�д��buf��
void dispinfo_read(void *buf, off_t offset, size_t len)
{
  strncpy(buf,dispinfo+offset,len);
}
//��buf�е�len�ֽ�д����Ļ�ϵ�offset��
void fb_write(const void *buf, off_t offset, size_t len)
{
  //VGA���ص�λΪuint32_t������ȳ�4
  offset = offset >> 2;
  len = len >> 2;
  int x, y;
  //x������
  x = offset % _screen.width;
  //y������
  y = offset / _screen.width;
  int firstlen, middlelen, lastlen;
  firstlen = middlelen = lastlen = 0;
  //���Ƶ�һ��
  firstlen = len <= _screen.width - x ? len : _screen.width - x;
  _draw_rect((uint32_t *)buf, x, y, firstlen, 1);

  if (len > firstlen) //���˵�һ�����⻹��
  {
    middlelen = (len - firstlen) / _screen.width; //�м�����
    lastlen = (len - firstlen) % _screen.width;   //���һ��ƫ��
    if (middlelen > 0)
    { //�����м���
      _draw_rect((uint32_t *)buf + firstlen, 0, y + 1, _screen.width, middlelen);
    }
    if (lastlen > 0)
    { //�������һ��
      _draw_rect((uint32_t *)buf + len - lastlen, 0, y + middlelen + 1, lastlen, 1);
    }
  }
}

//��/proc/dispinfo��������ǰд�뵽�ַ���dispinfo��
void init_device()
{
  _ioe_init();

  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
  sprintf(dispinfo, "WIDTH:%d\nHEIGHT:%d\n", _screen.width, _screen.height);
}
