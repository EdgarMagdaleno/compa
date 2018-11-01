#ifndef _LEXER_H_
#define _LEXER_H_

struct token *ident_or_kw();
struct token *char_lit();
struct token *string_lit();
struct token *new_token(token_type type);
struct token *follow(token_type normal, token_type follow, char next);
struct token *lex();
int keyword_comp(const void *a, const void *b);

#endif