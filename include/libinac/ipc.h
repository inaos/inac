/*
 * Copyright (c) 2013-2016, INAOS GmbH
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
#ifndef _LIBINAC_IPC_H_
#define _LIBINAC_IPC_H_

#include <libinac/lib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define INA_IPC_FLAGS_IGNORE       (-1)
#define INA_IPC_FLAGS_NAME_MAXLEN  (64)

#define INA_IPC_FLAGS_1   0x00000000000000001
#define INA_IPC_FLAGS_2   0x00000000000000002
#define INA_IPC_FLAGS_3   0x00000000000000004
#define INA_IPC_FLAGS_4   0x00000000000000010
#define INA_IPC_FLAGS_5   0x00000000000000020
#define INA_IPC_FLAGS_6   0x00000000000000040
#define INA_IPC_FLAGS_7   0x00000000000000080
#define INA_IPC_FLAGS_8   0x00000000000000100
#define INA_IPC_FLAGS_9   0x00000000000000200
#define INA_IPC_FLAGS_10  0x00000000000000400
#define INA_IPC_FLAGS_11  0x00000000000000800
#define INA_IPC_FLAGS_12  0x00000000000001000
#define INA_IPC_FLAGS_13  0x00000000000002000
#define INA_IPC_FLAGS_14  0x00000000000004000
#define INA_IPC_FLAGS_15  0x00000000000008000
#define INA_IPC_FLAGS_16  0x00000000000010000
#define INA_IPC_FLAGS_17  0x00000000000020000
#define INA_IPC_FLAGS_18  0x00000000000040000
#define INA_IPC_FLAGS_19  0x00000000000080000
#define INA_IPC_FLAGS_20  0x00000000000100000
#define INA_IPC_FLAGS_21  0x00000000000200000
#define INA_IPC_FLAGS_22  0x00000000000400000
#define INA_IPC_FLAGS_23  0x00000000000800000
#define INA_IPC_FLAGS_24  0x00000000001000000
#define INA_IPC_FLAGS_25  0x00000000002000000
#define INA_IPC_FLAGS_26  0x00000000004000000
#define INA_IPC_FLAGS_27  0x00000000008000000
#define INA_IPC_FLAGS_28  0x00000000010000000
#define INA_IPC_FLAGS_29  0x00000000020000000
#define INA_IPC_FLAGS_30  0x00000000040000000
#define INA_IPC_FLAGS_31  0x00000000080000000
#define INA_IPC_FLAGS_32  0x00000000100000000
#define INA_IPC_FLAGS_33  0x00000000200000000
#define INA_IPC_FLAGS_34  0x00000000400000000
#define INA_IPC_FLAGS_35  0x00000000800000000
#define INA_IPC_FLAGS_36  0x00000001000000000
#define INA_IPC_FLAGS_37  0x00000002000000000
#define INA_IPC_FLAGS_38  0x00000004000000000
#define INA_IPC_FLAGS_39  0x00000008000000000
#define INA_IPC_FLAGS_40  0x00000010000000000
#define INA_IPC_FLAGS_41  0x00000020000000000
#define INA_IPC_FLAGS_42  0x00000040000000000
#define INA_IPC_FLAGS_43  0x00000080000000000
#define INA_IPC_FLAGS_44  0x00000100000000000
#define INA_IPC_FLAGS_45  0x00000200000000000
#define INA_IPC_FLAGS_46  0x00000400000000000
#define INA_IPC_FLAGS_47  0x00000800000000000
#define INA_IPC_FLAGS_48  0x00001000000000000
#define INA_IPC_FLAGS_49  0x00002000000000000
#define INA_IPC_FLAGS_50  0x00004000000000000
#define INA_IPC_FLAGS_51  0x00008000000000000
#define INA_IPC_FLAGS_52  0x00010000000000000
#define INA_IPC_FLAGS_53  0x00020000000000000
#define INA_IPC_FLAGS_54  0x00040000000000000
#define INA_IPC_FLAGS_55  0x00080000000000000
#define INA_IPC_FLAGS_56  0x00100000000000000
#define INA_IPC_FLAGS_57  0x00200000000000000
#define INA_IPC_FLAGS_58  0x00400000000000000
#define INA_IPC_FLAGS_59  0x00800000000000000
#define INA_IPC_FLAGS_60  0x01000000000000000
#define INA_IPC_FLAGS_61  0x02000000000000000
#define INA_IPC_FLAGS_62  0x04000000000000000
#define INA_IPC_FLAGS_63  0x08000000000000000
#define INA_IPC_FLAGS_64  0x10000000000000000
#define INA_IPC_FLAGS_LAST INA_IPC_FLAGS_64
#define INA_IPC_FLAGS_MAX 64
#define INA_IPC_FLAGS_ALL 0xFFFFFFFFFFFFFFFF
#define INA_IPC_COUNTER_NAME_MAXLEN  (64)

/*
 * Opaque types for IPC flag
 */
typedef struct ina_ipc_flags_data_s ina_ipc_flags_data_t;
typedef struct ina_ipc_flags_s ina_ipc_flags_t;

/*
 * Opaque types for IPC counter
 */
typedef struct ina_ipc_counter_data_s ina_ipc_counter_data_t;
typedef struct ina_ipc_counter_s ina_ipc_counter_t;

