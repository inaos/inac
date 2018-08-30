/*
 * Copyright (c) 2014-2018, INAOS GmbH
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


struct ina_file_s {
    ina_file_ctx_t *ctx;
    ina_str_t file_path;
    ina_handle_t fh;
    FILE *stream;
	int cursors;
	ina_file_access_mode_t access;
	ina_file_create_mode_t create;
	ina_file_share_mode_t share;
};

typedef struct ina_file_entry_s {
    ina_file_t *file;
    UT_hash_handle hh;
} ina_file_entry_t;

struct ina_file_ctx_s {
    mode_t default_mode;
    ina_file_entry_t *files;
};

struct ina_file_stat_s {
    uint64_t file_size;
    time_t atime;
    time_t mtime;
    int is_dir;
    mode_t mode;
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
        case INA_FILE_ACCESS_MODE_WRITE:
            *posix_flags |= O_WRONLY;
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
    INA_VERIFY_NOT_NULL(ctx);
    *ctx = (ina_file_ctx_t*)ina_mem_alloc(sizeof(ina_file_ctx_t));
    INA_RETURN_IF_NULL(ctx);
    (*ctx)->default_mode = default_mode;
    if ((*ctx)->default_mode == 0) {
        (*ctx)->default_mode =  S_IWUSR | S_IRUSR | S_IRGRP | S_IWGRP | S_IROTH;
    }
    return INA_SUCCESS;
}


INA_API(ina_rc_t) ina_file_destroy(ina_file_ctx_t **ctx)
{
    ina_file_entry_t *fe, *fetmp;

    INA_VERIFY_NOT_NULL(ctx);
    INA_VERIFY_NOT_NULL(*ctx);

    /*
     * close files that are still open
     */
    HASH_ITER(hh, (*ctx)->files, fe, fetmp) {
        INA_MUST_SUCCEED(ina_file_free(&fe->file));
    }
    HASH_CLEAR(hh, (*ctx)->files);

    ina_mem_free(*ctx);
    *ctx = NULL;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_file_new(ina_file_ctx_t *ctx, const char *file_fqn,
                               ina_file_access_mode_t access, ina_file_create_mode_t create, 
                               ina_file_share_mode_t share, int flags, ina_file_t **file)
{
    ina_file_entry_t *fe;

    #ifdef INA_OS_WIN32
	DWORD dwDesiredAccess;
	DWORD dwShareMode;
	DWORD dwCreationDisposition;
	DWORD dwFlagsAndAttributes;
	HANDLE fhandle;
    INA_VERIFY_NOT_NULL(ctx);
    INA_VERIFY_NOT_NULL(file_fqn);
    INA_VERIFY_NOT_NULL(file);
    *file = NULL;
	__ina_file_win_map_flags(access, create, share, flags, 
		&dwDesiredAccess, &dwShareMode, &dwCreationDisposition, &dwFlagsAndAttributes);

	fhandle = CreateFileA(file_fqn, dwDesiredAccess, dwShareMode, NULL,
		dwCreationDisposition, dwFlagsAndAttributes, NULL);

	if (fhandle == INVALID_HANDLE_VALUE) {
		return INA_OS_ERROR(INA_NN_FILE|INA_ERR_OPEN);
	}
#else    
    int posix_flags = 0;
    int fhandle;
    INA_VERIFY_NOT_NULL(ctx);
    INA_VERIFY_NOT_NULL(file_fqn);
    INA_VERIFY_NOT_NULL(file);
    *file = NULL;

    __ina_file_posix_map_flags(access, create, share, flags, &posix_flags);

    fhandle = open(file_fqn, posix_flags, ctx->default_mode);
    if (fhandle < 0) {
        return INA_OS_ERROR(INA_NN_FILE|INA_ERR_OPEN);
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
    INA_RETURN_IF_NULL(*file);
    (*file)->access = access;
    (*file)->create = create;
    (*file)->share = share;
    (*file)->fh = fhandle;
    (*file)->file_path = ina_str_new_fromcstr(file_fqn);
    (*file)->ctx = ctx;
    fe = (ina_file_entry_t*)ina_mem_alloc(sizeof(ina_file_entry_t));
    INA_RETURN_IF_NULL(fe);
    fe->file = *file;
    HASH_ADD_PTR(ctx->files, file, fe);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_file_free(ina_file_t **file)
{
    ina_file_entry_t *fe;
    INA_VERIFY_NOT_NULL(file);
    INA_VERIFY_NOT_NULL(*file);

    HASH_FIND_PTR((*file)->ctx->files, file, fe);
    INA_ASSERT_NOTNULL(fe);
    HASH_DEL((*file)->ctx->files, fe);

    if ((*file)->stream) {
        fclose((*file)->stream);
    }
#ifdef INA_OS_WIN32
    CloseHandle((*file)->fh);
#else
    close((*file)->fh);
#endif
    if ((*file)->file_path != NULL) {
       ina_str_free((*file)->file_path);
    }
    ina_mem_free(*file);
    *file = NULL;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_file_stat_new(const ina_file_t *file, ina_file_stat_t **stat)
{
    INA_VERIFY_NOT_NULL(stat);
    *stat = (ina_file_stat_t*)ina_mem_alloc(sizeof(ina_file_stat_t));
    INA_RETURN_IF_NULL(*stat);
    if (file != NULL) {
        return ina_file_stat_synch(*stat, file);
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_file_stat_synch(ina_file_stat_t *stat,  const ina_file_t *file)
{
#ifdef INA_OS_WIN32
    LARGE_INTEGER pin;
	DWORD attrs;
	FILETIME ct,at,wt;
	LARGE_INTEGER utcFT = {0};
	SYSTEMTIME systime;
    struct _stat fst;

    INA_VERIFY_NOT_NULL(file);
    INA_VERIFY_NOT_NULL(stat);

	if (!GetFileSizeEx(file->fh, &pin)) {
		return INA_OS_ERROR(INA_NN_OPERATION|INA_ERR_FAILED);
	}
	attrs = GetFileAttributes((LPCSTR)file->file_path);
	if (!GetFileTime(file->fh, &ct, &at, &wt)) {
		return INA_OS_ERROR(INA_NN_OPERATION|INA_ERR_FAILED);
	}
	stat->file_size = pin.QuadPart;
	stat->is_dir = (FILE_ATTRIBUTE_DIRECTORY & attrs);
	FileTimeToSystemTime((FILETIME*)&wt, &systime);
	__ina_file_system_time_to_time_t(&systime, &stat->mtime);
	FileTimeToSystemTime((FILETIME*)&at, &systime);
	__ina_file_system_time_to_time_t(&systime, &stat->atime);
    _stat(file->file_path, &fst);
    stat->mode = fst.st_mode;
#else
    struct stat fst;

    INA_VERIFY_NOT_NULL(file);
    INA_VERIFY_NOT_NULL(stat);
    ina_mem_set(&fst, 0, sizeof(struct stat));
    if (fstat(file->fh, &fst) != 0) {
        return INA_OS_ERROR(INA_NN_OPERATION|INA_ERR_FAILED);
    }
    stat->file_size = (size_t)fst.st_size;
    if (fst.st_mode & S_IFDIR) {
        stat->is_dir = 1;
    } else {
        stat->is_dir = 0;
    }
#ifdef INA_OS_OSX
    stat->mtime = fst.st_mtimespec.tv_sec;
    stat->atime = fst.st_atimespec.tv_sec;
#else
    stat->mtime = fst.st_mtime;
    stat->atime = fst.st_atime;
#endif
    stat->mode = fst.st_mode;
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_file_stat_free(ina_file_stat_t **stat)
{
    INA_VERIFY_NOT_NULL(stat);
    INA_VERIFY_NOT_NULL(*stat);
    ina_mem_free(*stat);
    *stat = NULL;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_file_get_filepath(const ina_file_t *file, ina_str_t *filepath)
{
    INA_VERIFY_NOT_NULL(file);
    INA_VERIFY_NOT_NULL(filepath);
    *filepath = ina_str_dup(file->file_path);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_file_set_mode(const ina_file_t *file, mode_t mode)
{
    INA_VERIFY_NOT_NULL(file);
#ifndef INA_OS_WIN32
    mode_t old_mask = umask(0);
    if (fchmod(file->fh, mode) == -1) {
        return INA_OS_ERROR(INA_NN_OPERATION|INA_ERR_FAILED);
    }
    umask(old_mask);
#else
    _chmod(file->file_path, INA_MS_MODE_MASK|mode);
#endif
    return INA_SUCCESS;

}

INA_API(ina_rc_t) ina_file_get_mode(const ina_file_t *file, mode_t *mode)
{
    ina_file_stat_t *stat = NULL;

    INA_VERIFY_NOT_NULL(file);
    INA_VERIFY_NOT_NULL(mode);
    INA_RETURN_IF_FAILED(ina_file_stat_new(file, &stat));
    *mode = stat->mode;
    INA_MUST_SUCCEED(ina_file_stat_free(&stat));
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_file_stat_is_dir(ina_file_stat_t *stat)
{
    INA_VERIFY_NOT_NULL(stat);

    if (!stat->is_dir) {
        INA_ERROR(INA_NN_DIRECTORY|INA_ERR_NOT_A);
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_file_stat_file_size(ina_file_stat_t *stat, uint64_t *file_size)
{
    INA_VERIFY_NOT_NULL(stat);
    INA_VERIFY_NOT_NULL(file_size);

    /* we know the the file-size can not be negative */
    *file_size = stat->file_size;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_file_stat_atime(ina_file_stat_t *stat, time_t *last_access)
{
    INA_VERIFY_NOT_NULL(stat);
    INA_VERIFY_NOT_NULL(last_access);
    *last_access = stat->atime;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_file_stat_mtime(ina_file_stat_t *stat, time_t *last_modification)
{
    INA_VERIFY_NOT_NULL(stat);
    INA_VERIFY_NOT_NULL(last_modification);
    *last_modification = stat->mtime;
    return INA_SUCCESS;
}

INA_API(ina_handle_t) ina_file_os_handle(ina_file_t *file)
{
    INA_VERIFY_NOT_NULL(file);
    return file->fh;
}

INA_API(FILE*) ina_file_get_stream(ina_file_t *file)
{
#ifdef INA_OS_WIN32
#include <fcntl.h>
    int fd;
#endif
    ina_str_t mode;
    INA_VERIFY_NOT_NULL(file);
    if (file->stream != NULL) {
        return file->stream;
    }
    if (file->access == INA_FILE_ACCESS_MODE_READ) {
        mode = ina_str_new_fromcstr("r");
    } else if (file->access == INA_FILE_ACCESS_MODE_READWRITE) {
        if (file->create == INA_FILE_CREATE_MODE_CREATE) {
            mode = ina_str_new_fromcstr("w+");
        } else if (file->create == INA_FILE_CREATE_MODE_APPEND) {
            mode = ina_str_new_fromcstr("a+");
        } else {
            mode = ina_str_new_fromcstr("r+");
        }
    } else if (file->access == INA_FILE_ACCESS_MODE_WRITE) {
        if (file->create == INA_FILE_CREATE_MODE_CREATE) {
            mode = ina_str_new_fromcstr("w");
        } else if (file->create == INA_FILE_CREATE_MODE_APPEND) {
            mode = ina_str_new_fromcstr("a");
        } else {
            mode = ina_str_new_fromcstr("w");
        }
    }

#ifdef INA_OS_WIN32
    fd = _open_osfhandle((intptr_t)file->fh, _O_APPEND | _O_RDONLY);
    file->stream = _fdopen(fd, ina_str_cstr(mode));
    return file->stream;
#else
    file->stream = fdopen(file->fh, ina_str_cstr(mode));
    return file->stream;
#endif
}

INA_API(ina_rc_t) ina_file_read(ina_file_t *file, unsigned char *buf, int64_t len, int64_t *nread)
{
    INA_VERIFY_NOT_NULL(file);
    INA_VERIFY_NOT_NULL(buf);
    INA_VERIFY(len>0);
    INA_VERIFY_NOT_NULL(nread);

#ifdef INA_OS_WIN32
    if (!ReadFile(file->fh, (LPVOID)buf, (DWORD)len, (LPDWORD)nread, NULL)) {
        return INA_OS_ERROR(INA_NN_READ|INA_ERR_FAILED);
    }
#else
    *nread = read(file->fh, buf, len);
    if (*nread < 0) {
        return INA_OS_ERROR(INA_NN_READ|INA_ERR_FAILED);
    }
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_file_write(ina_file_t *file, unsigned char *buf, int64_t len, int64_t *wrote)
{
    INA_VERIFY_NOT_NULL(file);
    INA_VERIFY_NOT_NULL(buf);
    INA_VERIFY(len > 0);
    INA_VERIFY_NOT_NULL(wrote);
#ifdef INA_OS_WIN32
    if (!WriteFile(file->fh, (LPVOID)buf, (DWORD)len, (LPDWORD)wrote, NULL)) {
        return INA_OS_ERROR(INA_NN_WRITE|INA_ERR_FAILED);;
    }
#else
    *wrote = write(file->fh, buf, len);
    if (*wrote < 0) {
        return INA_OS_ERROR(INA_NN_WRITE|INA_ERR_FAILED);
    }
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_file_set_bof(ina_file_t *file)
{
#ifdef INA_OS_WIN32
    LONG high = 0;
    LONG low = 0;
    INA_VERIFY_NOT_NULL(file);
    if (SetFilePointer(file->fh, low, &high, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
        return INA_OS_ERROR(INA_NN_OPERATION|INA_ERR_FAILED);
    }
#else
    INA_VERIFY_NOT_NULL(file);
    if (lseek(file->fh, 0, SEEK_SET) == -1) {
        return INA_OS_ERROR(INA_NN_OPERATION|INA_ERR_FAILED);
    };
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_file_set_pos(ina_file_t *file, uint64_t offset, ina_file_seek_mode_t mode)
{
#ifdef INA_OS_WIN32
    static DWORD modes[2] = {FILE_BEGIN,FILE_CURRENT};
    LONG high = offset >> 32;
    LONG low = offset & 0xffffffff;
    INA_VERIFY_NOT_NULL(file);
    if (SetFilePointer(file->fh, low, &high, modes[mode]) == INVALID_SET_FILE_POINTER) {
        return INA_OS_ERROR(INA_NN_OPERATION|INA_ERR_FAILED);;
    }
#else
    static int modes[2] = {SEEK_SET, SEEK_CUR};
    INA_VERIFY_NOT_NULL(file);
    if (lseek(file->fh, offset, modes[mode]) == -1) {
        return INA_OS_ERROR(INA_NN_OPERATION|INA_ERR_FAILED);
    }
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_file_get_pos(ina_file_t *file, uint64_t *offset)
{
#ifdef INA_OS_WIN32
    DWORD dwOffset;
    INA_VERIFY_NOT_NULL(file);
    dwOffset = SetFilePointer(file->fh, 0, NULL, FILE_CURRENT);
    if (dwOffset == INVALID_SET_FILE_POINTER) {
        return INA_OS_ERROR(INA_NN_OPERATION|INA_ERR_FAILED);
    }
    *offset = dwOffset;
#else
    off_t off;
    INA_VERIFY_NOT_NULL(file);
    off = lseek(file->fh, 0, SEEK_CUR);
    if (off == -1) {
        return INA_OS_ERROR(INA_NN_OPERATION|INA_ERR_FAILED);
    }
    *offset = (uint64_t)off;

#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_file_set_eof(ina_file_t *file)
{
#ifdef INA_OS_WIN32
    LONG high = 0;
    LONG low = 0;
    INA_VERIFY_NOT_NULL(file);
    if (SetFilePointer(file->fh, low, &high, FILE_END) == INVALID_SET_FILE_POINTER) {
        return INA_OS_ERROR(INA_NN_OPERATION|INA_ERR_FAILED);
    }
#else
    INA_VERIFY_NOT_NULL(file);
    if (lseek(file->fh, 0, SEEK_END) == -1) {
        return INA_OS_ERROR(INA_NN_OPERATION|INA_ERR_FAILED);
    }
#endif
    return INA_SUCCESS;
}

