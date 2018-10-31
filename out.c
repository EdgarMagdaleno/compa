#include <stdio.h>
#include "msg.h"
#include "out.h"

FILE *out = NULL;

void set_out(char *name) {
	out = fopen(name, "r+");
	if (!out)
		error_exit("Invalid out file");
}