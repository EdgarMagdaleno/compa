#ifndef _TOKEN_H_
#define _TOKEN_H_

#include <stdio.h>
#include "ht.h"

#define RDCL	1
#define NDCL	2
#define EOT		3

typedef enum {
	tk_char, tk_int,  tk_dobl, tk_str,  tk_vchr, tk_vint, tk_vdbl, tk_vstr,
	tk_else, tk_if,   tk_whle, tk_eos,  tk_add,  tk_sub,  tk_mul,  tk_div,
	tk_mod,  tk_eq,   tk_neq,  tk_grt,  tk_grte, tk_les,  tk_lese, tk_id,
	tk_lchr, tk_lint, tk_ldbl, tk_lstr, tk_rpar, tk_lbrc, tk_rbrc, tk_coma,
	tk_asg,  tk_lpar, tk_lbrk, tk_rbrk, tk_prnt, tk_type
} token_type;

struct token {
	token_type type;
	int line, col;
	long line_offset;
	union {
		int i;
		double d;
		char *s;
	};
	struct token *next;
};

char *get_type_name(token_type type);
void print_token(struct token *tok);
void print_tokens(struct token *tokens);
int is_vector(int type);

#endif
