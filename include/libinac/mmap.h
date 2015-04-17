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
#ifndef _LIBINAC_MMAP_H_
#define _LIBINAC_MMAP_H_

#include <libinac/lib.h>

/**
 *
 * TODO:
 * -> CreateFileMappingNuma? Linux alternative.. or use huge-pages?
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ina_mmap_mem_prot_e {
    INA_MMAP_MEM_PROT_READ = 0x01,
    INA_MMAP_MEM_PROT_WRITE = 0x02,
    INA_MMAP_MEM_PROT_EXEC = 0x04,
} ina_mmap_mem_prot_t;

typedef enum ina_mmap_mem_share_e {
    INA_MMAP_MEM_SHARE_PRIVATE,
    INA_MMAP_MEM_SHARE_SHARED
} ina_mmap_mem_share_t;

typedef enum ina_mmap_map_type_e {
    INA_MMAP_MAP_TYPE_FILE,
    INA_MMAP_MAP_TYPE_MEMORY
} ina_mmap_map_type_t;

typedef enum ina_mmap_mem_advice_e {
    INA_MMAP_MEM_ADVICE_SEQUENTIAL,
    INA_MMAP_MEM_ADVICE_RANDOM
} ina_mmap_mem_advice_t;

/* opaque mmap types */
typedef struct ina_mmap_ctx_s ina_mmap_ctx_t;
typedef struct ina_mmap_mapping_s ina_mmap_mapping_t;

/*
 *
 */
INA_API(ina_rc_t) ina_mmap_init(ina_mmap_ctx_t **ctx);

/*
 *
 */
INA_API(ina_rc_t) ina_mmap_destroy(ina_mmap_ctx_t **ctx);

/*
 *
 */
INA_API(ina_rc_t) ina_mmap_new(ina_mmap_ctx_t *ctx, ina_file_t *fd, 
                               int prot_flags, ina_mmap_mem_share_t share,
                               ina_mmap_map_type_t map_type,
							   uint64_t offset, uint64_t length, ina_mmap_mapping_t **mapping);

/*
 *
 */
INA_API(ina_rc_t) ina_mmap_free(ina_mmap_ctx_t *ctx, ina_mmap_mapping_t **mapping);

/*
 *
 */
INA_API(ina_rc_t) ina_mmap_sync(ina_mmap_mapping_t *mapping);

/*
 *
 */
INA_API(ina_rc_t) ina_mmap_memory_head(ina_mmap_mapping_t *mapping, void **memory);

/*
 *
 */
INA_API(ina_rc_t) ina_mmap_memory_tail(ina_mmap_mapping_t *mapping, void **memory);


/*
 * In Windows 8 there will be PrefetchVirtualMemory for now this will be a noop in Windows
 */
INA_API(ina_rc_t) ina_mmap_advice(ina_mmap_mapping_t *mapping, size_t length, ina_mmap_mem_advice_t advice);

#ifdef __cplusplus
}
#endif

#endif
