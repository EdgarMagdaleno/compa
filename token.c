#include "token.h"
#include "msg.h"

char *get_type_name(token_type type) {
	return &"tk_char tk_int  tk_dobl tk_str  tk_vchr tk_vint tk_vdbl tk_vstr "
			"tk_else tk_if   tk_whle tk_eos  tk_add  tk_sub  tk_mul  tk_div  "
			"tk_mod  tk_eq   tk_grt  tk_les  tk_neq  tk_lese tk_grte tk_id   "
			"tk_lchr tk_lint tk_ldbl tk_lstr tk_rpar tk_lbrc tk_rbrc tk_coma "
			"tk_asg  tk_lpar tk_lbrk tk_rbrk tk_prnt tk_type tk_read tk_for  "[type * 8];
}

int opposite(int type) {
	if (type > tk_les)
		return type - 3;

	return type + 3;
}

void print_token(struct token *tok) {
	if (!tok) return;
	log_msg("%i %i %.8s", tok->line, tok->col, get_type_name(tok->type));
	
	switch (tok->type) {
		case tk_id: log_msg("%s", tok->s); break;
		case tk_lint: log_msg("%i" , tok->i); break;
		case tk_ldbl: log_msg("%lf", tok->d); break;
		case tk_lstr: tk_lchr: log_msg("\"%s\"", tok->s); break;
		default: break;
	}

	log_msg("\n");
}

void print_tokens(struct token *tokens) {
	log_msg("- Tokens\n");
	struct token *tok = tokens;

	while (tok) {
		print_token(tok);
		tok = tok->next;
	}

	log_msg("\n");
}

int is_vector(int type) {
	switch(type) {
		case tk_vchr ... tk_vstr: return 1; break;
		default: return 0; break;
	}
}