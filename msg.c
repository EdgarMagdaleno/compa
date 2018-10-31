#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "msg.h"

#define C_RED	"\e[1;31m"
#define C_GRN	"\e[1;32m"
#define C_BLU	"\e[1;34m"
#define C_YLW	"\e[1;33m"
#define C_RST	"\e[0m"

void error_exit(const char *msg) {
	printf("[ERROR] %s\n", msg);
	exit(1);
}

void error_log(int line, int col, const char *msg, ...) {
	va_list arglist;
	printf(C_RED"[ERROR] %i,%i "C_RST, line, col);
	va_start(arglist, msg);
	vprintf(msg, arglist);
	va_end(arglist);
	printf("\n");
}

void log_msg(const char *msg) {
	printf("%s", msg);
}