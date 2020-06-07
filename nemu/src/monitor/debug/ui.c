#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_si(char *args){
	args = strtok(args, " ");
	if(args == NULL){
		cpu_exec(1);
		return 0;
	}
	for(int i=0; args[i]!=0; i++){
		if(!isdigit(args[i])){
			printf("error: parameter must be integer\n");	
			return 0;
		}	
	}
	int n = atoi(args);
	cpu_exec(n);
	return 0;
}

static int cmd_info(char *args){
	args = strtok(args, " ");
	if(args == NULL){
		printf("error: missing parameter\n");
		return 0;
	}
	if(strlen(args) != 1){
		printf("error: parameter error\n\tinfo r: Display registers\n\tinfo w: Display watchpoints\n");	
		return 0;
	}
	int i;
	if(*args == 'r'){
		for(i=0;i<8;i++){
			printf("%s\t0x%08x\n", regsl[i], reg_l(i));		
		}
		for(i=0;i<8;i++){
			printf("%s\t0x%04x\n", regsw[i], reg_w(i));		
		}
		for(i=0;i<8;i++){
			printf("%s\t0x%02x\n", regsb[i], reg_b(i));		
		}
	}
	else if(*args == 'w'){
		display();
	}
	else{
		printf("info r: Display registers\n\tinfo w: Display watchpoints\n");	
	}
	return 0;
}

static int cmd_x(char* args){	
	if(args == NULL){
		printf("error: missing parameter\n");
		return 0;
	}
	char* end = args + strlen(args);
	char* ns = strtok(args, " ");
	if(ns == NULL){
		printf("error: missing parameter\n");
		return 0;
	}
	char* exprs = ns + strlen(ns) + 1;
	if(exprs >= end){
		printf("error: missing parameter\n");	
		return 0;
	}
	int n = atoi(ns);
	int i, j, data;

	bool success;
	vaddr_t addr = expr(exprs, &success);
	if(!success){
		return 0;
	}

	for(i=0;i<n;i++){
		data = vaddr_read(addr, 4);
		printf("%#x\t0x%08x\t", addr, data);
		for(j=0;j<4;j++){
			printf("0x%02x ", data&0xff);
			data >>= 8;
		}	
		printf("\n");
		addr += 4;
	}
	return 0;
}

static int cmd_p(char* args){
	bool success;
	int data = expr(args, &success);
	if(success){
		printf("%d\n", data);
	}
	return 0;
}
static int cmd_w(char* args){
	if(args == NULL){
		printf("error: missing parameter\n");
		return 0;
	}
	if(strlen(args) > 512){
		printf("error: expr too long\n");
		return 0;
	}
	WP* wp = new_wp();
	strcpy(wp->expr, args);
	bool flag;
	wp->old = expr(wp->expr, &flag);
	if(!flag){
		free_wp(wp);
		return 0;
	}
	printf("watchpoint %d: %s\n", wp->NO, wp->expr);
	return 0;
}
static int cmd_d(char* args){
	if(args == NULL){
		char* re;
		while(1){
			re = readline("Delete all watchpoints?(y/n) ");
			if(strcmp(re, "y") == 0 || strcmp(re, "Y") == 0){
				free_all();
				break;
			}
			else if(strcmp(re, "n") == 0 || strcmp(re, "N") == 0){
				break;
			}
		}
		return 0;
	}
	free_wp_no(atoi(args));
	return 0;
}
static int cmd_help(char *args);

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Single wxecution of the program", cmd_si},
  { "info", "Display states about the program\n\tinfo r: Display registers\n\tinfo w: Display watch points", cmd_info},
  { "x", "Scanning memory\n\tx n expr", cmd_x},
  { "p", "Evaluation of expression", cmd_p},
  { "w", "Add watchpoint", cmd_w},
  { "d", "Delete watchpoint", cmd_d},
};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }
	
  /*test eval of expresstion*/
  test_lexical();
  test_check_parentheses();
  test_expr();

  while (1) {
    char *str = rl_gets();
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
