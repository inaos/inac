/*
 * Copyright (c) 2012-2015, INAOS GmbH
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
#ifndef _LIBINAC_LIB_H_
#define _LIBINAC_LIB_H_

#ifndef _WIN32
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <sys/types.h>
#include <sys/param.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/sem.h>
#include <sys/syslog.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <execinfo.h>
#include <spawn.h>
#include <unistd.h>
#include <inttypes.h>
#endif

#ifdef _WIN32
#include <windows.h>
#include <minwindef.h>
#include <wincon.h>
#include <io.h>
#endif

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <time.h>
#include <ctype.h>

#include <contribs/luajit/src/luajit.h>
#include <contribs/luajit/src/lauxlib.h>
#include <contribs/luajit/src/lualib.h>

#include <libinac/portable.h>
#include <libinac/types.h>
#include <libinac/uthash.h>
#include <libinac/memory.h>
#include <libinac/string.h>
#include <libinac/log.h>
#include <libinac/error.h>
#include <libinac/time.h>
#include <libinac/timer.h>
#include <libinac/ullc.h>
#include <libinac/net.h>
#include <libinac/iscp.h>
#include <libinac/ljit.h>
#include <libinac/conffile.h>
#include <libinac/http.h>
#include <libinac/dns.h>
#include <libinac/ssl.h>
#include <libinac/xml.h>
#include <libinac/json.h>
#include <libinac/util.h>
#include <libinac/cio.h>
#include <libinac/cron.h>
#include <libinac/fsm.h>
#include <libinac/service.h>
#include <libinac/process.h>
#include <libinac/ipc.h>
#include <libinac/template.h>
#include <libinac/cpu.h>
#include <libinac/compression.h>
#include <libinac/file.h>
#include <libinac/mmap.h>
#include <libinac/file_cursor.h>
#include <libinac/histogram.h>
#include <libinac/pcap.h>
#include <libinac/gzip.h>
#include <libinac/uthash.h>
#include <libinac/utlist.h>
#include <libinac/debug.h>
#include <libinac/test.h>


#ifdef __cplusplus
extern "C" {
#endif

#if defined(INA_MBTIME_ENABLED) && defined(INA_OS_OSX)
#error "Meinberg time backend not supported."
#endif

#define INA_YES (1)
#define INA_NO  (0)

#define INA_NUM2STR_X(x) #x
#define INA_NUM2STR(x) INA_NUM2STR_X(x)
/*
 * Version
 */
#define INA_MAJOR_VERSION 0
#define INA_MINOR_VERSION 2
#define INA_MICRO_VERSION 0

#define INA_VERSION       "0.2.0"

/* Version as a 3-byte hex number, e.g. 0x010201 == 1.2.1. Use this
 * for numeric comparisons, e.g. #if INA_VERSION_HEX >= ... */
#define INA_VERSION_HEX  ((INA_MAJOR_VERSION << 16) |   \
                          (INA_MINOR_VERSION << 8)  |   \
                          (INA_MICRO_VERSION << 0))

/* Add flag option */
#define INA_OPT_FLAG(short_opt, long_opt, desc)           \
 { short_opt, long_opt, INA_OPT_TYPE_FLAG, NULL, desc }

/* Add string option */
#define INA_OPT_STRING(short_opt, long_opt, dft, desc)    \
 { short_opt, long_opt, INA_OPT_TYPE_STRING, dft, desc }
 
/* Add int option */
#define INA_OPT_INT(short_opt, long_opt, dft, desc)       \
 { short_opt, long_opt, INA_OPT_TYPE_INT, INA_NUM2STR(dft), desc }

/* Add float option */
#define INA_OPT_FLOAT(short_opt, long_opt, dft, desc)       \
 { short_opt, long_opt, INA_OPT_TYPE_FLOAT, INA_NUM2STR(dft), desc }

/* Define options map */
#define INA_OPTS(name, ...)                        \
ina_opt_t name[] = {                               \
    __VA_ARGS__,                                   \
    {NULL, NULL, INA_OPT_TYPE_INT, NULL, NULL}     \
};

typedef enum ina_opt_type_e {
    INA_OPT_TYPE_STRING = 0,
    INA_OPT_TYPE_INT,
    INA_OPT_TYPE_FLAG,
    INA_OPT_TYPE_FLOAT
} ina_opt_type_t;
    
/* Command line option builder */
typedef struct ina_opt_s {
    const char *short_opt;  /* short option, nomally 1 char */
    const char *long_opt;   /* long option */
    ina_opt_type_t type;    /* option type */
    const char *dft;        /* default value */
    const char *desc;       /* short description, used in usage */
} ina_opt_t;

