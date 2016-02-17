/*
 * Copyright (c) 2015, INAOS GmbH
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the INAOS GmbH nor the names of its contributors
 *       may be used to endorse or promote products derived from this software 
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL INAOS GmbH BE LIABLE FOR ANY DIRECT, 
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */
#include <libinac/lib.h>
#include "config.h"

typedef int (*__ina_percentile_heap_cmp_fn)(uint16_t, uint16_t);

typedef struct __ina_percentile_heap_s {
    size_t count;
    size_t capacity;
    uint16_t *data;
    ina_mempool_t *mem;
    __ina_percentile_heap_cmp_fn cmp;
} __ina_percentile_heap_t;

struct ina_percentile_s {
    double percentile;
    __ina_percentile_heap_t *upper; /* min-heap */
    __ina_percentile_heap_t *lower; /* max-heap */
    ina_mempool_t *mem;
};

static int __ina_percentile_heap_max_cmp(uint16_t a, uint16_t b)
{
    return a >= b;
}

static int __ina_percentile_heap_min_cmp(uint16_t a, uint16_t b)
{
    return a <= b;
}

static ina_rc_t __ina_percentile_heap_new(__ina_percentile_heap_t **h, ina_mempool_t *pool, 
                                          size_t initial_size, int max_heap)
{
    size_t base_size = sizeof(uint16_t)*(initial_size+1);
    
    *h = (__ina_percentile_heap_t*)ina_mempool_dalloc(pool, sizeof(__ina_percentile_heap_t));
    (*h)->count = 0;
    (*h)->capacity = initial_size;
    (*h)->mem = pool;
    (*h)->data = (uint16_t*)ina_mempool_dalloc(pool, base_size);
    if (max_heap) {
        (*h)->cmp = __ina_percentile_heap_max_cmp;
    }
    else {
        (*h)->cmp = __ina_percentile_heap_min_cmp;
    }

    return INA_SUCCESS;
}

static ina_rc_t __ina_percentile_heap_free(__ina_percentile_heap_t **h)
{
    /* not required since its part of the parent pool */
    return INA_SUCCESS;
}

static ina_rc_t __ina_percentile_heap_push(__ina_percentile_heap_t *h, uint16_t value)
{
    size_t i, parent;

    /* grow if necessary */
    if (h->count == h->capacity) {
        size_t old_size = h->capacity*sizeof(uint16_t);
        size_t new_size = old_size*2;
        h->data = ina_mempool_ralloc(h->mem, h->data, old_size, new_size);
        h->capacity = h->capacity * 2;
    }

    for (i = h->count++; i; i = parent) {
        parent = (i - 1) >> 1;
        if (h->cmp(h->data[parent], value)) {
             break;
        }
        h->data[i] = h->data[parent];
    }
    h->data[i] = value;

    return INA_SUCCESS;
}

static ina_rc_t __ina_percentile_heap_pop(__ina_percentile_heap_t *h)
{
    size_t i, swap, other;

    /* Remove the biggest element */
    uint16_t temp = h->data[--h->count];

    /* Reorder the elements */
    for (i = 0; 1; i = swap) {
        /* Find the child to swap with */
        swap = (i << 1) + 1;
        if (swap >= h->count) {
             break; /* If there are no children, the heap is reordered */
        }
        other = swap + 1;
        if ((other < h->count) && h->cmp(h->data[other], h->data[swap])) {
             swap = other;
        }
        if (h->cmp(temp, h->data[swap])) {
             break; /* If the bigger child is less than or equal to its parent, the heap is reordered */
        }
        h->data[i] = h->data[swap];
    }
    h->data[i] = temp;

    return INA_SUCCESS;
}

static ina_rc_t __ina_percentile_heap_front(__ina_percentile_heap_t *h, uint16_t *value)
{
    *value = *(h)->data;
    return INA_SUCCESS; 
}

INA_API(ina_rc_t) ina_percentile_new(ina_percentile_t **p, double percentile, size_t size)
{
    size_t lower_size = (size_t)floor(size * percentile);
    size_t upper_size = size - lower_size;
 
    *p = (ina_percentile_t*)ina_mem_alloc(sizeof(ina_percentile_t));
    (*p)->percentile = percentile;
    
    if (!INA_SUCCEED(ina_mempool_create(&(*p)->mem, 4*1024, INA_MEM_DYNAMIC, NULL))) {
        return INA_ERR_PUSH_LAST;
    }

    if (!INA_SUCCEED(__ina_percentile_heap_new(&(*p)->lower, (*p)->mem, lower_size, INA_YES))) {
        return INA_ERR_PUSH_LAST;
    }
    if (!INA_SUCCEED(__ina_percentile_heap_new(&(*p)->upper, (*p)->mem, upper_size, INA_NO))) {
        return INA_ERR_PUSH_LAST;
    }

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_percentile_free(ina_percentile_t **p)
{
    ina_mempool_release((*p)->mem, INA_YES);
    ina_mem_free(*p);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_percentile_add(ina_percentile_t *p, uint16_t value)
{
    size_t count_lower;

    if (p->lower->count == 0 || value <= *p->lower->data) {
        __ina_percentile_heap_push(p->lower, value);
    } 
    else {
        __ina_percentile_heap_push(p->upper, value);
    }

    count_lower = (size_t)((p->lower->count + p->upper->count) * p->percentile)+1;
    if (p->lower->count > count_lower) {
        /* lower to upper */
        __ina_percentile_heap_push(p->upper, *p->lower->data); 
        __ina_percentile_heap_pop(p->lower);
    }
    else if (p->lower->count < count_lower) {
        /* upper to lower */
        __ina_percentile_heap_push(p->lower, *p->upper->data);
        __ina_percentile_heap_pop(p->upper);
    }
    
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_percentile_get(ina_percentile_t *p, uint16_t *value)
{
    __ina_percentile_heap_front(p->lower, value);
    return INA_SUCCESS;
}

