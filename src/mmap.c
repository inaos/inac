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
	uint64_t offset;
	uint64_t length;
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
                               ina_mmap_map_type_t map_type,
                               uint64_t offset, uint64_t length, ina_mmap_mapping_t **mapping)
{
	void *data = NULL;
	ina_file_stat_t *fstat = NULL;
	uint64_t flen = 0;

#ifdef INA_OS_WIN32
	DWORD flProtect = 0;
	uint64_t llFileMapStart;
	uint64_t llMapViewSize;
    DWORD dwHigh;
    DWORD dwLow;
	DWORD dwDesiredAccess;
	uint64_t delta;
    DWORD dwAllocationGranularity;
    SYSTEM_INFO si;
#endif

	if (fd) {
		ina_file_stat_new(fd, &fstat);
		ina_file_stat_file_size(fstat, &flen);
		ina_file_stat_free(fd, &fstat);

		if (offset + length > flen) {
			/* FIXME: proper error handling */
			return INA_FAILURE;
		}
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

	(*mapping)->fmap = CreateFileMapping((HANDLE)ina_file_os_handle(fd), NULL, flProtect, 0, 0, NULL);
	if ((*mapping)->fmap == INVALID_HANDLE_VALUE) {
		/* FIXME: handle error */
		DWORD err = GetLastError();
		printf("%d", err);
		return INA_FAILURE;
	}

	// To calculate where to start the file mapping, round down the
	// offset of the data into the file to the nearest multiple of the
	// system allocation granularity.
    memset(&si, 0, sizeof(SYSTEM_INFO));
    GetSystemInfo(&si);
    dwAllocationGranularity = si.dwAllocationGranularity;
	llFileMapStart = ((uint64_t)(offset / dwAllocationGranularity)) * dwAllocationGranularity;
	
	// Calculate the size of the file mapping view.
	llMapViewSize = ( (offset % dwAllocationGranularity) + length );
	llMapViewSize = INA_MAX(llMapViewSize, length);
	
	// The data of interest isn't at the beginning of the
	// view, so determine how far into the view to set the pointer.
	delta = offset - llFileMapStart;

    dwHigh = ((llFileMapStart >> 32) & 0xFFFFFFFF);
    dwLow = (llFileMapStart & 0xFFFFFFFF);
	
    (*mapping)->lpMapAddress = MapViewOfFile((*mapping)->fmap, dwDesiredAccess, dwHigh, dwLow, (SIZE_T)llMapViewSize);
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
    switch (map_type) {
    	case INA_MMAP_MAP_TYPE_FILE:
    		pflags |= MAP_FILE;
    		break;
    	case INA_MMAP_MAP_TYPE_MEMORY:
    		pflags |= MAP_ANONYMOUS;
    		break;
    }
    
    if (pflags&MAP_FILE) {
    	(*mapping)->addr = mmap(0, length, pprot, pflags, *((int*)ina_file_os_handle(fd)), offset);
    } else {
    	(*mapping)->addr = mmap(0, length, pprot, pflags, -1, offset);
    }
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
	UnmapViewOfFile((*mappinmlockallg)->lpMapAddress);
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

