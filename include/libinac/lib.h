/*
 * Copyright INAOS GmbH, Thalwil, 2012-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#ifndef _LIBINAC_LIB_H_
#define _LIBINAC_LIB_H_

#ifdef __cplusplus
extern "C" {
#endif

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
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <windows.h>
#include <minwindef.h>
#include <wincon.h>
#include <io.h>
#include <sys/stat.h>
#include <sys/types.h>
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
#include <inttypes.h>
#include <errno.h>
#include <time.h>

#include <libinac/contribs.h>

#include <libinac/version.h>
#include <libinac/portable.h>
#include <libinac/types.h>
#include <libinac/debug.h>
#include <libinac/error.h>
#include <libinac/memory.h>
#include <libinac/mempool.h>
#include <libinac/string.h>
#include <libinac/log.h>
#include <libinac/time.h>
#include <libinac/timer.h>
#include <libinac/stopwatch.h>
#include <libinac/ullc.h>
#include <libinac/net.h>
#include <libinac/ljit.h>
#include <libinac/conffile.h>
#include <libinac/hash.h>
#include <libinac/util.h>
#include <libinac/cio.h>
#include <libinac/fsm.h>
#include <libinac/process.h>
#include <libinac/cron.h>
#include <libinac/ipc.h>
#include <libinac/cpu.h>
#include <libinac/compression.h>
#include <libinac/dir.h>
#include <libinac/file.h>
#include <libinac/mmap.h>
#include <libinac/file_cursor.h>
#include <libinac/hashtable.h>
#include <libinac/list.h>
#include <libinac/test.h>
#include <libinac/bench.h>


#define INA_UNUSED(x) (void)(x)

#define INA_YES (1)
#define INA_NO  (0)

#define INA_NUM2STR_X(x) #x
#define INA_NUM2STR(x) INA_NUM2STR_X(x)

#define INA_MID_BITS(number,k,p) (((1ULL << (k)) - 1ULL) & ((number) >> ((p) - 1ULL)))


#define INA_VERSION       INA_NUM2STR(INA_MAJOR_VERSION)"." \
                          INA_NUM2STR(INA_MINOR_VERSION)"." \
                          INA_NUM2STR(INA_PATCH_VERSION)

/* Version as a 3-byte hex number, e.g. 0x010201 == 1.2.1. Use this
 * for numeric comparisons, e.g. #if INA_VERSION_HEX >= ... */
#define INA_VERSION_HEX  ((INA_MAJOR_VERSION << 16) |   \
                          (INA_MINOR_VERSION << 8)  |   \
                          (INA_PATCH_VERSION << 0))

/* Revsion number as 2-byte hex number e.g 0x900 == 0.9. Use this
 * for numeric comparisons, e.g. #if INA_REVISION_HEX >= ... */
#define INA_REVISION_HEX ((INA_MINOR_VERSION << 8)  |   \
                          (INA_PATCH_VERSION << 0))


/* Return with last rc if condition x fails */
#define INA_RETURN_IF(x) do {if ((x)) return ina_err_get_rc(); } while(0)
/* Return with last rc if x == NULL */
#define INA_RETURN_IF_NULL(x) do {if ((x) == NULL) return ina_err_get_rc();} while(0)
/* Return with last rc if failed */
#define INA_RETURN_IF_FAILED(rc) do { if (INA_FAILED((rc))) return ina_err_get_rc(); } while (0)
/* Return with last rc if succeed */
#define INA_RETURN_IF_SUCCEED(rc) do {if (INA_SUCCEED((rc))) return ina_err_get_rc(); } while (0)

#ifndef INA_VERIFY_DISABLED
#define INA_VERIFY(x) do { if (INA_UNLIKELY(!(x))) return INA_ERROR(INA_ERR_INVALID_ARGUMENT); } while (0)
#define INA_VERIFY_NOT_NULL(x) INA_VERIFY((x) != NULL)
#define INA_VERIFY_FREE(ptrptr) do {                   \
    INA_ASSERT_NOT_NULL(ptrptr);                       \
    if (INA_UNLIKELY((*ptrptr == NULL))) { return; }   \
} while(0)
#else
#define INA_VERIFY_NOT_NULL(x) INA_ASSERT_NOT_NULL((x))
#define INA_VERIFY(x) INA_ASSERT_TRUE((x))
#define INA_VERIFY_FREE(ptrptr) INA_ASSERT_NOT_NULL(ptrptr)
#endif

