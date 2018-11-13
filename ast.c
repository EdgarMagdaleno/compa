#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "token.h"
#include "ht.h"
#include "ast.h"
#include "msg.h"
#include "parser.h"

int tree_link_comp(const void *a, const void *b) {
	return strcmp((char *) a, *(char **) b);
}

parser_f get_parser(char *name) {
	static struct {
		char *name;
		parser_f parser;
	} links[] = {
		{"asgn", asgn},
		{"decl", decl},
		{"eos", eos},
		{"eosp", eosp},
		{"expr", expr},
		{"exrl", exrl},
		{"idn", idn},
		{"lbrk", lbrk},
		{"lpar", lpar},
		{"prnt", prnt},
		{"prog", prog},
		{"rbrk", rbrk},
		{"rpar", rpar},
		{"scpe", scpe}
	}, *link;

	link = bsearch(name, links, sizeof(links) / sizeof(links[0]),
				   sizeof(links[0]), tree_link_comp);
	if (!link)
		error_exit("Invalid tree node");

	return link->parser;
}

struct ast_node *build_tree(FILE *tree, int level) {
	char buf[8];
	int n;
	fscanf(tree, "%s %i", buf, &n);

//	for (int i = 0; i < level; i++)
//		printf("    ");

	struct ast_node *root = malloc(sizeof(struct ast_node));
	root->parse = get_parser(buf);
//	printf("%s\n", buf);

	root->branches_size = n;
	root->branches = malloc(sizeof(struct ast_node));
	for (int i = 0; i < root->branches_size; i++)
		root->branches[i] = build_tree(tree, level + 1);
	return root;
}

struct ast_node *new_tree(char *name) {
	FILE *tree = fopen(name, "r+");
	if (!tree)
		error_exit("Invalid tree file");

//	printf("- Tree\n");
	struct ast_node *root = build_tree(tree, 0);
//	printf("\n");
	return root;
}
