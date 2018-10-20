#include <stdlib.h>
#include "ht.h"

struct ht *new_ht(int size) {
	struct ht *table = malloc(sizeof(struct ht));
	table->bucket_size = size;
	table->bucket= calloc(table->bucket_size, sizeof(struct ht_item *));

	return table;
}

void ht_add(struct ht *table, struct ht_item *item) {
	int index = item->id % table->bucket_size;
	struct ht_item *stop = table->bucket[index];

	if (!stop)
		table->bucket[index] = item;
	else {
		while (stop->next)
			stop = stop->next;

		stop->next = item;
	}
}

struct ht_item *ht_get(struct ht *table, int id) {
	int index = id % table->bucket_size;
	struct ht_item *current = table->bucket[index];

	while (current) {
		if (id == current->id)
			break;
		else
			current = current->next;
	}

	return current;
}

void ht_del(struct ht *table, int id) {
	int index = id % table->bucket_size;
	struct ht_item *current = table->bucket[index];

	if (current && current->id == id) {
		table->bucket[index] = current->next;
		return;
	}

	while (current->next) {
		if (current->next->id == id) {
			current->next = current->next->next;
			break;
		}

		current = current->next;
	}
}
