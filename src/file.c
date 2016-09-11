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
    mode_t default_mode;
};

struct ina_file_stat_s {
	uint64_t file_size;
	time_t atime;
	time_t mtime;
	int is_dir;
    mode_t mode;
};

struct ina_file_s {
    ina_str_t file_path;
#ifdef INA_OS_WIN32
	HANDLE fh;
#else
	int fh;
#endif
    FILE *stream;
	int cursors;
	ina_file_access_mode_t access;
	ina_file_create_mode_t create;
	ina_file_share_mode_t share;
};

#ifdef INA_OS_WIN32
static void __ina_file_win_map_flags(ina_file_access_mode_t access, 
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
}
static void __ina_file_system_time_to_time_t(SYSTEMTIME *systemTime, time_t *unixts)
{
	unsigned __int64 utcDosTime;
	LARGE_INTEGER jan1970FT = {0};
	LARGE_INTEGER utcFT = {0};

	jan1970FT.QuadPart = 116444736000000000I64; // january 1st 1970
    SystemTimeToFileTime(systemTime, (FILETIME*)&utcFT);
    utcDosTime = (utcFT.QuadPart - jan1970FT.QuadPart)/10000000;
    *unixts = (time_t)utcDosTime;
}
#else
static void __ina_file_posix_map_flags(ina_file_access_mode_t access,
                                           ina_file_create_mode_t create,
                                           ina_file_share_mode_t share,
                                           int flags,
                                           int *posix_flags)
{
    *posix_flags = 0;
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
#ifndef INA_OS_OSX
    if (flags & INA_FILE_FLAG_POSIX_DIRECT) {
        *posix_flags |= O_DIRECT;
    }
#endif
}
#endif

INA_API(ina_rc_t) ina_file_init(ina_file_ctx_t **ctx, mode_t default_mode)
{
    /*
     * - keep track of all the open files
	 */
	INA_ASSERT_NOTNULL(ctx);
	*ctx = (ina_file_ctx_t*)ina_mem_alloc(sizeof(ina_file_ctx_t));
    (*ctx)->default_mode = default_mode;
    if ((*ctx)->default_mode == 0) {
        (*ctx)->default_mode =  S_IWUSR | S_IRUSR | S_IRGRP | S_IWGRP | S_IROTH;
    }
    return INA_SUCCESS;
}


