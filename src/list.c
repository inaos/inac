/*
 * Copyright 2018-2020 INAOS GmbH, Thalwil
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <libinac/lib.h>
#include "config.h"

struct ina_list_s {
    ina_list_node_t *head;
    uint32_t cf;
    size_t count;
    size_t max_recyclable;
    ina_mempool_t *mp;
    ina_list_node_t **first_free;
    int last_free;
};

ina_list_node_t *__ina_split(ina_list_node_t *head)
{
    ina_list_node_t *tmp;
    ina_list_node_t *fast = head,*slow = head;
    while (fast->next && fast->next->next)
    {
        fast = fast->next->next;
        slow = slow->next;
    }
    tmp = slow->next;
    slow->next = NULL;
    return tmp;
}

ina_list_node_t *__ina_merge(ina_list_node_t *first, ina_list_node_t *second, ina_compare_fn_t compare_fn)
{
    /* If first linked list is empty */
    if (!first)
        return second;

    /* If second linked list is empty */
    if (!second)
        return first;

    /* Pick the smaller value */
    if (compare_fn(first->data, second->data) < 0) {
        first->next = __ina_merge(first->next,second, compare_fn);
        first->next->prev = first;
        first->prev = NULL;
        return first;
    } else {
        second->next = __ina_merge(first,second->next, compare_fn);
        second->next->prev = second;
        second->prev = NULL;
        return second;
    }
}

ina_list_node_t *__ina_mergesort(ina_list_node_t *head, ina_compare_fn_t compare_fn)
{
    ina_list_node_t *second;
    if (!head || head->next == NULL) {
        return head;
    }
    second = __ina_split(head);
    head = __ina_mergesort(head, compare_fn);
    second = __ina_mergesort(second, compare_fn);
    return __ina_merge(head, second, compare_fn);
}

ina_rc_t __ina_add_data(void *arg, void *data)
{
    ina_list_t *list = (ina_list_t*)arg;
    return ina_list_insert_tail_data(list, data);
}


