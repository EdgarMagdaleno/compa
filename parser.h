#ifndef _PARSER_H_
#define _PARSER_H_

int prec(struct token *tok);
void push(struct token **stack, struct token *tok);
struct token *pop(struct token **stack);
struct token *peek(struct token **stack);
void chain(struct token **list, struct token **last, struct token *tok);
void parse_expr(struct token **tokens, struct token *tok);
struct token *parse_scope(struct token *tokens);
void parse(struct token *tokens, struct ast_node *tree);
int asgn();
int decl();
int eos();
int eosp();
int expr();
int iden();
int prog();
int vind();

#endif
