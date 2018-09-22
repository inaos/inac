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
#include <stdio.h>
#include <libinac/lib.h>

typedef struct ina_node_data_s {
    int index;
    int revindex;
} ina_node_data_t;

INA_TEST(list, externally_data)
{
    int i;
    ina_list_t *list;
    ina_list_node_t *node;
    ina_node_data_t *data;
    INA_TEST_ASSERT_SUCCEED(ina_list_new(INA_LIST_CF_DEFAULT, &list));

    for (i=0; i< 1000; ++i) {
        data = ina_mem_alloc(sizeof(ina_node_data_t));
        data->index = i;
        INA_TEST_ASSERT_SUCCEED(ina_list_insert_tail_data(list, data));
    }
    ina_list_remove_data(list, data);

    ina_list_head(list, &node);
    while (node) {
        ((ina_node_data_t*)node->data)->revindex = 1000 - ((ina_node_data_t*)node->data)->index;
        node = node->next;
    }

    ina_list_free(&list);
}

