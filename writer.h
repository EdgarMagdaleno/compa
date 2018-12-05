#ifndef _WRITER_H_
#define _WRITER_H_

#include <stdarg.h>
#include <stdio.h>
#include "ht.h"
#include "out.h"
#include "token.h"

extern int err;

void writes(const char *msg, ...);
void writes_id(struct ht_item *id);
void writes_decl(struct ht_item *id);
void writes_pop_id(struct ht_item *id);
void writes_push(struct ht_item *id);
void writes_token(struct token *tok);
void writes_label(int label);

#endif
