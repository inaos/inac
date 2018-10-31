/*
 * Copyright INAOS GmbH, Thalwil, 2014-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <libinac/lib.h>
#include "config.h"

/* IPC flag */
struct ina_ipc_flags_s {
    ina_timer_t *timer;
    ina_mempool_t *m;
    ina_ipc_flags_data_t *data;   
};

/* IPC flag data*/
struct ina_ipc_flags_data_s {
    char name[INA_IPC_FLAGS_NAME_MAXLEN];
    volatile int64_t  l;
    volatile uint64_t v;
    uint64_t ka;
    uint32_t c_ref[INA_IPC_FLAGS_MAX];
};

/* IPC counter */
struct ina_ipc_counter_s {
    ina_mempool_t *m;
    ina_ipc_counter_data_t *data;
};

/* IPC counter data */
struct ina_ipc_counter_data_s {
    char name[INA_IPC_FLAGS_NAME_MAXLEN];
    volatile uint64_t c;
};

#define __INA_ENTER_LOCK(d)                     \
do {                                            \
    while (0 == INA_ATOMIC_SWAP(&d->l, 0, 1));  \
} while (0);

#define __INA_EXIT_LOCK(d) INA_ATOMIC_SWAP(&d->l, 1, 0);

INA_API(ina_rc_t) ina_ipc_flags_new(const char* name, int64_t initial, ina_ipc_flags_t **flags)
{
    ina_str_t mname;

    INA_VERIFY_NOT_NULL(flags);
    INA_ASSERT_NOT_NULL(name);
    INA_VERIFY(strlen(name) < INA_IPC_FLAGS_NAME_MAXLEN);

    *flags = (ina_ipc_flags_t*)ina_mem_alloc(sizeof(ina_ipc_flags_t));
    INA_RETURN_IF_NULL(*flags);
    INA_MEM_SET_ZERO(*flags, ina_ipc_flags_t);

    mname = ina_str_sprintf("/ina_ipc_flags_%s", name);
    INA_FAIL_IF(mname == NULL);
    INA_FAIL_IF_ERROR(ina_mempool_new(sizeof(ina_ipc_flags_data_t), mname,
                     INA_MEM_SHARED|INA_MEM_SHARED_CREATE|INA_MEM_SHARED_EXCL, 
                      &(*flags)->m));

    INA_FAIL_IF_ERROR(ina_timer_new(&(*flags)->timer));
    (*flags)->data = (ina_ipc_flags_data_t*)ina_mempool_dalloc((*flags)->m, sizeof(ina_ipc_flags_data_t));

    strncpy((*flags)->data->name, name, INA_IPC_FLAGS_NAME_MAXLEN-1);

    if (initial != INA_IPC_FLAGS_IGNORE) {
        INA_FAIL_IF_ERROR(ina_ipc_flags_set(*flags, (uint64_t) initial));
    }
    INA_STR_FREE_SAFE(mname);
    return INA_SUCCESS;
fail:
    INA_STR_FREE_SAFE(mname);
    ina_ipc_flags_free(flags);
    return ina_err_get_rc();
}

INA_API(ina_rc_t) ina_ipc_flags_open(const char* name, ina_ipc_flags_t **flags)
{
    ina_str_t mname;
    INA_VERIFY_NOT_NULL(flags);
    INA_VERIFY_NOT_NULL(name);
    INA_VERIFY(strlen(name) < INA_IPC_FLAGS_NAME_MAXLEN);

    *flags = (ina_ipc_flags_t*)ina_mem_alloc(sizeof(ina_ipc_flags_t));
    INA_RETURN_IF_NULL(*flags);
    INA_MEM_SET_ZERO(*flags, ina_ipc_flags_t);

    mname = ina_str_sprintf("/ina_ipc_flags_%s", name);
    INA_FAIL_IF(mname == NULL);

    INA_FAIL_IF_ERROR(ina_mempool_new(sizeof(ina_ipc_flags_data_t),mname,
                     INA_MEM_SHARED,&(*flags)->m));
    INA_FAIL_IF_ERROR(ina_timer_new(&(*flags)->timer));

    (*flags)->data = (ina_ipc_flags_data_t *) ina_mempool_dalloc((*flags)->m, sizeof(ina_ipc_flags_data_t));
    INA_STR_FREE_SAFE(mname);
    return INA_SUCCESS;
fail:
    INA_STR_FREE_SAFE(mname);
    ina_ipc_flags_free(flags);
    return ina_err_get_rc();
}

INA_API(void) ina_ipc_flags_free(ina_ipc_flags_t **flags)
{
    INA_VERIFY_FREE(flags);
    ina_timer_free(&(*flags)->timer);
    ina_mempool_free(&(*flags)->m);
    INA_MEM_FREE_SAFE(*flags);
}

