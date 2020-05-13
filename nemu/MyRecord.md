指令的执行流程： 取指IF、译码ID、执行EX、更新EIP

RISC的宗旨就是简单，指令少，指令长度固定，指令格式统一

注意i386手册的opcode map(Page 414)

# 一、过程要点

## 2.define用法

C语言有许多预处理命令，#define是其预处理命令之一。所有预处理命令以“#”号开头，如包含命令#include，标准错误指令#error，#pragma指令等。#define指令用于宏定义，可以提高源代码的可读性，为编程提供方便，一般放在源文件的前面部分。

本文简要总结#define指令的多种用法及其注意事项。

### 1、 无参数定义

定义形式如下：

#define 标识符 字符串

无参数宏定义不含参数，常用于常量定义或重新定义数据类型。

#### 1) 常量定义

在编程应用中，对于频繁使用或具有特殊意义的数字可以采用宏定义，在编译预处理时，对程序中所有出现的宏名，都用定义的字符串代替。如：#define PI 3.1416，这样在对源程序作编译时，将先由预处理程序进行宏代换，即用3.1416去置换所有的宏名PI，然后再进行编译。切记不要定义成#define PI=3.1416，这是新手常犯错误。

如果不再使用已定义过的宏，可以用#undef命令终止该宏定义的作用域。

#### 2) 重新定义数据类型

可以把已有类型定义成一个你想要的新类型名，如#define FT float。编写源程序时可用FT替代float；在编译预处理时则将FT全都替换成float。

### 2、 带参数定义

C语言允许宏带有参数，使用带参数的宏定义可完成函数调用的功能，又能减少系统开销，提高运行效率。同时也不需要像函数调用那样保留现场，以便子函数执行结束后能返回继续执行，同样在子函数执行完后要恢复调用函数的现场，这都带来一定的时间开销。与函数类似，在宏定义中的参数称为形式参数，在宏调用中的参数称为实际参数。对带参数的宏，在调用中，不仅要宏展开，而且要用实参去代换形参。

带参数宏定义的一般形式为：

#define 宏名(形参表) 字符串

如定义一个求两个量的乘积的带参数宏，可以按如下形式定义：

#define MULTIPLY(a,b) ((a)*(b))

假如源程序中有MULTIPLY（5,6），则在编译预处理时，会用((5)*(6))来替代。

注意，在定义宏时一定要把字符串用括号括起来，并且每一个参数均需括起来，否则程序有可能不会按照你的意图执行。如果你把宏简单地定义成了如下形式：

#define MULTIPLY(a,b) a*b

此种定义下，若源程序中有MULTIPLY（2+3,3+3），编译预处理时不会做任何计算，即绝不会先计算2+3和3+3再替换，而是直接替换。那么结果将会是2+3*3+3=14，已经不再符合编程的预期结果30。

### 3、 多行定义

#define可以进行多行定义，用于替代多行语句代码。定义形式如下：

#define MACRO（参数列表）do{ \

语句1; \

… \

语句n; \

}while(0)

切记，需要在每行的末尾一定要加上“\”，起到换行的作用。

### 4、 单行定义

#define Conn(x,y) x##y /* x##y表示什么？表示x连接y */

#define ToChar(x) #@x /* #@x，其实就是给x加上单引号 */

#define ToString(x) #x /* #x是给x加双引号 */

### 5、 用#define来处理头文件嵌套包含问题

由于头文件包含可以嵌套，那么c文件有可能包含多次同一个头文件，就可能出现重复定义的问题的，那么可以就通过条件编译开关来避免重复包含，一般头文件可以做如下定义：

#ifndef __headerfileXXX__

#define __headerfileXXX__

…

文件内容

…

#endif

### 6、 #define特性及使用说明

1) 宏名一般用大写，且宏定义末尾不加分号；

2) 宏定义通常在文件的最开头，作用域通常从定义处到文件末尾，也可以用#undef命令提前终止宏定义的作用域；

