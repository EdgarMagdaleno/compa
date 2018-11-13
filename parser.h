#ifndef _PARSER_H_
#define _PARSER_H_

int prec(struct token *tok);
void push(struct token **stack, struct token *tok);
struct token *pop(struct token **stack);
struct token *top(struct token **stack);
void chain(struct token **list, struct token **last, struct token *tok);
void parse_expr(struct token **tokens, struct token *tok);
struct token *parse_scope(struct token *tokens);
void parse(struct token *tokens, struct ast_node *tree);
int preorder(struct ht *node, struct ht *scope);
int scpe(struct ht *scope);
int ifs(struct ht *scope);
int asgn(struct ht *scope);
int decl(struct ht *scope);
int idn(struct ht *scope);
int eos(struct ht *scope);
int eosp(struct ht *scope);
int exrl(struct ht *scope);
int lpar(struct ht *scope);
int rpar(struct ht *scope);
int prnt(struct ht *scope);
int expr(struct ht *scope);
int prog(struct ht *scope);
int lbrk(struct ht *scope);
int rbrk(struct ht *scope);

#endif
