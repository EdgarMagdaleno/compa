#ifndef _AST_H_
#define _AST_H_

typedef int (*parser_f)();

struct ast_node {
	parser_f parse;
	int branches_size;
	struct ast_node **branches;
};

struct ast_node *build_tree(FILE *tree, int level);
struct ast_node *new_tree(char *name);
int tree_link_comp(const void *a, const void *b);

#endif
