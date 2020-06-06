#include "common.h"

static _RegSet* do_event(_Event e, _RegSet* r) {
  switch (e.event) {
    case _EVENT_SYSCALL:
      //return do_syscall(r);
      //pa4.2 分时多任务
      do_syscall(r);
      return schedule(r);
    case _EVENT_TRAP:
      return schedule(r);
      //printf("Receive an event trap!");break;
    default: panic("Unhandled event ID = %d", e.event);
  }

  return NULL;
}

void init_irq(void) {
  _asye_init(do_event);
}
