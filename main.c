#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#define MAX_WORD_SIZE 255

FILE *source, *out;
int line = 1, col = 0, ch = ' ';

void error_exit(const char *msg) {
	printf("[ERROR] %s\n", msg);
	exit(1);
}

typedef enum {
	tk_char, tk_dobl, tk_int,  tk_str,  tk_else, tk_if,   tk_whle, tk_eos,
	tk_add,  tk_sub,  tk_mul,  tk_div,  tk_mod,  tk_eq,   tk_neq,  tk_grt,
	tk_grte, tk_les,  tk_lese, tk_id, 	tk_lint, tk_ldbl, tk_asg,  tk_lpar,
	tk_rpar, tk_lbrc, tk_rbrc, tk_coma, tk_lstr, tk_lchr
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
}, *kw;

void print_tokens(struct token *tokens) {
	struct token *tok = tokens;

	while (tok) {
		printf("%i %i %.8s", tok->line, tok->col,
			&"tk_char tk_dobl tk_else tk_if   tk_int  tk_str  tk_whle tk_eos  "
			 "tk_add  tk_sub  tk_mul  tk_div  tk_mod  tk_eq   tk_neq  tk_grt  "
			 "tk_grte tk_les  tk_lese tk_id   tk_cint tk_cdbl tk_asg  tk_lpar "
			 "tk_rpar tk_lbrc tk_rbrc tk_coma tk_lstr tk_lchr "[tok->type * 8]
		);
		
		switch (tok->type) {
			case tk_id: printf("%s", tok->s); break;
			case tk_lint: printf("%i" , tok->i); break;
			case tk_ldbl: printf("%lf", tok->d); break;
			case tk_lstr: printf("\"%s\"", tok->s); break;
			case tk_lchr: printf("\'%c\'", tok->c); break;
			default: break;
		}

		printf("\n");
		tok = tok->next;
	}
}

struct token *new_token(token_type type) {
	struct token *tok = malloc(sizeof(struct token));
	tok->type = type;

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

void parse(struct token *tokens) {
	struct token *tok = tokens;

	while (tok) {
		switch(tok->type) {
			case tk_char ... tk_str:
				
			break;
		}
		tok = tok->next;
	}
}

int main() {
	source = fopen("example.auto", "r+");
	if (!source)
		error_exit("Invalid source file");
	setvbuf(source, NULL, _IONBF, 0);

	struct token *tokens = lex();
	print_tokens(tokens);
	parse(tokens);
	return 0;
}
