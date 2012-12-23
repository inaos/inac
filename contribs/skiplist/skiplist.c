/*
 * skiplist.c
 *
 * Copyright (c) 2008, Thomas Hurst <tom@hur.st>
 *
 * Based on the original skipList.c at ftp://ftp.cs.umd.edu/pub/skipLists/
 *
 * Some important differences from the traditional skiplist:
 *
 *  o Duplicates are always allowed.
 *  o There are no seperate keys stored in the list; just a void *value
 *    we assume is comparable.  DO NOT CHANGE KEYS WITHOUT DELETE + INSERT!
 *  o Comparisons are driven by a function passed to skiplist_create
 *  o The sentinal value is provided by the user.
 *  o Deletes operate by reference, not FIFO.
 *  o Items can also be accessed by their index in the list.
 *
 * Average storage cost is 3.33333 pointers + 0.33333 ints per node, expected
 * search costs are O(log n), and iteration from any node is just 1 dereference
 * and a pointer comparison.
 *
 * Basic usage:
 *
 * // Return <0 if a < b, 0 if a == b and >0 if a > b
 * static int IntCmp(const void *a, const void *b) {
 *         return *(const int *)a - *(const int *)b;
 * }
 * 
 * ...
 *
 * int max = 0x7fffffff;
 * int value = 42;
 * int othervalue = 42;
 * skipnode n,tmp;
 * skiplist l = skiplist_create(IntCmp, &max);
 *
 * skiplist_lock(l); // If MT
 * skiplist_insert(l, &value);
 * skiplist_unlock(l);
 *
 * printf("Skiplist contains %d items\n", skiplist_size(l));
 *
 * n = skiplist_search(l, &othervalue);  // Returns NULL if not found
 * n = skiplist_search_exact(l, &value); // Find only with address of value
 * n = skiplist_at(l, 0);                // Find at position 0
 * printf("value = %d\n", *(int *)skipnode_item(n)); // or just n->item;
 *
 * SKIPLIST_FOREACH(l,n)
 * 	printf("value = %d\n", *(int *)skipnode_item(n));
 *
 * SKIPLIST_FOREACH_SAFE(l,n,tmp)
 * 	skiplist_delete(l,n);
 *
 * skiplist_gc(l); // free() all deleted nodes; _delete doesn't to aid concurrency
 *
 * skiplist_destroy(l); // Will do the equivilent of the above if necessary
 */

#ifdef NDEBUG
# undef NDEBUG
#endif

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "skiplist.h"

#ifdef SKIPLIST_DEBUG
#define D(...) fprintf(stdout, __VA_ARGS__)
#else
#define D(...)
#endif

static skipnode
new_node(int level)
{
	int ns = sizeof(struct skipNode) + (sizeof(struct skipPointer) * (level + 1));
	skipnode sn = (skipnode)malloc(ns);
	assert(sn);
	return sn;
}

skiplist
skiplist_create(skiplist_cmp_fn cmp, void *max)
{
	int i;
	skiplist l;

	l = (skiplist)malloc(sizeof(struct skipList));
	assert(l);

	l->gc = malloc(sizeof(skipnode) * SKIPLIST_GC_NODES);
	assert(l->gc);
	l->gc_limit = SKIPLIST_GC_NODES;
	l->gc_used = 0;

	l->sentinal = new_node(-1);
	l->sentinal->item = max;

	l->level = -1;
	l->cmp = cmp;
	l->randleft = RAND_BITS / 2;
#ifdef WIN32
	l->rand = rand();
#else
	l->rand = random();
#endif
	l->header = new_node(MAX_NUMBER_OF_LEVELS);

	l->header->prev = l->sentinal;
	l->sentinal->prev = l->sentinal;
	l->header->next = l->sentinal;

	for (i=0; i < MAX_NUMBER_OF_LEVELS; i++)
	{
		l->header->forward[i].ptr = l->sentinal;
		l->header->forward[i].distance = 0;
	}

	l->size = 0;

	return l;
}

