/*
 * Copyright (c) 2018, INAOS GmbH
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
#ifndef _LIBINAC_LIST_H_
#define _LIBINAC_LIST_H_


#ifdef __cplusplus
extern "C" {
#endif

#include <libinac/lib.h>

#define INA_LIST_DEFAULT_SIZE (256)
#define INA_LIST_CF_NOMALLOC (1U)
#define INA_LIST_CF_DEFAULT  (0U)


typedef struct ina_list_node_s ina_list_node_t;

struct ina_list_node_s {
    ina_list_node_t *next;
    ina_list_node_t *prev;
    void *data;
};
typedef struct ina_list_s ina_list_t;


INA_API(ina_rc_t) ina_list_new(uint32_t cf, ina_list_t **list);
INA_API(ina_rc_t) ina_list_new_from_hashtable(ina_hashtable_t *ht, ina_list_t **list);

INA_API(ina_rc_t) ina_list_resize(ina_list_t *list, size_t min_nodes, size_t max_recyclable_nodes);

INA_API(void)     ina_list_free(ina_list_t **list);

INA_API(ina_rc_t) ina_list_node_new(ina_list_t *list, ina_list_node_t **node);
INA_API(void)     ina_list_node_free(ina_list_t *list, ina_list_node_t **node);

INA_API(ina_rc_t) ina_list_count(ina_list_t *list, size_t *count);

INA_API(ina_rc_t) ina_list_head(ina_list_t *list, ina_list_node_t **node);
INA_API(ina_rc_t) ina_list_tail(ina_list_t *list, ina_list_node_t **node);

INA_API(ina_rc_t) ina_list_insert_head(ina_list_t *list, ina_list_node_t *node);
INA_API(ina_rc_t) ina_list_insert_tail(ina_list_t *list, ina_list_node_t *node);

INA_API(ina_rc_t) ina_list_remove(ina_list_t *list, ina_list_node_t *node);

INA_API(ina_rc_t) ina_list_foreach(ina_list_t *list, ina_foreach_fn_t foreach_fn);
INA_API(ina_rc_t) ina_list_sort(ina_list_t *list, ina_compare_fn_t compare_fn);
INA_API(ina_rc_t) ina_list_find(ina_list_t *list, ina_foreach_fn_t foreach_fn, ina_list_node_t **node);



INA_INLINE ina_rc_t ina_list_insert_head_data(ina_list_t *list, void *data)
{
    ina_list_node_t *node;
    INA_MUST_SUCCEED(ina_list_node_new(list, &node));
    node->data = data;
    return ina_list_insert_head(list, node);
}

INA_INLINE ina_rc_t ina_list_insert_tail_data(ina_list_t *list, void *data)
{
    ina_list_node_t *node;
    INA_MUST_SUCCEED(ina_list_node_new(list, &node));
    node->data = data;
    return ina_list_insert_tail(list, node);
}

INA_API (ina_rc_t) ina_list_remove_data(ina_list_t *list, void *data);


#ifdef __cplusplus
}
#endif

#endif
