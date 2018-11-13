#include "writer.h"

int err = 0;

void writes(const char *msg, ...) {
	if (err) return;

	va_list arglist;
	va_start(arglist, msg);
	vfprintf(out, msg, arglist);
	va_end(arglist);
	fprintf(out, "\n");
}

void writes_decl(struct ht_item *id) {
	if (err) return; 

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

void writes_pop_id(struct ht_item *id) {
	if (err) return;

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

void writes_push(struct ht_item *id) {
	if (err) return;

	switch(id->type) {
		case tk_char ... tk_str: writes("PUSH %s", id->name); break;
		case tk_vchr ... tk_vstr: writes("PUSHV %s", id->name); break;
	}
}

void writes_id(struct ht_item *id) {
	if (is_vector(id->type))
		writes("PUSHV %s", id->name);
	else
		writes("PUSH %s", id->name);
}

void writes_token(struct token *tok) {
	if (err) return;

	switch(tok->type) {
		case tk_lchr: writes("PUSHKC \'%s\'", tok->s); break;
		case tk_lint: writes("PUSHKI %i", tok->i); break;
		case tk_ldbl: writes("PUSHKD %lf", tok->d); break;
		case tk_lstr: writes("PUSHKS \"%s\"", tok->s); break;
		case tk_add: writes("ADD"); break;
		case tk_sub: writes("SUB"); break;
		case tk_mul: writes("MUL"); break;
		case tk_div: writes("DIV"); break;
		case tk_eq: writes("CEQ"); break;
		case tk_neq: writes("CNE"); break;
		case tk_grt: writes("CGT"); break;
		case tk_grte: writes("CGE"); break;
		case tk_les: writes("CLT"); break;
		case tk_lese: writes("CLE"); break;
	}
}
