#include "FLOAT.h"
#include <stdint.h>
#include <assert.h>

FLOAT F_mul_F(FLOAT a, FLOAT b) {
  //相乘后，需要扩大位数为64位无浮点数
  uint64_t result=(uint64_t)a*(uint64_t)b;
  //在FLOAT类型下，还需要将结果除以2^16
  result=result>>16;
  return result;
}

FLOAT F_div_F(FLOAT a, FLOAT b) {
  //先获取符号位
  int sign = (a ^ b) & 0x80000000;
  //若ab为负数，则取正数部分
  if(a<0){
    a=-a;
  }
  if(b<-){
    b=-b;
  }
  //获取整数和余数
  int round = a / b,remain = a % b;
  //在FLOAT类型下，还需要将结果乘以2^16
  round<<16;
  remain<<16;
  round+=remain / b;
  //结果考虑符号位
  int result=round;
  if(sign){result=-result;}
  return result;
}

//float to FLOAT
FLOAT f2F(float a) {
  /* You should figure out how to convert `a' into FLOAT without
   * introducing x87 floating point instructions. Else you can
   * not run this code in NEMU before implementing x87 floating
   * point instructions, which is contrary to our expectation.
   *
   * Hint: The bit representation of `a' is already on the
   * stack. How do you retrieve it to another variable without
   * performing arithmetic operations on it directly?
   */
  //类型转换
	uint32_t* f = (uint32_t*)&a;
  //符号
	uint32_t sign = (*f) & 0x80000000;
	//移码
  uint32_t transcoding = (*f) & 0x7f000000;
  //小数部分
	uint32_t decimal = ((*f) & 0x7fffff) | 0x800000;
	//移码减127作为指数
  int exp = transcoding - 127;

	uint32_t res;
	
	if(exp > 7 && exp < 15){
		res = decimal << (exp - 7);
	}
	else if(exp < 7 && exp > -17){
		res = decimal >> (7 - exp);
	}
	else{
		assert(0);
	}
	return (sign) ? -res : res;
}

//取绝对值
FLOAT Fabs(FLOAT a) {
  return a < 0 ? -a : a; 
}

/* Functions below are already implemented */

FLOAT Fsqrt(FLOAT x) {
  FLOAT dt, t = int2F(2);

  do {
    dt = F_div_int((F_div_F(x, t) - t), 2);
    t += dt;
  } while(Fabs(dt) > f2F(1e-4));

  return t;
}

FLOAT Fpow(FLOAT x, FLOAT y) {
  /* we only compute x^0.333 */
  FLOAT t2, dt, t = int2F(2);

  do {
    t2 = F_mul_F(t, t);
    dt = (F_div_F(x, t2) - t) / 3;
    t += dt;
  } while(Fabs(dt) > f2F(1e-4));

  return t;
}
