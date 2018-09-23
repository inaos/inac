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
#include <libinac/lib.h>
#include "config.h"

struct ina_list_s {
    ina_list_node_t *head;
    uint32_t cf;
    size_t count;
    size_t max_recyclable;
    ina_mempool_t *mp;
    ina_list_node_t **frst_free;
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
        return ina_err_get_last_rc();
    }
    return INA_SUCCESS;
}


INA_API(ina_rc_t) ina_list_new_from_hashtable(ina_hashtable_t *ht, ina_list_t **list)
{
    int count;
    ina_hashtable_count(ht, &count);
    if (INA_SUCCEED(ina_list_new(INA_LIST_CF_DEFAULT, list)) &&
        INA_SUCCEED(ina_list_resize(*list, count, 0)) &&
        INA_SUCCEED(ina_hashtable_foreach_arg(ht, __ina_add_data, *list))) {
        return INA_SUCCESS;
    }
    return ina_err_get_last_rc();
}

INA_API(void) ina_list_free(ina_list_t **list)
{
    INA_FREE_CHECK(list);
    ina_mempool_free(&(*list)->mp);
    INA_MEM_FREE_SAFE((*list)->frst_free);
    INA_MEM_FREE_SAFE(*list);
}

INA_API(ina_rc_t) ina_list_node_new(ina_list_t *list, ina_list_node_t **node)
{
    INA_VERIFY_NOT_NULL(list);
    INA_VERIFY_NOT_NULL(node);

    if (list->last_free) {
        *node = list->frst_free[list->last_free];
        list->last_free--;
        return INA_SUCCESS;
    }
    *node = ina_mempool_dalloc(list->mp, sizeof(ina_list_node_t));
    return INA_SUCCESS;
}

INA_API(void) ina_list_node_free(ina_list_t *list, ina_list_node_t **node)
{
    INA_FREE_CHECK(node);
    INA_VERIFY_NOT_NULL(list);
    if (list->last_free < list->max_recyclable) {
        list->last_free++;
        list->frst_free[list->last_free] = *node;
    }
    *node = NULL;
}

INA_API(ina_rc_t) ina_list_resize(ina_list_t *list, size_t min_nodes, size_t max_recyclable_nodes)
{
    INA_VERIFY_NOT_NULL(list);
    ina_mempool_t *mp;

    if (min_nodes == 0) {
        min_nodes = INA_LIST_DEFAULT_SIZE;
    }

    if (min_nodes < list->count) {
        min_nodes = list->count;
    }

    if (INA_FAILED(ina_mempool_new(sizeof(ina_list_node_t) * min_nodes, NULL, INA_MEM_DYNAMIC, &mp))) {
        return ina_err_get_last_rc();
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

    INA_MEM_FREE_SAFE(list->frst_free);
    list->last_free = 0;
    if (max_recyclable_nodes) {
        list->max_recyclable = max_recyclable_nodes;
        list->frst_free = ina_mem_alloc(sizeof(void*)*max_recyclable_nodes);
        ina_mem_set(list->frst_free, 0, sizeof(void*)*max_recyclable_nodes);
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
    INA_RETURN_IF_FAILED(ina_mempool_getinfo(list->mp, &info));
    *usage = info.size;
    *usage += sizeof(void*)*list->max_recyclable;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_list_tail(ina_list_t *list, ina_list_node_t **node)
{
    INA_VERIFY_NOT_NULL(list);
    INA_VERIFY_NOT_NULL(node);
    if (list->head == NULL) {
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
    ina_list_node_t *head;
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
    ina_list_node_t *next;
    INA_VERIFY_NOT_NULL(foreach_fn);
    if (INA_SUCCEED(ina_list_head(list, &next))) {
        while (next && INA_SUCCEED((foreach_fn(next->data)))) {
            next = next->next;
        }
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_list_foreach_arg(ina_list_t *list, ina_foreach_arg_fn_t foreach_fn, void *arg)
{
    ina_list_node_t *next;
    INA_VERIFY_NOT_NULL(foreach_fn);
    if (INA_SUCCEED(ina_list_head(list, &next))) {
        while (next && INA_SUCCEED((foreach_fn(next->data, arg)))) {
            next = next->next;
        }
    }
    return INA_SUCCESS;
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