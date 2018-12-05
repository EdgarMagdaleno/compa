#include "ht.h"
#include "msg.h"
#include "writer.h"

struct ht *new_ht(int size) {
	struct ht *table = malloc(sizeof(struct ht));
	table->bucket_size = size;
	table->while_label = -1;
	table->if_label = -1;
	table->for_label = -1;
	table->for_end = -1;
	table->for_stm = NULL;
	table->bucket = calloc(table->bucket_size, sizeof(struct ht_item *));

	return table;
}

void ht_add(struct ht *table, struct ht_item *item) {
	int index = hash(item->name) % table->bucket_size;
	struct ht_item *stop = table->bucket[index];

	if (!stop)
		table->bucket[index] = item;
	else {
		while (stop->next)
			stop = stop->next;

		stop->next = item;
	}
}

struct ht_item *ht_get(struct ht *table, char *name) {
	int index = hash(name) % table->bucket_size;
	struct ht_item *current = table->bucket[index];

	while (current) {
		if (strcmp(current->name, name) == 0)
			break;
		else
			current = current->next;
	}

	return current;
}

void ht_del(struct ht *table, char *name) {
	int index = hash(name) % table->bucket_size;
	struct ht_item *current = table->bucket[index];

	log_msg("\tht_del: %s\n", name);
	if (current && strcmp(current->name, name) == 0) {
		table->bucket[index] = current->next;
		return;
	}

	while (current->next) {
		if (strcmp(current->next->name, name) == 0) {
			current->next = current->next->next;
			break;
		}

		current = current->next;
	}

}

unsigned long hash(char *name) {
	unsigned long hash = 5381;
	int c;

	while (c = *name++)
		hash = ((hash << 5) + hash) + c;

	return hash;
}

struct ht_item *first(struct ht *table) {
	for (int i = 0; i < table->bucket_size; i++) {
		if (table->bucket[i])
			return table->bucket[i];
	}
		
	return NULL;
}

struct ht_item *next(struct ht *table, struct ht_item *current) {
	if (!current)
		return NULL;

	if (current->next)
		return current->next;

	int index = hash(current->name) % table->bucket_size;
	while (++index < table->bucket_size) {	
		current = table->bucket[index];
		if (current)
			return current;
	}

	return NULL;
}

void clear_scope(struct ht *scope, int scope_id) {
	log_msg("\nclear_scope\n\n");
	struct ht_item *item = first(scope);
	struct ht_item *_next;

	while (item) {
		_next = next(scope, item);
		if (item->scope_id == scope_id) {
			ht_del(scope, item->name);
			writes("FREE %s", item->name);
		}
		item = _next;
	}
}