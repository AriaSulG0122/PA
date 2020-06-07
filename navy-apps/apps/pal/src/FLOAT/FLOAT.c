#include "FLOAT.h"
#include <stdint.h>
#include <assert.h>

FLOAT F_mul_F(FLOAT a, FLOAT b) {
  return ((uint64_t)a * (uint64_t)b) >> 16;
}

FLOAT F_div_F(FLOAT a, FLOAT b) {
  int sign = (a ^ b) & 0x80000000;
  a = a < 0 ? -a : a;
  b = b < 0 ? -b : b;
  int res = a / b;
  int c = a % b;

  for(int i=0; i < 16; i++){
	res <<= 1;
	c <<= 1;
	if(c >= b){
		c -= b;
		res += 1;
	}
  }
  return sign ? -res : res;
}

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

	uint32_t* f = (uint32_t*)&a;
	uint32_t sign = (*f >> 31) & 0x1;
	uint32_t uexp = (*f >> 23) & 0xff;
	uint32_t frac = (*f & 0x7fffff) | (1 << 23);
	int exp = uexp - 127;

	uint32_t res;
	
	if(exp > 7 && exp < 15){
		res = frac << (exp - 7);
	}
	else if(exp < 7 && exp > -17){
		res = frac >> (7 - exp);
	}
	else{
		assert(0);
	}
	return (sign) ? -res : res;
}

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