#define INA_INIT_GUARD() do {               \
    static int __initialized = 0;           \
    if (__initialized) return INA_SUCCESS;  \
    __initialized = 1;                      \
} while(0)

#define INA_DESTROY_GUARD() do {          \
    static int __destroyed = 0;           \
    if (__destroyed) return;              \
    __destroyed = 1;                      \
} while(0)

/* Source location */
#define INA_AT __FILE__ ":" INA_NUM2STR(__LINE__)

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
 * Parameters
 *  argc       argc of main() function
 *  argv       Pointer to the argv of main() function
 *  opt        Array of options to parse
 *
 * Return
 *  INA_SUCCESS  if no error occurred
 */
INA_API(ina_rc_t) ina_app_init(int argc,
                               char **argv,
                               ina_opt_t *opt);

/*
 * Get the string key and value of an option at index.
 *
 * Parameters
 *  index  options index starting by 0
 *  key    long name of option
 *  value  option value as string
 *
 * Return
 *  INA_SUCCESS if option is available otherwise INA_FAILURE
 */
INA_API(ina_rc_t) ina_opt_get_key_value(int index,
                                        ina_str_t *key,
                                        ina_str_t *value);

/*
 * Check whenever an option is available.
 *
 * Parameters
 *  opt   name of option
 *
 * Return
 *  INA_SUCCESS if option is available
 */
INA_API(ina_rc_t) ina_opt_isset(const char *opt);
/*
 * Get the string value of an option.
 *
 * Parameters
 *  opt    name of option
 *  value  Where to store the value
 *
 * Return
 *  INA_SUCCESS if option is available
 */
INA_API(ina_rc_t) ina_opt_get_string(const char *opt, ina_str_t *value);
/*
 * Get the integer value of an option.
 *
 * Parameters
 *  opt    name of option
 *  value  Where to store the value
 *
 * Return
 *  INA_SUCCESS if option is available
 */
INA_API(ina_rc_t) ina_opt_get_int(const char *opt, int *value);

/*
 * Get the float value of an option.
 *
 * Parameters
 *  opt   name of option
 *  value  Where to store the value
 *
 * Return
 *  INA_SUCCESS if option is available
 */
INA_API(ina_rc_t) ina_opt_get_float(const char *opt, float *value);

/*
 * Initialize all internal data structures. This must be the first function 
 * called for any library.
 *
 * Return
 *  INA_SUCCESS  if no error occurred
 */
INA_API(ina_rc_t) ina_init(void);

/*
 * Set a custom termination routine to call in case of an 
 * a termination signal. The purpose of such a routine is to give consumers
 * a last chance to cleanup before the program exits.
 *
 * Parameters
 *  handler  Cleanup routine. A cleanup should return EXIT_SUCCESS or
 *           EXIT_FAILURE depending on type of signal. On a program error
 *           the return of cleanup routines will be ignored.
 *
 * Return
 *  Previously defined handler
 */
INA_API(ina_cleanup_handler_t) ina_set_cleanup_handler(
                                        ina_cleanup_handler_t handler);


/*
 * Register a custom signal handler for sig.
 *
 * Parameters
 *  sig     Signal identifier
 *  handler Signal handler
 *
 * Return
 *  Previously register handler
 */
INA_API(ina_signal_handler_t) ina_register_signal_handler(ina_signal_t sig,
                                                ina_signal_handler_t handler);

/*
 * Release and cleanup all internal data structures. This function is called
 * automatically once before the application terminate.
 *
 * FIXME: make it private
 */
INA_API(void) ina_exit(void);

#ifdef __cplusplus
}
#endif 

#endif
