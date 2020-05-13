## Diffrential Testing
Differential Testing用于测试指令的实现是否正确，主要通过对比NEMU和真机的状态，如果状态不一致，表明可能存在问题。

这里选取了QEMU作为“真机”，因为它是一个虚拟出来的完整的x86计算机系统。

为了通过diffrential testing的方法来测试NEMU实现的正确性，只要让NEMU和QEMU逐条指令地执行同一个客户程序，双方每执行完一条指令，就检查两边的寄存器状态是否一致，如果不一致，则报错并终止程序运行。

其实现步骤如下：
1. 在include/common.h中定义宏#deinfe DIFF_TEST。如果重新编译NEMU，则会出现Connect to QEMU successfully的提示信息
2. 在monitor/diff-test/diff-test.c中，补充函数void difftest_step(uint32_t eip):
   ```C
   void difftest_step(uint32_t eip){
      ...
      // TODO: Check the registers state with QEMU.
      // Set `diff` as `true` if they are not the same.
      //TODO();
      if(r.eip!=cpu.eip){
        diff=true;
        printf("Different EIP!QEMU:0x%08x  NEMU:0x%08x\n",r.eip,cpu.eip);
      }
      if(r.eax!=cpu.eax){
        diff=true;
        printf("Different EAX!QEMU:0x%08x  NEMU:0x%08x\n",r.eax,cpu.eax);
      }
      if(r.ecx!=cpu.ecx){
        diff=true;
        printf("Different ECX!QEMU:0x%08x  NEMU:0x%08x\n",r.ecx,cpu.ecx);
      }
      if(r.edx!=cpu.edx){
        diff=true;
        printf("Different EDX!QEMU:0x%08x  NEMU:0x%08x\n",r.edx,cpu.edx);
      }
      if(r.ebx!=cpu.ebx){
        diff=true;
        printf("Different EBX!QEMU:0x%08x  NEMU:0x%08x\n",r.ebx,cpu.ebx);
      }
      if(r.esp!=cpu.esp){
        diff=true;
        printf("Different ESP!QEMU:0x%08x  NEMU:0x%08x\n",r.esp,cpu.esp);
      }
      if(r.ebp!=cpu.ebp){
        diff=true;
        printf("Different EBP!QEMU:0x%08x  NEMU:0x%08x\n",r.ebp,cpu.ebp);
      }
      if(r.esi!=cpu.esi){
        diff=true;
        printf("Different ESI!QEMU:0x%08x  NEMU:0x%08x\n",r.esi,cpu.esi);
      }
      if(r.edi!=cpu.edi){
        diff=true;
        printf("Different EDI!QEMU:0x%08x  NEMU:0x%08x\n",r.edi,cpu.edi);
      }

      //如果检测到diff标志为true，就停止客户程序的运行
      if (diff) {
        printf("EFLAGS:  QEMU:0x%08x  NEMU:0x%08x\n",r.eflags,cpu.eflags);
        nemu_state = NEMU_END;
      }
    }
   ```
3. 测试diff-test是否起效。

