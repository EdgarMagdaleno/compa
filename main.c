#include <stdio.h>
#include "msg.h"
#include "ht.h"
#include "token.h"
#include "ast.h"
#include "lexer.h"
#include "writer.h"
#include "parser.h"

int main(int argc, char **args) {
	FILE *source = fopen("example.auto", "r+");
	if (!source)
		error_exit("Invalid source file");
	setvbuf(source, NULL, _IONBF, 0);
	
	struct token *tokens = lex(source);
	printf("- Tokens\n");
	print_tokens(tokens);
	printf("\n");

	//parse_scope(tokens);
	return 0;
}
