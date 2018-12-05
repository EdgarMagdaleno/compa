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
struct ht *world;
int current_label = 0;
int current_scope = 0;

char *i_s(int n) {
	char *buf = malloc(64);
	sprintf(buf, "%i", n);
	return buf;
}

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

int decl() {
	if (!expect_rng(tk_char, tk_str, SAVE))
		return tk_type;

	step();
	if (!expect(tk_id, IGNR))
		return tk_id;

	if (ht_get(world, current->s))
		return -RDCL;
		
	id = malloc(sizeof(struct ht_item));
	id->scope_id = current_scope;
	id->name = current->s;
	id->type = save->type;
	step();

	if (current->type == tk_lbrk)
		id->type += VECTOR_DIFF;
	else {
		writes_decl(id);
		ht_add(world, id);
	}

	log_msg("decl ");
	return 0;
}

int read() {
	if (!expect(tk_read, IGNR))
		return tk_read;
	step();

	log_msg("read ");
	return 0;
}

int asgn() {
	if (!expect(tk_asg, IGNR))
		return tk_asg;

	step();
	log_msg("asgn ");
	return 0;
}

int prnt() {
	if (!expect(tk_prnt, IGNR))
		return tk_prnt;
	
	step();
	log_msg("prnt ");
	return 0;
}

int lpar() {
	if (!expect(tk_lpar, IGNR))
		return tk_lpar;
	
	step();
	log_msg("lpar ");
	return 0;
}

int rpar() {
	if (!expect(tk_rpar, IGNR))
		return tk_rpar;
	
	step();
	log_msg("rpar ");
	return 0;
}

int idn() {
	if (!expect(tk_id, IGNR))
		return tk_id;

	id = ht_get(world, current->s);
	if (!id)
		return -NDCL;
	
	if (!is_vector(id->type))
		goto cont;
	
	step();
	if (!expect(tk_lbrk, IGNR))
		return tk_lbrk;
	
	step();
	int err_code = expr();
	if (err_code)
		return err_code;

	if (!expect(tk_rbrk, IGNR))
		return tk_rbrk;
cont:
	step();
	log_msg("idn ");
	return 0;
}

int eos() {
	if (!expect(tk_eos, IGNR))
		return tk_eos;

	step();
	log_msg("eos ");
	return 0;
}

int eosr() {
	if (!expect(tk_eos, IGNR))
		return tk_eos;

	int type = id->type;
	if (is_vector(id->type))
		type -= 4;

	switch (type) {
		case tk_char: writes("RDC"); break;
		case tk_int: writes("RDI"); break;
		case tk_dobl: writes("RDD"); break;
		case tk_str: writes("RDS"); break;
	}

	step();
	writes_pop_id(id);
	log_msg("eosr ");
	return 0;
}

int _for() {
	if (!expect(tk_for, IGNR))
		return tk_for;
	step();

	log_msg("for ");
	return 0;
}

int for_end() {
	log_msg("for_end ");
	struct token *res = current;
	current = world->for_stm;
	print_token(current);

	int err_code = idn();
	if (err_code)
		return err_code;

	err_code = asgn();
	printf("err %i ", err_code);
	if (err_code)
		return err_code;

	err_code = expr();
	printf("err %i ", err_code);
	if (err_code)
		return err_code;

	err_code = _eosp();
	printf("err %i ", err_code);
	if (err_code)
		return err_code;

	current = res;
	log_msg("holaaaaa\n");
	print_token(current);
	err = 0;
	writes("JMP %s", i_s(world->for_cond));
	writes("%s:", i_s(world->for_end));
	return 0;
}

int for_st() {
	world->for_stm = current;
	while (current->type != tk_rpar)
		step();
	return 0;
}


int for_j() {
	world->for_end = current_label++;
	writes("JMPC %s", i_s(world->for_end));
}

int f_cnd() {
	world->for_cond = current_label++;
	writes("%s:", i_s(world->for_cond));
}

int eosp() {
	if (!expect(tk_eos, IGNR))
		return tk_eos;
	step();

	writes_pop_id(id);
	log_msg("eosp ");
	return 0;
}

int _eosp() {
	writes_pop_id(id);
	log_msg("eosp ");
	return 0;
}

