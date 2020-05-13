## Volatile
### 编写文件
### 对比
### Why？


## 串口
主要需要实现in、out两个指令  
### in 指令
i386手册P302页，以及Opcode Map第E行  
按照手册说明补充opcode_table  
主要实现的helper函数位于nemu\src\cpu\exec\system.c:  
```C
make_EHelper(in) {//DEST<-[SRC](Reads from I/O address space),来自手册
  //TODO();
  t0=pio_read(id_src->val,id_dest->width);
  operand_write(id_dest,&t0);
  print_asm_template2(in);

#ifdef DIFF_TEST
  diff_test_skip_qemu();//跳过与QEMU的检查
#endif
}
```
### out 指令
i386手册P358页，以及Opcode Map第E行  
按照手册说明补充opcode_table  
主要实现的helper函数位于nemu\src\cpu\exec\system.c:  
```C
make_EHelper(out) {//[DEST]<-SRC(I/O address space used),来自手册
  //TODO();
  pio_write(id_dest->val,id_dest->width,id_src->val);
  print_asm_template2(out);

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}
```
### HAS_SERIAL
// Define this macro after serial has been implemented
#define HAS_SERIAL

## 时钟
### _uptime()
nexus-am\am\arch\x86-nemu\src\ioe.c  
### timetest
#### dhrystone 
mark 92
#### coremark
有很多指令还没有实现，需要继续填充opcode_map  
主要是填补指令的操作数类型与宽度引起的空缺  
##### CBW指令
这里需要再补充完成一个指令:  
invalid opcode (eip = 0x0010094e): 98 29 c3 89 d8 8d 65 f8 ...  
查表，为CBW，在手册第281页解释了该指令:Convert Byte to Word/Convert Word to Doubleword  
按照手册实现make_EHelper(cwtl):
```C
make_EHelper(cwtl) {//Convert Byte to Word/Convert Word to Doubleword
  if (decoding.is_operand_size_16) {//AX<-SignExtend(AL),Convert Byte to Word
    //TODO();
    rtl_sext(&t0,&cpu.eax,1);//进行符号拓展
    cpu.eax = (cpu.eax & 0xffff0000) | (t0 & 0xffff);
  }
  else {//EAX<-SignExtend(AX),Convert Word to Doubleword
    //TODO();
    rtl_sext(&t0,&cpu.eax,2);
    cpu.eax = t0;
  }

  print_asm(decoding.is_operand_size_16 ? "cbtw" : "cwtl");
}
```
然后通过测试，mark 350

#### microbench
还需要填充指令:  
##### NEG
invalid opcode (eip = 0x001005b2): f7 d8 21 d8 29 c3 89 f9 ...
查阅i386手册，该指令属于Grp3，中间三位为011，查阅Group表，为NEG指令  
在手册第354页描述了该指令:Two's Complement Negation  
按照手册的Operation完善make_EHelper(neg):  
```C
make_EHelper(neg) {
  //TODO();
  if(id_dest->val==0){//IF r/m == 0
    rtl_set_CF(&tzero);//CF=0
  }
  else{
    rtl_addi(&t2,&tzero,1);
    rtl_set_CF(&t2);//CF=1
  }
  rtl_mv(&t2,&tzero);//t2=0
  rtl_sub(&t2,&tzero,&id_dest->val);//t2=0-r/m=-r/m
  operand_write(id_dest,&t2);
  //更新ZF与SF位
  rtl_update_ZFSF(&t2, id_dest->width);
  //设置OF位
  rtl_xor(&t0, &id_dest->val, &id_src->val);
  rtl_xor(&t1, &id_dest->val, &t2);
  rtl_and(&t0, &t0, &t1);
  rtl_msb(&t0, &t0, id_dest->width);
  rtl_set_OF(&t0);
  
  print_asm_template1(neg);
}
```

##### ROL（WITH Problem and Thought）
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

##### 测试

## 键盘
### _read_key()
```C
//返回按键的键盘码，若无按键，则返回_KEY_NONE
int _read_key() {
  uint32_t keyCode=_KEY_NONE;
  if(inb(0x64)){//状态寄存器生效，位于0x64端口
    keyCode=inl(0x60);//获取键盘码，位于0x60端口
  }
  return keyCode;
}
```
### 测试

## VGA
### paddr_read()与paddr_write()
```C
//***Get the last 8|16|24|32 bits of pmem_rw(addr,uint32_t)
uint32_t paddr_read(paddr_t addr, int len) {
  if(is_mmio(addr)==-1){//为-1，则不是内存映射I/O的访问
    return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
  }else{
    return mmio_read(addr,len,is_mmio(addr));//根据映射号访问内存映射I/O
  }
  //***(4-len)<<3 = (4-len)*2^3,     ~ = take inverse
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
  if(is_mmio(addr)==-1){
    memcpy(guest_to_host(addr), &data, len);
  }else{
    mmio_write(addr,len,data,is_mmio(addr));
  }
}
```
在测试运行时，发现is_mmio()等函数无法被调用。  
memory.c文件中包含了nemu.h，我查看nemu.h，并没有包含mmio.h  
因此还需要在nemu.h中加上一句: *#include "device/mmio.h"* 

### videotest
完善_draw_rect()函数:
```C
//用于将pixels指定的矩形像素绘制到屏幕中以(x,y)和(x+w,y+h)两点连线为对角线的矩形区域
void _draw_rect(const uint32_t *pixels, int x, int y, int w, int h) {
   int i;
   for(i=0;i<h;i++){//高度
       memcpy(fb+(y+i)*_screen.width+x, pixels+i*w, w*4);
   }
}
```
利用memcpy进行逐行绘制，从第y+i行的第x列开始，将从pixels+i*w位置开始的w*4长度的数据通过内存映射给fb，从而fb能实现在屏幕上显示对应像素数据。