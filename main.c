#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include "ht.h"

#define MAX_WORD_SIZE	256
#define HT_BUCKET_SIZE	256
#define MAX_LINE_SIZE	512

#define C_RED	"\e[1;31m"
#define C_GRN	"\e[1;32m"
#define C_BLU	"\e[1;34m"
#define C_YLW	"\e[1;33m"
#define C_RST	"\e[0m"

FILE *source, *out;
int line = 1, col = 0, ch = ' ', current_label = 0;

void error_exit(const char *msg) {
	printf("[ERROR] %s\n", msg);
	exit(1);
}

typedef enum {
	tk_char, tk_int,  tk_dobl, tk_str,  tk_vchr, tk_vint, tk_vdbl, tk_vstr,
	tk_else, tk_if,   tk_whle, tk_eos,  tk_add,  tk_sub,  tk_mul,  tk_div,
	tk_mod,  tk_eq,   tk_neq,  tk_grt,  tk_grte, tk_les,  tk_lese, tk_id,
	tk_lint, tk_ldbl, tk_lchr, tk_lstr, tk_rpar, tk_lbrc, tk_rbrc, tk_coma,
	tk_asg,  tk_lpar, tk_lbrk, tk_rbrk, tk_prnt
} token_type;

struct token {
	token_type type;
	int line, col;
	union {
		int i;
		double d;
		char *s;
	};
	struct token *next;
};

char *get_type_name(token_type type) {
	return &"tk_char tk_int  tk_dobl tk_str  tk_vchr tk_vint tk_vdbl tk_vstr "
			"tk_else tk_if   tk_whle tk_eos  tk_add  tk_sub  tk_mul  tk_div  "
			"tk_mod  tk_eq   tk_neq  tk_grt  tk_grte tk_les  tk_lese tk_id   "
			"tk_lint tk_ldbl tk_lchr tk_lstr tk_rpar tk_lbrc tk_rbrc tk_coma "
			"tk_asg  tk_lpar tk_lbrk tk_rbrk tk_prnt"[type * 8];
}

void error_log(int line, int col, const char *msg, ...) {
	va_list arglist;
	printf(C_RED"[ERROR] %i,%i "C_RST, line, col);
	va_start(arglist, msg);
	vprintf(msg, arglist);
	va_end(arglist);
	printf("\n");
}

struct {
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
	struct token *tok = tokens;

	while (tok) {
		print_token(tok);
		tok = tok->next;
	}
}

int prec(struct token *tok) {
	switch(tok->type) {
		case tk_mul ... tk_mod: return 3; break;
		case tk_add ... tk_sub: return 2; break;
		default: return 1; break;
	}
}

struct token *new_token(token_type type) {
	struct token *tok = malloc(sizeof(struct token));
	tok->type = type;

	return tok;
}

struct token *new_token_op(token_type type, int precedence) {
	struct token *tok = new_token(type);
	tok->i = precedence;
	
	return tok;
}

