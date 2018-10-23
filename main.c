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
char vline[MAX_LINE_SIZE];
int line = 1, col = 0, ch = ' ', vline_i;

void error_exit(const char *msg) {
	printf("[ERROR] %s\n", msg);
	exit(1);
}

void error_log(int line, int col, const char *msg, ...) {
	va_list arglist;
	printf(C_RED"[ERROR]"C_RST);
	va_start(arglist, msg);
	vprintf(msg, arglist);
	va_end(arglist);
	printf(" %i,%i %s\n", line, col, msg);
}

typedef enum {
	tk_char, tk_dobl, tk_int,  tk_str,  tk_else, tk_if,   tk_whle, tk_eos,
	tk_add,  tk_sub,  tk_mul,  tk_div,  tk_mod,  tk_eq,   tk_neq,  tk_grt,
	tk_grte, tk_les,  tk_lese, tk_id, 	tk_lint, tk_ldbl, tk_lchr,  tk_lstr,
	tk_rpar, tk_lbrc, tk_rbrc, tk_coma, tk_asg,  tk_lpar, tk_lbrk,  tk_rbrk,
	tk_prnt
} token_type;

struct token {
	token_type type;
	int line, col;
	union {
		char c;
		int i;
		double d;
		char *s;
	};
	struct token *next;
};

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

char *get_type_name(token_type type) {
	return &"tk_char tk_dobl tk_int  tk_str  tk_else tk_if   tk_whle tk_eos  "			  "tk_add  tk_sub  tk_mul  tk_div  tk_mod  tk_eq   tk_neq  tk_grt  "
		    "tk_grte tk_les  tk_lese tk_id   tk_lint tk_ldbl tk_lchr tk_lstr "
		    "tk_rpar tk_lbrc tk_rbrc tk_coma tk_asg  tk_lpar tk_lbrk tk_rbrk "
			"tk_prnt"[type * 8];
}

void print_token(struct token *tok) {
	printf("%i %i %.8s", tok->line, tok->col, get_type_name(tok->type));
	
	switch (tok->type) {
		case tk_id: printf("%s", tok->s); break;
		case tk_lint: printf("%i" , tok->i); break;
		case tk_ldbl: printf("%lf", tok->d); break;
		case tk_lstr: printf("\"%s\"", tok->s); break;
		case tk_lchr: printf("\'%c\'", tok->c); break;
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
		default: return 0; break;
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

struct token *number() {
	double n;
	struct token *tok = NULL;
	ungetc(ch, source);
	
	if (fscanf(source, "%lf", &n) != 1)
		return NULL;

	tok = malloc(sizeof(struct token));
	if (ceil(n) == n) {
		tok->type = tk_lint;
		tok->i = (int) n;
		return tok;
	}

	tok->type = tk_ldbl;
	tok->d = n;
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
	tok = malloc(sizeof(struct token));
	tok->type = tk_lstr;
	tok->s = malloc(len);
	strncpy(tok->s, buf, len);
	return tok;
}

struct token *char_lit() {
	struct token *tok;
	char c;
	fscanf(source, "%c\'", &c);

	tok = malloc(sizeof(char));
	tok->type = tk_lchr;
	tok->c = c;
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
				while (prec(peek(&output)) >= prec(tok))
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
}

void write_decl(char *name, token_type type) {
	switch(type) {
		case tk_char: fprintf(out, "DCLC %s\n", name); break;
		case tk_int: fprintf(out, "DCLI %s\n", name); break;
		case tk_dobl: fprintf(out, "DCLD %s\n", name); break;
		case tk_str: fprintf(out, "DCLS %s\n", name); break;
	}
}

void declare_id(struct ht *scope, char *name, token_type type) {
	struct ht_item *new_id = malloc(sizeof(struct ht_item));
	new_id->name = name;
	new_id->type = type;
	ht_add(scope, new_id);
}

int step(struct token **tokens, struct token *tok, token_type type) {
	int line = tok->line;
	int col = tok->col;
		error_log(line, col, "Expected %s", get_type_name(type));
	if (!tok || tok->type != type) {
		while (tok && tok->type != tk_eos)
			tok = tok->next;
		
		*tokens = tok;
		return 0;
	}
	
	*tokens = tok;
	return 1;
}

int step_over(struct token **tokens, struct token *tok, token_type type) {
	if (!tok || tok->type != type) {
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
			case tk_lchr: writes("PUSHKC \'%c\'", expr->c); break;
			case tk_lint: writes("PUSHKI %i", expr->i); break;
			case tk_ldbl: writes("PUSHKD %lf", expr->d); break;
			case tk_lstr: writes("PUSHKS \"%s\"", expr->s); break;
			case tk_id:
			
			break;
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

void write_pop_id(char *name, token_type type) {
	switch(type) {
		case tk_int: writes("POPI %s", name); break;
		case tk_char: writes("POPC %s", name); break;
		case tk_dobl: writes("POPD %s", name); break;
		case tk_str: writes("POPS %s", name); break;
	}
}

int parse_scope(struct token *tokens) {
	struct token *tok = tokens;
	struct ht *scope = new_ht(HT_BUCKET_SIZE);

	while (tok) {
		switch(tok->type) {
			case tk_char ... tk_str: {
				token_type type = tok->type;
				if(!step(&tok, tok->next, tk_id))
					break;
				
				char *name = tok->s;
				write_decl(name, type);
				declare_id(scope, name, type);

				if (!step_over(&tok, tok->next, tk_asg))
					error_exit("expected tk_asg");

				struct token *expr = NULL;
				parse_expr(&tok, tok, &expr);
				write_expr(expr);
				write_pop_id(name, type);
			} break;

			case tk_if:
			break;

			case tk_rbrk:
				
			break;

			case tk_prnt: {
				struct token *expr;
				parse_expr(&tok, tok->next, &expr);
				write_expr(expr);
				writes("WRT");
			} break;

			default: printf("default\n"); print_token(tok); break;
		}

		step_over(&tok, tok, tk_eos);
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
