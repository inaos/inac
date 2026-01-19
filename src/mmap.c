/*
 * Copyright 2014-2020 INAOS GmbH, Thalwil
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <libinac/lib.h>
#include "config.h"

#ifndef INA_OS_WINDOWS
#include <sys/mman.h>
#endif

struct ina_mmap_ctx_s {
	size_t page_size;
};

struct ina_mmap_mapping_s {
#ifdef INA_OS_WINDOWS
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

INA_API(ina_rc_t) ina_mmap_ctx_new(ina_mmap_ctx_t **ctx)
{
    INA_VERIFY_NOT_NULL(ctx);
    *ctx = (ina_mmap_ctx_t*)ina_mem_alloc(sizeof(ina_mmap_ctx_t));
	INA_RETURN_IF_NULL(*ctx);
    ina_mem_get_pagesize(&(*ctx)->page_size);
    return INA_SUCCESS;
}

INA_API(void) ina_mmap_ctx_free(ina_mmap_ctx_t **ctx)
{
	INA_VERIFY_FREE(ctx);
	INA_MEM_FREE_SAFE(*ctx);
	ctx = NULL;
}

INA_API(ina_rc_t) ina_mmap_new(ina_mmap_ctx_t *ctx, ina_file_t *fd, 
                               int prot_flags, ina_mmap_mem_share_t share,
                               ina_mmap_map_type_t map_type,
                               uint64_t offset, uint64_t length, ina_mmap_mapping_t **mapping)
{
	void *data = NULL;
	ina_file_stat_t *fstat = NULL;
	uint64_t flen = 0;

#ifdef INA_OS_WINDOWS
	DWORD flProtect = 0;
	uint64_t llFileMapStart;
	uint64_t llMapViewSize;
    DWORD dwHigh;
    DWORD dwLow;
	DWORD dwDesiredAccess = FILE_MAP_ALL_ACCESS;
	uint64_t delta;
    DWORD dwAllocationGranularity;
    SYSTEM_INFO si;
#endif

	INA_VERIFY_NOT_NULL(ctx);
	INA_VERIFY_NOT_NULL(fd);
	INA_VERIFY_NOT_NULL(mapping);
	*mapping = NULL;
	if (NULL != fd) {
		INA_RETURN_IF_FAILED(ina_file_stat_new(fd, &fstat));
        size_t slen;
		INA_MUST_SUCCEED(ina_file_stat_file_size(fstat, &slen));
        flen = slen;
		ina_file_stat_free(&fstat);

		if (offset > flen) {
			return INA_ERROR(INA_ES_POSITION | INA_ERR_OUT_OF_RANGE);
		}
	}

	*mapping = (ina_mmap_mapping_t*)ina_mem_alloc(sizeof(ina_mmap_mapping_t));
	INA_RETURN_IF_NULL(*mapping);
	(*mapping)->length = length;
	(*mapping)->offset = offset;

#ifdef INA_OS_WINDOWS
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

	if (map_type == INA_MMAP_MAP_TYPE_FILE) {
		(*mapping)->fmap = CreateFileMapping((HANDLE)ina_file_os_handle(fd), NULL, flProtect, 0, 0, NULL);
	} else {
		(*mapping)->fmap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, flProtect, (DWORD)offset, (DWORD)length, NULL);		
	}
	if ((*mapping)->fmap == INVALID_HANDLE_VALUE) {
		return INA_OS_ERROR(INA_ES_OPERATION|INA_ERR_FAILED);
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
		return INA_OS_ERROR(INA_ES_OPERATION|INA_ERR_FAILED);;
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
#ifdef INA_OS_OSX
            pflags |= MAP_ANON;
#else
    		pflags |= MAP_ANONYMOUS;
#endif
    		break;
    }
    
    if (map_type == INA_MMAP_MAP_TYPE_FILE) {
    	(*mapping)->addr = mmap(0, length, pprot, pflags, ina_file_os_handle(fd), offset);
    } else {
    	(*mapping)->addr = mmap(0, length, pprot, pflags, -1, offset);
    }
    if ((*mapping)->addr == MAP_FAILED) {
		ina_mmap_free(mapping);
        return INA_OS_ERROR(INA_ES_OPERATION | INA_ERR_FAILED);
    }
    data = (unsigned char*)(*mapping)->addr;
#endif
	
	(*mapping)->begin_mmap = data;
	(*mapping)->end_mmap = (unsigned char*)data+length;

	return INA_SUCCESS;
}

INA_API(void) ina_mmap_free(ina_mmap_mapping_t **mapping)
{
   INA_VERIFY_FREE(mapping);

#ifdef INA_OS_WINDOWS
	UnmapViewOfFile((*mapping)->lpMapAddress);
	CloseHandle((*mapping)->fmap);
#else
    if ((*mapping)->addr != MAP_FAILED && (*mapping)->addr != NULL) {
		munmap((*mapping)->addr, (*mapping)->length);
	}
#endif
	INA_MEM_FREE_SAFE(*mapping);
}

INA_API(ina_rc_t) ina_mmap_sync(ina_mmap_mapping_t *mapping)
{
	INA_VERIFY_NOT_NULL(mapping);
#ifdef INA_OS_WINDOWS
	if (!FlushViewOfFile(mapping->begin_mmap, 0)) {
		return INA_OS_ERROR(INA_ES_OPERATION|INA_ERR_FAILED);
	}
	if (!FlushFileBuffers((HANDLE)ina_file_os_handle(mapping->fd))) {
		return INA_OS_ERROR(INA_ES_OPERATION|INA_ERR_FAILED);
	}
#else
    if (msync(mapping->addr, mapping->length, MS_SYNC) == -1) {
        return INA_OS_ERROR(INA_ES_OPERATION | INA_ERR_FAILED);
    }
#endif
	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_mmap_memory_head(ina_mmap_mapping_t *mapping, void **memory)
{
	INA_VERIFY_NOT_NULL(mapping);
	INA_VERIFY_NOT_NULL(memory);
	*memory = mapping->begin_mmap;
	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_mmap_memory_tail(ina_mmap_mapping_t *mapping, void **memory)
{
	INA_VERIFY_NOT_NULL(mapping);
	INA_VERIFY_NOT_NULL(memory);
	*memory = mapping->end_mmap;
	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_mmap_advice(ina_mmap_mapping_t *mapping, size_t length, ina_mmap_mem_advice_t advice)
{
#ifdef INA_OS_WINDOWS
	INA_UNUSED(mapping);
	INA_UNUSED(length);
	INA_UNUSED(advice);
#else
    int padvice = 0;
    INA_VERIFY_NOT_NULL(mapping);
    INA_VERIFY(advice == INA_MMAP_MEM_ADVICE_RANDOM ||
               advice == INA_MMAP_MEM_ADVICE_SEQUENTIAL);
    INA_UNUSED(length);
    switch (advice) {
        case INA_MMAP_MEM_ADVICE_SEQUENTIAL:
            padvice = MADV_SEQUENTIAL;
            break;
        case INA_MMAP_MEM_ADVICE_RANDOM:
            padvice = MADV_RANDOM;
            break;
    }
    if (madvise(mapping->addr, mapping->length, padvice) != 0) {
        return INA_OS_ERROR(INA_ES_OPERATION | INA_ERR_FAILED);
    } 
#endif
	return INA_SUCCESS;
}

