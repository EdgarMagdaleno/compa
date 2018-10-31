#include <stdlib.h>
#include "msg.h"
#include "ht.h"
#include "writer.h"
#include "token.h"
#include "parser.h"

#define HT_BUCKET_SIZE	256

int current_label = 0;

int prec(struct token *tok) {
	switch(tok->type) {
		case tk_mul ... tk_mod: return 3; break;
		case tk_add ... tk_sub: return 2; break;
		default: return 1; break;
	}
}

void push(struct token **stack, struct token *tok) {
	if (!(*stack)) {
		(*stack) = tok;
		(*stack)->next = NULL;
		return;
	}
	
	tok->next = (*stack);
	(*stack) = tok;
}

struct token *pop(struct token **stack) {
	struct token *tok = *stack;
	(*stack) = (*stack)->next;

	tok->next = NULL;
	return tok;
}

struct token *peek(struct token **stack) {
	return (*stack);
}

void chain(struct token **list, struct token **last, struct token *tok) {
	if (!*last) {
		*list = tok;
		*last = tok;
	} else {
		(*last)->next = tok;
		*last = (*last)->next;
	}
}

void parse_expr(struct token **tokens, struct token *tok) {
	struct token *output = NULL;
	struct token *last = NULL;
 	struct token *operator = NULL;
	struct token *next = NULL;

	while (tok) {
		next = tok->next;
		switch(tok->type) {
			case tk_id ... tk_lstr: chain(&output, &last, tok); break;
			case tk_add ... tk_lese:
				while (peek(&operator) && prec(peek(&operator)) >= prec(tok))
					chain(&output, &last, pop(&operator));

				push(&operator, tok); 
			break;
			case tk_lpar: push(&operator, tok); break;
			case tk_rpar:
				while (peek(&operator) && peek(&operator)->type != tk_lpar)
					chain(&output, &last, pop(&operator));

				if (!peek(&operator))
					goto ret;

				pop(&operator);
			break;
			default: goto ext; break;
		}

		tok = next;
	}

ext:
	while (peek(&operator))
		chain(&output, &last, pop(&operator));

ret:
	*tokens = tok;
	last->next = NULL;

	while (output) {
		switch(output->type) {
			case tk_lchr: writes("PUSHKC \'%s\'", output->s); break;
			case tk_lint: writes("PUSHKI %i", output->i); break;
			case tk_ldbl: writes("PUSHKD %lf", output->d); break;
			case tk_lstr: writes("PUSHKS \"%s\"", output->s); break;
			case tk_id: writes("PUSH %s", output->s); break;
			case tk_add: writes("ADD"); break;
			case tk_sub: writes("SUB"); break;
			case tk_mul: writes("MUL"); break;
			case tk_div: writes("DIV"); break;
			case tk_eq: writes("CEQ"); break;
			case tk_neq: writes("CNE"); break;
			case tk_grt: writes("CGT"); break;
			case tk_grte: writes("CGE"); break;
			case tk_les: writes("CLT"); break;
			case tk_lese: writes("CLE"); break;
		}

		output = output->next;
	}
}

struct ht_item *declare_id(struct ht *scope, struct token *tok, token_type type) {
	struct ht_item *new_id = malloc(sizeof(struct ht_item));
	new_id->name = tok->s;
	new_id->type = type;
	ht_add(scope, new_id);
	return new_id;
}

int expect(struct token **tokens, struct token *tok, token_type type) {
	if (!tok || tok->type != type) {
		error_log(tok->line, tok->col, "Expected %.8s", get_type_name(type));
		while (tok && tok->type != tk_eos)
			tok = tok->next;
		
		*tokens = tok;
		return 0;
	}
	
	*tokens = tok;
	return 1;
}

int expect_over(struct token **tokens, struct token *tok, token_type type) {
	if (!tok || tok->type != type) {
		error_log(tok->line, tok->col, "Expected %.8s", get_type_name(type));
		while (tok && tok->type != tk_eos)
			tok = tok->next;

		*tokens = tok;
		return 0;
	}

	*tokens = tok->next;
	return 1;
		
}

int step(struct token **tokens, struct token *tok, token_type type) {
	*tokens = tok;
	if (!tok || tok->type != type) {
		return 0;
	}
	
	return 1;
}

struct token *parse_scope(struct token *tokens) {
	struct token *tok = tokens;
	struct ht *scope = new_ht(HT_BUCKET_SIZE);

	while (tok) {
		switch(tok->type) {
			case tk_char ... tk_str: {
				
			} break;

			case tk_id: {
				struct ht_item *id = ht_get(scope, tok->s);

				if(id->type < tk_vchr || id->type > tk_vstr)
					goto cont;
				
				if (!step(&tok, tok->next, tk_lbrk));
				
				tok = tok->next;
				parse_expr(&tok, tok);

				if (!expect_over(&tok, tok, tk_rbrk))
					break;
			cont:
				if (!expect(&tok, tok->next, tk_asg))
					break;
				
				parse_expr(&tok, tok->next);
				write_pop_id(id);
			} break;

			case tk_if: {
				if (!expect_over(&tok, tok->next, tk_lpar))
					break;

				parse_expr(&tok, tok);

				if (!expect_over(&tok, tok, tk_rpar))
					break;

				if (!expect_over(&tok, tok, tk_lbrc))
					break;

				int label = current_label++;
				writes("JMPC %i", label);
				tok = parse_scope(tok);
				writes("%i:", label);

				if (!expect_over(&tok, tok, tk_rbrc))
					break;
			} break;

			case tk_rbrc: {
				struct ht_item *item = first(scope);

				while (item) {
					writes("FREE %s", item->name);
					item = next(scope, item);
				}

				return tok;
			} break;

			case tk_prnt:
				if (!expect(&tok, tok->next, tk_lpar))
					break;

				do {
					tok = tok->next;
					parse_expr(&tok, tok);
					writes("WRT");
				} while (tok->type == tk_coma);

				expect_over(&tok, tok, tk_rpar);
			break;

			default: log_msg("Default: "); print_token(tok); break;
		}

		expect_over(&tok, tok, tk_eos);
	}

	
}