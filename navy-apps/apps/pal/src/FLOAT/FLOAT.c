#include "FLOAT.h"
#include <stdint.h>
#include <assert.h>

FLOAT F_mul_F(FLOAT a, FLOAT b) {
  //��˺���Ҫ����λ��Ϊ64λ�޸�����
  uint64_t result=(uint64_t)a*(uint64_t)b;
  //��FLOAT�����£�����Ҫ���������2^16
  result=result>>16;
  return result;
}

FLOAT F_div_F(FLOAT a, FLOAT b) {
  //�Ȼ�ȡ����λ
  int sign = (a ^ b) & 0x80000000;
  //��abΪ��������ȡ��������
  if(a<0){
    a=-a;
  }
  if(b<-){
    b=-b;
  }
  //��ȡ����������
  int round = a / b,remain = a % b;
  //��FLOAT�����£�����Ҫ���������2^16
  round<<16;
  remain<<16;
  round+=remain / b;
  //������Ƿ���λ
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
  //����ת��
	uint32_t* f = (uint32_t*)&a;
  //����
	uint32_t sign = (*f) & 0x80000000;
	//����
  uint32_t transcoding = (*f) & 0x7f000000;
  //С������
	uint32_t decimal = ((*f) & 0x7fffff) | 0x800000;
	//�����127��Ϊָ��
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

//ȡ����ֵ
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
