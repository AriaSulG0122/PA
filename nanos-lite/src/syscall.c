#include "common.h"
#include "syscall.h"

_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);
  a[1] = SYSCALL_ARG2(r);
  a[2] = SYSCALL_ARG3(r);
  a[3] = SYSCALL_ARG4(r);

  switch (a[0]) {
	case SYS_none:
		//Log("sys_none");
		r->eax = sys_none();
		break;
	case SYS_exit:
		//Log("sys_exit");
		sys_exit(a[1]);
		break;
	case SYS_write:
		//Log("sys_write");
		r->eax = sys_write(a[1], a[2], a[3]);
		break;
	case SYS_read:
		//Log("sys_read");
		r->eax = sys_read(a[1], a[2], a[3]);
		break;
	case SYS_open:
		//Log("sys_open");
		r->eax = sys_open(a[1], a[2], a[3]);
		break;
	case SYS_close:
		//Log("sys_close");
		r->eax = sys_close(a[1]);
		break;
	case SYS_lseek:
		//Log("sys_lseek");
		r->eax = sys_lseek(a[1], a[2], a[3]);
		break;
	case SYS_brk:
		//Log("sys_brk");
		r->eax = sys_brk(a[1]);
		break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}

uintptr_t sys_open(uintptr_t pathname, uintptr_t flags, uintptr_t mode){
	return fs_open((char*)pathname, flags, mode);
}
uintptr_t sys_close(uintptr_t fd){
	return fs_close(fd);
}
uintptr_t sys_lseek(uintptr_t fd, uintptr_t offset, uintptr_t whence){
	return fs_lseek(fd, offset, whence);
}
uintptr_t sys_read(uintptr_t fd, uintptr_t buf, uintptr_t len){
	return fs_read(fd, (void*)buf, len);
}
uintptr_t sys_write(uintptr_t fd, uintptr_t buf, uintptr_t len){
	/*
	if(fd==1 || fd==2){
		for(int i=0;i<len;i++){
			_putc(((char*)buf)[i]);
		}
	}
	return len;
	*/
	return fs_write(fd, (void*)buf, len);
}

uintptr_t sys_brk(uintptr_t end){
	return mm_brk(end);
}

uintptr_t sys_none(){
	return 1;
}

void sys_exit(uintptr_t code){
	_halt(code);
}
