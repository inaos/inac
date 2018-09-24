/*
 * Copyright INAOS GmbH, Thalwil, 2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
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


typedef struct ina_list_s ina_list_t;
typedef struct ina_list_node_s ina_list_node_t;

struct ina_list_node_s {
    ina_list_node_t *next;
    ina_list_node_t *prev;
    void *data;
};



INA_API(ina_rc_t) ina_list_new(uint32_t cf, ina_list_t **list);
INA_API(ina_rc_t) ina_list_new_from_hashtable(ina_hashtable_t *ht, ina_list_t **list);

INA_API(ina_rc_t) ina_list_resize(ina_list_t *list, size_t min_nodes, size_t max_recyclable_nodes);

INA_API(void)     ina_list_free(ina_list_t **list);

INA_API(ina_rc_t) ina_list_node_new(ina_list_t *list, ina_list_node_t **node);
INA_API(void)     ina_list_node_free(ina_list_t *list, ina_list_node_t **node);

INA_API(ina_rc_t) ina_list_count(ina_list_t *list, size_t *count);
INA_API(ina_rc_t) ina_list_usage(ina_list_t *list, size_t *usage);

INA_API(ina_rc_t) ina_list_head(ina_list_t *list, ina_list_node_t **node);
INA_API(ina_rc_t) ina_list_tail(ina_list_t *list, ina_list_node_t **node);

INA_API(ina_rc_t) ina_list_insert_head(ina_list_t *list, ina_list_node_t *node);
INA_API(ina_rc_t) ina_list_insert_tail(ina_list_t *list, ina_list_node_t *node);
INA_API(ina_rc_t) ina_list_remove(ina_list_t *list, ina_list_node_t *node);

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

INA_API(ina_rc_t) ina_list_insert_concat(ina_list_t *dest, ina_list_t *src);

INA_API(ina_rc_t) ina_list_foreach(ina_list_t *list, ina_foreach_fn_t foreach_fn);
INA_API(ina_rc_t) ina_list_foreach_arg(ina_list_t *list, ina_foreach_arg_fn_t foreach_fn, void *arg);
INA_API(ina_rc_t) ina_list_sort(ina_list_t *list, ina_compare_fn_t compare_fn);
INA_API(ina_rc_t) ina_list_find(ina_list_t *list, ina_find_fn_t find_fn, const void *find_arg, ina_list_node_t **node);






#ifdef __cplusplus
}
#endif

#endif
