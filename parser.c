#include <stdio.h>
#include <stdlib.h>
#include "msg.h"
#include "ht.h"
#include "writer.h"
#include "token.h"
#include "ast.h"
#include "parser.h"

#define HT_BUCKET_SIZE	256
#define DFS_STACK_SIZE	64
#define SAVE			1
#define IGNR			0

struct token *current = NULL;
struct token *save = NULL;
struct token *restore = NULL;
struct ht_item *id = NULL;
int current_label = 0;

int expect_rng(token_type min, token_type max, int save_read) {
	while (min <= max) {
		if (current->type == min) {
			if (save_read) save = current;
			return 1;
		}

		min++;
	}

	return 0;
}

int expect(token_type type, int save_read) {
	if (current->type == type) {
		if (save_read) save = current;
		return 1;
	}

	return 0;
}

void step() {
	current = current->next;
}

int asgn(struct ht *scope) {
	if (!expect(tk_asg, IGNR))
		return 0;

	step();
	printf("asgn ");
	return 1;
}

int type(struct ht *scope) {
	if (!expect_rng(tk_char, tk_str, SAVE))
		return 0;

	step();
	printf("type ");
	return 1;
}

int idn(struct ht *scope) {
	if (!expect(tk_id, IGNR))
		return 0;

	if (!save)
		goto out;

	id = malloc(sizeof(struct ht_item));
	id->name = current->s;
	id->type = save->type;
	ht_add(scope, id);

out:

	step();
	printf("idn ");
	return 1;
}

int vidn(struct ht *scope) {
	if (!expect(tk_id, IGNR))
		return 0;

	id = malloc(sizeof(struct ht_item));
	id->name = current->s;
	id->type = save->type + 4;
	ht_add(scope, id);

	step();
	printf("vidn ");
	return 1;
}

int eos(struct ht *scope) {
	if (!expect(tk_eos, IGNR))
		return 0;

	step();
	printf("eos ");
	return 1;
}

int eosp(struct ht *scope) {
	printf("eosp ");
	return 1;
}

int expr(struct ht *scope) {
	struct token *output = NULL;
	struct token *last = NULL;
	struct token *operator = NULL;
	struct token *next = NULL;

	while (current) {
		next = current->next;
		switch(current->type) {
			case tk_lchr ... tk_lstr:
				chain(&output, &last, current);
			break;

			case tk_id: {
				struct ht_item *iden = ht_get(scope, current->s);
				if (!ht_get(scope, current->s))
					return 1;
			} break;

			case tk_add ... tk_lese:
				while (top(&operator) && prec(top(&operator)) >= prec(current))
					chain(&output, &last, pop(&operator));

				push(&operator, current);
			break;

			case tk_lpar: push(&operator, current); break;
			case tk_rpar:
				while (top(&operator) && top(&operator)->type != tk_lpar)
					chain(&output, &last, pop(&operator));

				if (!top(&operator))
					goto out;

				pop(&operator);
			break;

			default: goto out; break;
		}

		current = next;
	}

out:
	while (top(&operator))
		chain(&output, &last, pop(&operator));

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

	printf("expr ");
	return 1;
}

int prog(struct ht *scope) {
	return 1;
}

int lbrk(struct ht *scope) {
	if (save)
		save->type += 4;

	if (!expect(tk_lbrk, IGNR))
		return 0;

	step();
	printf("lbrk ");
	return 1;
}

int rbrk(struct ht *scope) {
	if (!expect(tk_rbrk, IGNR))
		return 0;

	step();
	printf("rbrk ");
	return 1;
}

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

struct token *top(struct token **stack) {
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

struct token *parse_scope(struct token *tokens) {
	/*
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
				expr();

				if (!expect_over(&tok, tok, tk_rbrk))
					break;
			cont:
				if (!expect(&tok, tok->next, tk_asg))
					break;
				
				expr();
				write_pop_id(id);
			} break;

			case tk_if: {
				if (!expect_over(&tok, tok->next, tk_lpar))
					break;

				expr();

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
					expr();
					writes("WRT");
				} while (tok->type == tk_coma);

				expect_over(&tok, tok, tk_rpar);
			break;

			default: log_msg("Default: "); print_token(tok); break;
		}

		expect_over(&tok, tok, tk_eos);
	}

	*/
}

int preorder(struct ast_node *node, struct ht *scope) {
	int res = 0;
	if (!node)
		return 1;

	restore = current;
	res = node->parse(scope);
	if (!res) {
		current = restore;
		return 0;
	}

	if (node->branches_size == 0)
		return 1;

	printf("-> ");
	for (int i = 0; i < node->branches_size; i++) {
		res = preorder(node->branches[i], scope);
		if (res)
			return 1;
	}

	return 0;
}

void parse(struct token *tokens, struct ast_node *root) {
	struct ht *scope = new_ht(HT_BUCKET_SIZE);
	current = tokens;

	while (current) {
		printf("prog ");
		preorder(root, scope);
		printf("\n");
	}
}
