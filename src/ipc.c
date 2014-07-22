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
#include "config.h"

/* IPC flag */
struct ina_ipc_flags_s {
    ina_mempool_t *m;
    ina_ipc_flags_data_t *data;   
};

/* IPC flag data*/
struct ina_ipc_flags_data_s {
    char name[INA_IPC_FLAGS_NAME_MAXLEN];
    int64_t  l;
    uint64_t v;
    uint64_t ka;
    uint32_t c_ref[64];
};

#define __INA_ENTER_LOCK(d)
#define __INA_EXIT_LOCK(d)

INA_API(ina_rc_t) ina_ipc_flags_new(const char* name, int64_t initial, ina_ipc_flags_t **flags)
{
    char mname[INA_IPC_FLAGS_NAME_MAXLEN+15];

    INA_ASSERT_NOTNULL(flags);

    *flags = ina_mem_alloc(sizeof(ina_ipc_flags_t));
    if (*flags == NULL) {
        return INA_ERR_PUSH_LAST;
    }
    strcpy(mname, "/ina_ipc_flags_");
    strncat(mname, name, INA_IPC_FLAGS_NAME_MAXLEN);

    if (!INA_SUCCEED(ina_mempool_create(&(*flags)->m, sizeof(ina_ipc_flags_data_t), 
                     INA_MEM_SHARED|INA_MEM_SHARED_CREATE|INA_MEM_SHARED_EXCL, 
                     mname))) {
        ina_ipc_flags_free(flags);
        return INA_ERR_PUSH_LAST;
    }
    (*flags)->data = ina_mempool_dalloc((*flags)->m, sizeof(ina_ipc_flags_data_t));
    if ((*flags)->data == NULL) {
        ina_ipc_flags_free(flags);
    }
    strncpy((*flags)->data->name, name, INA_IPC_FLAGS_NAME_MAXLEN);
    if (initial != INA_IPC_FLAGS_IGNORE) {
        return ina_ipc_flags_set(*flags, (uint64_t)initial);
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ipc_flags_open(const char* name, ina_ipc_flags_t **flags)
{
    char mname[INA_IPC_FLAGS_NAME_MAXLEN+15];
    INA_ASSERT_NOTNULL(flags);

    *flags = ina_mem_alloc(sizeof(ina_ipc_flags_t));
    if (*flags == NULL) {
        return INA_ERR_PUSH_LAST;
    }
    strcpy(mname, "/ina_ipc_flags_");
    strncat(mname, name, INA_IPC_FLAGS_NAME_MAXLEN);
    if (!INA_SUCCEED(ina_mempool_create(&(*flags)->m, sizeof(ina_ipc_flags_data_t), 
                     INA_MEM_SHARED, 
                     mname))) {
        ina_ipc_flags_free(flags);
        return INA_ERR_PUSH_LAST;
    }
    (*flags)->data = ina_mempool_dalloc((*flags)->m, sizeof(ina_ipc_flags_data_t));
    if ((*flags)->data == NULL) {
        ina_ipc_flags_free(flags);
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ipc_flags_free(ina_ipc_flags_t **flags)
{
    INA_ASSERT_NOTNULL(flags);

    if (*flags == NULL) {
        return INA_SUCCESS;
    }
    if ((*flags)->m != NULL) {
        ina_mempool_release((*flags)->m, INA_YES);
    }
    ina_mem_free(*flags);
    *flags = NULL;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ipc_flags_get_name(const ina_ipc_flags_t *flags, const char **name)
{
    INA_ASSERT_NOTNULL(flags);
    INA_ASSERT_NOTNULL(flags->data);
    INA_ASSERT_NOTNULL(name);
    *name = flags->data->name;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ipc_flags_get(const ina_ipc_flags_t *flags, uint64_t *value)
{
    INA_ASSERT_NOTNULL(flags);
    INA_ASSERT_NOTNULL(flags->data);
    INA_ASSERT_NOTNULL(value);
    *value = flags->data->v;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ipc_flags_set(ina_ipc_flags_t *flags, uint64_t value)
{
    INA_ASSERT_NOTNULL(flags);
    INA_ASSERT_NOTNULL(flags->data);
    __INA_ENTER_LOCK(flags->data);
    flags->data->v |= value;
    __INA_EXIT_LOCK(flags->data);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ipc_flags_is_set(const ina_ipc_flags_t* flags, uint64_t value)
{
    INA_ASSERT_NOTNULL(flags);
    INA_ASSERT_NOTNULL(flags->data);
    if (flags->data->v&(value)) {
        return INA_SUCCESS;
    }
    return INA_FAILURE;
}

INA_API(ina_rc_t) ina_ipc_flags_unset(ina_ipc_flags_t *flags, uint64_t value)
{
    INA_ASSERT_NOTNULL(flags);
    INA_ASSERT_NOTNULL(flags->data);
    __INA_ENTER_LOCK(flags->data);
    flags->data->v =~ (value);
    __INA_EXIT_LOCK(flags->data);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ipc_flags_wait(const ina_ipc_flags_t* flags, uint64_t wait_for, time_t msec_timeout)
{
    ina_timer_t *timer;
    ina_time_event_t *event;
    int timeout = INA_NO;

    INA_ASSERT_NOTNULL(flags);

    if (!INA_SUCCEED(ina_timer_init(&timer))) {
        return INA_ERR_PUSH_LAST;
    }
    
    event = ina_timer_create_event(timer, msec_timeout);
    if (event == NULL) {
        ina_timer_destroy(&timer);
        return INA_ERR_PUSH_LAST;
    }

    while (!INA_SUCCEED(ina_ipc_flags_is_set(flags, wait_for))) {
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


