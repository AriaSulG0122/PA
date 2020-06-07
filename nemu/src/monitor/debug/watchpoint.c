#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}


WP* new_wp(){
	if(free_ == NULL){
		assert(0);	
	}
	WP* tmp = free_->next;
	free_->next = head;
	head = free_;
	free_ = tmp;
	return head;
}

void free_wp(WP* wp){
  if(head == NULL || wp == NULL){
  	assert(0);
  }
  if(wp == head){
	head = wp->next;	
	wp->next = free_;
	free_ = wp;
	return;	
  }

  WP* s;
  for (s = head; s->next!= NULL; s = s->next) {
    if(s->next->NO == wp->NO)
    	break;
  }
  if(s->next == NULL){
	assert(0);	
  }
  
  s->next = wp->next;
  wp->next = free_;
  free_ = wp;
}

void free_wp_no(int no){
  WP* s;
  for (s = head; s; s = s->next) {
	if(s->NO == no){
		break;
	}
  }
  free_wp(s);
}

void free_all(){
  if(free_ == NULL){
	free_ = head;
	head = NULL;
  }
  WP* s;
  for (s = free_; s->next; s = s->next) {}
  s->next = head;
  head = NULL;	
}

void display(){
  WP* s;
  printf("No\tWhat\n"); 
  for (s = head; s; s = s->next) {
	printf("%d\t%s\n", s->NO, s->expr); 
  }
}

bool eval_watchpoint(){
  WP* s;
  bool flag = false, _s;
  for (s = head; s; s = s->next) {
	int data = expr(s->expr, &_s);
	if(data != s->old){
		flag = true;
		printf("trigger watchpoint %d: %s\n", s->NO, s->expr);
	}
	s->old = data;	
  }
  return flag;
}


