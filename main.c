#include <stdio.h>
#include <stdlib.h>

#define MAX_BUF_SIZE 255

void error_exit(const char *msg) {
	printf("[ERROR] %s\n", msg);
	exit(1);
}

typedef enum {
	tk_int, tk_kint, tk_mul, tk_div, tk_add, tk_sub, tk_opar, tk_cpar, tk_eos
} token_type;

struct token {
	token_type type;
	union {
		char c;
		int i;
		double d;
		char *s;
	};
	struct token *next;
};

struct token *lex(

int main(int argc, char **args) {
	FILE *fp = fopen("ast.txt", "r+");
	if (!fp)
		error_exit("Invalid code file");

	struct token *tokens = lex(fp);
	struct token *tok = tokens;
	while (tok) {
		printf("%.15s",
			"tk_int  tk_kint tk_mul  tk_div  tk_add  tk_sub  tk_opar tk_cpar tk_eos  "[tok.type * 5]
		);
		tok = tok->next;
	}
	return 0;
}
