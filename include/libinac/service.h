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
#ifndef _LIBINAC_SERVICE_H_
#define _LIBINAC_SERVICE_H_

#include <libinac/lib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* opaque service context */
typedef struct ina_service_ctx_s ina_service_ctx_t;

typedef ina_rc_t (*ina_service_run)(void *user_data);
typedef ina_rc_t (*ina_service_shutdown)(void *user_data);

typedef enum ina_service_mode_e {
    INA_SERVICE_MODE_SERVICE,
    INA_SERVICE_MODE_CONSOLE
} ina_service_mode_t;

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
    ina_service_run run_func;
    ina_service_shutdown shutdown_func;
    ina_service_startup_type_t startup;
    ina_str_t working_directory;
    ina_str_t startup_args;
    int exclusive_flag;
    void *user_data;
} ina_service_descriptor_t;

/* Section holding service descriptor */
#ifndef INA_OS_WIN32
#ifdef INA_OS_OSX
#define INA_SERVICE_SECTION __attribute__ ((unused,section ("__DATA, .inaservice")))
#define INA_SERVICE_SECTION_PUSH
#else
#define INA_SERVICE_SECTION __attribute__ ((unused,section (".inaservice")))
#define INA_SERVICE_SECTION_PUSH
#endif
#else
#pragma section(".inaservice", read)
#define INA_SERVICE_SECTION
#define INA_SERVICE_SECTION_PUSH __declspec(allocate(".inaservice"))
#endif

#define INA_SERVICE_NAME_MAXLEN              (64)
#define INA_SERVICE_USERNAME_MAXLEN          (64)
#define INA_SERVICE_PASSWORD_MAXLEN          (64)
#define INA_SERVICE_STARTUP                  (255)
#define INA_SERVICE_SHORT_DESCRIPTION_MAXLEN (128)
#define INA_SERVICE_LONG_DESCRIPTION_MAXLEN  (1024)


/* Service section for Unix deamons */
typedef struct ina_service_section_s {
    char name[INA_SERVICE_NAME_MAXLEN];
    char username[INA_SERVICE_USERNAME_MAXLEN];
    char password[INA_SERVICE_PASSWORD_MAXLEN];
    char startup[INA_SERVICE_STARTUP];
    char short_description[INA_SERVICE_SHORT_DESCRIPTION_MAXLEN];
    char long_description[INA_SERVICE_LONG_DESCRIPTION_MAXLEN];
} ina_service_section_t;

/* Setup service section  */
#define INA_SERVICE_SETUP(name, username, password, startup,                 \
                          short_description, long_description)               \
    INA_SERVICE_SECTION_PUSH ina_service_section_t __ina_service_section INA_SERVICE_SECTION = { \
        name,                                                               \
        username,                                                           \
        password,                                                           \
        startup,                                                            \
        short_description,                                                  \
        long_description                                                    \
    }
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
/*
 * 
 */
INA_API(ina_rc_t) ina_service_mode(ina_service_ctx_t *ctx, ina_service_mode_t *mode);

#ifdef __cplusplus
}
#endif

#endif