int exrl() {
	if (!current)
		return 0;
parse:
	expr();
	writes("WRT");

	if (current->type == tk_coma) {
		step();
		goto parse;
	}

	return 0;
}

int expr() {
	struct token *operator = NULL;
	struct token *next = NULL;

	while (current) {
		next = current->next;
		switch(current->type) {
			case tk_lchr ... tk_lstr:
				writes_token(current);
			break;

			case tk_id: {
				struct ht_item *id_local = ht_get(world, current->s);
				if (!id_local) {
					return -NDCL;
				}

				if (!is_vector(id_local->type))
					goto cont;
				step();

				if (!expect(tk_lbrk, IGNR))
					return tk_lbrk;
				step();

				int err_code = expr();
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

int prog() {
	return 0;
}

int ifs() {
	if (!expect(tk_if, IGNR))
		return 1;
	step();

	world->if_label = current_label++;
	return 0;
}

int whle() {
	if (!expect(tk_whle, IGNR))
		return 1;
	step();

	writes_label(current_label);
	world->while_label = current_label++;
	world->if_label = current_label++;
	log_msg("whle ");
	return 0;
}

int scpe() {
	log_msg("\n");
	current_scope++;

	if (world->if_label != -1) {
		writes("JMPC %s", i_s(world->if_label));
 	}

	int old_label = world->if_label;
	int old_while_label = world->while_label;
	world->while_label = -1;
	world->if_label = -1;
	log_msg(" changed_labels ");
	int err_code;

	while (current) {
		log_msg("scpe ");

		err_code = parse_statement(root);
		if (!err_code)
			goto next;

		if (!current)
			goto out;

		if (current->type == tk_rbrc)
			goto out;

		print_token(current);
		

		error_log(current, err_code);
		while (current->type != tk_eos)
			current = current->next;
		step();

	next:
		log_msg("\n");
	}

out:
	world->if_label = old_label;
	world->while_label = old_while_label;
	log_msg(" labels: %i %i ", world->if_label, world->while_label);

	if (world->while_label != -1) {
		writes("JMP %s", i_s(world->while_label));
	}

	if (world->if_label != -1) {
		writes("%s:", i_s(world->if_label));
	}
	return 0;
}

int lbrk() {
	if (!expect(tk_lbrk, IGNR))
		return tk_lbrk;

	step();
	log_msg("lbrk ");
	return 0;
}

int rbrk() {
	if (!expect(tk_rbrk, IGNR))
		return tk_rbrk;
	step();

	if (!ht_get(world, id->name)) {
		writes_decl(id);
		ht_add(world, id);
	}

	log_msg("rbrk ");
	return 0;
}

int lbrc() {
	if (!expect(tk_lbrc, IGNR))
		return tk_lbrc;
	step();

	log_msg("lbrc ");
	return 0;
}

int rbrc() {
	if (!expect(tk_rbrc, IGNR))
		return tk_rbrc;
	step();
	clear_scope(world, current_scope--);
	world->if_label = -1;
	world->while_label = -1;
	world->for_stm = NULL;
	world->for_end = -1;
	world->for_label = -1;
	log_msg("rbrc ");
	return 0;
}

int parse_statement(struct ast_node *node) {
	int res = 0;

	restore = current;
	assert(node->parse);
	res = node->parse();
	if (res) {
		current = restore;
		return res;
	}

	if (node->branches_size == 0)
		return 0;

	log_msg("-> ");
	for (int i = 0; i < node->branches_size; i++) {
		res = parse_statement(node->branches[i]);
		if (!res)
			return res;
	}

	return res;
}

void first_scope() {
	int err_code = 0;

	while (current) {
		log_msg("prog ");

		err_code = parse_statement(root);
		if (err_code) {
			if (!current || !current->type)
				return;

			print_token(current);
			error_log(current, err_code);
			while (current && current->type != tk_eos)
				current = current->next;
			step();

		}
		log_msg("\n");
	}
}

void start_parse(struct token *tokens, struct ast_node *ast_root) {
	current = tokens;
	root = ast_root;
	world = new_ht(HT_BUCKET_SIZE);
	first_scope();
	writes("EXT");
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
