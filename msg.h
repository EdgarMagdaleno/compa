#ifndef _MSG_H_
#define _MSG_H_

void error_exit(const char *msg);
void error_log(int line, int col, const char *msg, ...);
void log_msg(const char *msg);

#endif