/*
 * Create a new IPC flags.
 *
 * Parameters
 *  name     Name of flags. Must be a unique name at host level.
 *  initial  Defines initial flags value
 *  flags     Where to store the newly created IPC flags
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_ipc_flags_new(const char* name,
                                    int64_t initial,
                                    ina_ipc_flags_t **flags);

/*
 * Open an new IPC flags. A IPC flag must be created using ina_ipc_flags_new()
 * before i can be opened.
 *
 * Parameters
 *  name  Name of flags to open.
 *  flag  Where to store the IPC flags
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_ipc_flags_open(const char* name, ina_ipc_flags_t **flags);

/* 
 * Destroy IPC flags.
 *
 * Parameters
 *  flag  IPC flags to free
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_ipc_flags_free(ina_ipc_flags_t **flags);

/*
 * Get the name of a IPC flag.
 *
 * Parameters
 *  flags  IPC flags
 *  name   Where the store the name
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_ipc_flags_get_name(const ina_ipc_flags_t *flags,
                                         const char **name);

/* 
 * Get the current value of IPC flags.
 *
 * Parameters
 *  flags  IPC flags
 *  value  Where to store the current value
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_ipc_flags_get(const ina_ipc_flags_t *flags,
                                    uint64_t *value);

/* 
 * Turn on one or more IPC flags. For each turned on flag a reference counter
 * is incremented.
 *
 * Parameters
 *  flags  IPC flags
 *  value  Bit mask
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_ipc_flags_set(ina_ipc_flags_t *flags, uint64_t value);

/* 
 * Query if one or more flags are set (on).
 *
 * Parameters
 *  flags  IPC flags
 *  value  Bit mask
 *
 * Return
 * INA_SUCCESS if all flags defined by the bit mask are set (on)
 * INA_FAILURE if one or more flags defined by the bit mask are not set (off)
 */
INA_API(ina_rc_t) ina_ipc_flags_is_set(const ina_ipc_flags_t *flags,
                                       uint64_t value);

/* 
 * Unset one or more IPC Flags. For each turned off flag a reference counter
 * is decremented.
 *
 * Parameters
 *  flags  IPC flags
 *  value  Bit mask to turn off one or more flags
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_ipc_flags_unset(ina_ipc_flags_t *flags, uint64_t value);

/*
 * Clear one or more IPC Flags. For each cleared flag his reference counter
 * is set to 0.
 *
 * Parameters
 *  flags  IPC flags
 *  value  Bit mask to clear one or more flags
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_ipc_flags_clear(ina_ipc_flags_t *flags, uint64_t value);


/* 
 * Wait until flags are set.
 *
 * Parameters
 *  flags         IPC flags
 *  wait_for      Bit mask defining IPC flags waiting for.
 *  msec_timeout  Number of milliseconds before timeout occurs.
 *
 * Return
 *  INA_SUCCESS if all went well.
 *  INA_FAILURE if timeout occurred
 *
 * FIXME: Return specific error when timeout occurs
 */
INA_API(ina_rc_t) ina_ipc_flags_wait(const ina_ipc_flags_t *flags,
                                     uint64_t wait_for,
                                     time_t msec_timeout);

/*
 * Dumps IPC flags to the standard output
 *
 * Parameters
 *  flags  IPC flags
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_ipc_flags_dump(const ina_ipc_flags_t *flags);

/*
 * Create an new IPC counter.
 *
 * Parameters
 *  name     Counter name. Must be unique on host level
 *  initial  Initial counter value
 *  counter  Where to store the newly created counter
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_ipc_counter_new(const char* name,
                                      uint64_t initial,
                                      ina_ipc_counter_t **counter);

/*
 * Open an IPC counter. A counter must be created by calling ina_ipc_counter_new()
 * before it can be opened.
 *
 * Parameters
 *  name     Counter name
 *  counter  Where to store the opened counter
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_ipc_counter_open(const char* name,
                                       ina_ipc_counter_t **counter);

/* 
 * Destroy an IPC counter.
 *
 * Parameters
 *  counter  IPC counter to free.
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_ipc_counter_free(ina_ipc_counter_t **counter);

/*
 * Get current value of an IPC counter.
 *
 * Parameters
 *  counter  IPC counter
 *  value    Where to store the current counter value
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_ipc_counter_get(const ina_ipc_counter_t *counter,
                                      uint64_t *value);

/*
 * Increment an IPC counter by a value (e.g. 1)
 *
 * Parameters
 *  counter  IPC counter
 *  value    Value to add
 *
 * Return
 *  INA_SUCCESS
 *
 * FIXME: should at least return INA_FAULIRE if atomic operation failed
 */
INA_API(ina_rc_t) ina_ipc_counter_increment(ina_ipc_counter_t *counter,
                                            uint64_t value);

/*
 * Decrement an IPC counter by a value (e.g. 1)
 *
 * Parameters
 *  counter  IPC counter
 *  value    Value to subtract
 *
 * Return
 *  INA_SUCCESS
 *
 * FIXME: should at least return INA_FAULIRE if atomic operation failed
 */
INA_API(ina_rc_t) ina_ipc_counter_decrement(ina_ipc_counter_t *counter,
                                            uint64_t value);

/*
 * Set an IPC counter to a specific value.
 *
 * Parameters
 *  counter  IPC counter
 *  value    New counter value
 *
 * Return
 *  INA_SUCCESS
 *
 * FIXME: should at least return INA_FAULIRE if atomic operation failed
 */
INA_API(ina_rc_t) ina_ipc_counter_set(ina_ipc_counter_t *counter,
                                      uint64_t value);

#ifdef __cplusplus
}
#endif
#endif
