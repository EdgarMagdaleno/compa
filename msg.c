#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "source.h"
#include "msg.h"

#define MAX_LINE_SIZE	256

#define C_RED	"\e[1;31m"
#define C_GRN	"\e[1;32m"
#define C_BLU	"\e[1;34m"
#define C_YLW	"\e[1;33m"
#define C_RST	"\e[0m"

void error_exit(const char *msg) {
	printf("[ERROR] %s\n", msg);
	exit(1);
}

void error_log(struct token *tok, int err_code) {
	printf("\n");
	va_list arglist;
	printf(C_RED"[ERROR] %i,%i "C_RST, tok->line, tok->col);
	printf("err_code: %i\n", err_code);

	fseek(source, tok->line_offset, SEEK_SET);
	char line[MAX_LINE_SIZE];
	fgets(line, MAX_LINE_SIZE, source);
	printf("    %s", line);
}

void log_msg(const char *msg) {
	printf("%s", msg);
}
