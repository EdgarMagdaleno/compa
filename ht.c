#include <stdlib.h>
#include <string.h>
#include "ht.h"

struct ht *new_ht(int size) {
	struct ht *table = malloc(sizeof(struct ht));
	table->bucket_size = size;
	table->bucket= calloc(table->bucket_size, sizeof(struct ht_item *));

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
