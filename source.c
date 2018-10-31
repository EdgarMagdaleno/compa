#include <stdio.h>
#include "msg.h"
#include "source.h"

FILE *source = NULL;

void set_source(char *name) {
	source = fopen(name, "r+");
	if (!source)
		error_exit("Invalid source file");
	setvbuf(source, NULL, _IONBF, 0);
}