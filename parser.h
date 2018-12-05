#ifndef _PARSER_H_
#define _PARSER_H_

int prec(struct token *tok);
void push(struct token **stack, struct token *tok);
struct token *pop(struct token **stack);
struct token *top(struct token **stack);
void chain(struct token **list, struct token **last, struct token *tok);
void parse_expr(struct token **tokens, struct token *tok);
struct token *parse_scope(struct token *tokens);
void start_parse(struct token *tokens, struct ast_node *tree);
int parse_statement(struct ast_node *node);
int scpe();
int ifs();
int asgn();
int decl();
int idn();
int eos();
int eosr();
int eosp();
int exrl();
int lpar();
int rpar();
int prnt();
int expr();
int prog();
int lbrk();
int rbrk();
int lbrc();
int rbrc();
int whle();
int read();
int _eosp();
int _for();
int f_cnd();
int for_end();
int for_j();
int for_st();

#endif