void
skiplist_destroy(skiplist l)
{
	skipnode si,st;

	SKIPLIST_FOREACH_SAFE(l,si,st)
#ifdef SKIPLIST_DEBUG
	{
		assert(skiplist_delete(l, si->item));
		skiplist_gc(l);
	}

	assert(skiplist_size(l) == 0);

#else
		free(si);
#endif

	free(l->gc);
	free(l->header);
	free(l->sentinal);
	free(l);
}

static int
rand_level(skiplist l)
{
	int level = -1;
	int b;
	do {
		b = l->rand & 3; /* 25% of the time, go up a level */
		if (!b) level++;
		l->rand >>= 2;

		if (--l->randleft == 0)
		{
#ifdef WIN32
			l->rand = rand();
#else
			l->rand = random();
#endif
			l->randleft = RAND_BITS / 2;

		}
	} while (!b);
	return(level > MAX_LEVEL ? MAX_LEVEL : level);
}

void
skiplist_insert(skiplist l, void *item)
{
	skipnode update[MAX_NUMBER_OF_LEVELS];
	int updatepos[MAX_NUMBER_OF_LEVELS];
	int level, newlevel, pos;
	skipnode prev,next,vnew;

	// Upgrade the skiplist level if necessary.
	newlevel = rand_level(l);
	if (newlevel > l->level)
	{
		for (pos = l->level + 1; pos <= newlevel; pos++)
		{
			l->header->forward[pos].ptr = l->sentinal;
			l->header->forward[pos].distance = l->size + 1;
		}
		l->level = newlevel; // keep level update atomic
	}

	pos = 0;
	level = l->level;
	prev = l->header;
	while (level >= 0)
	{
		updatepos[level] = pos;
		while (next = prev->forward[level].ptr, l->cmp(next->item, item) < 0)
		{
			pos += prev->forward[level].distance;
			updatepos[level] += prev->forward[level].distance;
			prev = next;
		}

		update[level] = prev;
		level--;
	}

	while (next = prev->next, l->cmp(next->item, item) < 0)
	{
		pos++;
		prev = next;
	}

	// We now have our place.  We could check for the existance of dupes here
	// and abort if necessary; note we may have updated l->level with now
	// pointless entries at this point.

	vnew = new_node(newlevel);
	vnew->item = item;

	// Setup for level -1
	vnew->next = prev->next;
	// Avoid vnew->prev = prev unless you don't want l->sentinal to be head->next->prev.
	vnew->prev = vnew->next->prev;

	// Safe for insert now, we're fully linked at -1.
	vnew->next->prev = vnew;
	prev->next = vnew;

	// Insert at level 0, 1, 2 .. n, in that order.
	for (level = 0; level <= l->level; level++)
	{
		if (level > newlevel)
		{
			update[level]->forward[level].distance++;
		}
		else
		{
			prev = update[level];
			next = prev->forward[level].ptr;

			vnew->forward[level].ptr = next;
			prev->forward[level].ptr = vnew; // insert to level

			vnew->forward[level].distance = updatepos[level] + (prev->forward[level].distance - pos);
			prev->forward[level].distance = pos + 1 - updatepos[level];
		}
	}

	l->size++;
}