void getc_source() {
	ch = getc(source);
	col++;

	if (ch == '\n') {
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
	while (isdigit(ch) || (ch == '.' && !found_point)) {
		if (ch == '.') found_point = 1;
		buf[i++] = ch;
		getc_source();
	}
	ungetc(ch, source);

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

int comp(const void *a, const void *b) {
	return strcmp((char *) a, *(char **) b);
}

struct token *follow(token_type normal, token_type follow, char next) {
	struct token *tok = malloc(sizeof(struct token));
	
	getc_source();
	tok->type = ch == next ? follow : normal;
	return tok;
}

struct token *ident_or_kw() {
	struct token *tok = malloc(sizeof(struct token));
	ungetc(ch, source);

	char buf[MAX_WORD_SIZE];
	fscanf(source, "%255[a-zA-Z0-9_]", buf);
	tok->type = !(kw = bsearch(buf, keywords, sizeof(keywords) / sizeof(keywords[0]),
				  sizeof(keywords[0]), comp)) ? tk_id : kw->type;

	if (tok->type == tk_id) {
		tok->s = malloc(strlen(buf));
		strcpy(tok->s, buf);
	}

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

struct token *get_token() {
	struct token *tok = NULL;
	while (isspace(ch))
		getc_source();

	int tok_line = line;
	int tok_col = col;
	switch(ch) {
		case '=': tok = follow(tk_asg, tk_eq, '='); break;
		case '>': tok = follow(tk_grt, tk_grte, '='); break;
		case '<': tok = follow(tk_les, tk_lese, '='); break;
		case '!': tok = new_token_op(tk_neq, 1); break;
		case '+': tok = new_token_op(tk_add, 2); break;
		case '-': tok = new_token_op(tk_sub, 2); break;
		case '*': tok = new_token_op(tk_mul, 3); break;
		case '/': tok = new_token_op(tk_div, 3); break;
		case '%': tok = new_token_op(tk_mod, 3); break;
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
		default: printf("Unrecognized character: \'%c\': %i\n", ch, ch); case EOF: break;
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

void writes(const char *msg, ...) {
	va_list arglist;
	va_start(arglist, msg);
	vfprintf(out, msg, arglist);
	va_end(arglist);
	fprintf(out, "\n");
}

void chain(struct token **list, struct token **last, struct token *tok) {
	if (!*last) {
		*list = tok;
		*last = tok;
		return;
	}

 	(*last)->next = tok;
	*last = (*last)->next;
}

void parse_expr(struct token **tokens, struct token *tok,
			   struct token **expr) {
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
	*expr = output;
	last->next = NULL;
}

void write_decl(struct ht_item *id) {
	switch(id->type) {
		case tk_char: fprintf(out, "DCLC %s\n", id->name); break;
		case tk_int: fprintf(out, "DCLI %s\n", id->name); break;
		case tk_dobl: fprintf(out, "DCLD %s\n", id->name); break;
		case tk_str: fprintf(out, "DCLS %s\n", id->name); break;
		case tk_vchr: fprintf(out, "DCLVC %s\n", id->name); break;
		case tk_vint: fprintf(out, "DCLVI %s\n", id->name); break;
		case tk_vdbl: fprintf(out, "DCLVD %s\n", id->name); break;
		case tk_vstr: fprintf(out, "DCLVS %s\n", id->name); break;
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

void write_expr(struct token *expr) {
	while (expr) {
		switch(expr->type) {
			case tk_lchr: writes("PUSHKC \'%s\'", expr->s); break;
			case tk_lint: writes("PUSHKI %i", expr->i); break;
			case tk_ldbl: writes("PUSHKD %lf", expr->d); break;
			case tk_lstr: writes("PUSHKS \"%s\"", expr->s); break;
			case tk_id: writes("PUSH %s", expr->s); break;
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

		expr = expr->next;
	}
}

void write_pop_id(struct ht_item *id) {
	switch(id->type) {
		case tk_char: writes("POPC %s", id->name); break;
		case tk_int: writes("POPI %s", id->name); break;
		case tk_dobl: writes("POPD %s", id->name); break;
		case tk_str: writes("POPS %s", id->name); break;
		case tk_vchr: writes("POPVC %s", id->name); break;
		case tk_vint: writes("POPVI %s", id->name); break;
		case tk_vdbl: writes("POPSVD %s", id->name); break;
		case tk_vstr: writes("POPSVS %s", id->name); break;
	}
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
				token_type type = tok->type;
				struct token *expr = NULL;
				struct ht_item *id = NULL;

				if (!step(&tok, tok->next, tk_lbrk))
					goto not_arr;
				
				tok = tok->next;
				type += 4;
				parse_expr(&tok, tok, &expr);
				write_expr(expr);

				if (!expect_over(&tok, tok, tk_rbrk))
					break;

				if (!expect(&tok, tok, tk_id))
					break;
				
				id = declare_id(scope, tok, type);
				write_decl(id);
				tok = tok->next;
				break;
			not_arr:
				if(!expect(&tok, tok, tk_id))
					break;
				
				id = declare_id(scope, tok, type);
				write_decl(id);

				if (tok->next && tok->next->type == tk_eos) {
					tok = tok->next;
					break;
				}

				if (!expect_over(&tok, tok->next, tk_asg))
					break;

				parse_expr(&tok, tok, &expr);
				write_expr(expr);
				write_pop_id(id);
			} break;

			case tk_id: {
				struct ht_item *id = ht_get(scope, tok->s);

				if(id->type < tk_vchr || id->type > tk_vstr)
					goto cont;
				
				if (!step(&tok, tok->next, tk_lbrk))
					goto not_arr;
				
				struct token *expr = NULL;
				tok = tok->next;
				parse_expr(&tok, tok, &expr);
				write_expr(expr);

				if (!expect_over(&tok, tok, tk_rbrk))
					break;
			cont:
				if (!expect(&tok, tok->next, tk_asg))
					break;
				
				parse_expr(&tok, tok->next, &expr);
				write_expr(expr);
				write_pop_id(id);
			} break;

			case tk_if: {
				if (!expect_over(&tok, tok->next, tk_lpar))
					break;

				struct token *expr = NULL;
				parse_expr(&tok, tok, &expr);
				write_expr(expr);

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
					struct token *expr;
					parse_expr(&tok, tok, &expr);
					write_expr(expr);
					writes("WRT");
				} while (tok->type == tk_coma);

				expect_over(&tok, tok, tk_rpar);
			break;

			default: printf("default -> "); print_token(tok); break;
		}

		expect_over(&tok, tok, tk_eos);
	}

	
}

int main(int argc, char **args) {
	source = fopen("example.auto", "r+");
	if (!source)
		error_exit("Invalid source file");
	setvbuf(source, NULL, _IONBF, 0);
	
	out = fopen("asm.we", "w+");
	struct token *tokens = lex();
	printf("Tokens:\n");
	print_tokens(tokens);
	printf("\n");

	parse_scope(tokens);
	return 0;
}
