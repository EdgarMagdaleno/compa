#ifndef _MSG_H_
#define _MSG_H_

#include "token.h"

void error_exit(const char *msg);
void error_log(struct token *tok, int err_code);
void log_msg(const char *msg, ...);

#endif
