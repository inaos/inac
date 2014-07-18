/*
 * Copyright (c) 2014, INAOS GmbH
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
#include <libinac/ipc.h>
#include "config.h"

/* IPC flag */
struct ina_ipc_flag_s {
    ina_mempool_t *m;
    ina_ipc_flag_data_t *data;   
};

/* IPC flag data*/
struct ina_ipc_flag_data_s {
    char name[INA_IPC_FLAG_NAME_MAXLEN];
    int64_t  l;
    uint64_t v;
    uint64_t ka;
    uint32_t c_ref[64];
};

#define __INA_ENTER_LOCK(d)
#define __INA_EXIT_LOCK(d)

INA_API(ina_rc_t) ina_ipc_flag_new(const char* name, uint64_t initial, ina_ipc_flag_t **flag)
{
    *flag = ina_mem_alloc(sizeof(ina_ipc_flag_t));
    if (*flag == NULL) {
        return INA_ERR_PUSH_LAST;
    }
    if (!INA_SUCCEED(ina_mempool_create(&(*flag)->m, sizeof(ina_ipc_flag_data_t), 
                     INA_MEM_SHARED|INA_MEM_SHARED_CREATE, 
                     name))) {
        ina_ipc_flag_free(flag);
        return INA_ERR_PUSH_LAST;
    }
    (*flag)->data = ina_mempool_dalloc((*flag)->m, sizeof(ina_ipc_flag_data_t));
    if ((*flag)->data == NULL) {
        ina_ipc_flag_free(flag);
    }
    strncpy((*flag)->data->name, name, INA_IPC_FLAG_NAME_MAXLEN);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ipc_flag_free(ina_ipc_flag_t **flag)
{
    if (*flag == NULL) {
        return INA_SUCCESS;
    }
    if ((*flag)->m != NULL) {
        ina_mempool_release((*flag)->m, INA_YES);
    }
    ina_mem_free(*flag);
    *flag = NULL;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ipc_flag_get_name(const ina_ipc_flag_t *flag, const char **name)
{
    INA_ASSERT_NOTNULL(flag);
    INA_ASSERT_NOTNULL(flag->data);
    INA_ASSERT_NOTNULL(name);
    *name = flag->data->name;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ipc_flag_get(const ina_ipc_flag_t *flag, uint64_t *value)
{
    INA_ASSERT_NOTNULL(flag);
    INA_ASSERT_NOTNULL(flag->data);
    INA_ASSERT_NOTNULL(value);
    *value = flag->data->v;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ipc_flag_set(ina_ipc_flag_t *flag, uint64_t value)
{
    INA_ASSERT_NOTNULL(flag);
    INA_ASSERT_NOTNULL(flag->data);
    __INA_ENTER_LOCK(flag->data);
    flag->data->v |= value;
    __INA_EXIT_LOCK(flag->data);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ipc_flag_is_set(const ina_ipc_flag_t* flag, uint64_t value)
{
    INA_ASSERT_NOTNULL(flag);
    INA_ASSERT_NOTNULL(flag->data);
    if (flag->data->v&(value)) {
        return INA_SUCCESS;
    }
    return INA_FAILURE;
}

INA_API(ina_rc_t) ina_ipc_flag_unset(ina_ipc_flag_t *flag, uint64_t value)
{
    INA_ASSERT_NOTNULL(flag);
    INA_ASSERT_NOTNULL(flag->data);
    __INA_ENTER_LOCK(flag->data);
    flag->data->v =~ (value);
    __INA_EXIT_LOCK(flag->data);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ipc_flag_wait(const ina_ipc_flag_t* flag, uint64_t wait_for, time_t msec_timeout)
{
    ina_timer_t *timer;
    ina_time_event_t *event;
    int timeout = INA_NO;

    if (!INA_SUCCEED(ina_timer_init(&timer))) {
        return INA_ERR_PUSH_LAST;
    }
    
    event = ina_timer_create_event(timer, msec_timeout);
    if (event == NULL) {
        ina_timer_destroy(&timer);
        return INA_ERR_PUSH_LAST;
    }

    while (!INA_SUCCEED(ina_ipc_flag_is_set(flag, wait_for))) {
        if (ina_timer_next_event(timer) != NULL) {
            timeout = INA_YES;
            break;
        }
    }
    ina_timer_destroy(&timer);

    if (timeout == INA_YES) {
        return INA_FAILURE;
    }
    return INA_SUCCESS;
}


