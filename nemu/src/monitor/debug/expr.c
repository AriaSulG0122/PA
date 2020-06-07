#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>

enum {
  TK_NOTYPE = 256, TK_EQ, Integer, MINUS, Hex, VALUE, TK_NEQ, AND, OR,
  eax, ecx, edx, ebx, esp, ebp, esi, edi,
  ax, cx, dx, bx, sp, bp, si, di, 
  al, cl, dl, bl, ah, ch, dh, bh, eip
};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"\\-", '-'},
  {"\\*", '*'},
  {"\\/", '/'},
  {"\\(", '('},
  {"\\)", ')'},

  {"==", TK_EQ},         // equal
  {"\\!\\=", TK_NEQ}, 
    
  {"\\&\\&", AND},     
  {"\\|\\|", OR},  
  {"\\!", '!'},
  
  {"0\\x(0|[1-9a-fA-F][0-9a-fA-F]*)", Hex},
  {"0|[1-9][0-9]*", Integer},

  {"\\$eax", eax},
  {"\\$ecx", ecx},
  {"\\$edx", edx},
  {"\\$ebx", ebx},
  {"\\$esp", esp},
  {"\\$ebp", ebp},
  {"\\$esi", esi},
  {"\\$edi", edi},

  {"\\$ax", ax},
  {"\\$cx", cx},
  {"\\$dx", dx},
  {"\\$bx", bx},
  {"\\$sp", sp},
  {"\\$bp", bp},
  {"\\$si", si},
  {"\\$di", di},

  {"\\$al", al},
  {"\\$cl", cl},
  {"\\$dl", dl},
  {"\\$bl", bl},
  {"\\$ah", ah},
  {"\\$ch", ch},
  {"\\$dh", dh},
  {"\\$bh", bh},
  {"\\$eip", eip},
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;
  memset(&tokens, 0, 32 * sizeof(Token));

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        //char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;
	/*
        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
	*/        
	position += substr_len;

        assert(substr_len < 32);

        switch (rules[i].token_type) {
          case '+':
		tokens[nr_token++].type = rules[i].token_type;
		break;
          case '-':
		tokens[nr_token++].type = rules[i].token_type;
		break;
          case '*':
		tokens[nr_token++].type = rules[i].token_type;
		break;
          case '/':
		tokens[nr_token++].type = rules[i].token_type;
		break;
          case '(':
		tokens[nr_token++].type = rules[i].token_type;
		break;
          case ')':
		tokens[nr_token++].type = rules[i].token_type;
		break;
          case '!':
		tokens[nr_token++].type = rules[i].token_type;
		break;
          case Integer:
		tokens[nr_token].type = rules[i].token_type;
		strncpy(tokens[nr_token++].str,
			e + position - substr_len,
			substr_len);
		*(tokens[nr_token].str+substr_len) = '\0';
		break;
	  case Hex:
		tokens[nr_token].type = rules[i].token_type;
		strncpy(tokens[nr_token++].str,
			e + position - substr_len,
			substr_len);
		*(tokens[nr_token].str+substr_len) = '\0';
		break;
          case TK_EQ:
		tokens[nr_token++].type = rules[i].token_type;
		break;
          case TK_NEQ:
		tokens[nr_token++].type = rules[i].token_type;
		break;
          case AND:
		tokens[nr_token++].type = rules[i].token_type;
		break;
          case OR:
		tokens[nr_token++].type = rules[i].token_type;
		break;
	  case TK_NOTYPE:
		break;
	  default:
		if(rules[i].token_type >= eax 
		&& rules[i].token_type <= eip){
			tokens[nr_token++].type = rules[i].token_type;
		}
		break;
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  for(i=0;i<nr_token;i++){
	if(tokens[i].type == '-' 
	&& (i==0 || 
		(tokens[i-1].type != Integer 
		&& tokens[i-1].type != ')'
		&& tokens[i-1].type != Hex
		&& (tokens[i-1].type < eax|| tokens[i-1].type > eip)))){
		tokens[i].type = MINUS;
	}
	if(tokens[i].type == '*' 
	&& (i==0 || 
		(tokens[i-1].type != Integer 
		&& tokens[i-1].type != ')'
		&& tokens[i-1].type != Hex
		&& (tokens[i-1].type < eax|| tokens[i-1].type > eip)))){
		tokens[i].type = VALUE;
	}
  }

  return true;
}
void test_lexical(){
  /*
  258	3
  42	
  258	7
  47	
  258	9
  43	
  258	0
  45	
  258	9
  */
  char* e = "3 *7   /9 +0--91 * $eax * $si - $al + 0x12 + !1 -*0x100000";
  make_token(e);

  assert(tokens[0].type == Integer);
  assert(tokens[1].type == '*');
  assert(tokens[2].type == Integer);
  assert(tokens[3].type == '/');
  assert(tokens[4].type == Integer);
  assert(tokens[5].type == '+');
  assert(tokens[6].type == Integer);
  assert(tokens[7].type == '-');
  assert(tokens[8].type == MINUS);
  assert(tokens[9].type == Integer);
  assert(tokens[10].type == '*');
  assert(tokens[11].type == eax);
  assert(tokens[12].type == '*');
  assert(tokens[13].type == si);
  assert(tokens[14].type == '-');
  assert(tokens[15].type == al);
  assert(tokens[16].type == '+');
  assert(tokens[17].type == Hex);
  assert(tokens[18].type == '+');
  assert(tokens[19].type == '!');
  assert(tokens[20].type == Integer);
  assert(tokens[21].type == '-');
  assert(tokens[22].type == VALUE);
  assert(tokens[23].type == Hex);



  e = "0x100000 ||1 + 1&&2 -2==2*1!=3";
  make_token(e);

  assert(tokens[0].type == Hex);
  assert(tokens[1].type == OR);
  assert(tokens[2].type == Integer);
  assert(tokens[3].type == '+');
  assert(tokens[4].type == Integer);
  assert(tokens[5].type == AND);
  assert(tokens[6].type == Integer);
  assert(tokens[7].type == '-');
  assert(tokens[8].type == Integer);
  assert(tokens[9].type == TK_EQ);
  assert(tokens[10].type == Integer);
  assert(tokens[11].type == '*');
  assert(tokens[12].type == Integer);
  assert(tokens[13].type == TK_NEQ);
  assert(tokens[14].type == Integer);

  printf("test_lexical pass!\n");
  /*
  int i;
  for(i=0;i<nr_token;i++){
  	printf("%d", tokens[i].type);
	if(tokens[i].str != NULL){
		printf("\t%s", tokens[i].str);
	}
	printf("\n");
  }
  */
}

