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
    size_t nodes;
    size_t count;
    ina_mempool_t *mp;
    ina_list_node_t **frst_free;
    ina_list_node_t **last_free;
};

ina_rc_t __ina_add_data(void *arg, void *data)
{
    ina_list_t *list = (ina_list_t*)arg;
    return ina_list_insert_tail_data(list, data);
}


INA_API(ina_rc_t) ina_list_new(uint32_t cf, size_t nodes, ina_list_t **list)
{
    INA_VERIFY_NOT_NULL(*list);
    *list = ina_mem_alloc(sizeof(ina_list_t));
    INA_RETURN_IF_NULL(*list);
    ina_mem_set(*list, 0, sizeof(ina_list_t));
    (*list)->cf = cf;
    (*list)->nodes = nodes;
    if (cf|INA_LIST_CF_NOMALLOC) {
        return INA_SUCCESS;
    }
    if (INA_SUCCEED(ina_mempool_new(&(*list)->mp, sizeof(ina_list_node_t)*nodes + 1024*(sizeof(void*)), INA_MEM_DYNAMIC, NULL))) {

    }

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_list_new_from_hashtable(ina_hashtable_t *ht, ina_list_t **list)
{
    int count;
    ina_hashtable_count(ht, &count);
    if (INA_FAILED(ina_list_new(INA_LIST_CF_DEFAULT, count, list))) {
        ina_list_free(list);
        return ina_err_get_last_rc();
    }
    if (INA_FAILED(ina_hashtable_foreach_arg(ht, __ina_add_data, *list))) {
        ina_list_free(list);
        return ina_err_get_last_rc();
    }
    return INA_SUCCESS;
}

INA_API(void) ina_list_free(ina_list_t **list)
{
    INA_FREE_CHECK(list);
    ina_mempool_free(&(*list)->mp);
    INA_MEM_FREE_SAFE(list);
}

INA_API(ina_rc_t) ina_list_node_new(ina_list_t *list, ina_list_node_t **node)
{
    INA_VERIFY_NOT_NULL(list);
    INA_VERIFY_NOT_NULL(node);
    *node = *list->last_free;

    if (*node != *list->frst_free) {
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
    list->last_free++;
    *list->last_free = *node;
    *node = NULL;
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
    return INA_SUCCESS;
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