INA_API(ina_rc_t) ina_list_new(uint32_t cf, ina_list_t **list)
{
    INA_VERIFY_NOT_NULL(list);
    *list = ina_mem_alloc(sizeof(ina_list_t));
    INA_RETURN_IF_NULL(*list);
    ina_mem_set(*list, 0, sizeof(ina_list_t));
    (*list)->cf = cf;
    (*list)->max_recyclable = INA_LIST_DEFAULT_SIZE;
    if (cf&INA_LIST_CF_NOMALLOC) {
        return INA_SUCCESS;
    }
    if (INA_FAILED(ina_list_resize(*list, INA_LIST_CF_DEFAULT, INA_LIST_DEFAULT_SIZE))) {
        ina_list_free(list);
        return ina_err_get_rc();
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_list_new_from_hashtable(ina_hashtable_t *ht, ina_list_t **list)
{
    size_t count;
    ina_hashtable_count(ht, &count);
    if (INA_SUCCEED(ina_list_new(INA_LIST_CF_DEFAULT, list)) &&
        INA_SUCCEED(ina_list_resize(*list, count, 0)) &&
        INA_SUCCEED(ina_hashtable_foreach_arg(ht, __ina_add_data, *list))) {
        return INA_SUCCESS;
    }
    return ina_err_get_rc();
}

INA_API(void) ina_list_free(ina_list_t **list)
{
    INA_VERIFY_FREE(list);
    ina_mempool_free(&(*list)->mp);
    INA_MEM_FREE_SAFE((*list)->first_free);
    INA_MEM_FREE_SAFE(*list);
}

INA_API(ina_rc_t) ina_list_node_new(ina_list_t *list, ina_list_node_t **node)
{
    INA_VERIFY_NOT_NULL(list);
    INA_VERIFY_NOT_NULL(node);

    if (list->last_free) {
        *node = list->first_free[list->last_free];
        list->last_free--;
        return INA_SUCCESS;
    }
    *node = ina_mempool_dalloc(list->mp, sizeof(ina_list_node_t));
    return INA_SUCCESS;
}

INA_API(void) ina_list_node_free(ina_list_t *list, ina_list_node_t **node)
{
    INA_VERIFY_FREE(node);
    INA_ASSERT_NOT_NULL(list);
    if (list->last_free < list->max_recyclable) {
        list->last_free++;
        list->first_free[list->last_free] = *node;
    }
    *node = NULL;
}

INA_API(ina_rc_t) ina_list_resize(ina_list_t *list, size_t min_nodes, size_t max_recyclable_nodes)
{
    INA_VERIFY_NOT_NULL(list);
    ina_mempool_t *mp;

    if (list->cf&INA_LIST_CF_NOMALLOC) {
        return INA_ERROR(INA_ERR_OPERATION_INVALID);
    }

    if (min_nodes == 0) {
        min_nodes = INA_LIST_DEFAULT_SIZE;
    }

    if (min_nodes < list->count) {
        min_nodes = list->count;
    }

    if (INA_FAILED(ina_mempool_new(
            sizeof(ina_list_node_t) * min_nodes,
            NULL,

            INA_MEM_DYNAMIC, &mp))) {
        return ina_err_get_rc();
    }

    if (list->mp != NULL) {
        ina_list_node_t* next;
        ina_list_node_t* new_mode;
        ina_list_node_t* new_head = NULL;
        if (INA_SUCCEED(ina_list_head(list, &next))) {
            while (next) {
                new_mode = ina_mempool_dalloc(mp, sizeof(ina_list_node_t));
                if (new_head == NULL) {
                    new_head = new_mode;
                }
                ina_mem_cpy(new_mode, next, sizeof(ina_list_node_t));
                next = next->next;
            }
        }
        list->head = new_head;
        ina_mempool_free(&list->mp);
    }

    list->mp = mp;

    INA_MEM_FREE_SAFE(list->first_free);
    list->last_free = 0;
    if (max_recyclable_nodes) {
        list->max_recyclable = max_recyclable_nodes;
        list->first_free = ina_mem_alloc(sizeof(void*)*max_recyclable_nodes);
        ina_mem_set(list->first_free, 0, sizeof(void*)*max_recyclable_nodes);
    }
    return INA_SUCCESS;
}


INA_API(ina_rc_t) ina_list_count(ina_list_t *list, size_t *count)
{
    INA_VERIFY_NOT_NULL(list);
    INA_VERIFY_NOT_NULL(count);
    *count = list->count;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_list_head(ina_list_t *list, ina_list_node_t **node)
{
    INA_VERIFY_NOT_NULL(node);
    *node = list->head;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_list_usage(ina_list_t *list, size_t *usage)
{
    ina_mempool_info_t info;
    INA_VERIFY_NOT_NULL(list);
    INA_VERIFY_NOT_NULL(usage);
    *usage = 0;
    INA_RETURN_IF_FAILED(ina_mempool_info(list->mp, &info));
    *usage = info.size;
    *usage += sizeof(void*)*list->max_recyclable;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_list_tail(ina_list_t *list, ina_list_node_t **node)
{
    INA_VERIFY_NOT_NULL(list);
    INA_VERIFY_NOT_NULL(node);
    if (list->head == NULL) {
        *node = NULL;
        return INA_ERROR(INA_ERR_EMPTY);
    }
    *node = list->head;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_list_insert_head(ina_list_t *list, ina_list_node_t *node)
{
    ina_list_node_t *head;
    INA_VERIFY_NOT_NULL(list);
    INA_VERIFY_NOT_NULL(node);
    ++(*list).count;
    head = list->head;
    if (list->head) {
        node->prev = head->prev;
        head->prev = node;
        node->next = head;
        return INA_SUCCESS;
    }
    node->prev = node;
    node->next = NULL;
    list->head = node;
    return INA_SUCCESS;

}
INA_API(ina_rc_t) ina_list_insert_tail(ina_list_t *list, ina_list_node_t *node)
{
    ina_list_node_t *head;
    INA_VERIFY_NOT_NULL(node);
    ++(*list).count;
    head = list->head;
    if (head != NULL) {
        node->prev = head->prev;
        head->prev = node;
        node->next = NULL;
        node->prev->next = node;
        return INA_SUCCESS;
    }
    node->prev = node;
    node->next = NULL;
    list->head = node;
    return INA_SUCCESS;

}

INA_API(ina_rc_t) ina_list_remove(ina_list_t *list, ina_list_node_t *node)
{
    ina_list_node_t *head = NULL;
    INA_VERIFY_NOT_NULL(node);
    ina_list_head(list, &head);

    if (node->next) {
        node->next->prev = node->prev;
    } else {
        head->prev = node->prev;
    }

    if (head == node) {
        list->head = node->next;
    } else {
        node->prev->next = node->next;
    }
    --list->count;
    return INA_SUCCESS;
}

INA_API (ina_rc_t) ina_list_remove_data(ina_list_t *list, void *data)
{
    ina_list_node_t *node;
    INA_VERIFY_NOT_NULL(list);
    INA_VERIFY_NOT_NULL(data);

    if(INA_SUCCEED(ina_list_head(list, &node))) {
        while (node) {
            if (node->data == data) {
                ina_list_remove(list, node);
                ina_list_node_free(list, &node);
                return INA_SUCCESS;
            }
            node = node->next;
        }
    }
    return INA_ERROR(INA_ERR_NOT_EXISTS);
}

INA_API(ina_rc_t) ina_list_foreach(ina_list_t *list, ina_foreach_fn_t foreach_fn)
{
    ina_list_node_t *next, *next_next;
    ina_rc_t rc = INA_SUCCESS;

    INA_VERIFY_NOT_NULL(foreach_fn);
    if (INA_SUCCEED(ina_list_head(list, &next))) {

        while (next) {
			next_next = next->next;
			if (INA_FAILED(foreach_fn(next->data))) {
				break;
			}
            next = next_next;
        }
    }
    return rc;
}

INA_API(ina_rc_t) ina_list_concat(ina_list_t *dest, ina_list_t *src)
{
    INA_VERIFY_NOT_NULL(dest);

    if (src == NULL || src->head == NULL) {
        return INA_SUCCESS;
    }

    if (dest->head == NULL) {
        dest->head = src->head;
        dest->count = src->count;
        src->head = NULL;
    } else {
        ina_list_node_t *dest_tail;
        dest_tail = dest->head->prev;
        dest->head->prev = src->head->prev;
        src->head->prev = dest_tail;
        dest_tail->next = src->head;
        dest->count += src->count;
    }

    src->head = NULL;
    src->count = 0;
    if (dest->mp != NULL) {
        ina_mempool_merge(dest->mp, src->mp);
    } else {
        dest->mp = src->mp;
    }
    src->mp = NULL;
    return INA_SUCCESS;
}


INA_API(ina_rc_t) ina_list_foreach_arg(ina_list_t *list, ina_foreach_arg_fn_t foreach_fn, void *arg)
{
    ina_list_node_t *next, *next_next;
    ina_rc_t rc = INA_SUCCESS;

    INA_VERIFY_NOT_NULL(foreach_fn);
    if (INA_SUCCEED(ina_list_head(list, &next))) {
        while (next) {
			next_next = next->next;
			rc = foreach_fn(next->data, arg);
			if (INA_FAILED(rc)) {
				break;
			}
            next = next_next;
        }
    }
    return rc;
}


INA_API(ina_rc_t) ina_list_find(ina_list_t *list, ina_compare_fn_t compare_fn, const void *find_arg, ina_list_node_t **node)
{
    ina_list_node_t *next;
    INA_VERIFY_NOT_NULL(compare_fn);
    INA_VERIFY_NOT_NULL(find_arg);
    INA_VERIFY_NOT_NULL(node);
    if (INA_SUCCEED(ina_list_head(list, &next))) {
        while (next) {
            if (0 == compare_fn(next->data, find_arg)) {
				*node = next;
                return INA_SUCCESS;
            }
            next = next->next;
        }
    }
    return INA_ERROR(INA_ERR_NOT_FOUND);
}

INA_API(ina_rc_t) ina_list_sort(ina_list_t *list, ina_compare_fn_t compare_fn)
{
    ina_list_node_t *head;
    if (INA_SUCCEED(ina_list_head(list, &head))) {
        list->head = __ina_mergesort(head, compare_fn);
        return INA_SUCCESS;
    }
    return INA_ERROR(INA_ERR_EMPTY);
}