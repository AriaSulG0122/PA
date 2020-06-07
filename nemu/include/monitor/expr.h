#ifndef __EXPR_H__
#define __EXPR_H__

#include "common.h"

uint32_t expr(char *, bool *);
void test_lexical();
bool check_parentheses(int p, int q);
int eval(int p, int q, bool *success);
void test_check_parentheses();
void test_expr();
uint32_t find_dominant(int p, int q);

#endif
