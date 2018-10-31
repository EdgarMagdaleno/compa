#include <stdio.h>
#include <stdlib.h>
#include "token.h"
#include "ast.h"
#include "msg.h"

int scope(struct token **current) {

}

struct ast_node *build_tree(FILE *tree) {
	char buf[8];
	int n;

	fscanf(tree, "%s %i", buf, &n);
	printf("%s\n", buf);
	struct ast_node *root = malloc(sizeof(struct ast_node));
	root->branches_size = n;
	root->branches = malloc(sizeof(struct ast_node));

	for (int i = 0; i < root->branches_size; i++)
		root->branches[i] = build_tree(tree);
}

struct ast_node *new_tree(char *name) {
	FILE *tree = fopen(name, "r+");
	if (!tree)
		error_exit("Invalid tree file");

	printf("- Tree\n");
	struct ast_node *root = build_tree(tree);
	printf("\n");
}