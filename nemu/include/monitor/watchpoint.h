#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO; //***The serial number of watchpoint
  struct watchpoint *next;  

  /* TODO: Add more members if necessary */
  int cur_value;
  char tar[32];
  int hitNum;
} WP;

bool new_wp(char *arg);
bool free_wp(int num);
void print_wp();
bool watch_wp();


#endif