3) 宏定义不存在类型问题，它的参数也是无类型的，编译预处理不做语法检查，不分配内存；

4) 字符串" "中永远不包含宏；

5) 编程时使用宏可提高程序的通用性和易读性，减少不一致性，减少输入错误和便于修改。


## 3.关于EFLAGS

### 位域
有些信息在存储时，并不需要占用一个完整的字节， 而只需占几个或一个二进制位。例如在存放一个开关量时，只有0和1 两种状态， 用一位二进位即可。为了节省存储空间，并使处理简便，Ｃ语言又提供了一种数据结构，称为“位域”或“位段”。所谓“位域”是把一个字节中的二进位划分为几个不同的区域，并说明每个区域的位数。每个域有一个域名，允许在程序中按域名进行操作。 这样就可以把几个不同的对象用一个字节的二进制位域来表示。

### EFLAGS初始值
i386手册的Figure 2-8，记录EFLAGS结构  
i386手册的P174,EFLAGS初始值为0000002H

关注CF、ZF、SF、IF、OF：  
-  CF为Carry Flag，ZF为Zero Flag，SF为Sign Flag，OF为Overflow。这四者属于STATUS FLAG。
-  IF为Interrupt Flag，属于CONTROL FLAG  

STATUS FLAG允许结果影响后续指令。算数指令用到了CF、ZF、SF、OF。  
附录C记录了各个位的功能：
- CF:进位标志，记录进位或借位
- ZF:零标志，记录结果为0
- SF:符号标志，与运算结果的最高位相同（0为正，1为负）
- OF:溢出标志，记录是否数值溢出
- IF:中断允许标志，是否允许响应CPU外部的可屏蔽中断请求
  
## 4.关于ModR/M
位于i386手册241页
### mod field
最高两比特，与r/m field组合成五比特可表示32种可能值：8个寄存器与24种索引模式
### reg/opcode field
中间三比特，指明寄存器编号或者作为操作码opcode的延长位
### r/m field
最低三比特，指明某个寄存器作为操作数值或者与mod field组成索引模式



# 二、i386手册阅读

## 第17章

## 附录A

# 三、代码实现

## Part One

### Call (Av)

pushl %eax  ==   subl $4,%esp + movl %eax,(%esp)

1. 查看手册，opcode map的e行8列(Call Av)，或者查看反汇编文件。    
   - A表示直接寻址，指令没有modR/M字节，操作数地址位于指令中。  
   - v表示为字或者双字，取决于操作数大小属性。  
2. 手册第275页详细解释了call指令。其中Call E8为cw，根据17.2.2.1节，可知cw表示在opcode后有两个字节长度的值表示偏移量。根据Description知道，Call E8会将后面两个字节的值作为偏移跳转相对于下一条指令的位置。  
这里的e8 01 00 00 00的意思就是跳转相对于下一个地址的00 01长的位置，也就是跳过一个字节（**注意这里的字节序！！！为小端编址，因此是00 01而不是01 00**）。在这里就是跳过了nop指令进入到0x100010，后面还有一个e8 05 00 00 00也是同理，跳过了5个字节长度的指令，来到了0x100020。
3. 填写opcode table里的相应位置。e8上填入IDEX(J,call)
4. 实现对应的译码函数。
在make_DHelper(J)中
```C
decode_op_SI(eip, id_dest, false);
decoding.jmp_eip = id_dest->simm + *eip;
```
第一句将立即数值记录到操作数id_dest中，因为后面一条指令会记录decoding.jmp_eip，不需要再进行全局存储了，所以load_val=false
第二句根据读取到的立即数值设置跳转地址。   
然后完善decode_op_SI，加入语句（和上面的decode_op_I极为相似，这里SI是不在i386手册的附录A中的，从我的理解来看，SI相比于I仅仅多了一个字节数的判断，要求2字节或者3字节）
```C
  op->simm=instr_fetch(eip,op->width);//利用instr_fetch从eip开始读取op->width长度的指令，然后赋值给op->simm
  rtl_li(&op->val, op->simm);//将立即数值记录到op->val中
```
#### rtl_push
实现对应的执行函数。在all_instr.h中声明函数make_EHelper(call)。实现函数exec_call()，因为call为跳转控制类指令，因此应该到control.c中实现。为实现call，需要先实现rtl_push函数:  
    ```C
    static inline void rtl_push(const rtlreg_t* src1) {
    rtl_subi(&cpu.esp,&cpu.esp,4);  
    rtl_sm(&cpu.esp,4,src1);
    }
    ```  
    再实现exec_call函数:
    ```C
    make_EHelper(call) {
    decoding.is_jmp=1;//This is jump instruction
    rtl_push(eip);
    print_asm("call %x", decoding.jmp_eip);
    }
    ```

