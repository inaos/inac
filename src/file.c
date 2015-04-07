/*
 * Copyright (c) 2014-2015, INAOS GmbH
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

#ifndef INA_OS_WIN32
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

struct ina_file_ctx_s {
	int files;
	
};

struct ina_file_stat_s {
	uint64_t file_size;
	time_t atime;
	time_t mtime;
	int is_dir;
};

struct ina_file_s {
#ifdef INA_OS_WIN32
	HANDLE fh;
	char file_path[MAX_PATH];
#else
	int fh;
#endif
	int cursors;
	ina_file_access_mode_t access;
	ina_file_create_mode_t create;
	ina_file_share_mode_t share;
};

#ifdef INA_OS_WIN32
static ina_rc_t __ina_file_win_map_flags(ina_file_access_mode_t access, 
										 ina_file_create_mode_t create,
										 ina_file_share_mode_t share,
										 int flags,
										 DWORD *dwDesiredAccess,
										 DWORD *dwShareMode,
										 DWORD *dwCreationDisposition,
										 DWORD *dwFlagsAndAttributes)
{
	switch (access) {
		case INA_FILE_ACCESS_MODE_READ:
			*dwDesiredAccess = GENERIC_READ;
			break;
		case INA_FILE_ACCESS_MODE_READWRITE:
			if (create == INA_FILE_CREATE_MODE_APPEND) {
				*dwDesiredAccess = FILE_APPEND_DATA;
			}
			else {
				*dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
			}
			break;
	}
	switch (create) {
		case INA_FILE_CREATE_MODE_OPEN:
			*dwCreationDisposition = OPEN_EXISTING;
			break;
		case INA_FILE_CREATE_MODE_CREATE:
			*dwCreationDisposition = CREATE_ALWAYS;
			break;
		case INA_FILE_CREATE_MODE_APPEND:
			*dwCreationDisposition = OPEN_ALWAYS;
			break;
	}
	switch (share) {
		case INA_FILE_SHARE_MODE_EXCLUSIVE:
			*dwShareMode = 0;
			break;
		case INA_FILE_SHARE_MODE_READ:
			*dwShareMode = FILE_SHARE_READ;
			break;
		case INA_FILE_SHARE_MODE_WRITE:
			*dwShareMode = FILE_SHARE_WRITE;
			break;
	}
	*dwFlagsAndAttributes = 0;
	if (flags & INA_FILE_FLAG_ATTR_NORMAL) {
		*dwFlagsAndAttributes |= FILE_ATTRIBUTE_NORMAL;
	}
	if (flags & INA_FILE_FLAG_ATTR_HIDDEN) {
		*dwFlagsAndAttributes |= FILE_ATTRIBUTE_HIDDEN;
	}
	if (flags & INA_FILE_FLAG_RANDOM_ACCESS) {
		*dwFlagsAndAttributes |= FILE_FLAG_RANDOM_ACCESS;
	}
	if (flags & INA_FILE_FLAG_SEQUENTIAL_ACCESS) {
		*dwFlagsAndAttributes |= FILE_FLAG_SEQUENTIAL_SCAN;
	}
	if (flags & INA_FILE_FLAG_WIN32_OVERLAPPED) {
		*dwFlagsAndAttributes |= FILE_FLAG_OVERLAPPED;
	}
	return INA_SUCCESS;
}
static ina_rc_t __ina_file_system_time_to_time_t(SYSTEMTIME *systemTime, time_t *unixts)
{
	unsigned __int64 utcDosTime;
	LARGE_INTEGER jan1970FT = {0};
	LARGE_INTEGER utcFT = {0};

	jan1970FT.QuadPart = 116444736000000000I64; // january 1st 1970
    SystemTimeToFileTime(systemTime, (FILETIME*)&utcFT);
    utcDosTime = (utcFT.QuadPart - jan1970FT.QuadPart)/10000000;
    *unixts = (time_t)utcDosTime;

	return INA_SUCCESS;
}
#else
static ina_rc_t __ina_file_posix_map_flags(ina_file_access_mode_t access,
                                           ina_file_create_mode_t create,
                                           ina_file_share_mode_t share,
                                           int flags,
                                           int *posix_flags,
                                           int *mode)
{
    *posix_flags = 0;
    *mode = 0;
    switch (access) {
        case INA_FILE_ACCESS_MODE_READ:
            *posix_flags |= O_RDONLY;
            break;
        case INA_FILE_ACCESS_MODE_READWRITE:
            *posix_flags |= O_RDWR;
            break;
    }
    switch (create) {
        case INA_FILE_CREATE_MODE_OPEN:
            break;
        case INA_FILE_CREATE_MODE_CREATE:
            *posix_flags |= O_CREAT;
            *mode = 664;
            break;
        case INA_FILE_CREATE_MODE_APPEND:
            *posix_flags |= O_APPEND;
            break;
    }
    switch (share) {
        case INA_FILE_SHARE_MODE_READ:
            break;
        case INA_FILE_SHARE_MODE_WRITE:
            break; 
        case INA_FILE_SHARE_MODE_EXCLUSIVE:
#ifdef INA_OS_OSX
            *posix_flags |= O_EXLOCK;
#else
            *posix_flags |= O_EXCL;
#endif
            break;
    }
    return INA_SUCCESS;
}
#endif

INA_API(ina_rc_t) ina_file_init(ina_file_ctx_t **ctx)
{
    /*
	 * - keep track of all the open files
	 */
	INA_ASSERT_NOTNULL(ctx);
	*ctx = NULL;
    return INA_SUCCESS;
}


