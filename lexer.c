#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "token.h"
#include "lexer.h"
#include "source.h"

#define MAX_WORD_SIZE	256

int line = 1, col = 0, chr = ' ';

void getc_source() {
	chr = getc(source);
	col++;

	if (chr == '\n') {
		line++;
		col = 0;
	}
}

struct token *number() {
	char buf[MAX_WORD_SIZE];
	int i = 0;
	int c;
	struct token *tok = NULL;
	
	int found_point = 0;
	while (isdigit(chr) || (chr == '.' && !found_point)) {
		if (chr == '.') found_point = 1;
		buf[i++] = chr;
		getc_source();
	}
	ungetc(chr, source);

	tok = malloc(sizeof(struct token));
	if (found_point) {
		tok->type = tk_ldbl;
		tok->d = atof(buf);
	} else {
		tok->type = tk_lint;
		tok->i = atoi(buf);
	}

	return tok;
}

int keyword_comp(const void *a, const void *b) {
	return strcmp((char *) a, *(char **) b);
}

struct token *ident_or_kw() {
	static struct {
		char *s;
		token_type type;
	} keywords[] = {
		{"char",	tk_char},
		{"double",	tk_dobl},
		{"else",	tk_else},
		{"if",		tk_if},
		{"int",		tk_int},
		{"print",	tk_prnt},
		{"string",	tk_str},
		{"while",	tk_whle},
	}, *kw;

	struct token *tok = malloc(sizeof(struct token));
	ungetc(chr, source);

	char buf[MAX_WORD_SIZE];
	fscanf(source, "%255[a-zA-Z0-9_]", buf);
	tok->type = !(kw = bsearch(buf, keywords, sizeof(keywords) / sizeof(keywords[0]),
				  sizeof(keywords[0]), keyword_comp)) ? tk_id : kw->type;

	if (tok->type == tk_id) {
		tok->s = malloc(strlen(buf));
		strcpy(tok->s, buf);
	}

	return tok;
}

struct token *char_lit() {
	struct token *tok;
	char buf[3];
	fscanf(source, "%2s\'", buf);
	int len = strlen(buf);

	tok = malloc(sizeof(struct token));
	tok->type = tk_lchr;
	tok->s = malloc(len);
	strcpy(tok->s, buf);
	return tok;
}

struct token *string_lit() {
	struct token *tok;
	char buf[MAX_WORD_SIZE];
	fscanf(source, "%255[^\"]\"", buf);

	size_t len = strnlen(buf, MAX_WORD_SIZE);
	col += len;
	tok = malloc(sizeof(struct token));
	tok->type = tk_lstr;
	tok->s = malloc(len);
	strncpy(tok->s, buf, len);
	return tok;
}

struct token *new_token(token_type type) {
	struct token *tok = malloc(sizeof(struct token));
	tok->type = type;

	return tok;
}

struct token *follow(token_type normal, token_type follow, char next) {
	struct token *tok = malloc(sizeof(struct token));
	
	getc_source();
	tok->type = chr == next ? follow : normal;
	return tok;
}

struct token *get_token() {
	struct token *tok = NULL;
	while (isspace(chr))
		getc_source();

	int tok_line = line;
	int tok_col = col;
	switch(chr) {
		case '=': tok = follow(tk_asg, tk_eq, '='); break;
		case '>': tok = follow(tk_grt, tk_grte, '='); break;
		case '<': tok = follow(tk_les, tk_lese, '='); break;
		case '!': tok = new_token(tk_neq); break;
		case '+': tok = new_token(tk_add); break;
		case '-': tok = new_token(tk_sub); break;
		case '*': tok = new_token(tk_mul); break;
		case '/': tok = new_token(tk_div); break;
		case '%': tok = new_token(tk_mod); break;
		case '{': tok = new_token(tk_lbrc); break;
		case '}': tok = new_token(tk_rbrc); break;
		case '(': tok = new_token(tk_lpar); break;
		case ')': tok = new_token(tk_rpar); break;
		case '[': tok = new_token(tk_lbrk); break;
		case ']': tok = new_token(tk_rbrk); break;
		case ',': tok = new_token(tk_coma); break;
		case ';': tok = new_token(tk_eos); break;
		case '"': tok = string_lit(); break;
		case '\'': tok = char_lit(); break;
		case 'a' ... 'z': case 'A' ... 'Z': tok = ident_or_kw(); break;
		case '0' ... '9': tok = number(); break;
		default: printf("Unrecognized character: \'%c\': %i\n", chr, chr); case EOF: break;
	}

	if (tok) {
		tok->line = tok_line;
		tok->col = tok_col;
		getc_source();
	}

	return tok;
}

struct token *lex() {
	struct token *tok = get_token();
	struct token *tokens = tok;

	while (tok) {
		tok->next = get_token();
		tok = tok->next;
	}

	return tokens;
}
