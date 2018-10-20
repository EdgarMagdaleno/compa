struct ht_item {
	int id;
	struct ht_item *next;
};

struct ht {
	int bucket_size;
	struct ht_item **bucket;
};

struct ht *new_ht(int size);
void ht_add(struct ht *table, struct ht_item *item);
struct ht_item *ht_get(struct ht *table, int id);
void ht_del(struct ht *table, int id);
