#ifndef __COMMON_H__
#define __COMMON_H__

#include <am.h>
#include <klib.h>
#include "debug.h"

typedef char bool;
#define true 1
#define false 0

_RegSet* do_syscall(_RegSet *r);

#endif
