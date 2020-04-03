#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

//***Single character token uses ASCII as type, multi character token defines in enum
//***Why start from 256? Because single character has used 0-255(ASCII)
enum {
  TK_NOTYPE = 256, 
  TK_NUMBER,
  TK_HEX,   //*** begin with "0x"
  TK_REG,   //*** begin with "$" 
  TK_EQ, TK_NEQ,  //*** ==,!=
  TK_AND,TK_OR,   //*** &&,||
  TK_NEGATIVE,    //****** -
  TK_DEREF  //****** Pointer dereference*
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
  //***  "//" on behalf of escape
  {" +", TK_NOTYPE},    // spaces, and " +" means one or more blank space
  
  {"0x[1-9A-Fa-f][0-9A-Fa-f]*",TK_HEX}, // 0x...
  {"0|[1-9][0-9]*", TK_NUMBER},         // numbers
  
  {"\\$(eip|eax|ecx|edx|ebx|ebp|esp|esi|edi|ax|cx|dx|bx|sp|bp|si|di|al|cl|dl|bl|ah|ch|dh|bh)",TK_REG},

  {"==",TK_EQ},
  {"!=",TK_NEQ},

  {"&&",TK_AND},
  {"\\|\\|",TK_OR},
  {"!",'!'},            // not

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

//***Recognize token and segment sentence
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
          case TK_HEX:
            strncpy(tokens[nr_token].str,substr_start+2,substr_len-2);
            *(tokens[nr_token].str+substr_len-2)='\0';
            break;
          case TK_REG:
            strncpy(tokens[nr_token].str,substr_start+1,substr_len-1);
            *(tokens[nr_token].str+substr_len-1)='\0';
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
  int pos[5]={-1,-1,-1,-1,-1};//***In level = 0, record the position of the last symbol of the i-th priority
  /*** Operator Priority:
   *    pos[4]:-(begative) *(deref) !
   *    pos[3]:/ *
   *    pos[2]:+ -
   *    pos[1]:== !=
   *    pos[0]:&& ||
   *  ***/
  for(int cur=p;cur<=q;cur++)
  {
    if(tokens[cur].type=='('){count++;}
    if(tokens[cur].type==')'){
      count--;
      if(count<0){
        panic("Error: the parentheses are not matched.");return 0;
        }
    }
    if(count>0){continue;}//***token inside a pair of parentheses is not dominant operator.
    //***level=0
    switch(tokens[cur].type){
      //***Larger number has higher priority
      case TK_NEGATIVE:
      case TK_DEREF:
      case '!':
        pos[4]=cur;break;
      case '/':
      case '*':
        pos[3]=cur;break;
      case '+':
      case '-':
        pos[2]=cur;break;
      case TK_EQ:
      case TK_NEQ:
        pos[1]=cur;break;
      case TK_AND:
      case TK_OR:
        pos[0]=cur;break;
    }
  }
  for(int i=0;i<5;i++)
  {
    if (pos[i]!=-1){return pos[i];}//***dominant operator has the lowest priority.
  }
  //for(int i=0;i<5;i++){printf("%d ",pos[i]);}
  panic("Error:in findDaminantOp, p=%d,q=%d\n",p,q);
}

//***count the value of a expression
int eval(int p,int q)
{
  if(p>q){panic("Error:p>q in eval\n");return -1;}//******Panic will show in red color!!!
  else if(p==q){//single token. This token should be a number|a hex number|a reg;
    int num;
    switch(tokens[p].type){
      case TK_NUMBER:
        sscanf(tokens[p].str,"%d",&num);
        return num;
      case TK_HEX:
        sscanf(tokens[p].str,"%x",&num);
        return num;
      case TK_REG:
        for(int i=0;i<8;i++){
          if(strcmp(tokens[p].str,regsl[i])==0){return reg_l(i);}
          if(strcmp(tokens[p].str,regsw[i])==0){return reg_w(i);}
          if(strcmp(tokens[p].str,regsb[i])==0){return reg_b(i);}
        }
        if(strcmp(tokens[p].str,"eip")==0){return cpu.eip;}
      default: 
        panic("Error:in eval when p==q");
        return -1;
    }//end switch
  }//end p==q
  else if(check_parentheses(p,q)==true)//***The expression is surrounded by a matched pair of parentheses, just throw away the parentheses
  {return eval(p+1,q-1);}
  else{
    int op,val1,val2,op_type,result;
    uint32_t addr;
    op=find_domainantOp(p,q);
    op_type=tokens[op].type;
    switch(op_type){
      case TK_NEGATIVE:
        return -eval(p+1,q);
      case TK_DEREF:
        addr=eval(p+1,q);
        result=vaddr_read(addr,4);//***uint32 has 4 byte
        printf("addr=%u(0x%x)---->value=%d(0x%08x)\n",addr,addr,result,result);
        return result;
      case '!':
        result=eval(p+1,q);
        if(result==0)return 1;
        else return 0;
    }//end switch
    //printf("op_type:%d,p:%d,q:%d,op:%d\n",op_type,p,q,op);
    val1=eval(p,op-1);
    val2=eval(op+1,q);
    switch(op_type){
      case '+':   return val1+val2;
      case '-':   return val1-val2;
      case '*':   return val1*val2;
      case '/':   return val1/val2;
      case TK_EQ: return val1==val2;
      case TK_NEQ:return val1!=val2;
      case TK_AND:return val1&&val2;
      case TK_OR: return val1||val2;
      default:assert(0);
    }
  }//end else
}


uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  /* TODO: Insert codes to evaluate the expression. */
  //TODO();
  *success=true;
  for(int i=0;i<nr_token;i++)
  {
    if (tokens[i].type=='*'){
      if(i==0){tokens[i].type=TK_DEREF;continue;}//*** previous token is null
      int pre_type=tokens[i-1].type;
      if(pre_type=='+'||pre_type=='-'||pre_type=='*'||pre_type=='/'||pre_type=='('||pre_type==TK_NEGATIVE){
        tokens[i].type=TK_DEREF;continue;
      }
    }
    if (tokens[i].type=='-'){
      if(i==0){tokens[i].type=TK_NEGATIVE;continue;}//*** previous token is null
      int pre_type=tokens[i-1].type;
      if(pre_type=='+'||pre_type=='-'||pre_type=='*'||pre_type=='/'||pre_type=='('||pre_type==TK_NEGATIVE){
        tokens[i].type=TK_NEGATIVE;continue;
      }
    }
  }
  return eval(0,nr_token-1);//***all tokens are saved from tokens[0] to tokens[nr_token-1]
}