INA_API(ina_rc_t) ina_file_destroy(ina_file_ctx_t **ctx)
{
    /*
	 * close files that are still open
	 */
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_file_new(ina_file_ctx_t *ctx, const char *file_fqn,
                               ina_file_access_mode_t access, ina_file_create_mode_t create, 
                               ina_file_share_mode_t share, int flags, ina_file_t **file)
{
    #ifdef INA_OS_WIN32
	DWORD dwDesiredAccess;
	DWORD dwShareMode;
	DWORD dwCreationDisposition;
	DWORD dwFlagsAndAttributes;
	HANDLE fhandle;
	
	if (!INA_SUCCEED(__ina_file_win_map_flags(access, create, share, flags, 
		&dwDesiredAccess, &dwShareMode, &dwCreationDisposition, &dwFlagsAndAttributes))) {
			return INA_ERR_PUSH_LAST;
	}

	fhandle = CreateFileA(file_fqn, dwDesiredAccess, dwShareMode, NULL,
		dwCreationDisposition, dwFlagsAndAttributes, NULL);

	if (fhandle == INVALID_HANDLE_VALUE) {
		/* FIXME: handle error */
		DWORD err = GetLastError();
		printf("%d", err);
		return INA_FAILURE;
	}
#else    
    int posix_flags = 0;
    int mode = 0;
    int fhandle;

    if (!INA_SUCCEED(__ina_file_posix_map_flags(access, create, share, flags, &posix_flags, &mode))) {
        return INA_ERR_PUSH_LAST;
    }

    if (mode == 0) {
        fhandle = open(file_fqn, posix_flags);
    }
    else {
        fhandle = open(file_fqn, posix_flags, mode);
    }
    if (fhandle < 0) {
        /* FIXME: handle error */
        printf("%d", errno);
        return INA_FAILURE;
    }
    if (flags & INA_FILE_FLAG_RANDOM_ACCESS) {
		posix_fadvise(fhandle, 0, 0, POSIX_FADV_RANDOM);
	}
	else if (flags & INA_FILE_FLAG_SEQUENTIAL_ACCESS) {
		posix_fadvise(fhandle, 0, 0, POSIX_FADV_SEQUENTIAL);
	}
#endif

    *file = (ina_file_t*)ina_mem_alloc(sizeof(ina_file_t));
    (*file)->access = access;
    (*file)->create = create;
    (*file)->share = share;
    (*file)->fh = fhandle;
#ifdef INA_OS_WIN32
    strcpy((*file)->file_path, file_fqn);
#endif

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_file_free(ina_file_ctx_t *ctx, ina_file_t **file)
{
#ifdef INA_OS_WIN32
	CloseHandle((*file)->fh);
#else
    close((*file)->fh);
#endif
    ina_mem_free(*file);
    *file = NULL;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_file_stat_new(ina_file_t *file, ina_file_stat_t **stat)
{
#ifdef INA_OS_WIN32
	LARGE_INTEGER pin;
	DWORD attrs;
	FILETIME ct,at,wt;
	LARGE_INTEGER utcFT = {0};
	SYSTEMTIME systime;

	if (!GetFileSizeEx(file->fh, &pin)) {
		/* FIXME: handle error */
		return INA_FAILURE;
	}
	attrs = GetFileAttributes((LPCSTR)file->file_path);
	if (!GetFileTime(file->fh, &ct, &at, &wt)) {
		/* FIXME: handle error */
		return INA_FAILURE;
	}
	*stat = (ina_file_stat_t*)ina_mem_alloc(sizeof(ina_file_stat_t));
	(*stat)->file_size = pin.QuadPart;
	(*stat)->is_dir = (FILE_ATTRIBUTE_DIRECTORY & attrs);
	FileTimeToSystemTime((FILETIME*)&wt, &systime);
	__ina_file_system_time_to_time_t(&systime, &(*stat)->mtime);
	FileTimeToSystemTime((FILETIME*)&at, &systime);
	__ina_file_system_time_to_time_t(&systime, &(*stat)->atime);
#else
    struct stat fst;
    
    ina_mem_set(&fst, 0, sizeof(struct stat));
    if (fstat(file->fh, &fst) != 0) {
        /* FIXME: handle error */
        return INA_FAILURE;
    }
    *stat = (ina_file_stat_t*)ina_mem_alloc(sizeof(ina_file_stat_t));
    (*stat)->file_size = (size_t)fst.st_size;
    if (fst.st_mode & S_IFDIR) {
        (*stat)->is_dir = 1;
    }
    else {
        (*stat)->is_dir = 0;
    }
#ifdef INA_OS_OSX
    (*stat)->mtime = fst.st_mtimespec.tv_sec;
    (*stat)->atime = fst.st_atimespec.tv_sec; 
#else
    (*stat)->mtime = fst.st_mtime;
    (*stat)->atime = fst.st_atime;
#endif
    
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_file_stat_free(ina_file_t *file, ina_file_stat_t **stat)
{
    ina_mem_free(*stat);
    *stat = NULL;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_file_stat_is_dir(ina_file_stat_t *stat, int *dir)
{
    if (stat->is_dir) {
        *dir = 1;
    }
    else {
        *dir = 0;
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_file_stat_file_size(ina_file_stat_t *stat, uint64_t *file_size)
{
    /* we know the the file-size can not be negative */
    *file_size = stat->file_size;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_file_stat_atime(ina_file_stat_t *stat, time_t *last_access)
{
    *last_access = stat->atime;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_file_stat_mtime(ina_file_stat_t *stat, time_t *last_modification)
{
    *last_modification = stat->mtime;
    return INA_SUCCESS;
}

INA_API(void*) ina_file_os_handle(ina_file_t *file)
{
    INA_ASSERT_NOTNULL(file);
#ifdef INA_OS_WIN32
    return file->fh;
#else
    return &file->fh;
#endif
}

INA_API(ina_rc_t) ina_file_read(ina_file_t *file, unsigned char *buf, uint64_t len, uint64_t *nread)
{
    INA_ASSERT_NOTNULL(file);
#ifdef INA_OS_WIN32
#else
    *nread = read(file->fh, buf, len);
    if (*nread < 0) {
        /* FIXME: proper error handling */
        return INA_FAILURE;
    }
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_file_write(ina_file_t *file, unsigned char *buf, uint64_t len, uint64_t *wrote)
{
    INA_ASSERT_NOTNULL(file);
#ifdef INA_OS_WIN32
#else
    *wrote = write(file->fh, buf, len);
    if (*wrote < 0) {
        /* FIXME: proper error handling */
        return INA_FAILURE;
    }
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_file_set_bof(ina_file_t *file)
{
    INA_ASSERT_NOTNULL(file);
#ifdef INA_OS_WIN32
#else
    lseek (file->fh, 0, SEEK_SET);
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_file_set_pos(ina_file_t *file, uint64_t offset)
{
    INA_ASSERT_NOTNULL(file);
#ifdef INA_OS_WIN32
#else
    lseek (file->fh, offset, SEEK_CUR);
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_file_set_eof(ina_file_t *file)
{
    INA_ASSERT_NOTNULL(file);
#ifdef INA_OS_WIN32
#else
    lseek (file->fh, 0, SEEK_END);
#endif
    return INA_SUCCESS;
}

