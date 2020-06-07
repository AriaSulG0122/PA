#ifndef __COMMON_H__
#define __COMMON_H__

#include <am.h>
#include <klib.h>
#include "debug.h"

typedef char bool;
#define true 1
#define false 0
void changeGame();

_RegSet* schedule(_RegSet *prev);
int mm_brk(uint32_t new_brk);
void load_prog(const char *filename);
void* new_page(void);

void ramdisk_read(void *buf, off_t offset, size_t len);
void ramdisk_write(const void *buf, off_t offset, size_t len);
size_t get_ramdisk_size();

_RegSet* do_syscall(_RegSet *r);
uintptr_t sys_none();
void sys_exit(uintptr_t code);
uintptr_t sys_write(uintptr_t fd, uintptr_t buf, uintptr_t len);
uintptr_t sys_brk(uintptr_t end);
uintptr_t sys_open(uintptr_t pathname, uintptr_t flags, uintptr_t mode);
uintptr_t sys_close(uintptr_t fd);
uintptr_t sys_lseek(uintptr_t fd, uintptr_t offset, uintptr_t whence);
uintptr_t sys_read(uintptr_t fd, uintptr_t buf, uintptr_t len);

int fs_open(const char* pathname, int flags, int mode);
ssize_t fs_read(int fd, void* buf, size_t len);
ssize_t fs_write(int fd, const void* buf, size_t len);
off_t fs_lseek(int fd, off_t offset, int whence);
int fs_close(int fd);
size_t fs_filesz(int fd);

void dispinfo_read(void *buf, off_t offset, size_t len);
size_t events_read(void *buf, size_t len);
void fb_write(const void *buf, off_t offset, size_t len);
#endif
