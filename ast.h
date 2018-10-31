#ifndef _AST_H_
#define _AST_H_

struct ast_node {
	int (*node)(struct token **current);
	int branches_size;
	struct ast_node **branches;
};

struct ast_node *build_tree(FILE *tree);
struct ast_node *new_tree(char *name);

#endif