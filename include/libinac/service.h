

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

/* service status */
typedef enum ina_service_status_e {
  INA_SERVICE_STATUS_STOP = 0,    /* After shutdown terninated */
  INA_SERVICE_STATUS_START,        /* Starting */
  INA_SERVICE_STATUS_RUN,          /* Running */
  INA_SERVICE_STATUS_SHUTDOWN,     /* Service should shutdown */
  INA_SERVICE_STATUS_INSTALL,      /* Service install request */
  INA_SERVICE_STATUS_UNINSTALL,    /* Service uninstall request */
  INA_SERVICE_STATUS_REPORT,       /* Service report request */ 
  INA_SERVICE_STATUS_ERROR,        /* Service error, stopped  */
  INA_SERVICE_STATUS_INIT          /* Before before initialization */
} ina_service_status_t;

typedef ina_rc_t (*ina_service_fn_t)(const ina_service_ctx_t *ctx, ina_service_status_t status, void *user_data);

typedef enum ina_service_mode_e {
    INA_SERVICE_MODE_SERVICE,
    INA_SERVICE_MODE_CONSOLE
} ina_service_mode_t;

typedef enum ina_service_startup_type_e {
    INA_SERVICE_STARTUP_TYPE_AUTO,
    INA_SERVICE_STARTUP_TYPE_MANUAL,
} ina_service_startup_type_t;

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

#define INA_SERVICE_OPT_NAME                 "service"
#define INA_SERVICE_CMD_INSTALL              "install"
#define INA_SERVICE_CMD_UNINSTALL            "uninstall"
#define INA_SERVICE_CMD_CONSOLE              "console"
#define INA_SERVICE_CMD_DEAMON               "deamon"
#define INA_SERVICE_CMD_REPORT               "report"

#define INA_SERVICE_NAME_MAXLEN              (64)
#define INA_SERVICE_DISPLAY_NAME_MAXLEN      (64)
#define INA_SERVICE_USERNAME_MAXLEN          (64)
#define INA_SERVICE_PASSORD_MAXLEN           (64)
#define INA_SERVICE_STARTUP_ARGS_MAXLEN      (512)
#define INA_SERVICE_DESCRIPTION_MAXLEN       (128)
#define INA_SERVICE_WORKINGDIR_MAXLEN        (1024)
#define INA_SERVICE_CHKCONFIG_MAXLEN         (24)

#define INA_OPT_SERVICE INA_OPT_STRING(NULL, "service", "console", "Service control: install, uninstall, deamon, console, report")

/* Service descriptor */
typedef struct ina_service_descriptor_s {
    char name[INA_SERVICE_NAME_MAXLEN];
    char display_name[INA_SERVICE_DISPLAY_NAME_MAXLEN];
    char description[INA_SERVICE_DESCRIPTION_MAXLEN];
    char username[INA_SERVICE_USERNAME_MAXLEN];
    char password[INA_SERVICE_PASSORD_MAXLEN];
    char startup_args[INA_SERVICE_STARTUP_ARGS_MAXLEN];
    char working_directory[INA_SERVICE_WORKINGDIR_MAXLEN];
    char chkconfig[INA_SERVICE_CHKCONFIG_MAXLEN];
    ina_service_fn_t service_fn;
    ina_service_startup_type_t startup;
    int32_t exclusive_flag;
} ina_service_descriptor_t;

/* Setup service section  */
#define INA_SERVICE_DESCRIPTOR(name, display_name,                           \
                               description,                                  \
                               username, password,                           \
                               startup_args,                                 \
                               working_directory,                            \
                               chkconfig,                                    \
                               service_fn,                                   \
                               startup,                                      \
                               exclusive_flag)                               \
INA_SERVICE_SECTION_PUSH ina_service_descriptor_t __ina_service_section INA_SERVICE_SECTION = { \
        name,                                                                \
        display_name,                                                        \
        description,                                                         \
        username,                                                            \
        password,                                                            \
        startup_args,                                                        \
        working_directory,                                                   \
        chkconfig,                                                           \
        service_fn,                                                          \
        startup,                                                             \
        exclusive_flag}

/*
 *
 */
INA_API(ina_rc_t) ina_service_init(ina_service_ctx_t **ctx);

/*
 * 
 */
INA_API(ina_rc_t) ina_service_destroy(ina_service_ctx_t **ctx);

/*
 *
 */
INA_API(ina_rc_t) ina_service_dispatch(const ina_service_ctx_t *ctx, const void *user_data);


/*
 *
 */
INA_API(ina_rc_t) ina_service_get_data(const ina_service_ctx_t *ctx, const void **user_data);

/*
 *
 */
INA_API(ina_rc_t) ina_service_set_data(const ina_service_ctx_t *ctx, const void *user_data);

/*
 *
 */
INA_API(ina_rc_t) ina_service_get_descriptor(const ina_service_ctx_t *ctx, 
                                    ina_service_descriptor_t **descriptor);
/*
 * WIN: Installs the app as a service via the Service API
 * UNX: Installs the app as a deamon and enable service <app> commands.
 *      It stores the servicescript in a section in the binary and 
 *      copy it to /etc/init.d upon install
 */
INA_API(ina_rc_t) ina_service_install(const ina_service_ctx_t *ctx);

/*
 * 
 */
INA_API(ina_rc_t) ina_service_uninstall(const ina_service_ctx_t *ctx);

/*
 * 
 */
INA_API(ina_rc_t) ina_service_run_service(const ina_service_ctx_t *ctx, int console, const void *user_data);

/*
 * 
 */
INA_API(ina_rc_t) ina_service_get_mode(const ina_service_ctx_t *ctx, 
                                       ina_service_mode_t *mode);

/*
 *
 */
INA_API(ina_rc_t) ina_service_is_deamon(const ina_service_ctx_t *ctx);

/*
 *
 */
INA_API(ina_rc_t) ina_service_mgnt_start(const char *name);

/*
 *
 */
INA_API(ina_rc_t) ina_service_mgnt_stop(const char *name);

/*
 *
 */
INA_API(ina_rc_t) ina_service_mgnt_status(const char *name, ina_service_status_t *status);
#ifdef __cplusplus
}
#endif

#endif

