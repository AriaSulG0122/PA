#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];//***store all the wp in this pool
static WP *head, *free_;//***head store wps which are being used, free store wps which are not.
static int count;

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

/* TODO: Implement the functionality of watchpoint */

//***Add a wp
bool new_wp(char *args){
  if(free_==NULL){panic("There is no more place for new watchpoint!");}
  WP* wp_new=free_;//***get a new wp
  free_=free_->next;
  //***record info about new wp
  wp_new->NO=count++;
  wp_new->next=NULL;
  strcpy(wp_new->tar,args);
  bool success;
  wp_new->old_value=expr(wp_new->tar,&success);
  if(!success){
    printf("Error: expression fault in new_wp:!\n");
    return false;
  }
  wp_new->hitNum=0;
  
  //***add new wp to the head of the link
  if(head==NULL){head=wp_new;}
  else{
    wp_new->next=head;
    head=wp_new;
  }
  printf("Success to set watchpoint. No:%d, cur value:%d\n",wp_new->NO,wp_new->old_value);
  return true;
}

//***Delete a wp with input number, if success to delete, return true 
bool free_wp(int num){
  WP *curNode;
  if(head==NULL){
    printf("There is no watch point\n");
    return false;
  }
  curNode=head;
  if(head->NO==num){//***judge head first for 
    head=head->next;
  }
  else{
    while(curNode->next!=NULL){
      if(curNode->next->NO==num){//***find the node to free
        WP *tempNode;
        tempNode=curNode->next;
        curNode->next=curNode->next->next;
        curNode=tempNode;
        break;
      }
      curNode=curNode->next;
    }//end while
  }
  if(curNode!=NULL){//***Add the node to free list
    curNode->next=free_;
    free_=curNode;
    return true;
  }
  return false;
}

//***Print the info of a wp
void print_wp(){
  if(head==NULL){panic("There is no watch point.\n");}
  printf("Watchpoint Situation:\n");
  WP *tempNode=head;
  while(tempNode!=NULL){
    printf("No:%d   Target:%s   TitTimes:%d   CurrentValue:%d",tempNode->NO,tempNode->tar,tempNode->hitNum,tempNode->old_value);
    tempNode=tempNode->next;
  }
}

//***If there are any changes about wp, print its info and return false
bool watch_wp(){
  bool success;
  int cur_value;
  if(head==NULL){return true;}
  WP *tempNode=head;
  while(tempNode!=NULL){
    cur_value=expr(tempNode->tar,&success);
    if(cur_value!=tempNode->old_value){
      tempNode->hitNum++;
      printf("Watchpoint  No:%d   Target:%s   OldValue:%d   NewValue:%d\n",
      tempNode->NO,tempNode->tar,tempNode->old_value,cur_value);
      tempNode->old_value=cur_value;
      return false;//***in this case, if trigger once, return back.
    }
    tempNode=tempNode->next;
  }
  return true;
}