INA_API(ina_rc_t) ina_file_destroy(ina_file_ctx_t **ctx)
{
    /*
	 * close files that are still open
	 */
    INA_ASSERT_NOTNULL(ctx);
    if (*ctx == NULL) {
        return INA_SUCCESS;
    }
    ina_mem_free(*ctx);
    *ctx = NULL;
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
	
	__ina_file_win_map_flags(access, create, share, flags, 
		&dwDesiredAccess, &dwShareMode, &dwCreationDisposition, &dwFlagsAndAttributes);

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
    int fhandle;

    __ina_file_posix_map_flags(access, create, share, flags, &posix_flags);

    fhandle = open(file_fqn, posix_flags, ctx->default_mode);
    if (fhandle < 0) {
        /* FIXME: handle error */
        printf("%d", errno);
        return INA_FAILURE;
    }
#ifndef INA_OS_OSX
    if (flags & INA_FILE_FLAG_RANDOM_ACCESS) {
		posix_fadvise(fhandle, 0, 0, POSIX_FADV_RANDOM);
	}
	else if (flags & INA_FILE_FLAG_SEQUENTIAL_ACCESS) {
		posix_fadvise(fhandle, 0, 0, POSIX_FADV_SEQUENTIAL);
	}
#endif
#endif

    *file = (ina_file_t*)ina_mem_alloc(sizeof(ina_file_t));
    (*file)->access = access;
    (*file)->create = create;
    (*file)->share = share;
    (*file)->fh = fhandle;
    (*file)->file_path = ina_str_new_fromcstr(file_fqn);

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_file_free(ina_file_ctx_t *ctx, ina_file_t **file)
{
    INA_ASSERT_NOTNULL(file);
    if (*file == NULL) {
        return INA_SUCCESS;
    }

#ifdef INA_OS_WIN32
    CloseHandle((*file)->fh);
#else
    close((*file)->fh);
#endif
    ina_str_free((*file)->file_path);
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
    struct _stat fst;

    INA_ASSERT_NOTNULL(file);
    INA_ASSERT_NOTNULL(stat);

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
    _stat(file->file_path, &fst);
    (*stat)->mode = fst.st_mode;
#else
    struct stat fst;
   
    INA_ASSERT_NOTNULL(file);
    INA_ASSERT_NOTNULL(stat);
 
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
    (*stat)->mode = fst.st_mode;
    
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_file_stat_free(ina_file_t *file, ina_file_stat_t **stat)
{
    ina_mem_free(*stat);
    *stat = NULL;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_file_get_filepath(const ina_file_t *file, ina_str_t *filepath)
{
    INA_ASSERT_NOTNULL(file);
    INA_ASSERT_NOTNULL(filepath);
    *filepath = ina_str_dup(file->file_path);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_file_set_mode(const ina_file_t *file, mode_t mode)
{
    INA_ASSERT_NOTNULL(file);
#ifndef INA_OS_WIN32
    mode_t old_mask = umask(0);
    fchmod(file->fh, mode);
    umask(old_mask);
#else
    _chmod(file->file_path, INA_MS_MODE_MASK|mode);
#endif
    return INA_SUCCESS;

}

INA_API(ina_rc_t) ina_file_get_mode(const ina_file_t *file, mode_t *mode)
{
    ina_file_stat_t *stat = NULL;

    INA_ASSERT_NOTNULL(file);
    if (INA_SUCCEED(ina_file_stat_new((ina_file_t*)file, &stat))) {
        *mode = stat->mode;
    }
    if (stat != NULL) {
        ina_file_stat_free((ina_file_t*)file, &stat);
    }
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

INA_API(FILE*) ina_file_get_stream(ina_file_t *file)
{
    if (file->stream != NULL) {
        return file->stream;
    }

#ifdef INA_OS_WIN32
#else
    file->stream = fdopen(file->fh, "+r");
    return file->stream;
#endif
}

INA_API(ina_rc_t) ina_file_read(ina_file_t *file, unsigned char *buf, int64_t len, int64_t *nread)
{
    INA_ASSERT_NOTNULL(file);
#ifdef INA_OS_WIN32
    if (!ReadFile(file->fh, (LPVOID)buf, (DWORD)len, (LPDWORD)nread, NULL)) {
        return INA_FAILURE;
    }
#else
    *nread = read(file->fh, buf, len);
    if (*nread < 0) {
        /* FIXME: proper error handling */
        return INA_FAILURE;
    }
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_file_write(ina_file_t *file, unsigned char *buf, int64_t len, int64_t *wrote)
{
    INA_ASSERT_NOTNULL(file);
#ifdef INA_OS_WIN32
    if (!WriteFile(file->fh, (LPVOID)buf, (DWORD)len, (LPDWORD)wrote, NULL)) {
        return INA_FAILURE;
    }
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
#ifdef INA_OS_WIN32
    LONG high = 0;
    LONG low = 0;
    INA_ASSERT_NOTNULL(file);
    if (SetFilePointer(file->fh, low, &high, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
        /* FIXME: proper error handling */
        return INA_FAILURE;
    }
#else
    INA_ASSERT_NOTNULL(file);
    lseek (file->fh, 0, SEEK_SET);
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_file_set_pos(ina_file_t *file, uint64_t offset, ina_file_seek_mode_t mode)
{
#ifdef INA_OS_WIN32
    static DWORD modes[2] = {FILE_BEGIN,FILE_CURRENT};
    LONG high = offset >> 32;
    LONG low = offset & 0xffffffff;
    INA_ASSERT_NOTNULL(file);
    if (SetFilePointer(file->fh, low, &high, modes[mode]) == INVALID_SET_FILE_POINTER) {
        /* FIXME: proper error handling */
        return INA_FAILURE;
    }
#else
    static int modes[2] = {SEEK_SET, SEEK_CUR};
    INA_ASSERT_NOTNULL(file);
    lseek (file->fh, offset, modes[mode]);
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_file_get_pos(ina_file_t *file, uint64_t *offset)
{
#ifdef INA_OS_WIN32
    DWORD dwOffset;
    INA_ASSERT_NOTNULL(file);
    dwOffset = SetFilePointer(file->fh, 0, NULL, FILE_CURRENT);
    if (dwOffset == INVALID_SET_FILE_POINTER) {
        /* FIXME: proper error handling */
        return INA_FAILURE;
    }
    *offset = dwOffset;
#else
    INA_ASSERT_NOTNULL(file);
    *offset = (uint64_t)lseek(file->fh, 0, SEEK_CUR);
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_file_set_eof(ina_file_t *file)
{
#ifdef INA_OS_WIN32
    LONG high = 0;
    LONG low = 0;
    INA_ASSERT_NOTNULL(file);
    if (SetFilePointer(file->fh, low, &high, FILE_END) == INVALID_SET_FILE_POINTER) {
        /* FIXME: proper error handling */
        return INA_FAILURE;
    }
#else
    INA_ASSERT_NOTNULL(file);
    lseek (file->fh, 0, SEEK_END);
#endif
    return INA_SUCCESS;
}

