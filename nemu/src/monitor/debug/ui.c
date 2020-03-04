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

  //***If the line is not null, store the line to the history of the command list for future useage.
  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

//***Why is parameter -1 passed in?
//***The complement of -1 is the maximum number!!It means looping until cut down.
static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);
static int cmd_si(char *args);
static int cmd_info(char *args);
static int cmd_p(char *args);
static int cmd_x(char *args);
static int cmd_w(char *args);
static int cmd_d(char *args);


static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  /* TODO: Add more commands */
  {"si", "si [N]:Execute N instructions and then stop, default value of [N] is 1",cmd_si},
  {"info","info r|w:Show the information about regs|watch points",cmd_info},
  {"p","p EXPR:Count the value of the expression",cmd_p},
  {"x","x [N] EXPR:Count the value of the expression and then show the memory from the position of this value, totally output [N] consecutive 4 bytes",cmd_x},
  {"w","w VAR:watch the changes of value",cmd_w},
  {"d","delete the Nth watch point.",cmd_d}
};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");//***use strtok to decompose strings.
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

//***si [N]:Execute N instructions and then stop, default value of [N] is 1
static int cmd_si(char *args) 
{
  uint64_t N=1;//***The default value of N is 1
  if(args!=NULL)
  {
    int nRes=sscanf(args,"%llu",&N);//***analytic parameter
    if (nRes<=0){//***Resolution failure
      printf("The args of command 'si' was wrong, please input integer.\n");
      return 0;
    }
  }
  cpu_exec(N);//***Do the job
  
  return 0;//!!!Be careful!!! Can't Return -1, or you will exit the nemu.
}
static int cmd_info(char *args) {
  char c;
  if(args!=NULL)
  {
    int nRes=sscanf(args,"%c",&c);
    if(nRes<=0 || (c!='r' && c!='w') ){
      printf("The args of command 'info' was wrong, please input 'r' or 'w'.\n");
      return 0;
    }
    if(c=='r'){
      printf("eip    0x%x\n",cpu.eip);
      int i;
      for(i=0;i<8;i++)
      {
        printf("%s    0x%x\n",regsl[i],reg_l(i));
        printf(" %s        0x%x\n",regsw[i],reg_w(i));
        if(i<4){
          int k=i+4;
          printf("  %s       0x%x\n",regsb[k],reg_b(k));
          printf("  %s         0x%x\n",regsb[i],reg_b(i));
        }
      }
    }
    else if (c=='w')
    {
      printf("Waiting for perfection...\n");
    }
  }else {
       printf("The args of command 'info' was wrong, please input 'r' or 'w'.\n");
  }
  return 0;
}//***end cmd_info
  

static int cmd_p(char *args) {return -1;}
static int cmd_x(char *args) {return -1;}
static int cmd_w(char *args) {return -1;}
static int cmd_d(char *args) {return -1;}

//***User interface main loop
void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

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
        if (cmd_table[i].handler(args) < 0) { return; }//***The command is not exist.
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
