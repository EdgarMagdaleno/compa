#include <stdio.h>
#include "msg.h"
#include "out.h"

FILE *out = NULL;

void set_out(char *name) {
	out = fopen(name, "w+");
	if (!out)
		error_exit("Invalid out file");
}
