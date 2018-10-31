#ifndef _WRITER_H_
#define _WRITER_H_

void writes(const char *msg, ...);
void write_decl(struct ht_item *id);
void write_pop_id(struct ht_item *id);

#endif