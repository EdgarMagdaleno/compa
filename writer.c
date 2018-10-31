#include <stdarg.h>
#include <stdio.h>
#include "token.h"
#include "ht.h"
#include "writer.h"
#include "out.h"

void writes(const char *msg, ...) {
	va_list arglist;
	va_start(arglist, msg);
	vfprintf(out, msg, arglist);
	va_end(arglist);
	fprintf(out, "\n");
}

void write_decl(struct ht_item *id) {
	switch(id->type) {
		case tk_char: fprintf(out, "DCLC %s\n", id->name); break;
		case tk_int: fprintf(out, "DCLI %s\n", id->name); break;
		case tk_dobl: fprintf(out, "DCLD %s\n", id->name); break;
		case tk_str: fprintf(out, "DCLS %s\n", id->name); break;
		case tk_vchr: fprintf(out, "DCLVC %s\n", id->name); break;
		case tk_vint: fprintf(out, "DCLVI %s\n", id->name); break;
		case tk_vdbl: fprintf(out, "DCLVD %s\n", id->name); break;
		case tk_vstr: fprintf(out, "DCLVS %s\n", id->name); break;
	}
}

void write_pop_id(struct ht_item *id) {
	switch(id->type) {
		case tk_char: writes("POPC %s", id->name); break;
		case tk_int: writes("POPI %s", id->name); break;
		case tk_dobl: writes("POPD %s", id->name); break;
		case tk_str: writes("POPS %s", id->name); break;
		case tk_vchr: writes("POPVC %s", id->name); break;
		case tk_vint: writes("POPVI %s", id->name); break;
		case tk_vdbl: writes("POPSVD %s", id->name); break;
		case tk_vstr: writes("POPSVS %s", id->name); break;
	}
}