#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "source.h"
#include "writer.h"
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
	err = 0;
	printf("\n");
	va_list arglist;
	printf(C_RED"[ERROR] %i,%i "C_RST, tok->line, tok->col);
	if (err_code >= 0)
		printf("expected %.8s\n", get_type_name(err_code));
	else {
		printf("NIGGGEEER");
	}

	fseek(source, tok->line_offset, SEEK_SET);
	char line[MAX_LINE_SIZE];
	fgets(line, MAX_LINE_SIZE, source);
	printf("    %s", line);
	fflush(stdout);
}

void log_msg(const char *msg, ...) {
	va_list arglist;
	va_start(arglist, msg);
	vprintf(msg, arglist);
	va_end(arglist);

	fflush(stdout);
}
