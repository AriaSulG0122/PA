#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

size_t events_read(void *buf, size_t len) {
  int key = _read_key();
  bool down =false;
  if(key & 0x8000){
	down=true;
	key ^= 0x8000;
  }
  if(key != _KEY_NONE){
	if(down && (key == _KEY_F12 || key == _KEY_C)){
		changeGame();
	}
	return snprintf(buf, len, "k%c %s\n", down?'d':'u', keyname[key])-1;
  }
  else{
	unsigned long t = _uptime();
	return snprintf(buf, len, "t %d\n", t)-1;
  }
}

static char dispinfo[128] __attribute__((used));

void dispinfo_read(void *buf, off_t offset, size_t len) {
	memcpy(buf, dispinfo + offset, len);
}

void fb_write(const void *buf, off_t offset, size_t len) {
	offset /= sizeof(uint32_t);
	int x = offset % _screen.width;
	int y = offset / _screen.width;
	_draw_rect(buf, x, y, len / sizeof(uint32_t), 1);
}

void init_device() {
  _ioe_init();

  sprintf(dispinfo, "WIDTH: %d\nHEIGHT: %d", _screen.width, _screen.height);
}