测试结果，能通过。

### PUSH general register(eBP)

invalid opcode(eip=0x00100010):55 89 35 83 ec 08 e8 05 ...  
 make_DopHelper(r)中
 ```C 
 op->reg = decoding.opcode & 0x7;
 ```
 将opcode的末三位和0x111做与运算，找到对应的寄存器。  

 ```C 
 if (load_val) {
    rtl_lr(&op->val, op->reg, op->width);
  }
  ```
  如果需要读取信息，则利用rtl_lr进行读取，并存入到全局译码信息id_dest->val中，以备后续push指令进行读取使用。 
 make_EHelper(push)中
  ```C 
 rtl_push(&id_dest->val);
 ```
 实现将读取到的信息push进栈

### sub指令
**(个人觉得这个指令的实现很复杂，涉及很多内容）**
invalid opcode(eip=0x00100013):83 ec 08 e8 05 00 00 00 ...  
在附录A中为Grp1 Ev,Iv  
sub指令相关描述位于i386手册P404,而相关的sbb指令描述位于P386  
在opcode_table中的0x83处已经有了IDEX(SI2E, gp1)  
其中  
  S:利用modR/M字段来选择一个段寄存器  
  I:立即数  
  E:利用modR/M字段来选择一个通用寄存器或者内存地址，而内存地址基于段寄存器与另一个值（基寄存器、数值寄存器、缩放因子、偏移量）  
#### ModR/M字段解析
ModR/M字段相关介绍在上文  
```C
void read_ModR_M(vaddr_t *eip, Operand *rm, bool load_rm_val, Operand *reg, bool load_reg_val) {
  ModR_M m;
  m.val = instr_fetch(eip, 1);//先获取ModR_M字段值，一个字节
  decoding.ext_opcode = m.opcode;//获取拓展指令字段
  if (reg != NULL) {//如果Operand reg不为空，则表明中间三比特字段为寄存器的编号
    reg->type = OP_TYPE_REG;//操作数类型为OP_TYPE_REG
    reg->reg = m.reg;//记录操作数的寄存器编号
    if (load_reg_val) {//如果需要加载操作数值，则加载到reg->val中
      rtl_lr(&reg->val, reg->reg, reg->width);
    }
```
#### grp1理解
grp1为一个指令组，其中包含了多种指令，需要我们进行填充。  
关于make_group的define定义如下：  
```C
#define make_group(name, item0, item1, item2, item3, item4, item5, item6, item7) \
  static opcode_entry concat(opcode_table_, name) [8] = { \
    /* 0x00 */	item0, item1, item2, item3, \
    /* 0x04 */	item4, item5, item6, item7  \
  }; \
static make_EHelper(name) { \
  idex(eip, &concat(opcode_table_, name)[decoding.ext_opcode]); \
}
```
这声明一组opcode_table_gp[x] [8]，每一个元素都包含函数opcode_table_gp[x] [i]，该函数最终落实到IDEXW(id,ex,w)  
idex一句将根据decoding的ext_opcode来确定执行哪个函数，而ext_opcode是属于ModR/M字段的中间三位。  
我们知道sub为83 ec，其中83是操作码，而ec则是ModR/M字段，其数值为1110 1100，可以得到中间三位是101，即5，因此需要在gp1的第五个位置（从0开始）填入EX(sub)
#### 创建EFLAGS结构
EFLAGS相关内容在前面已介绍  
在reg.h中的CPU_STATE结构中
```C
//说明EFLAGS Register结构
  union
  {
    struct
    {
      uint32_t CF:1;
      uint32_t :5;//占用空位
      uint32_t ZF:1;
      uint32_t SF:1;
      uint32_t :1;
      uint32_t IF:1;
      uint32_t :1;
      uint32_t OF:1;
      uint32_t :20;
    };
    uint32_t eflags;
  };
```
#### 初始化EFLAGS
在monitor.c的restart()函数中，初始化EFLAGS值：
```C
cpu.eflags=0x0000002;//为EFLAGS设置初始值
```
#### 完善sub指令
sub指令相关描述位于i386手册P404,而相关的sbb指令描述位于P386  
sub指令和sbb指令极为相似，我认为仅有的区别是sbb增加了对CF位的处理  
因此在make_EHelper(sub)中，只需要复制sbb中的内容，并删除处理CF位相关的两行:  
```C
make_EHelper(sub) {
  //TODO();
  rtl_sub(&t2, &id_dest->val, &id_src->val);
  rtl_sltu(&t3, &id_dest->val, &t2);

  operand_write(id_dest, &t2);//完成计算，写入结果

  rtl_update_ZFSF(&t2, id_dest->width);//更新ZF与SF位
  //设置CF位
  rtl_sltu(&t0, &id_dest->val, &t2);
  rtl_or(&t0, &t3, &t0);
  rtl_set_CF(&t0);
  //设置OF位
  rtl_xor(&t0, &id_dest->val, &id_src->val);
  rtl_xor(&t1, &id_dest->val, &t2);
  rtl_and(&t0, &t0, &t1);
  rtl_msb(&t0, &t0, id_dest->width);
  rtl_set_OF(&t0);

  print_asm_template2(sub);
}
```
但是其中用到的部分rtl指令需要进一步完善。

#### 完善rtl_set
直接对相应的位赋值就行了  
```C
//set为写，get为读
#define make_rtl_setget_eflags(f) \
  static inline void concat(rtl_set_, f) (const rtlreg_t* src) { \
    cpu.f=*src;/*TODO();*/ \
  } \
  static inline void concat(rtl_get_, f) (rtlreg_t* dest) { \
    *dest=cpu.f;/*TODO();*/ \
  }
```
#### 完善rtl_update_ZFSF
- rtl_update_ZF
```C
static inline void rtl_update_ZF(const rtlreg_t* result, int width) {
  // eflags.ZF <- is_zero(result[width * 8 - 1 .. 0])
  //TODO();
  t0=(*result&(~0u>>((4-width)<<3)))==0;
  rtl_set_ZF(&t0);
}
```
第一句根据输入参数width的大小，1、2、3、4分别会返回对应地址的 8、16、24、32位情况，然后判断其是否为0。*这个方法在扫描内存的时候我们碰到过。*  
第二句利用刚完善的rtl_set来设置ZF位  
- rtl_update_SF
```C
static inline void rtl_update_SF(const rtlreg_t* result, int width) {
  // eflags.SF <- is_sign(result[width * 8 - 1 .. 0])
  //TODO();
  rtl_msb(&t0,result,width);
  rtl_set_SF(&t0);
}
```
第一句利用rtl_msb()函数获取最高位:
```C
static inline void rtl_msb(rtlreg_t* dest, const rtlreg_t* src1, int width) {
  // dest <- src1[width * 8 - 1]
  //TODO();
  rtl_shri(dest,src1,width*8-1);
}
```
第二句利用刚完善的rtl_set来设置SF位

#### 测试
最后在all-instr.h中填入make_EHelper(sub);  
然后前往nemu进行dummy的测试，可以通过

### XOR
#### 填表与译码
在附录A中，31为XOR Ev,Gv  
其中E和v前面已经提到过了，而G代表modR/M字节选择了一个通用寄存器  
因此在opcode_table中的0x31位置，填入IDEX(G2E,xor)。表明指令译码为通用寄存器到通用寄存器或内存地址。31 c0中的c0为1100 0000，中间三位为000，因此代表寄存器eax，结合read_ModR_M函数，可得到id_dest为eax。而mod为11，则表明R/M字段也是寄存器，为000，因此代表寄存器eax，结合read_ModR_M函数，可得到id_src也为eax。  
#### 执行
xor异或为逻辑运算，需要在logic.c中实现。  
i386手册的P410对xor指令进行了解释，其操作为:
```
DEST <- LeftSrc Xor RightSrc
CF <- 0
OF <- 0
```
根据此操作解释进行代码填写，主要利用rtl_xor进行计算  
```C
make_EHelper(xor) {
  //TODO();
  rtl_xor(&t2, &id_dest->val, &id_src->val);//利用rtl基本操作进行运算
  operand_write(id_dest, &t2);//完成计算，写入结果
  //更新各个标志位
  rtl_update_ZFSF(&t2, id_dest->width);
  rtl_set_OF(&tzero);
  rtl_set_CF(&tzero);
  print_asm_template2(xor);
}
```

### POP
pop指令的实现与push较为相似，因为这两者是一个反操作，前者是出栈到寄存器，后者是寄存器进栈  
查附录A的5d，为POP(into general register) eBP  
#### 译码
在opcode_table中的58~5f处填入IDEX(r,pop)，关于译码函数已在push中解释，不赘述。

#### 执行
i386手册的P361页对POP 58+rw/rd指令进行了解释：Pop top of stack into word/dword register。  
主要就是完善rtl_pop指令:
```C
static inline void rtl_pop(rtlreg_t* dest) {
  // dest <- M[esp]
  // esp <- esp + 4
  //TODO();
  rtl_sm(&cpu.esp,4,dest);//利用rtl_sm(store memory)，在esp处存入长度为4的src1
  rtl_addi(&cpu.esp,&cpu.esp,4);
}
```

**注意！！！！**  
我在make_EHelper(pop)中一开始写的是这么一句话rtl_pop(&id_dest->val);  
但是在diff-test中没有通过。  
原因在于我没有真正理解pop的含义！而是想当然了！  
因为我在make_EHelper(push)中写的是rtl_push(&id_dest->val);这就是把val值push到栈顶  然而在push中之所以能这么做，是因为在make_DopHelper(r)中已经将对应寄存器的值读取到了val中，可是在pop中，我是需要将栈顶的值读取到对应的寄存器中，而不是读取到val中，这个对应的寄存器由操作码的末三位决定。  
因此这里的正确写法应该是先取出值到临时寄存器，再利用operand_write将值写入到目标的寄存器中:  
```C
/* 调用rtl_pop()取出一个32位值到临时寄存器 */
  rtl_pop(&t2);
  /* 使用operand_write()将取出的临时值写入目标寄存器 */
  operand_write(id_dest,&t2);
```

### ret
查阅i386手册附录A的c3:RET near  
指令的相关介绍位于手册的P378页:RET Return from Procedure，可以知道ret就是起到函数过程回归的作用  
其中关于Opcode C3的Description为:Return (near) to caller  
ret指令不需要译码，因此可以直接在opcode_table中填入EX(ret)  
在control.c中来实现make_EHelper(ret)  
```C
make_EHelper(ret) {
  //TODO();
  rtl_pop(&decoding.jmp_eip);//取出栈顶保存的eip值，然后将其设置为跳转的eip值
  decoding.is_jmp=1;//指令跳转为真
  print_asm("ret");
}
```
其中rtl_pop指令已经在前面实现

至此，PA2第一阶段结束
