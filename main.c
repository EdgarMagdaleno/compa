#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

FILE *source, out;
int line = 1, col = 0;

void error_exit(const char *msg) {
	printf("[ERROR] %s\n", msg);
	exit(1);
}

typedef enum {
	tk_char, tk_dobl, tk_else, tk_if,   tk_int,  tk_str,  tk_whle, tk_eos,
	tk_add,  tk_sub,  tk_mul,  tk_div,  tk_mod,  tk_eq,   tk_neq,  tk_grt,
	tk_grte, tk_les,  tk_lese
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
	{"string",	tk_str},
	{"while",	tk_whle},
};

int next_char() {
	int c;
	while (isspace(c = getc(source)))
	col++;

	if (c == '\n') {
		line++;
		col = 0;
	}
	
	return c;
}

struct token *ident_or_kw() {
	return NULL;
}



struct token *get_token() {
	int c = next_char();
	switch(c) {
		case 'a' ... 'z':
		case 'A' ... 'Z':
			
			break;
		case '0' ... '9': return &((struct token) {tk_int, 0, 0, {get_number()}, NULL}); break;
		default: printf("Unrecognized character\n"); break;
/*
        case '}':  next_ch(); return (tok_s){tk_Rbrace, err_line, err_col, {0}};
        case '(':  next_ch(); return (tok_s){tk_Lparen, err_line, err_col, {0}};
        case ')':  next_ch(); return (tok_s){tk_Rparen, err_line, err_col, {0}};
        case '+':  next_ch(); return (tok_s){tk_Add, err_line, err_col, {0}};
        case '-':  next_ch(); return (tok_s){tk_Sub, err_line, err_col, {0}};
        case '*':  next_ch(); return (tok_s){tk_Mul, err_line, err_col, {0}};
        case '%':  next_ch(); return (tok_s){tk_Mod, err_line, err_col, {0}};
        case ';':  next_ch(); return (tok_s){tk_Semi, err_line, err_col, {0}};
        case ',':  next_ch(); return (tok_s){tk_Comma,err_line, err_col, {0}};
        case '/':  next_ch(); return div_or_cmt(err_line, err_col);
        case '\'': next_ch(); return char_lit(the_ch, err_line, err_col);
        case '<':  next_ch(); return follow('=', tk_Leq, tk_Lss,    err_line, err_col);
        case '>':  next_ch(); return follow('=', tk_Geq, tk_Gtr,    err_line, err_col);
        case '=':  next_ch(); return follow('=', tk_Eq,  tk_Assign, err_line, err_col);
*/
	}

	return NULL;
}

void print_token(struct token tok) {
	printf("%.8s",
		&"tk_char tk_dobl tk_else tk_if   tk_int  tk_str  tk_whle tk_eos  "
		 "tk_add  tk_sub  tk_mul  tk_div  tk_mod  tk_eq   tk_neq  tk_grt  "
		 "tk_grte tk_les  tk_lese "[tok.type * 8]
	);
	
	if (tok.type == tk_int) {
		printf("%i", tok.i);
	}

	printf("\n");
}

void lex() {
	struct token *tok = get_token();
	print_token(*tok);
}

int main() {
	source = fopen("example.auto", "r+");
	if (!source)
		error_exit("Invalid source file");

	lex();
	return 0;
}