## 指令补充实现
在nexus-am\Makefile.check中，将一句ARCH ?= native改为ARCH ?= x86-nemu，以实现将AM项目上的程序默认编译到x86-nemu的AM中。  
以后通过make ALL=xxx run就能进行测试样例。
### add-longlong.c
第一次运行后，可以在nexus-am/tests/cputest/build中找到其反汇编文件，用于辅助查看未完善的指令。
#### LEA
invalid opcode(eip = 0x0010005c): 8d 4c 24 04 83 e4 f0 ff ...  
i386手册的Opcode Map指示LEA:Gv,M，即M2G  
查阅i386手册第327页。  
LEA--Load Effective Address：Store effective address for m in register。  
LEA用于取有效地址，LEA r16,m 代表 r16<--Addr(m)  
在nemu\src\cpu\exec\data-mov.c中已经有函数make_EHelper(lea)了。  
在opcode_table对应位置填上，完善all-instr.h就行。  
#### AND
invalid opcode(eip = 0x00100060): 83 e4 f0 ff 71 fc 55 89 ...  
这条指令也属于grp1，0xe4=0b11100100，中间三位为100  
查阅i386手册的第416页的Group表，第1行的100为AND，因此这是个AND指令。  
查阅i386手册第262页。  
opcode为83的and代表：AND sign-extended immediate byte with r/m word/dword  
在group1中的对应位置填上
**注意!!!!!!**  
这里涉及到符号拓展。我一开始一直报错，却不知道哪里出了问题。  
后来对照错误的指令和反汇编文件的命令，发现我输出的是andl $0xf0,%esp  
而反汇编文件中的是and    $0xfffffff0,%esp  
显然是符号拓展出了问题，其实i386手册已经提醒我了:  
AND **sign-extended** immediate byte...   
为此，需要先完善符号拓展的rtl函数:  
```C
static inline void rtl_sext(rtlreg_t* dest, const rtlreg_t* src1, int width) {
  // dest <- signext(src1[(width * 8 - 1) .. 0])
  //TODO();
  rtl_li(&t2,32-width*8);//t2为多余位
  rtl_shl(dest,src1,&t2);//dest为src1左移t2位
  rtl_sar(dest,dest,&t2);//dest为dest右移t2位
}
```
其中，rtl_li为立即数加载，rtl_shl为左移，rtl_sar为**带符号**右移   
```C
#define c_shr(a, b) ((a) >> (b))
#define c_sar(a, b) ((int32_t)(a) >> (b))
```
这里尤其要注意是带符号右移，因此在右移时继续了强制的有符号的类型转换，这样才能起到符号拓展填充首位符号的作用，否则只会填充零，我用简单的C++样例进行测试:
```C
#include <iostream>
using namespace std;
int main()
{
	uint32_t a = 0xf1;
	a = a << 24;
	a = a >> 24;
	printf("%08x\n", a);
	int32_t b = 0xf1;
	b = b << 24;
	b = b >> 24;
	printf("%08x", b);
}
```
输出结果为:  
000000f1  
fffffff1  

然后我在nemu\src\cpu\decode\decode.c中的static inline make_DopHelper(SI)函数中一开始写的是
```C
op->simm=instr_fetch(eip,op->width);  
```
需要改为
```C
  t0 = instr_fetch(eip,op->width);
  rtl_sext(&t0,&t0,op->width);//进行符号拓展
  op->simm = t0;
```

#### push
invalid opcode(eip = 0x00100063): ff 71 fc 55 89 e5 56 53 ...
查Opcode Map，其属于Grp5。  
0x71=0b0111 0001，中间三位为110，查看Group表，为PUSH Ev。  
在group5的对应位置填上EX(push)，编译通过。  

#### xchg与nop
查Opecode Map，66位置为Operand Size，没看出啥信息。  
于是查看反汇编文件:xchg   %ax,%ax，可知为xchg指令  
查看i386手册P409页，描述如下:  
Exchange Register/Memory with Register  
也就是寄存器和寄存器之间交换或者内存和寄存器之间交换。  
根据汇编指令，这里是ax和ax交换，等于啥也没干。相当于nop指令。  
而nop指令的opcode为90，框架内已经完善，只要填写opcode_table和all-instr.h就行  
查看opcode_table的66的位置，发现已经填上了EX(operand_size)  
在prefix.c中定义如下:
```C
make_EHelper(operand_size) {
  decoding.is_operand_size_16 = true;
  exec_real(eip);
  decoding.is_operand_size_16 = false;
}
```
即先定义操作数的长度为16，**也就是但凡出现了66，就代表操作数长度为16，然后接着去读取执行下一条指令**，这里为66后面为90，也就是下一条指令为nop。  
- **深入理解**：为什么这里的90就代表nop呢？  
  在i386手册P408页讲述了XCHG，其中当opcode为90+r时,代表:  
  Exchange word register with AX  
  也就是90,91...分别代表AX、BX...和AX进行交换。  
  如果是91或者往后，则这种交换是有意义的  
  但是在90，代表AX与AX交换，这是没有意义的，在i386中就把这种情况作为一个nop来处理了。  

