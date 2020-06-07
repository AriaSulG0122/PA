#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  uint32_t old;
  char expr[512];

} WP;
WP* new_wp();
void free_wp(WP* wp);
void free_wp_no(int no);
void display();
bool eval_watchpoint();
void free_all();
#endif
