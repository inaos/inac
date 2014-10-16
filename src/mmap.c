/*
 * Copyright (c) 2014, INAOS GmbH
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
#include <sys/mman.h>
#endif

struct ina_mmap_ctx_s {
	size_t page_size;
};

struct ina_mmap_mapping_s {
#ifdef INA_OS_WIN32
	HANDLE fmap;
	LPVOID lpMapAddress;
#else
    void *addr;
#endif
	ina_file_t *fd;
	size_t offset;
	size_t length;
	void *begin_mmap;
	void *end_mmap;
};

INA_API(ina_rc_t) ina_mmap_init(ina_mmap_ctx_t **ctx)
{
    *ctx = (ina_mmap_ctx_t*)ina_mem_alloc(sizeof(ina_mmap_ctx_t));

    if (!INA_SUCCEED(ina_mem_get_pagesize(&(*ctx)->page_size))) {
        return INA_ERR_PUSH_LAST;
    }	
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_mmap_destroy(ina_mmap_ctx_t **ctx)
{
	ina_mem_free(*ctx);
	*ctx = NULL;
	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_mmap_new(ina_mmap_ctx_t *ctx, ina_file_t *fd, 
                               int prot_flags, ina_mmap_mem_share_t share,
                               size_t offset, size_t length, ina_mmap_mapping_t **mapping)
{
	void *data = NULL;
	ina_file_stat_t *fstat;
	size_t flen;

#ifdef INA_OS_WIN32
	DWORD flProtect = 0;
	DWORD dwMaximumSizeHigh = 0;
	DWORD dwMaximumSizeLow = 0;
	DWORD dwFileMapStart;
	DWORD dwMapViewSize;
	DWORD dwDesiredAccess;
	size_t delta;

	dwMaximumSizeLow = flen;
#endif

	ina_file_stat_new(fd, &fstat);
	ina_file_stat_file_size(fstat, &flen);
	ina_file_stat_free(fd, &fstat);

	if (offset + length > flen) {
		/* FIXME: proper error handling */
		return INA_FAILURE;
	}

	*mapping = (ina_mmap_mapping_t*)ina_mem_alloc(sizeof(ina_mmap_mapping_t));
	(*mapping)->length = length;
	(*mapping)->offset = offset;

#ifdef INA_OS_WIN32
	(*mapping)->fmap = NULL;
	if (prot_flags & INA_MMAP_MEM_PROT_READ) {
		if (share & INA_MMAP_MEM_PROT_EXEC) {
			flProtect = PAGE_EXECUTE_READ;
		}
		else {
			flProtect = PAGE_READONLY;
		}
		dwDesiredAccess = FILE_MAP_READ;
	}
	if (prot_flags & INA_MMAP_MEM_PROT_EXEC) {
		flProtect = PAGE_EXECUTE_READ;
	}
	if (prot_flags & INA_MMAP_MEM_PROT_WRITE) {
		if (share & INA_MMAP_MEM_PROT_EXEC) {
			flProtect = PAGE_EXECUTE_READWRITE;
		}
		else {
			flProtect = PAGE_READWRITE;
		}
		dwDesiredAccess = FILE_MAP_WRITE;
	}

	(*mapping)->fmap = CreateFileMapping((HANDLE)ina_file_os_handle(fd), NULL, flProtect, dwMaximumSizeHigh, dwMaximumSizeLow, NULL);
	if ((*mapping)->fmap == INVALID_HANDLE_VALUE) {
		/* FIXME: handle error */
		DWORD err = GetLastError();
		printf("%d", err);
		return INA_FAILURE;
	}

	// To calculate where to start the file mapping, round down the
	// offset of the data into the file to the nearest multiple of the
	// system allocation granularity.
	dwFileMapStart = ((DWORD)(offset / ctx->page_size)) * ctx->page_size;
	
	// Calculate the size of the file mapping view.
	dwMapViewSize = (offset % ctx->page_size) + length;
	dwMapViewSize = max(dwMapViewSize, length);
	
	// The data of interest isn't at the beginning of the
	// view, so determine how far into the view to set the pointer.
	delta = offset - dwFileMapStart;
	
	(*mapping)->lpMapAddress = MapViewOfFile((*mapping)->fmap, dwDesiredAccess, 0, dwFileMapStart, dwMapViewSize);
	if ((*mapping)->lpMapAddress == NULL) {
		/* FIXME: handle error */
		DWORD err = GetLastError();
		printf("%d", err);
		return INA_FAILURE;
	}
	data = (unsigned char*)(*mapping)->lpMapAddress + delta;
#else
    int pprot = 0;
    int pflags = 0;

    if (prot_flags & INA_MMAP_MEM_PROT_READ) {
        pprot |= PROT_READ;
    }
    if (prot_flags & INA_MMAP_MEM_PROT_WRITE) {
        pprot |= PROT_WRITE;
    }
    if (prot_flags & INA_MMAP_MEM_PROT_EXEC) {
        pprot |= PROT_EXEC;
    }
    switch (share) {
        case INA_MMAP_MEM_SHARE_PRIVATE:
            pflags |= MAP_PRIVATE;
            break;
        case INA_MMAP_MEM_SHARE_SHARED:
            pflags |= MAP_SHARED;
            break;
    }
    pflags |= MAP_FILE;
    
    (*mapping)->addr = mmap(0, length, pprot, pflags, *((int*)ina_file_os_handle(fd)), offset);
    if ((*mapping)->addr == MAP_FAILED) {
        /* FIXME: handle error */
        return INA_FAILURE;
    }
    data = (unsigned char*)(*mapping)->addr;
#endif
	
	(*mapping)->begin_mmap = data;
	(*mapping)->end_mmap = (unsigned char*)data+length;

	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_mmap_free(ina_mmap_ctx_t *ctx, ina_mmap_mapping_t **mapping)
{
#ifdef INA_OS_WIN32
	UnmapViewOfFile((*mapping)->lpMapAddress);
	CloseHandle((*mapping)->fmap);
#else
    munmap((*mapping)->addr, (*mapping)->length);
#endif
	ina_mem_free(*mapping);
	*mapping = NULL;
	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_mmap_sync(ina_mmap_mapping_t *mapping)
{
#ifdef INA_OS_WIN32
	if (!FlushViewOfFile(mapping->begin_mmap, 0)) {
		/* FIXME: handle error */
		return INA_FAILURE;
	}
	if (!FlushFileBuffers((HANDLE)ina_file_os_handle(mapping->fd))) {
		/* FIXME: handle error */
		return INA_FAILURE;
	}
#else
    if (msync(mapping->addr, mapping->length, MS_SYNC) != 0) {
        /* FIXME: handle error */
        return INA_FAILURE;
    }
#endif
	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_mmap_memory_head(ina_mmap_mapping_t *mapping, void **memory)
{
	*memory = mapping->begin_mmap;
	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_mmap_memory_tail(ina_mmap_mapping_t *mapping, void **memory)
{
	*memory = mapping->end_mmap;
	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_mmap_advice(ina_mmap_mapping_t *mapping, size_t length, ina_mmap_mem_advice_t advice)
{
#ifndef INA_OS_WIN32
    int padvice = 0;

    switch (advice) {
        case INA_MMAP_MEM_ADVICE_SEQUENTIAL:
            padvice = MADV_SEQUENTIAL;
            break;
        case INA_MMAP_MEM_ADVICE_RANDOM:
            padvice = MADV_RANDOM;
            break;
    }
    if (madvise(mapping->addr, mapping->length, padvice) != 0) {
        /* FIXME: handle error */
        return INA_FAILURE;
    } 
#endif
	return INA_SUCCESS;
}