typedef enum ina_signal_e {
    INA_SIGNAL_FPE = 1,
    INA_SIGNAL_ABRT,
    INA_SIGNAL_ILL,
    INA_SIGNAL_INT,
    INA_SIGNAL_SEGV,
    INA_SIGNAL_TERM,
    #ifndef INA_OS_WIN32
    INA_SIGNAL_HUP,
    INA_SIGNAL_QUIT,
    INA_SIGNAL_KILL,
    INA_SIGNAL_STOP,
    INA_SIGNAL_TTOU,
    INA_SIGNAL_TTIN
    #endif
 } ina_signal_t;

/* Signal handling behavior */
typedef enum ina_signal_behavior_e {
    INA_SIGNAL_BEHAVIOR_DFT,      /* Default behavior */
    INA_SIGNAL_BEHAVIOR_IGNORE    /* Ignore default behavior */
} ina_signal_behavior_t;

/* Application cleanup handler . */
typedef void (*ina_cleanup_handler_t) (int, int*);

/* Signal handler */
typedef void (*ina_signal_handler_t) (ina_signal_t, ina_signal_behavior_t*, int*);

/*
 * Return the program name
 */
INA_API(const char*) ina_app_get_name(void);

/*
 * Return path to the running application
 */
INA_API(const char*) ina_app_get_path(void);

/*
 * Startup application with argc, argv in order to deal with 
 * platform-specific quirks. This must be the first function called for any
 * program.
 *
 * Parameters:
 *  argc      -  argc of main() function
 *  argv      -  Pointer to the argv of main() function
 *  pool_size - Initial size of internal memory pool. if 0 passed a pool
 *              with size INA_MEM_DFT_POOL_SIZE will be created.
 *  opt         Array of options to parse
 *
 * Return:
 * INA_SUCCESS  if no error occured
 */
INA_API(ina_rc_t) ina_app_init(const int argc,  char **argv, size_t pool_size, ina_opt_t *opt);

/*
 * Get the string key and value of an option at index.
 *
 * Parameters:
 *  index   options index starting by 0
 *  key     long name of option
 *  value   option value as string
 *
 * Return Value
 * INA_SUCCESS if option is available otherwise INA_FAILURE
 */
INA_API(ina_rc_t) ina_opt_get_key_value(int index, ina_str_t *key, ina_str_t *value);
/*
 * Check whenever an option is available.
 *
 * Parameters:
 *  opt   name of option
 *
 * Return Value
 * INA_SUCCESS if option is available
 */
INA_API(ina_rc_t) ina_opt_isset(const char *opt);
/*
 * Get the string value of an option.
 *
 * Parameters:
 *  opt     name of option
 *  value
 *
 * Return Value
 * INA_SUCCESS if option is available
 */
INA_API(ina_rc_t) ina_opt_get_string(const char *opt, ina_str_t *value);
/*
 * Get the integer value of an option.
 *
 * Parameters:
 *  opt     name of option
 *  value
 *
 * Return Value
 * INA_SUCCESS if option is available
 */
INA_API(ina_rc_t) ina_opt_get_int(const char *opt, int *value);

/*
 * Get the float value of an option.
 *
 * Parameters:
 *  opt     name of option
 *  value
 *
 * Return Value
 * INA_SUCCESS if option is available
 */
INA_API(ina_rc_t) ina_opt_get_float(const char *opt, float *value);

/*
 * Initialize all internal data structures. This must be the first function 
 * called for any library.
 *
 * Parameters:
 *  pool_size - Initial size of internal memory pool. if 0 passed a pool
 *              with size INA_MEM_DFT_POOL_SIZE will be created.
 * Return:
 * INA_SUCCESS  if no error occured
 */
INA_API(ina_rc_t) ina_init(size_t pool_size);

/*
 * Set a custom termination routine to call in case of an 
 * a terminiation signal. The purpose of such a routine is to give consumers
 * a last chance to cleanup before the program exits.
 *
 * Parameters
 * handler  Cleanup routine. A cleanup should return EXIT_SUCCESS or 
 *          EXIT_FAILURE depending on type of signal. On a programm error
 *          the return of cleanup routines will be ignored. 
 *
 * Return Value
 * Previously defined handler
 */
INA_API(ina_cleanup_handler_t) ina_set_cleanup_handler(
                                        ina_cleanup_handler_t handler);


INA_API(ina_signal_handler_t) ina_register_signal_handler(ina_signal_t, 
                                                ina_signal_handler_t handler);

/*
 * Relase and cleanup all internal data structures. This function must be
 * called once before the application terminate.
 */
INA_API(void) ina_exit(void);

#ifdef __cplusplus
}
#endif 

#endif
