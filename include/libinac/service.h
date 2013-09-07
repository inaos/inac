/*
 * Copyright (c) 2013, INAOS GmbH
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
#ifndef _LIBINAC_SERVICE_H_
#define _LIBINAC_SERVICE_H_

#include <libinac/lib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* opaque service context */
typedef struct ina_service_ctx_s ina_service_ctx_t;

typedef ina_rc_t (*ina_service_main)(void *user_data);

typedef enum ina_service_startup_type_e {
    INA_SERVICE_STARTUP_TYPE_AUTO,
    INA_SERVICE_STARTUP_TYPE_MANUAL,
} ina_service_startup_type_t;

typedef struct ina_service_descriptor_s {
    ina_str_t name;
    ina_str_t display_name;
    ina_str_t short_description;
    ina_str_t long_description;
    ina_str_t username;
    ina_str_t password;
    ina_service_main main_func;
    ina_service_startup_type_t startup;
    int exclusive_flag;
} ina_service_descriptor_t;

/*
 * 
 */
INA_API(ina_rc_t) ina_service_init(ina_service_ctx_t **ctx);
/*
 * 
 */
INA_API(ina_rc_t) ina_service_destroy(ina_service_ctx_t **ctx);
/*
 * WIN: Installs the app as a service via the Service API
 * UNX: Installs the app as a deamon and enable service <app> commands.
 *      It stores the servicescript in a section in the binary and 
 *      copy it to /etc/init.d upon install
 */
INA_API(ina_rc_t) ina_service_install(ina_service_ctx_t *ctx, ina_service_descriptor_t *descriptor);
/*
 * 
 */
INA_API(ina_rc_t) ina_service_uninstall(ina_service_ctx_t *ctx, ina_service_descriptor_t *descriptor);
/*
 * 
 */
INA_API(ina_rc_t) ina_service_run_service(ina_service_ctx_t *ctx, ina_service_descriptor_t *descriptor);
/*
 * 
 */
INA_API(ina_rc_t) ina_service_run_console(ina_service_ctx_t *ctx, ina_service_descriptor_t *descriptor);

#ifdef __cplusplus
}
#endif

#endif