bool check_parentheses(int p, int q, bool* isBad){
	int left = 0;
	int i;
	bool stop = false;
	for(i=p;i<=q;i++){
  		if(tokens[i].type == '('){
			left++;		
		}
		else if(tokens[i].type == ')'){
			if(left > 0){
				left--;		
				if(left==0 && i!=q){
					stop = true;
				}	
			}
			else{
				*isBad = true;
				return false;			
			}
		}
  	}
	if(left>0){
		*isBad = true;
		return false;			
	}
	*isBad = false;
	if(tokens[p].type == '(' && tokens[q].type == ')' && !stop){
		return true; 	
	}
	return false;
}
void test_check_parentheses(){
	bool isbad;
	char* e = "(2-1)";
	make_token(e);
	assert(check_parentheses(0, 4, &isbad));
	assert(!isbad);

	e = "(4 + 3*(2-1))";
	make_token(e);
	assert(check_parentheses(0, 10, &isbad));
	assert(!isbad);

	e = "4 +3*(2-1)";
	make_token(e);
	assert(!check_parentheses(0, 8, &isbad));
	assert(!isbad);

	e = "(4+3))*((2-1)";
	make_token(e);
	assert(!check_parentheses(0, 12, &isbad));
	assert(isbad);

	e = "(4+3) *(2-1)";
	make_token(e);
	assert(!check_parentheses(0, 10, &isbad));
	assert(!isbad);

	printf("test_check_parentheses pass!\n");
}

