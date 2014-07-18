/*
 * Copyright (c) 2013-2014, INAOS GmbH
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

#define INA_IPC_FLAG_NAME_MAXLEN  (64)
/*
 * Opaque types for IPC flag
 */
typedef struct ina_ipc_flag_data_s ina_ipc_flag_data_t;
typedef struct ina_ipc_flag_s ina_ipc_flag_t;

/*
 * Create or open an new IPC flag
 */
INA_API(ina_rc_t) ina_ipc_flag_new(const char* name, uint64_t initial, ina_ipc_flag_t **flag);
/* 
 * Free IPC flag
 */
INA_API(ina_rc_t) ina_ipc_flag_free(ina_ipc_flag_t **flag);

/*
 * Get flag name
 */
INA_API(ina_rc_t) ina_ipc_flag_get_name(const ina_ipc_flag_t *flag, const char **name);

/* 
 * Get flag
 */
INA_API(ina_rc_t) ina_ipc_flag_get(const ina_ipc_flag_t *flag, uint64_t *value);
/* 
 * Set flag mask
 */
INA_API(ina_rc_t) ina_ipc_flag_set(ina_ipc_flag_t *flag, uint64_t value);
/* 
 * Query flag mask
 */
INA_API(ina_rc_t) ina_ipc_flag_is_set(const ina_ipc_flag_t *flag, uint64_t value);
/* 
 * Unset flag mask
 */
INA_API(ina_rc_t) ina_ipc_flag_unset(ina_ipc_flag_t *flag, uint64_t value);
/* 
 * Wait until flags are set
 */
INA_API(ina_rc_t) ina_ipc_flag_wait(const ina_ipc_flag_t* flag, uint64_t wait_for, time_t msec_timeout);


#ifdef __cplusplus
}
#endif
#endif
