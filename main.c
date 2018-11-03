#include <stdio.h>
#include "source.h"
#include "out.h"
#include "ht.h"
#include "token.h"
#include "ast.h"
#include "lexer.h"
#include "writer.h"
#include "parser.h"

int main(int argc, char **args) {
	set_source("example.auto");
	set_out("asm.we");
	
	struct token *tokens = lex();
	print_tokens(tokens);
	struct ast_node *root = new_tree("tree.txt");
	printf("root = %p\n", root);
	parse(tokens, root);
	return 0;
}
