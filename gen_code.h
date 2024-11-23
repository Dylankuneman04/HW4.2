#ifndef _GEN_CODE_H
#define _GEN_CODE_H
#include <stdio.h>
#include "ast.h"
#include "bof.h"
#include "instruction.h"
#include "code.h"

#define STACK_SPACE 4096

void gen_print_str(code_seq *seq, const char *str);
void gen_print_int(code_seq *seq, int num);
void gen_read_char(code_seq *seq);
void gen_print_char(code_seq *seq, char c);
void gen_push_number(code_seq *seq, int number);

#endif
