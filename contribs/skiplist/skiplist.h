#ifndef _SKIPLIST_H
#define _SKIPLIST_H

typedef int (*skiplist_cmp_fn)(const void *, const void *);

typedef struct skipNode *skipnode;

#ifdef WIN32
struct skipPointer {
	skipnode ptr;
	int distance;
};
#else
struct skipPointer {
	skipnode ptr;
	int distance;
} __attribute__((__packed__)); /* or gcc adds 4 bytes of padding on amd64 */
#endif

struct skipNode {
	void *item;
	skipnode prev; /* single link back */
	skipnode next; /* single link forward, level "-1" */
	struct skipPointer forward[0]; /* k-1 links forward + size */
};

typedef struct skipList {
	int level;
	int size;
	skiplist_cmp_fn cmp;

	skipnode sentinal;
	skipnode header;

	skipnode *gc;
	int gc_limit;
	int gc_used;

	int rand;
	char randleft;
} *skiplist;

#define RAND_BITS 31
#define MAX_NUMBER_OF_LEVELS 16
#define MAX_LEVEL (MAX_NUMBER_OF_LEVELS - 1)
#define SKIPLIST_GC_NODES 512

#define skipnode_item(n) (n ? n->item : NULL)

skiplist skiplist_create(skiplist_cmp_fn cmp, void *max);
void skiplist_destroy(skiplist l);
int skiplist_size(skiplist l);

skipnode skiplist_first(skiplist l);
skipnode skiplist_last(skiplist l);
skipnode skiplist_next(skiplist l, skipnode n);
skipnode skiplist_prev(skiplist l, skipnode n);

void skiplist_insert(skiplist l, void *item);
int skiplist_delete(skiplist l, void *item);
int skiplist_delete_gc(skiplist l, void *item);
void skiplist_gc(skiplist l);

skipnode skiplist_search(skiplist l, void *item);
skipnode skiplist_search_exact(skiplist l, void *item);
skipnode skiplist_at(skiplist l, int index);
void *skiplist_fetch(skiplist l, void *item);
void *skiplist_fetch_exact(skiplist l, void *item);
void *skiplist_fetch_at(skiplist l, int index);

#define SKIPLIST_FOREACH(l,si)                                                 \
	for ((si) = skiplist_first((l));                                       \
	     (si);                                                             \
	     (si) = skiplist_next((l), (si)))

#define SKIPLIST_FOREACH_REVERSE(l,si)                                         \
	for ((si) = skiplist_last((l));                                        \
	     (si);                                                             \
	     (si) = skiplist_prev((l), (si)))

#define SKIPLIST_FOREACH_SAFE(l,si,st)                                         \
	for ((si) = skiplist_first(l);                                         \
	     (si) && ((st) = skiplist_next((l), (si)),1);                      \
	     (si) = (st))

#define SKIPLIST_FOREACH_REVERSE_SAFE(l,si,st)                                 \
	for ((si) = skiplist_last(l);                                          \
	     (si) && ((st) = skiplist_prev((l), (si)),1);                      \
	     (si) = (st))

#endif