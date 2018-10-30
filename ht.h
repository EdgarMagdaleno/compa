struct ht_item {
	char *name;
	int type;
	struct ht_item *next;
};

struct ht {
	int bucket_size;
	struct ht_item **bucket;
};

struct ht *new_ht(int size);
void ht_add(struct ht *table, struct ht_item *item);
struct ht_item *ht_get(struct ht *table, char *name);
void ht_del(struct ht *table, char *name);
unsigned long hash(char *name);
struct ht_item *first(struct ht *table);
struct ht_item *next(struct ht *table, struct ht_item *current);
