指令的执行流程： 取指IF、译码ID、执行EX、更新EIP  
RISC的宗旨就是简单，指令少，指令长度固定，指令格式统一

## 1. 关于环境设置

make ARCH=x86-nemu ALL=dummy run  
Makefile:1:  /Makefile.check: No such file or directory.  
你可以先试着添加环境变量：  
export AM_HOME=/home/your name/ics2017/nexus-am  
然后你再跑一次，发现会出现：  
make ARCH=x86-nemu ALL=dummy run  
Building am [x86-nemu]  
make[2] *** No targets specified and no makefile found  
...  
如果遇到，检查~/.bashrc目录，是否存在  
export NEMU_HOME=/home/your name/ics2017/nemu  
export AM_HOME=/home/your name/ics2017/nexus-am  
export NAVY_HOME=/home/your name/ics2017/navy-apps  
三个变量 这个问题出现的原因可能是修改了环境（我是把 bash 改成了 zsh）。所以尽量在 PA0 之后就不要再修改环境了。


## 3. 关于pop理解
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

## 4. 关于符号拓展
and（83）涉及到符号拓展。我一开始一直报错，却不知道哪里出了问题。  
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
## 关于测试输出
在测试样例时，我碰到很奇怪的一点，就是si [N]时，如果N<10，则会把每条指令都输出来，如果N>=10，则仅仅是执行这些指令，
并不会输出每条指令。  
于是我前往cmd_si查看为什么会这样，其关键在void cpu_exec(uint64_t n)函数中:
```C
 bool print_flag = n < MAX_INSTR_TO_PRINT;
```
这里的print_flag代表输出标志，而其为真当且仅当n<MAX_INSTR_TO_PRINT  
而MAX_INSTR_TO_PRINT的值定义如下：
```C
#define MAX_INSTR_TO_PRINT 10
```
这就解释了我的疑问。是我在PA1中没有注意到的一个点。  

## 关于临时变量重用
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

## ROL（WITH Problem and Thought）
invalid opcode (eip = 0x001005b2): 83 f0 01 c1 e0 04 29 74 ...  
属于grp2，中间三位为000，查Group表发现是ROL，该指令在手册第372页有介绍:  
Rotate r/m word/dword left CL times  
也就是左移一定的位数  
我打算去实现该指令时，发现原框架中并没有rol指令的实现，因此需要自己进行添加  
一开始我实现如下:  
```C
//原框架中没有该指令，自己添加
make_EHelper(rol) {
  rtl_shl(&t0,&id_dest->val,&id_src->val);//左移n
  rtl_shri(&t1,&id_dest->val,id_dest->width*8-id_src->val);//对于最左边的位，左移n相当于右移width-n
  rtl_or(&t0,&t1,&t0);
  operand_write(id_dest, &t0);
}
```
但是运行microbech显示Failed(PIC)  
我认为实现左移这个步骤没有问题，应该是更新标志位的时候出了点问题，于是照着手册P372的Operation进行标志位的更新:  
```C
make_EHelper(rol) {
  rtl_shl(&t0,&id_dest->val,&id_src->val);//左移n
  rtl_shri(&t1,&id_dest->val,id_dest->width*8-id_src->val);//对于最左边的位，左移n相当于右移width-n
  rtl_or(&t0,&t1,&t0);
  operand_write(id_dest, &t0);

  //注意，下面的部分按照手册P372来
  rtl_get_CF(&t2);//得到CF位
  t0 = id_src->val; //count
  if(t0 == 1)
  {
      rtl_msb(&t1,&id_dest->val,id_dest->width);//Get high-order bit of r/m
      if(t2!=t1)
      {
          rtl_set_OF(&t0);
      }
      else{
          rtl_set_OF(&tzero);
      }
  }
  print_asm_template2(rol);
}
```
这次，成功通过测试。这一次错误再次说明了**RTFM**的重要性，否则很可能会出现问题。  
填完该指令，我有了一个想法，就是我们完全能够靠自己来实现一个ISA指令集，这个指令集可以不和x86体系结构一样，而完全可以由自己设计。    
但是改变了指令集，机器语言就需要改变，如果需要测试正确性，就不能用NEMU中的那些测试程序了。  
因此说，如果要自己完全设计并实现一个新的指令集，这个工程量太浩繁了。在设计、完善、高效、安全的路上，需要大量精力投入。
而测试指令集正确性，也是比较复杂的。我们这里参照QEMU能够方便地进行比较。  
但是对于自己设计的一个全新的指令集，可不能和QEMU进行比较，即无法diff-test，或许只能通过程序运行结果来判断指令实现是否正确。  
如果结果出现，找Bug就成了问题，没有diff-test，就需要查看所有寄存器甚至整个内存空间的变化情况，太麻烦了。  
因此，要设计新的指令集，在设置测试样本与Debug上也是十分困难的。  
想到这里，感觉NEMU就没有那么恐怖了，相反，前人给我们已经铺好了路，我们走着前人铺成的路。  
如果要从另一条路开辟一个新世界，那便任重而道远...  