struct Op{
	int type;
	int sup;
	bool left;
}ops[] =
{
	{OR, 13, true},
	{AND, 12, true},
	{TK_EQ, 8, true},
	{TK_NEQ, 8, true},
	{'+', 4, true},
	{'-', 4, true},
	{'*', 3, true},
	{'/', 3, true},
	{MINUS, 2, false},
	{'!', 2, false},
	{VALUE, 2, false}
};
#define NR_OP (sizeof(ops) / sizeof(struct Op) )
int getSup(int type){
	for(int i=0;i<NR_OP;i++){
		if(ops[i].type == type){
			return ops[i].sup;		
		}
	}
	return 0;
}
uint32_t find_dominant(int p, int q){
	int i;
	int left = 0;
	int res = -1;
	for(i=p;i<=q;i++){
		if(tokens[i].type == '+' ||
			tokens[i].type == '-' ||
			tokens[i].type == '/' ||
			tokens[i].type == '*' ||
			tokens[i].type == TK_EQ ||
			tokens[i].type == TK_NEQ ||
			tokens[i].type == AND ||
			tokens[i].type == OR){
			if(left == 0){
				if(res == -1){
					res = i;				
				}		
				else if(getSup(tokens[i].type)
					>getSup(tokens[res].type)){
					res = i;
				}
				else if(getSup(tokens[i].type)
					==getSup(tokens[res].type)){
					res = i;
				}
			}
		}
		else if(tokens[i].type == MINUS
			|| tokens[i].type == VALUE
			|| tokens[i].type == '!'){
			if(left == 0){
				if(res == -1){
					res = i;				
				}		
				else if(getSup(tokens[i].type)
					>getSup(tokens[res].type)){
					res = i;
				}
			}
		}
		else if(tokens[i].type == '('){
			left++;		
		}	
		else if(tokens[i].type == ')'){
			left--;		
		}
	}
	return res;
}
int eval(int p, int q, bool *success){
	bool isbad;
	if(p > q){
		printf("Bad expression!!!\n");
		*success = false;
		return 0;	
	}
	else if(p == q){
		if(tokens[p].type == Integer){
			return atoi(tokens[p].str);
		}
		else if(tokens[p].type == Hex){
			return strtol(tokens[p].str, NULL, 16);
		}
		else if(tokens[p].type >= eax && tokens[p].type <= edi){
			return reg_l(tokens[p].type - eax);
		}
		else if(tokens[p].type >= ax && tokens[p].type <= di){
			return reg_w(tokens[p].type - ax);
		}
		else if(tokens[p].type >= al && tokens[p].type <= bh){
			return reg_b((tokens[p].type - al));
		}
		else if(tokens[p].type == eip){
			return cpu.eip;		
		}
		else{
			printf("Bad expression!!!\n");
			*success = false;
			return 0;		
		}
			
	}
	else if(check_parentheses(p, q, &isbad) == true){
		
		return eval(p + 1, q - 1, success);	
	}
	else if(check_parentheses(p, q, &isbad) == false && isbad == true){
		printf("Bad expression!!!\n");	
		*success = false;
		return 0;		
	}
	else{
		int dominant = find_dominant(p, q);
		if(tokens[dominant].type == MINUS){
			return -eval(dominant + 1, q, success);	
		}
		else if(tokens[dominant].type == VALUE){
			return vaddr_read(eval(dominant + 1, q, success), 4);	
		}
		else if(tokens[dominant].type == '!'){
			return !eval(dominant + 1, q, success);	
		}
		int val1 = eval(p, dominant - 1, success);
		if(!success){
			printf("Bad expression!!!\n");	
			return 0;
		}
		int val2 = eval(dominant + 1, q, success);
		if(!success){
			printf("Bad expression!!!\n");	
			return 0;
		}
		switch(tokens[dominant].type){
			case '+':
				return 	val1 + val2;	
				break;
			case '-':
				return 	val1 - val2;	
				break;
			case '*':
				return 	val1 * val2;	
				break;
			case '/':
				return 	val1 / val2;	
				break;
			case TK_NEQ:
				return 	val1 != val2;	
				break;
			case TK_EQ:
				return 	val1 == val2;	
				break;
			case AND:
				return 	val1 && val2;	
				break;
			case OR:
				return 	val1 || val2;	
				break;
			default:
				printf("Bad expression!!!\n");
				*success = false;
				return 0;
		}
	}
}
uint32_t expr(char *e, bool *success) {
  *success = true;
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  return eval(0, nr_token - 1, success);
}
void test_expr(){
	bool flag;

	assert(expr("1", &flag) == 1);
	assert(expr("(2)", &flag) == 2);

	assert(expr("", &flag) == 0);
	assert(flag == false);

	assert(expr("((1)", &flag) == 0);
	assert(flag == false);

	assert(expr("(", &flag) == 0);
	assert(flag == false);

	assert(expr("1 + 2", &flag) == 3);
	assert(expr("4 * 12", &flag) == 48);
	assert(expr("(1-3)*8/4 - 3 *(1+(2-1))", &flag) == -10);
	assert(expr("4 / 2", &flag) == 2);
	assert(expr("(1+5)*2 + 2", &flag) == 14);
	assert(expr("(1) + 2", &flag) == 3);
	assert(expr("(1 + 2)", &flag) == 3);
	assert(expr("1 + (1 + (3*4-2)/2)", &flag) == 7);
	assert(expr("(1+5)*2 + 2", &flag) == 14);
	assert(expr("--2", &flag) == 2);
	assert(expr("2--2", &flag) == 4);
	assert(expr("3--2", &flag) == 5);
	assert(expr("3 * (1+-2+4 +1) /2 ", &flag) == 6);

	assert(expr("0x12 * 2 -2 /2", &flag) == 35);	
	assert(expr("$eax", &flag) == reg_l(eax-eax));
	assert(expr("$bx * 2", &flag) == 2 * reg_w(bx-ax));
	assert(expr("($bh * $ah) + $bl + 1", &flag) == 
		reg_b((bh-al))* reg_b((ah-al)) + reg_b((bl-al)) + 1);

	assert(expr("!1", &flag) == 0);
	assert(expr("*0x100000", &flag) == vaddr_read(0x100000, 4));
	assert(expr("2**0x100000 - 2", &flag) == 2*vaddr_read(0x100000, 4)-2);
	assert(expr("!0 + !2 * !2 -1", &flag) == 0);

	assert(expr("1||0", &flag) == 1);
	assert(expr("1&&0", &flag) == 0);
	assert(expr("1&&2", &flag) == 1);
	assert(expr("1==2 + 1!=1", &flag) == 1);
	assert(expr("(1==2) + (1!=1)", &flag) == 0);
	assert(expr("1==1 * 2", &flag) == 0);
	assert(expr("(1==1) * 2", &flag) == 2);

	printf("test_expr pass!\n");
}
