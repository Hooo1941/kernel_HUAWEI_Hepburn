#ifndef DUBAI_HASHTABLE_H
#define DUBAI_HASHTABLE_H

#include <linux/types.h>
#include <linux/hashtable.h>

#define dubai_dynamic_hash_bits(htable)		((htable)->bits)
#define dubai_dynamic_hash_size(htable)		BIT(dubai_dynamic_hash_bits(htable))
#define dubai_dynamic_hash_empty(htable)	__hash_empty((htable)->head, dubai_dynamic_hash_size(htable))
#define dubai_dynamic_hash_add(htable, node, key) \
	hlist_add_head(node, &(htable)->head[hash_min(key, dubai_dynamic_hash_bits(htable))])
#define dubai_dynamic_hash_for_each(htable, bkt, obj, member) \
	for ((bkt) = 0, (obj) = NULL; (obj) == NULL && (bkt) < dubai_dynamic_hash_size(htable); (bkt)++) \
		hlist_for_each_entry((obj), &(htable)->head[bkt], member)
#define dubai_dynamic_hash_for_each_safe(htable, bkt, tmp, obj, member) \
	for ((bkt) = 0, (obj) = NULL; (obj) == NULL && (bkt) < dubai_dynamic_hash_size(htable); (bkt)++) \
		hlist_for_each_entry_safe((obj), (tmp), &(htable)->head[bkt], member)
#define dubai_dynamic_hash_for_each_possible(htable, obj, member, key) \
	hlist_for_each_entry((obj), &(htable)->head[hash_min(key, dubai_dynamic_hash_bits(htable))], member)

struct dubai_dynamic_hashtable {
	uint32_t bits;
	struct hlist_head *head;
};

struct dubai_dynamic_hashtable *dubai_alloc_dynamic_hashtable(uint32_t bits);
void dubai_free_dynamic_hashtable(struct dubai_dynamic_hashtable *htable);

#endif // DUBAI_HASHTABLE_H
