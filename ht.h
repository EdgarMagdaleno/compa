#ifndef _HT_H_
#define _HT_H_

#include <stdlib.h>
#include <string.h>

struct ht_item {
	char *name;
	int type;
	int scope_id;
	struct ht_item *next;
};

struct ht {
	int bucket_size;
	int if_label;
	int while_label;
	int for_label;
	int for_cond;
	int for_end;
	void *for_stm;
	struct ht_item **bucket;
};

struct ht *new_ht(int size);
void ht_add(struct ht *table, struct ht_item *item);
struct ht_item *ht_get(struct ht *table, char *name);
void ht_del(struct ht *table, char *name);
unsigned long hash(char *name);
struct ht_item *first(struct ht *table);
struct ht_item *next(struct ht *table, struct ht_item *current);
void clear_scope(struct ht *table, int scope_id);

#endif