int
skiplist_delete(skiplist l, void *item)
{
	skipnode update[MAX_NUMBER_OF_LEVELS];
	int level,offset = 0;
	skipnode prev,next,old,oldprev;

	level = l->level;
	prev = l->header;
	while (level >= 0)
	{
		while (next = prev->forward[level].ptr, l->cmp(next->item, item) < 0)
		{
			assert(next != prev);
			prev = next;
		}

		oldprev = prev;
		if (next->item != item)
		{
			while (next = prev->forward[level].ptr, l->cmp(next->item, item) == 0 && next->item != item)
			{
				prev = next;
			}
		}

		if (next->item != item)
		{
			// overshot, backtrack so we can update the count
			prev = oldprev;
		}
		update[level] = prev;
		level--;
	}

	while (next = prev->next, l->cmp(next->item, item) < 0)
	{
		prev = next;
	}

	// XXX should we have skiplist_delete_exact?
	if (next->item != item)
	{
		prev = next;
		while (next = prev->next, l->cmp(next->item, item) == 0 && next->item != item)
		{
			offset++;
			prev = next;
		}
	}

	// Opposite to insert, we start at the top level and go down.  I think there's a good
	// reason for this ;)
	if (next->item == item)
	{
		old = next;
		oldprev = prev;
		for (level = l->level; level >= 0; level--)
		{
			prev = update[level];
			next = prev->forward[level].ptr;
			if (next == old)
			{
				prev->forward[level].ptr = old->forward[level].ptr;
				prev->forward[level].distance += old->forward[level].distance - 1;
			}
			else
			{
				prev->forward[level].distance--;
			}
		}
		prev = oldprev;
		prev->next = old->next;
		old->next->prev = old->prev;

		l->size--;

		// free(old);
		if (l->gc_used + 1 >= l->gc_limit)
		{
			l->gc = realloc(l->gc, sizeof(skipnode) * (l->gc_limit *= 2));
			assert(l->gc);
		}

		l->gc[l->gc_used++] = old;

		level = l->level;
		while (l->header->forward[level].ptr == l->sentinal && level > 0)
			level--;

		if (l->header->next == l->sentinal)
		{
			assert(level == 0 || l->level == -1);
			level = -1;
		}

		l->level = level;
		return 1;
	}

	return 0;
}

void
skiplist_gc(skiplist l)
{
	while (l->gc_used)
		free(l->gc[--l->gc_used]);
}


int
skiplist_delete_gc(skiplist l, void *item)
{
	int r;
	r = skiplist_delete(l, item);
	skiplist_gc(l);
	return r;
}

skipnode
skiplist_at(skiplist l, int index)
{
	int level;
	int pos = 0;
	skipnode node = l->header;

	if (index > l->size - 1)
	{
		return NULL;
	}

	for (level = l->level; level >= 0; level--)
	{
		while (pos + node->forward[level].distance < index)
		{
			pos += node->forward[level].distance;
			node = node->forward[level].ptr;
		}
	}

	while (pos < index + 1)
	{
		pos++;
		node = node->next;
	}

	return node;
}


void *
skiplist_fetch_at(skiplist l, int index)
{
	skipnode n;
	if ((n = skiplist_at(l, index)))
		return skipnode_item(n);
	else
		return NULL;
}

skipnode
skiplist_search(skiplist l, void *item)
{
	int level;
	skipnode prev,next;

	prev = l->header;
	level = l->level;

	while (level >= 0)
	{
		while (next = prev->forward[level].ptr, l->cmp(next->item, item) < 0)
			prev = next;
		level--;
	}

	while (next = prev->next, l->cmp(next->item, item) < 0)
		prev = next;

	if (l->cmp(next->item, item) == 0)
		return next;

	return NULL;
}

void *
skiplist_fetch(skiplist l, void *item)
{
	skipnode n;

	if ((n = skiplist_search(l, item)))
		return(skipnode_item(n));
	else
		return NULL;
}

skipnode
skiplist_search_exact(skiplist l, void *item)
{
	skipnode n = skiplist_search(l, item);
	if (NULL == n)
		return NULL;

	while (n->item != item && l->cmp(n->item, item) == 0)
		n = skiplist_next(l, n);

	if (n->item == item)
		return n;

	return NULL;
}


void *
skiplist_fetch_exact(skiplist l, void *item)
{
	skipnode n;

	if ((n = skiplist_search_exact(l, item)))
		return(skipnode_item(n));
	else
		return NULL;
}

skipnode
skiplist_first(skiplist l)
{
	return(l->header->next == l->sentinal ? NULL : l->header->next);
}

skipnode
skiplist_last(skiplist l)
{
	return(l->sentinal->prev == l->sentinal ? NULL : l->sentinal->prev);
}

skipnode
skiplist_next(skiplist l, skipnode n)
{
	return(n->next == l->sentinal ? NULL : n->next);
}

skipnode
skiplist_prev(skiplist l, skipnode n)
{
	return(n->prev == l->sentinal ? NULL : n->prev);
}

int
skiplist_size(skiplist l)
{
	return(l->size);
}
