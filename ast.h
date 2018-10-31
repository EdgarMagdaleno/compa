#ifndef _AST_H_
#define _AST_H_

struct ast_node {
	int (*node)(struct token **current);
	int branches_size;
	struct ast_node *branches;
};

#endif