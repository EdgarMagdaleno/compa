#ifndef _WRITER_H_
#define _WRITER_H_

extern int err;

void writes(const char *msg, ...);
void writes_decl(struct ht_item *id);
void writes_pop_id(struct ht_item *id);
void writes_push(struct ht_item *id);

#endif
