#ifndef _AST_H_
#define _AST_H_

typedef int (*parser_f)(struct token **);

struct ast_node {
	parser_f parser;
	int branches_size;
	struct ast_node **branches;
};

struct ast_node *build_tree(FILE *tree, int level);
struct ast_node *new_tree(char *name);
int asgn(struct token **current);
int decl(struct token **current);
int eost(struct token **current);
int expr(struct token **current);
int iden(struct token **current);
int prog(struct token **current);
int vind(struct token **current);
int tree_link_comp(const void *a, const void *b);

#endif