INA_API(ina_rc_t) ina_ipc_flags_get_name(const ina_ipc_flags_t *flags, const char **name)
{
    INA_VERIFY_NOT_NULL(flags);
    INA_VERIFY_NOT_NULL(flags->data);
    INA_VERIFY_NOT_NULL(name);
    *name = flags->data->name;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ipc_flags_get(const ina_ipc_flags_t *flags, uint64_t *value)
{
    INA_VERIFY_NOT_NULL(flags);
    INA_VERIFY_NOT_NULL(flags->data);
    INA_VERIFY_NOT_NULL(value);
    *value = flags->data->v;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ipc_flags_set(ina_ipc_flags_t *flags, uint64_t value)
{
    uint64_t j;

    INA_VERIFY_NOT_NULL(flags);
    INA_VERIFY_NOT_NULL(flags->data);
    __INA_ENTER_LOCK(flags->data);
            
    INA_ATOMIC_SWAP((int64_t*)&flags->data->v, flags->data->v, flags->data->v | value);

    /* Increment reference count for each single flag */
    for (j = 0; j < INA_IPC_FLAGS_MAX; ++j) {
        if ((value & ( 1ULL << j)) >> j) {
            ++flags->data->c_ref[j];
        }
    }

    __INA_EXIT_LOCK(flags->data);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ipc_flags_is_set(const ina_ipc_flags_t* flags, uint64_t value)
{
    INA_VERIFY_NOT_NULL(flags);
    INA_VERIFY_NOT_NULL(flags->data);
    if ((value&flags->data->v) == (value)) {
        return INA_SUCCESS;
    }
    return INA_ERROR(INA_ES_OPERATION | INA_ERR_FAILED);
}

INA_API(ina_rc_t) ina_ipc_flags_unset(ina_ipc_flags_t *flags, uint64_t value)
{
    uint64_t j;
    INA_VERIFY_NOT_NULL(flags);
    INA_VERIFY_NOT_NULL(flags->data);
    __INA_ENTER_LOCK(flags->data);

    /* Unset flag if reference count is zero */
    for (j = 0; j < INA_IPC_FLAGS_MAX; ++j) {
        if ((value & ( 1ULL << j)) >> j) {
            if (flags->data->c_ref[j] > 0 && --flags->data->c_ref[j] == 0) {
                uint64_t mask = 0;
                mask = 1ULL << (uint64_t)(j)|0;
                INA_ATOMIC_SWAP((int64_t*)&flags->data->v, flags->data->v, flags->data->v & ~(mask));
            }
        }
    }
    __INA_EXIT_LOCK(flags->data);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ipc_flags_clear(ina_ipc_flags_t *flags, uint64_t value)
{
    uint64_t j;
    INA_VERIFY_NOT_NULL(flags);
    INA_VERIFY_NOT_NULL(flags->data);
    __INA_ENTER_LOCK(flags->data);

    /* Unset flag if reference count is zero */
    for (j = 0; j < INA_IPC_FLAGS_MAX; ++j) {
        if ((value & ( 1ULL << j)) >> j) {
            uint64_t mask = 0;
            mask = 1ULL << (uint64_t)(j)|0;
            INA_ATOMIC_SWAP((int64_t*)&flags->data->v, flags->data->v, flags->data->v & ~(mask));
            flags->data->c_ref[j] = 0;
        }
    }
    __INA_EXIT_LOCK(flags->data);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ipc_flags_wait(const ina_ipc_flags_t* flags, uint64_t wait_for, time_t msec_timeout)
{
    ina_timer_event_t *event;
    int timeout = INA_NO;

    INA_VERIFY_NOT_NULL(flags);

    INA_RETURN_IF_FAILED(ina_timer_event_new(flags->timer, msec_timeout, &event));

    while (!INA_SUCCEED(ina_ipc_flags_is_set(flags, wait_for))) {
        if (INA_SUCCEED(ina_timer_next_event(flags->timer, &event))) {
            timeout = INA_YES;
            break;
        }
    }
    ina_timer_event_free(flags->timer, event);

    if (timeout == INA_YES) {
        return INA_ERROR(INA_ES_OPERATION | INA_ERR_TIMED_OUT);
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ipc_flags_dump(const ina_ipc_flags_t *flags)
{
    uint64_t j;
    INA_VERIFY_NOT_NULL(flags);
    __INA_ENTER_LOCK(flags->data);

    printf("IPC flags :%s\n", flags->data->name);
    /* Unset flag if reference count is zero */
    for (j = 0; j < INA_IPC_FLAGS_MAX; ++j) {
        printf(" - %3"INA_UINT64_T_FMT":", j);
        if ((flags->data->v & ( 1ULL << j)) >> j) {
           printf("ON  (c_ref=%d)\n", flags->data->c_ref[j]);
        } else {
           printf("OFF (c_ref=%d)\n", flags->data->c_ref[j]);            
        }
    }

    __INA_EXIT_LOCK(flags->data);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ipc_counter_new(const char* name, uint64_t initial, ina_ipc_counter_t **counter)
{
    ina_str_t mname;

	INA_UNUSED(initial);
    INA_VERIFY_NOT_NULL(counter);
    INA_VERIFY_NOT_NULL(name);
    INA_VERIFY(strlen(name) < INA_IPC_COUNTER_NAME_MAXLEN);

    *counter = (ina_ipc_counter_t*)ina_mem_alloc(sizeof(ina_ipc_counter_t));
    INA_RETURN_IF_NULL(*counter);
    INA_MEM_SET_ZERO(*counter, ina_ipc_counter_t);

    mname = ina_str_sprintf("/ina_ipc_counter_%s", name);

    INA_FAIL_IF_ERROR(ina_mempool_new(sizeof(ina_ipc_counter_data_t),mname,
                     INA_MEM_SHARED|INA_MEM_SHARED_CREATE|INA_MEM_SHARED_EXCL, 
                      &(*counter)->m));

    (*counter)->data = (ina_ipc_counter_data_t *) ina_mempool_dalloc((*counter)->m, sizeof(ina_ipc_counter_data_t));
    INA_STR_FREE_SAFE(mname);
    INA_FAIL_IF_ERROR(ina_ipc_counter_set(*counter, initial));
    return INA_SUCCESS;

fail:
    INA_STR_FREE_SAFE(mname);
    ina_ipc_counter_free(counter);
    return ina_err_get_rc();
}

INA_API(ina_rc_t) ina_ipc_counter_open(const char* name, ina_ipc_counter_t **counter)
{
    ina_str_t mname;

    INA_VERIFY_NOT_NULL(counter);
    INA_VERIFY_NOT_NULL(name);
    INA_VERIFY(strlen(name) < INA_IPC_COUNTER_NAME_MAXLEN);

    *counter = (ina_ipc_counter_t*)ina_mem_alloc(sizeof(ina_ipc_counter_t));
    INA_RETURN_IF_NULL(*counter);
    INA_MEM_SET_ZERO(*counter, ina_ipc_counter_t);

    mname = ina_str_sprintf("/ina_ipc_counter_%s", name);

    INA_FAIL_IF_ERROR(ina_mempool_new(sizeof(ina_ipc_counter_data_t),mname,
                     INA_MEM_SHARED, &(*counter)->m));

    (*counter)->data = (ina_ipc_counter_data_t *) ina_mempool_dalloc((*counter)->m, sizeof(ina_ipc_counter_data_t));

    INA_STR_FREE_SAFE(mname);
    return INA_SUCCESS;

fail:
    INA_STR_FREE_SAFE(mname);
    ina_ipc_counter_free(counter);
    return ina_err_get_rc();
}

INA_API(void) ina_ipc_counter_free(ina_ipc_counter_t **counter)
{
    INA_VERIFY_FREE(counter);
    ina_mempool_free(&(*counter)->m);
    INA_MEM_FREE_SAFE(*counter);
}

INA_API(ina_rc_t) ina_ipc_counter_get(const ina_ipc_counter_t *counter, uint64_t *value)
{
    INA_VERIFY_NOT_NULL(counter);
    INA_VERIFY_NOT_NULL(value);
    *value = counter->data->c;
    return INA_SUCCESS;
}

INA_API(uint64_t) ina_ipc_counter_increment(ina_ipc_counter_t *counter, uint64_t value)
{
    INA_VERIFY_NOT_NULL(counter);
    INA_ATOMIC_SWAP((int64_t*)&counter->data->c, counter->data->c, counter->data->c + value);
    return counter->data->c;
}

INA_API(uint64_t) ina_ipc_counter_decrement(ina_ipc_counter_t *counter, uint64_t value)
{
    INA_VERIFY_NOT_NULL(counter);
    INA_ATOMIC_SWAP((int64_t*)&counter->data->c, counter->data->c, counter->data->c - value);
    return counter->data->c;
}

INA_API(ina_rc_t) ina_ipc_counter_set(ina_ipc_counter_t *counter, uint64_t value)
{
    uint64_t v;
    INA_VERIFY_NOT_NULL(counter);

    v = counter->data->c;
    if (v == value) {
        return INA_SUCCESS;
    }

    INA_ATOMIC_SWAP((int64_t*)&counter->data->c, v, value);
    if (v == counter->data->c) {
        return INA_ERROR(INA_ES_OPERATION | INA_ERR_FAILED);
    }
    return INA_SUCCESS;
}
