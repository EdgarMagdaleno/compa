#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "msg.h"
#include "ht.h"
#include "writer.h"
#include "token.h"
#include "ast.h"
#include "parser.h"
#include "source.h"

#define MAX_LINE_SIZE	256
#define HT_BUCKET_SIZE	256
#define DFS_STACK_SIZE	64
#define VECTOR_DIFF		4
#define SAVE			1
#define IGNR			0

struct token *current = NULL;
struct token *save = NULL;
struct token *restore = NULL;
struct ht_item *id = NULL;
struct ast_node *root = NULL;
int current_label = 0;

int expect_rng(token_type min, token_type max, int save_read) {
	if (!current)
		return 0;

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
	if (!current)
		return 0;

	if (current->type == type) {
		if (save_read) save = current;
		return 1;
	}

	return 0;
}

void step() {
	if (current)
		current = current->next;
}

int decl(struct ht *scope) {
	if (!expect_rng(tk_char, tk_str, SAVE))
		return tk_type;

	step();
	if (!expect(tk_id, IGNR))
		return tk_id;

	if (ht_get(scope, current->s))
		return -RDCL;
		
	id = malloc(sizeof(struct ht_item));
	id->name = current->s;
	id->type = save->type;
	step();

	if (current->type == tk_lbrk)
		id->type += VECTOR_DIFF;
	else {
		writes_decl(id);
		ht_add(scope, id);
	}

	log_msg("decl ");
	return 0;
}

int asgn(struct ht *scope) {
	if (!expect(tk_asg, IGNR))
		return tk_asg;

	step();
	log_msg("asgn ");
	return 0;
}

int prnt(struct ht *scope) {
	if (!expect(tk_prnt, IGNR))
		return tk_prnt;
	
	step();
	log_msg("prnt ");
	return 0;
}

int lpar(struct ht *scope) {
	if (!expect(tk_lpar, IGNR))
		return tk_lpar;
	
	step();
	log_msg("lpar ");
	return 0;
}

int rpar(struct ht *scope) {
	if (!expect(tk_rpar, IGNR))
		return tk_rpar;
	
	step();
	log_msg("rpar ");
	return 0;
}

int idn(struct ht *scope) {
	if (!expect(tk_id, IGNR))
		return tk_id;

	id = ht_get(scope, current->s);
	if (!id)
		return -NDCL;
	
	if (!is_vector(id->type))
		goto cont;
	
	step();
	if (!expect(tk_lbrk, IGNR))
		return tk_lbrk;
	
	step();
	int err_code = expr(scope);
	if (err_code)
		return err_code;

	if (!expect(tk_rbrk, IGNR))
		return tk_rbrk;
cont:
	step();
	log_msg("idn ");
	return 0;
}

int eos(struct ht *scope) {
	if (!expect(tk_eos, IGNR))
		return tk_eos;

	step();
	log_msg("eos ");
	return 0;
}

int eosp(struct ht *scope) {
	if (!expect(tk_eos, IGNR))
		return tk_eos;
	step();
	
	writes_pop_id(id);
	log_msg("eosp ");
	return 0;
}

int exrl(struct ht *scope) {
	if (!current)
		return 0;
parse:
	expr(scope);
	writes("WRT");

	if (current->type == tk_coma) {
		step();
		goto parse;
	}

	return 0;
}

int expr(struct ht *scope) {
	struct token *operator = NULL;
	struct token *next = NULL;

	while (current) {
		next = current->next;
		switch(current->type) {
			case tk_lchr ... tk_lstr:
				writes_token(current);
			break;

			case tk_id: {
				struct ht_item *id_local = ht_get(scope, current->s);
				if (!id_local)
					return -NDCL;

				if (!is_vector(id_local->type))
					goto cont;
				step();

				if (!expect(tk_lbrk, IGNR))
					return tk_lbrk;
				step();

				int err_code = expr(scope);
				if (err_code)
					return err_code;

				if (!expect(tk_rbrk, IGNR))
					return tk_rbrk;
				step();

				next = current;
			cont:
				writes_id(id_local);
			} break;

			case tk_add ... tk_lese:
				while (top(&operator) && prec(top(&operator)) >= prec(current))
					writes_token(pop(&operator));

				push(&operator, current);
			break;

			case tk_lpar: push(&operator, current); break;
			case tk_rpar:
				while (top(&operator) && top(&operator)->type != tk_lpar)
					writes_token(pop(&operator));

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
		writes_token(pop(&operator));

	log_msg("expr ");
	return 0;
}

int prog(struct ht *scope) {
	return 0;
}

int ifs(struct ht *scope) {
	if (!expect(tk_if, IGNR))
		return 1;
	step();
	return 0;
}

int scpe(struct ht *scope) {
	struct ht *new_scope = new_ht(HT_BUCKET_SIZE);
	int err_code;

	while (current) {
		log_msg("scpe ");

		err_code = parse_statement(root, scope);
		if (err_code) {
			print_token(current);
			err = 1;

			error_log(current, err_code);
			while (current->type != tk_eos)
				current = current->next;
			step();

		}
		log_msg("\n");
	}
	return 0;
}

int lbrk(struct ht *scope) {
	if (!expect(tk_lbrk, IGNR))
		return tk_lbrk;

	step();
	log_msg("lbrk ");
	return 0;
}

int rbrk(struct ht *scope) {
	if (!expect(tk_rbrk, IGNR))
		return tk_rbrk;
	step();

	if (!ht_get(scope, id->name)) {
		writes_decl(id);
		ht_add(scope, id);
	}

	log_msg("rbrk ");
	return 0;
}

int lbrc(struct ht *scope) {
	if (!expect(tk_lbrc, IGNR))
		return tk_lbrc;

	step();
	log_msg("lbrc ");
	return 0;
}

int rbrc(struct ht *scope) {
	if (!expect(tk_rbrc, IGNR))
		return tk_rbrc;

	step();
	log_msg("rbrc ");
	return 0;
}

int parse_statement(struct ast_node *node, struct ht *scope) {
	int res = 0;

	restore = current;
	assert(node->parse);
	res = node->parse(scope);
	if (res) {
		current = restore;
		return res;
	}

	if (node->branches_size == 0)
		return 0;

	log_msg("-> ");
	for (int i = 0; i < node->branches_size; i++) {
		res = parse_statement(node->branches[i], scope);
		if (!res)
			return res;
	}

	return res;
}

void first_scope() {
	struct ht *scope = new_ht(HT_BUCKET_SIZE);
	int err_code = 0;

	while (current) {
		log_msg("prog ");

		err_code = parse_statement(root, scope);
		if (err_code) {
			print_token(current);
			err = 1;

			error_log(current, err_code);
			while (current->type != tk_eos)
				current = current->next;
			step();

		}
		log_msg("\n");
	}
}

void start_parse(struct token *tokens, struct ast_node *ast_root) {
	current = tokens;
	root = ast_root;
	return first_scope();
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
