#include <stdio.h>
#include "token.h"

char *get_type_name(token_type type) {
	return &"tk_char tk_int  tk_dobl tk_str  tk_vchr tk_vint tk_vdbl tk_vstr "
			"tk_else tk_if   tk_whle tk_eos  tk_add  tk_sub  tk_mul  tk_div  "
			"tk_mod  tk_eq   tk_neq  tk_grt  tk_grte tk_les  tk_lese tk_id   "
			"tk_lint tk_ldbl tk_lchr tk_lstr tk_rpar tk_lbrc tk_rbrc tk_coma "
			"tk_asg  tk_lpar tk_lbrk tk_rbrk tk_prnt"[type * 8];
}

void print_token(struct token *tok) {
	printf("%i %i %.8s", tok->line, tok->col, get_type_name(tok->type));
	
	switch (tok->type) {
		case tk_id: printf("%s", tok->s); break;
		case tk_lint: printf("%i" , tok->i); break;
		case tk_ldbl: printf("%lf", tok->d); break;
		case tk_lstr: tk_lchr: printf("\"%s\"", tok->s); break;
		default: break;
	}

	printf("\n");
}

void print_tokens(struct token *tokens) {
	printf("- Tokens\n");
	struct token *tok = tokens;

	while (tok) {
		print_token(tok);
		tok = tok->next;
	}

	printf("\n");
}
