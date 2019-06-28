/*
 * Copyright INAOS GmbH, Thalwil, 2013-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <libinac/lib.h>
#include "config.h"

void ex(void)
{
}

#ifdef INA_OS_WINDOWS

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

INA_API(int) gettimeofday(struct timeval *tv, struct timezone *tz)
{
    static int tzflag;

    if (NULL != tv) {
        ULARGE_INTEGER ul; /* As specified on MSDN. */
        FILETIME ft;

        /* Returns a 64-bit value representing the number of
           100-nanosecond intervals since January 1, 1601 (UTC). */
        GetSystemTimeAsFileTime(&ft);

        /* Fill ULARGE_INTEGER low and high parts. */
        ul.LowPart = ft.dwLowDateTime;
        ul.HighPart = ft.dwHighDateTime;
        /* Convert to microseconds. */
        ul.QuadPart /= 10ULL;
        /* Remove Windows to UNIX Epoch delta. */
        ul.QuadPart -= 11644473600000000ULL;
        /* Modulo to retrieve the microseconds. */
        tv->tv_usec = (long) (ul.QuadPart % 1000000LL);
        /* Divide to retrieve the seconds. */
        tv->tv_sec = (long) (ul.QuadPart / 1000000LL);
    }
    if (NULL != tz) {
        if (!tzflag) {
            _tzset();
            tzflag++;
        }
        tz->tz_minuteswest = _timezone / 60;
        tz->tz_dsttime = _daylight;
    }
    return 0;
}

typedef ptrdiff_t handle_type; /* C99's intptr_t not sufficiently portable */

struct DIR {
    handle_type         handle; /* -1 for failed rewind */
    struct _finddata_t  info;
    struct dirent       result; /* d_name null iff first time */
    char                *name;  /* null-terminated char string */
};

DIR *opendir(const char *name)
{
    DIR *dir = 0;

    if (name && name[0]) {
        size_t base_length = strlen(name);
        const char *all = /* search pattern must end with suitable wildcard */
            strchr("/\\", name[base_length - 1]) ? "*" : "/*";
        size_t new_len = base_length + strlen(all) + 1;
        if ((dir = (DIR *) ina_mem_alloc(sizeof *dir)) != 0 &&
           (dir->name = (char *) ina_mem_alloc(new_len)) != 0) {
            strncat(strncpy(dir->name, name, new_len - 1), all, new_len - base_length - 1);

            if ((dir->handle =
                (handle_type) _findfirst(dir->name, &dir->info)) != -1) {
                dir->result.d_name = 0;
            } else /* rollback */ {
                ina_mem_free(dir->name);
                ina_mem_free(dir);
                dir = 0;
            }
        } else /* rollback */ {
            ina_mem_free(dir);
            dir   = 0;
            errno = ENOMEM;
        }
    } else {
        errno = EINVAL;
    }
    return dir;
}

int closedir(DIR *dir)
{
    int result = -1;

    if (dir) {
        if (dir->handle != -1) {
            result = _findclose(dir->handle);
        }
        ina_mem_free(dir->name);
        ina_mem_free(dir);
    }

    if (result == -1) /* map all errors to EBADF */ {
        errno = EBADF;
    }
    return result;
}

struct dirent* readdir(DIR *dir)
{
    struct dirent *result = 0;

    if (dir && dir->handle != -1) {
        if (!dir->result.d_name || _findnext(dir->handle, &dir->info) != -1) {
            result = &dir->result;
            result->d_name = dir->info.name;
            result->d_type = DT_REG;
            if (dir->info.attrib&_A_SUBDIR) {
                result->d_type = DT_DIR;
            }
        }
    } 
    else {
        errno = EBADF;
    }
    return result;
}

void rewinddir(DIR *dir)
{
    if (dir && dir->handle != -1) {
        _findclose(dir->handle);
        dir->handle = (handle_type) _findfirst(dir->name, &dir->info);
        dir->result.d_name = 0;
    } else {
        errno = EBADF;
    }
}

int inet_aton(const char *address, struct in_addr *sock)
{
    int s;
    s = inet_addr(address);
    if (s == INADDR_NONE) {
        return(0);
    }
    sock->s_addr = s;
    return(1);
}

#endif