#### ADD
invalid opcode(eip = 0x00100087): 03 83 e0 02 10 00 13 93 ...  
查阅Opcode Map，03为ADD Gv,Ev  
在i386手册的第261页讲述了ADD 03:Add r/m word/dword to word/dword register  
和实现sub类似，可以参考adc的实现，adc代表Add with carry，也就是多考虑了carry flag位CF。  
因此复制make_EHelper(adc)并删除CF相关的两行即可  
```C
make_EHelper(add) {
  //TODO();
  rtl_add(&t2, &id_dest->val, &id_src->val);
  rtl_sltu(&t3, &t2, &id_dest->val);
  operand_write(id_dest, &t2);

  rtl_update_ZFSF(&t2, id_dest->width);

  rtl_sltu(&t0, &t2, &id_dest->val);
  rtl_or(&t0, &t3, &t0);
  rtl_set_CF(&t0);

  rtl_xor(&t0, &id_dest->val, &id_src->val);
  rtl_not(&t0);
  rtl_xor(&t1, &id_dest->val, &t2);
  rtl_and(&t0, &t0, &t1);
  rtl_msb(&t0, &t0, id_dest->width);
  rtl_set_OF(&t0);

  print_asm_template2(add);
}
```
运行，报错:  
please implement me
nemu: ./include/cpu/rtl.h:145: rtl_not: Assertion `0' failed.
因此还需要完善rtl指令not:
```C
//非
static inline void rtl_not(rtlreg_t* dest) {
  // dest <- ~dest
  //TODO();
  *dest = ~(*dest);
}
```


#### ADC
invalid opcode(eip = 0x0010008d): 13 93 e4 02 10 00 8b 8c ...  
查Opcode Map，为ADC Gv,Ev  
已经实现了ADC内容，直接填opcode_table和all-instr.h就行  

#### Or
invalid opcode(eip = 0x001000a5): 09 c1 0f 94 c0 0f b6 c0 ...
查Opcode Map，为Or Ev,Gv  
主要就是完善make_EHelper(or):  
```C
make_EHelper(or) {
  //TODO();
  rtl_or(&t2, &id_dest->val, &id_src->val);//利用rtl基本操作进行运算
  operand_write(id_dest, &t2);//完成计算，写入结果
  //更新各个标志位
  rtl_update_ZFSF(&t2, id_dest->width);
  rtl_set_OF(&tzero);
  rtl_set_CF(&tzero);
  print_asm_template2(or);
}
```

#### 2byte_esc与SETcc
invalid opcode(eip = 0x001000a7): 0f 94 c0 0f b6 c0 50 e8 ...  
of为2byte_esc，为两字节转义码，已经实现:
```C
static make_EHelper(2byte_esc) {
  uint32_t opcode = instr_fetch(eip, 1) | 0x100;//用两个字节来确定指令格式！
  decoding.opcode = opcode;
  set_width(opcode_table[opcode].width);
  idex(eip, &opcode_table[opcode]);
}
```
根据反汇编文件或者i386手册第415页的第二张Opcode Map，这里的下一个指令为SETZ，在i386手册的389页有介绍:  
sete 94 Set byte if equal  
框架已经实现了make_EHelper(setcc)，还需要实现位于src\cpu\exec\cc.c中的rtl_setcc:
```C
void rtl_setcc(rtlreg_t* dest, uint8_t subcode) {
  bool invert = subcode & 0x1;
  enum {
    CC_O, CC_NO, CC_B,  CC_NB,
    CC_E, CC_NE, CC_BE, CC_NBE,
    CC_S, CC_NS, CC_P,  CC_NP,
    CC_L, CC_NL, CC_LE, CC_NLE
  };

  // TODO: Query EFLAGS to determine whether the condition code is satisfied.
  // dest <- ( cc is satisfied ? 1 : 0)
  switch (subcode & 0xe) {
    case CC_O:  *dest = cpu.OF == 1 ? 1 : 0;break;
    case CC_NO: *dest = cpu.OF == 0 ? 1 : 0;break;
    case CC_B:  *dest = cpu.CF == 1 ? 1 : 0;break;
    case CC_NB: *dest = cpu.CF == 0 ? 1 : 0;break;
    case CC_E:  *dest = cpu.ZF == 1 ? 1 : 0;break;
    case CC_NE: *dest = cpu.ZF == 0 ? 1 : 0;break;
    case CC_BE: *dest = (cpu.CF == 1)||(cpu.ZF == 1) ? 1 : 0;break;
    case CC_NBE:*dest = (cpu.CF == 0)&&(cpu.ZF == 0) ? 1 : 0;break;
    case CC_S:  *dest = cpu.SF == 1 ? 1 : 0;break;
    case CC_NS: *dest = cpu.SF == 0 ? 1 : 0;break;
    //因为没有创建PF，这里不用完善
    //case CC_P:  
    //case CC_NP:
    case CC_L:  *dest = cpu.SF != cpu.OF ? 1 : 0;break;
    case CC_NL: *dest = cpu.SF == cpu.OF ? 1 : 0;break;
    case CC_LE: *dest = (cpu.SF != cpu.OF)||(cpu.ZF == 1) ? 1 : 0;break;  
    case CC_NLE:*dest = (cpu.SF == cpu.OF)&&(cpu.ZF == 0) ? 1 : 0;break;  
      //TODO();
    default: panic("should not reach here");
    case CC_P: panic("n86 does not have PF");
  }

  if (invert) {
    rtl_xori(dest, dest, 0x1);
  }
}
```
这里的实现主要就是参考i386手册的389页，里面囊括解释了这里出现的每一种case。

#### MOVZX
invalid opcode(eip = 0x001000aa): 0f b6 c0 50 e8 79 ff ff ...  
查i386手册Opcode Map2，得到这里为MOVZX Gv,Eb  
在i386手册P351页解释了MOVZX:Move with Zero-Extend  
这个指令已经实现，填表即可

#### TEST 
invalid opcode(eip = 0x00100032): 85 c0 74 02 5d c3 c7 45 ...  
查表，该指令为TEST Ev,Gv  
在i386手册第405页，TEST:Logical Compare  
其中TEST 85:AND word/dword register with r/m word/dword  
其操作为：
```
DEST := LeftSRC AND RightSRC;
CF <- 0;
OF <- 0;
```
完善make_EHelper(test) :
```C
make_EHelper(test) {
  //TODO();
  rtl_and(&t2, &id_dest->val, &id_src->val);//利用rtl基本操作进行运算
  //更新各个标志位
  rtl_update_ZFSF(&t2, id_dest->width);
  rtl_set_OF(&tzero);
  rtl_set_CF(&tzero);
  print_asm_template2(test);
}
```

#### JCC
invalid opcode(eip = 0x00100034): 74 02 5d c3 c7 45 08 01 ...  
查表，为Short displacement jump of condition(Jb)  
在i386的第316页讲述了jcc的各种情况  
make_EHelper(jcc)将根据rtl_setcc(&t2, subcode)的情况来判断是否需要跳跃。  

#### add
invalid opcode(eip = 0x001000b3): 83 c3 08 83 c4 10 83 fb ...  
属于Grp1，0xc3=0b 1100 0011，中间三位为000  
查Group表，为add，填入grp1  

#### cmp
invalid opcode(eip = 0x001000b9): 83 fb 40 75 ba 83 c6 08 ...  
0xfb=0b 1111 1011，中间三位为111  
查Group表，为cmp，填入grp1  
在i386手册的第287页描述了CMP:Compare Two Operand  
Operation: LeftSRC-SignExtend(RightSRC)  
CMP does not store a result; its purpose is to set the flags  
完善make_EHelper(cmp)   
```C
make_EHelper(cmp) {
  //TODO();
  rtl_sub(&t2, &id_dest->val, &id_src->val);
  rtl_sltu(&t3, &id_dest->val, &t2);
  
  rtl_update_ZFSF(&t2, id_dest->width);//更新ZFSF
  rtl_set_CF(&t3); //设置CF
  //设置OF位
  rtl_xor(&t0, &id_dest->val, &id_src->val);
  rtl_xor(&t1, &id_dest->val, &t2);
  rtl_and(&t0, &t0, &t1);
  rtl_msb(&t0, &t0, id_dest->width);
  rtl_set_OF(&t0);
  print_asm_template2(cmp);
}
```

至此，add-longlong已经HIT GOOD TRAP了。 

### add.c

#### cmp
invalid opcode(eip = 0x00100083): 3b 84 b3 e0 00 10 00 0f ...  
查表发现为cmp，前面已实现，填opcode_table就行了。  

#### push
invalid opcode(eip = 0x001000a4): 6a 01 e8 81 ff ff ff 83 ...  
查表，发现为Push。在P367页，描述了6A:PUSH imm8，即推入8位立即数  
因此在opcode_table写入IDEXW(push_SI,push,1)即可  

### bit.c
#### SAR
invalid opcode(eip = 0x00100050): c1 fa 03 83 e1 07 b8 01 ...  
属于Grp2，0xfa=0b 1111 1010，中间3位为111  
查Group表，该位置为SAR，在grp2相应位置填上EX(sar)。  
然后实现make_EHelper(sar):  
```C
make_EHelper(sar) {
  //TODO();
  // unnecessary to update CF and OF in NEMU
  rtl_sar(&t2, &id_dest->val, &id_src->val);//利用rtl基本操作进行运算
  operand_write(id_dest, &t2);//完成计算，写入结果
  //更新ZF和SF
  rtl_update_ZFSF(&t2, id_dest->width);
  print_asm_template2(sar);
}
```

#### SHL
invalid opcode(eip = 0x0010005b): d3 e0 8b 4d 08 84 04 11 ...
查表d3，属于Grp2，0xe0=0b 1110 0000，中间3位为100，代表SHL  
在grp2相应位置填上EX(shl)  
然后实现make_EHelper(shl):  
```
make_EHelper(shl) {
  //TODO();
  // unnecessary to update CF and OF in NEMU
  rtl_shl(&t2, &id_dest->val, &id_src->val);//利用rtl基本操作进行运算
  operand_write(id_dest, &t2);//完成计算，写入结果
  //更新ZF和SF
  rtl_update_ZFSF(&t2, id_dest->width);
  print_asm_template2(shl);
}
```
#### DEC
invalid opcode(eip = 0x001000f2): fe c8 0f 94 c0 0f b6 c0 ...  
属于Grp 4，0xc8=0b 1100 1000，中间三位为001  
查Group表，为DEC Eb  
主要是完善:
```C
make_EHelper(dec) {
  //TODO();
  rtl_subi(&t2, &id_dest->val, 1);
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
  print_asm_template1(dec);
}
```

#### NOT
invalid opcode(eip = 0x001000a9): f7 d0 22 03 88 02 83 c4 ...   
属于Grp3，0xd0=0b 1101 0000，中间三位为010  
查Group表，代表NOT  
主要是完善make_EHelper(not):
```C
make_EHelper(not) {
  //TODO();
  rtl_mv(&t2,&id_dest->val);
  rtl_not(&t2);
  operand_write(id_dest,&t2);
  print_asm_template1(not);
}
```

### bubble-sort.c
#### INC
invalid opcode(eip = 0x001000af): 40 8b 0c 85 80 01 10 00 ...  
查表，为INC general register  
在opcode_table填入IDEX(r,inc)  
然后完善  
```C
make_EHelper(inc) {
  //TODO();
  rtl_addi(&t2, &id_dest->val, 1);
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
  print_asm_template1(inc);
}
```

### fact.c
#### JMP
invalid opcode(eip = 0x001000cc): eb ae 00 00 00 00 00 00 ...  
查表，为JNP Jb  
在opcode_table填入IDEXW(J,jmp,1)即可，jmp指令已经实现了  

#### IMUL
invalid opcode(eip = 0x001000b0): 0f af d0 48 83 f8  
查阅Opcode Map2，为IMUL Gv,EV  
注意，在arith.c中，有imul with one operand的imul1和imul with two operands的imul2  
这条指令有两个操作数，因此在opcode_table写上IDEX(E2G,imul2)  

### goldbach.c
#### TEST 
invalid opcode(eip = 0x001000d0): f7 c6 01 00 00 00 74 20 ...
属于Grp3,0xc6=0b 1100 0110，中间三位为000，因此为TEST Ib/Iv  
由于test已经实现，在grp3中填入对应的操作即可  

#### CWD
invalid opcode(eip = 0x001000e2): 99 f7 f9 85 d2 74 0f 41 ...  
查阅i386手册，为CWD  
在手册P290页，进行了解释:  
CWD/CDQ: Convert Word to Doubleword / Conver Doubleword to Quadword  
实验指导书上也说了，这个指令就是cldt  
按照手册中的operation来完善make_EHelper(cltd):
```C
make_EHelper(cltd) {
  if (decoding.is_operand_size_16) {//CWD instruction，DX:AX(sign-extend of AX)
    //TODO();
    rtl_msb(&t0,&cpu.eax,2);//获取16位数的最高位，看看是否<0，即获取AX的最高位
    if(t0 == 1)cpu.edx = cpu.edx | 0xffff;//AX<0，则以AX的符号位拓展DX
    else cpu.edx = 0;
  }
  else {//CDQ instruction,EDX:EAX(sign-extend of EAX)
    //TODO();
    rtl_msb(&t0,&cpu.eax,4);
    if(t0 == 1)cpu.edx = cpu.edx | 0xffffffff;
    else cpu.edx = 0;
  }
  print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
}
```

#### IDIV
invalid opcode(eip = 0x001000e3): f7 f9 85 d2 74 0f 41 39 ...  
属于grp3，查group表，为IDIV AL/eAX  
由于IDIV已经实现，直接填表就行了  


### hello-str.c
#### MOVSX
invalid opcode(eip = 0x0010059a): 0f be 06 84 c0 74 1d 3c ...  
属于Opcode Map2，这个实现和MOVZX十分相似，略  
本以为可以略了这部分，没想到后来出现了一个Bug。  
由于make_EHelper(movsx)已经实现，我认为只需要填表就行，不会出什么问题。  
但是却没有通过diff-test，我考虑哪里代码出了问题  
检查了一会儿，发现是临时变量重用了，框架代码下:
```C
make_EHelper(movsx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  rtl_sext(&t2, &id_src->val, id_src->width);
  operand_write(id_dest, &t2);
  print_asm_template2(movsx);
}
```
而我实现的rtl_sext:
```C
//符号拓展
static inline void rtl_sext(rtlreg_t* dest, const rtlreg_t* src1, int width) {
  // dest <- signext(src1[(width * 8 - 1) .. 0])
  //TODO();
  rtl_li(&t2,32-width*8);//t2为多余位
  rtl_shl(dest,src1,&t2);//dest为src1左移t2位
  rtl_sar(dest,dest,&t2);//dest为dest右移t2位
}
```
我都用到了临时寄存器t2，这样就导致一个t2被两个函数混着用，显然会出错。  
因此，在这里将rtl_sext中的t2改成t3即可。  

#### LEAVE
invalid opcode(eip = 0x001007d5): c9 c3 90 55 89 e5 8b 55 ...  
查表，为LEAVE，在手册P329页，解释为:  
LEAVE--Hign Level Procedure Exit  
Set SP/ESP to BP/EBP,then pop BP/EBP  
主要需要完善make_EHelper(leave):
```C
make_EHelper(leave) {
  //TODO();
  rtl_mv(&cpu.esp,&cpu.ebp);//Set ESP to EBP
  rtl_pop(&cpu.ebp);//pop EBP
  print_asm("leave");
}
```

#### DIV
invalid opcode(eip = 0x001001e4): f7 75 0c 89 d6 50 e8 29 ...  
属于grp3，查group表，为DIV AL/eAX  
由于make_EHelper(div)已经实现，直接填表就行了  

### recursion.c
#### CALL_RM
invalid opcode(eip = 0x00100190): ff 15 f4 01 10 00 39 05 ...  
属于Grp5，查表得为CALL Ev，其相邻的为CALL eP。  
其反汇编指令为：  
100190:	ff 15 f4 01 10 00    	call   *0x1001f4  
在这里，我一开始填充的是call指令，但是没有通过diff-test。  
后来我思考，之前在dummy就完善过call指令，这里怎么不行？
dummy反汇编文件中有类似的一句话：  
100016:	e8 05 00 00 00        call   100020 \<main>  
其中main表示跳转到的地址。  
这么一对比，区别就很明显了，前者是调取了绝对的地址，而后者是调取了相对地址。  
我之前实现的call指令关键在于译码时的这一句:  
```C
  decoding.jmp_eip = id_dest->simm + *eip;//根据读取到的立即数值设置跳转地址
```
而这里的call_rm则会在译码时根据ModR/M字段来进行值读取，和前面的立即数偏移不同，最终该值会被记录在id_dest->val中  
因此完善make_EHelper(call_rm)
```C
make_EHelper(call_rm) {
  //TODO();
  decoding.is_jmp=1;//This is jump instruction
  rtl_push(eip);
  decoding.jmp_eip = id_dest->val;
  print_asm("call *%s", id_dest->str);
}
```
此外，jmp和jmp rm也有类似的区别。

### sub-longlong.c
#### SBB
invalid opcode(eip = 0x0010008d): 1b 93 e4 02 10 00 8b 8c ...  
查表，为SBB指令。在手册386页说明了该指令:Interger Subtraction with Borrow  
该指令已经在arith.c中实现，填表即可  

