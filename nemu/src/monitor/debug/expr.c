#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

//***Single character token uses ASCII as type, multi character token defines in enum
//***Why start from 256? Because single character has used 0-255(ASCII)
enum {
  TK_NOTYPE = 256, TK_EQ, TK_NUMBER

  /* TODO: Add more token types */
};

//******This struct record two things: regular expressions and corresponding types
static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces, and " +" means one or more blank space
  {"0|[1-9][0-9]*", TK_NUMBER},    // numbers

  {"\\+", '+'},         // plus
  {"\\-", '-'},         // minus
  {"\\*", '*'},         // mul
  {"\\/", '/'},         // div
  
  {"\\(", '('},         // LB
  {"\\)", ')'},         // RB
  {"==", TK_EQ}         // equal
};//end rule

//***Record the number of rules
#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];
/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
//***Compile the regular expressions
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

//***record the type of token, for some token like number, also need to record its value in str[32]
typedef struct token {
  int type;
  char str[32];//***what if out of buffer? assert(0)!
} Token;

Token tokens[32];//***Record the recognized token information
int nr_token;//***Record the number of tokens identified

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;
  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;
        //******Logs will show in blue!
/*         Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start); */
        position += substr_len;//***goto next token

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
        if(substr_len>32){assert(0);}//***if out of buffer, prompt error
        if(rules[i].token_type==TK_NOTYPE){break;}//***discard spaces
        tokens[nr_token].type=rules[i].token_type;
        switch(rules[i].token_type){
          case TK_NUMBER:
            strncpy(tokens[nr_token].str,substr_start,substr_len);
            *(tokens[nr_token].str+substr_len)='\0';//***  "abc"[1] = *("abc"+1)
            //*** '\0' is usually placed at the end of a string, indicating the end of the string
            break;
          //default:TODO();
        }
        nr_token++;
        break;
      }
    }//end for

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }//end while
  return true;
}

//***Check that the expression is surrounded by a matching pair of parentheses, 
//***and that the parentheses inside the expression match
bool check_parentheses(int p,int q)
{
  if(p>=q){printf("Error:p>=q in check_parenthese\n");return false;}
  if(tokens[p].type!='('||tokens[q].type!=')'){
    return false;
    }
  int count=0;
  for(int cur=p+1;cur<q;cur++)
  {
    if(tokens[cur].type=='('){count++;}
    if(tokens[cur].type==')'){count--;if(count<0){return false;}}
  }
  if(count==0){return true;}
  else{return false;}
}

//***find dominant operator in a expression, 
//***return the position of dominant operator, if the operator is not exist, return -1;
int find_domainantOp(int p,int q)
{
  int count=0;//***Record bracket pairs
  int pos[2];
  pos[0]=-1;pos[1]=-1;
  for(int cur=p;cur<=q;cur++)
  {
    if(tokens[cur].type=='('){count++;}
    if(tokens[cur].type==')'){count--;if(count<0){panic("Error: the parentheses are not matched.");return 0;}}
    if(count>0){continue;}//***token inside a pair of parentheses is not dominant operator.
    if(tokens[cur].type=='+'||tokens[cur].type=='-') {pos[0]=cur;}
    if(tokens[cur].type=='*'||tokens[cur].type=='/') {pos[1]=cur;}
  }
  if(pos[0]!=-1){return pos[0];}
  else if(pos[0]!=-1){return pos[1];}
  else {return -1;}
}

//***count the value of a expression
int eval(int p,int q)
{
  if(p>q){panic("Error:p>q in eval\n");return -1;}//******Panic will show in red color!!!
  else if(p==q){//single token. This token should be a number;
    if(tokens[p].type!=TK_NUMBER){printf("Error:The single token should be a number\n");return -1;}
    int value=0,i=0;
    while(tokens[p].str[i]!='\0'){value=value*10+tokens[p].str[i]-48;i++;}//***count the value of number
    return value;
  }
  else if(check_parentheses(p,q)==true)//***The expression is surrounded by a matched pair of parentheses, just throw away the parentheses
  {return eval(p+1,q-1);}
  else{
    int op,val1,val2,op_type;
    op=find_domainantOp(p,q);
    op_type=tokens[op].type;
    val1=eval(p,op-1);
    val2=eval(op+1,q);
    switch(op_type){
      case '+': return val1+val2;
      case '-': return val1-val2;
      case '*': return val1*val2;
      case '/': return val1/val2;
      default:assert(0);
    }
  }
}


uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  /* TODO: Insert codes to evaluate the expression. */
  //TODO();
  *success=true;
  return eval(0,nr_token-1);//***all tokens are saved from tokens[0] to tokens[nr_token-1]
}
