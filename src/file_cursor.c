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

struct ina_file_cursor_s {
	ina_file_t *file;
	ina_file_cursor_type_t cur_type;
	ina_file_cursor_mode_t mode;
    ina_mempool_t *mpref;
	union {
		struct {
			int mmap_flags;
			uint64_t position;
			void *mem_pos;
			ina_mmap_ctx_t *mmap_ctx;
			ina_mmap_mapping_t *fm;
			int eof;
			uint64_t len;
            uint64_t carry;
			uint64_t buffer_size;
			int buffer_idx;
		} m;
		struct {
            int eof;
            uint64_t buffer_size;
            uint64_t len;
            uint64_t position;
            unsigned char *buffer;
            ina_str_t line;
            ina_str_t next_line;
            ina_mempool_t *lmp;
		} f;
	} ext;
	ina_file_cursor_free_fp free_fp;
	ina_file_cursor_get_buffer_size_fp get_buffer_size_fp;
	ina_file_cursor_get_pos_fp get_pos_fp;
	ina_file_cursor_set_pos_fp set_pos_fp;
	ina_file_cursor_set_bof_fp set_bof_fp;
	ina_file_cursor_set_eof_fp set_eof_fp;
	ina_file_cursor_text_read_line_fp text_read_line_fp;
	ina_file_cursor_binary_read_chunk_fp bin_read_chunk_fp;
	ina_file_cursor_binary_readwrite_chunk_fp bin_readwrite_fp;
	ina_file_cursor_text_read_chunk_fp text_read_chunk_fp;
};

/* FILE I/O */

static ina_rc_t ina_file_cursor_fileio_free(ina_file_cursor_t **cursor)
{
    if ((*cursor)->ext.f.lmp != NULL) {
        ina_mempool_free(&(*cursor)->ext.f.lmp);
    }
    if ((*cursor)->ext.f.line != NULL) {
        ina_str_free((*cursor)->ext.f.line);
    }
    if ((*cursor)->ext.f.next_line != NULL) {
        ina_str_free((*cursor)->ext.f.next_line);
    }
    ina_mem_free((*cursor)->ext.f.buffer);
	ina_mem_free(*cursor);
    *cursor = NULL;
    return INA_SUCCESS;
}

static ina_rc_t ina_file_cursor_fileio_get_buffer_size(const ina_file_cursor_t *cursor, uint64_t *buffer_size)
{
	*buffer_size = cursor->ext.f.buffer_size;
	return INA_SUCCESS;
}

static ina_rc_t ina_file_cursor_fileio_get_pos(const ina_file_cursor_t *cursor, uint64_t *position)
{
	*position = cursor->ext.f.position;
	return INA_SUCCESS;
}

static ina_rc_t ina_file_cursor_fileio_set_pos(ina_file_cursor_t *cursor, uint64_t position)
{
	if (INA_SUCCEED(ina_file_set_pos(cursor->file, position, INA_FILE_SEEK_MODE_SET))) {
		cursor->ext.f.position = position;
		return INA_SUCCESS;
	}
	return ina_err_get_rc();
}

static ina_rc_t ina_file_cursor_fileio_set_eof(ina_file_cursor_t *cursor)
{
	return ina_file_set_eof(cursor->file);
}

static ina_rc_t ina_file_cursor_fileio_set_bof(ina_file_cursor_t *cursor)
{
	return ina_file_set_bof(cursor->file);
}

static ina_rc_t ina_file_cursor_fileio_binary_read_chunk(ina_file_cursor_t *cursor, size_t requested,
                                                         size_t *nread, const unsigned char **chunk)
{
    if (INA_FAILED(ina_file_read(cursor->file, cursor->ext.f.buffer,
                                    requested, nread))) {
        return ina_err_get_rc();
    }
    *chunk = cursor->ext.f.buffer;
    cursor->ext.f.position += *nread;
    if (*nread == 0) {
        cursor->ext.f.eof = 1;
    }
	return INA_SUCCESS;
}

static ina_rc_t ina_file_cursor_fileio_text_read_chunk(ina_file_cursor_t *cursor, size_t requested,
                                                       size_t *nread, const char **chunk)
{
    if (INA_FAILED(ina_file_cursor_fileio_binary_read_chunk(cursor, requested,
            nread, (const unsigned char **)chunk))) {
        return ina_err_get_rc();
    }
    *chunk = (const char*)cursor->ext.f.buffer;
	return INA_SUCCESS;
}

static ina_rc_t ina_file_cursor_fileio_binary_readwrite_chunk(ina_file_cursor_t *cursor, size_t requested,
                                                              size_t *actual, unsigned char **chunk)
{
	INA_UNUSED(cursor);
	INA_UNUSED(requested);
	INA_UNUSED(actual);
	INA_UNUSED(chunk);
    /* this function only works with an mmap cursor */
    return INA_ERROR(INA_ES_FUNCTION | INA_ERR_ILLEGAL);
}

static ina_rc_t ina_file_cursor_fileio_text_read_line(ina_file_cursor_t *cursor, const char **begin_line, size_t *len)
{
    size_t nread = 0;
    const char *chunk;
    if (cursor->ext.f.line != NULL) {
        ina_str_free(cursor->ext.f.line);
    }
    if (INA_FAILED(ina_file_cursor_fileio_text_read_chunk(cursor, (size_t)cursor->ext.f.buffer_size, &nread, &chunk))) {
        return ina_err_get_rc();
    }
    if (nread == 0) {
        *begin_line = NULL;
        *len = 0;
        return INA_SUCCESS;
    }
    for (;;) {
        unsigned int i;
        for (i = 0; i < nread; i++) {
            char c = chunk[i];
            if (c == '\n') {
                cursor->ext.f.next_line = ina_str_ncatcstr(cursor->ext.f.next_line, chunk, i);
                cursor->ext.f.line = ina_str_dup(cursor->ext.f.next_line);
                *begin_line = ina_str_cstr(cursor->ext.f.line);
                *len = ina_str_len(cursor->ext.f.line);
                ina_str_free(cursor->ext.f.next_line);
                cursor->ext.f.next_line = ina_str_new(nread - i);
                cursor->ext.f.next_line = ina_str_ncatcstr(cursor->ext.f.next_line, chunk+i, nread);
                return INA_SUCCESS;
            }
        }
        cursor->ext.f.next_line = ina_str_ncatcstr(cursor->ext.f.next_line, chunk, nread);
        if (INA_FAILED(ina_file_cursor_fileio_text_read_chunk(cursor, (size_t)cursor->ext.f.buffer_size, &nread, &chunk))) {
            return ina_err_get_rc();
        }
    }
}

static ina_rc_t ina_file_cursor_fileio_text_read_line_mp(ina_file_cursor_t *cursor, const char **begin_line, size_t *len)
{
    size_t nread = 0;
    const char *chunk;
    if (cursor->ext.f.lmp == NULL) {
        if (INA_FAILED(ina_mempool_new(1024, NULL, INA_MEM_DYNAMIC, &cursor->ext.f.lmp))) {
            return ina_err_get_rc();
        }
    }
    else {
        ina_mempool_reset(cursor->ext.f.lmp);
    }
    if (INA_FAILED(ina_file_cursor_fileio_text_read_chunk(cursor, (size_t)cursor->ext.f.buffer_size, &nread, &chunk))) {
        return ina_err_get_rc();
    }
    if (nread == 0) {
        *begin_line = NULL;
        *len = 0;
        return INA_SUCCESS;
    }
    for (;;) {
        unsigned int i;
        for (i = 0; i < nread; i++) {
            char c = chunk[i];
            if (c == '\n') {
                cursor->ext.f.next_line = ina_str_ncatcstr_using_pool(cursor->ext.f.next_line, chunk, i, cursor->ext.f.lmp);
                cursor->ext.f.line = ina_str_dup_using_pool(cursor->ext.f.next_line, cursor->ext.f.lmp);
                *begin_line = ina_str_cstr(cursor->ext.f.line);
                *len = ina_str_len(cursor->ext.f.line);
                cursor->ext.f.next_line = ina_str_new_using_pool(nread - i, cursor->ext.f.lmp);
                cursor->ext.f.next_line = ina_str_ncatcstr_using_pool(cursor->ext.f.next_line, chunk+i, nread, cursor->ext.f.lmp);
                return INA_SUCCESS;
            }
        }
        cursor->ext.f.next_line = ina_str_ncatcstr_using_pool(cursor->ext.f.next_line, chunk, nread, cursor->ext.f.lmp);
        if (INA_FAILED(ina_file_cursor_fileio_text_read_chunk(cursor, (size_t)cursor->ext.f.buffer_size, &nread, &chunk))) {
            return ina_err_get_rc();
        }
    }
}

/* MMAP */

static ina_rc_t ina_file_cursor_mmap_free(ina_file_cursor_t **cursor)
{
	ina_mmap_free(&(*cursor)->ext.m.fm);
	ina_mem_free(*cursor);
	*cursor = NULL;
	return INA_SUCCESS;
}

static ina_rc_t ina_file_cursor_mmap_get_buffer_size(const ina_file_cursor_t *cursor, uint64_t *buffer_size)
{
	*buffer_size = cursor->ext.m.buffer_size;
	return INA_SUCCESS;
}

static ina_rc_t ina_file_cursor_mmap_get_pos(const ina_file_cursor_t *cursor, uint64_t *position)
{
	*position = cursor->ext.m.position;
	return INA_SUCCESS;
}

static ina_rc_t ina_file_cursor_mmap_set_pos(ina_file_cursor_t *cursor, uint64_t position)
{
	void *head;
	int buffer_idx = (int)floor((double)(position/cursor->ext.m.buffer_size));
    uint64_t tmp = (uint64_t)buffer_idx * (uint64_t)cursor->ext.m.buffer_size;
	uint64_t offset = tmp - cursor->ext.m.carry;

	if (position > cursor->ext.m.len) {
		return INA_ERROR(INA_ES_POSITION | INA_ERR_OUT_OF_RANGE);
	}
	if (cursor->ext.m.buffer_idx != buffer_idx) {
		uint64_t len = INA_MIN(cursor->ext.m.buffer_size, cursor->ext.m.len);
        uint64_t tmp2 = (uint64_t)cursor->ext.m.buffer_idx * (uint64_t)cursor->ext.m.buffer_size;
        uint64_t carry = tmp2 + len - cursor->ext.m.position;
        offset = tmp - carry;
        cursor->ext.m.carry = carry;
		if (offset + len > cursor->ext.m.len) {
			len = cursor->ext.m.len - offset;
		}
		ina_mmap_free(&cursor->ext.m.fm);
		ina_mmap_new(cursor->ext.m.mmap_ctx, cursor->file, cursor->ext.m.mmap_flags, 
			INA_MMAP_MEM_SHARE_SHARED,INA_MMAP_MAP_TYPE_FILE, offset, len, &cursor->ext.m.fm);
		cursor->ext.m.buffer_idx = buffer_idx;
	}

	ina_mmap_memory_head(cursor->ext.m.fm, &head);
	cursor->ext.m.position = position;
	cursor->ext.m.mem_pos = (unsigned char*)head + (position-offset);

	return INA_SUCCESS;
}

static ina_rc_t ina_file_cursor_mmap_set_eof(ina_file_cursor_t *cursor)
{
	return ina_file_cursor_set_pos(cursor, cursor->ext.m.len);
}

static ina_rc_t ina_file_cursor_mmap_set_bof(ina_file_cursor_t *cursor)
{
	return ina_file_cursor_set_pos(cursor, 0);
}

static ina_rc_t ina_file_cursor_mmap_text_read_line(ina_file_cursor_t *cursor, const char **begin_line, size_t *len)
{
	size_t l = 0;
	unsigned char *d = (unsigned char*)cursor->ext.m.mem_pos;
	void *tail;
	const void *end;
	int modify_cursor = 1;
	
	ina_mmap_memory_tail(cursor->ext.m.fm, &tail);
	end = tail;

	if (cursor->ext.m.eof) {
		*len = l;
		*begin_line = NULL;
	}

	*begin_line = (const char*)d;

	while (*d != '\n') {
		const void *c;
		++d;
		++l;
		c = (const void*)d;
		if (c >= end || *d == '\0') {
			if (cursor->ext.m.position + l >= cursor->ext.m.len) {
				cursor->ext.m.eof = 1;
				l = 0;
			}
			else {
				ina_file_cursor_set_pos(cursor, cursor->ext.m.position + l);
			}
			modify_cursor = 0;
			break;
		}
	}
	if (modify_cursor && *d == '\n') {
		++d;
		++l;
	}
	if (modify_cursor) {
		cursor->ext.m.position += l;
		cursor->ext.m.mem_pos = (void*)d;
	}
	
	*len = l;

	return INA_SUCCESS;
}

static ina_rc_t ina_file_cursor_mmap_binary_read_chunk(ina_file_cursor_t *cursor, size_t requested,
							  size_t *read, const unsigned char **chunk)
{
	if (INA_SUCCEED(ina_file_cursor_set_pos(cursor, cursor->ext.m.position + requested))) {
		*read = requested;
        *chunk = (unsigned char*)cursor->ext.m.mem_pos - *read;
	}
	else {
		*read = 0;
        *chunk = NULL;
	}

	return INA_SUCCESS;
}

static ina_rc_t ina_file_cursor_mmap_text_read_chunk(ina_file_cursor_t *cursor, size_t requested,
							size_t *read, const char **chunk)
{
	const unsigned char *bin_chunk;
	ina_file_cursor_binary_read_chunk(cursor, requested, read, &bin_chunk);
	*chunk = (const char*)bin_chunk;
	return INA_SUCCESS;
}

static ina_rc_t ina_file_cursor_mmap_binary_readwrite_chunk(ina_file_cursor_t *cursor, size_t requested,
							       size_t *actual, unsigned char **chunk)
{
	INA_UNUSED(cursor);
	INA_UNUSED(requested);
	INA_UNUSED(actual);
	INA_UNUSED(chunk);
	return INA_ERROR(INA_ES_FUNCTION | INA_ERR_ILLEGAL);
}

static ina_rc_t ina_file_cursor_init_internal(ina_file_t *file, 
                                              ina_file_cursor_type_t cursor_type,
                                              ina_file_cursor_mode_t mode, uint64_t buffer_size,
                                              ina_file_cursor_t *cursor,
                                              ina_mmap_ctx_t *mmap_ctx)
{
    ina_file_stat_t *fstat = NULL;
	uint64_t flen = 0;

	if (INA_FAILED(ina_file_stat_new(file, &fstat))) {
		return ina_err_get_rc();
	}
	ina_file_stat_file_size(fstat, &flen);
	ina_file_stat_free(&fstat);

	cursor->file = file;
    cursor->cur_type = cursor_type;
    cursor->mode = mode;

	if (cursor_type == INA_FILE_CURSOR_TYPE_MMAP) {
		int proto_flags = INA_MMAP_MEM_PROT_READ;
		uint64_t map_len = 0;
		void *head = NULL;

		if (mode == INA_FILE_CURSOR_MODE_READWRITE_BINARY
                || mode == INA_FILE_CURSOR_MODE_READWRITE_TEXT_CHUNK
                || mode == INA_FILE_CURSOR_MODE_READWRITE_TEXT_LINE) {
                proto_flags |= INA_MMAP_MEM_PROT_WRITE;
        }
		cursor->ext.m.mmap_ctx = mmap_ctx;
        if (flen <= buffer_size) {
                map_len = flen;
            }
        else {
                map_len = buffer_size;
        }

        cursor->ext.m.fm = NULL;

        ina_mmap_new(mmap_ctx, file, proto_flags,
                INA_MMAP_MEM_SHARE_SHARED,
                INA_MMAP_MAP_TYPE_FILE, 
                0, map_len, &cursor->ext.m.fm);

        ina_mmap_memory_head(cursor->ext.m.fm, &head);

        cursor->ext.m.mmap_flags = proto_flags;
        cursor->ext.m.position = 0;
        cursor->ext.m.mem_pos = head;
        cursor->ext.m.eof = 0;
        cursor->ext.m.buffer_idx = 0;
        cursor->ext.m.buffer_size = buffer_size;
        cursor->ext.m.len = flen;
		cursor->ext.m.carry = 0;
		cursor->free_fp = ina_file_cursor_mmap_free;
		cursor->get_buffer_size_fp = ina_file_cursor_mmap_get_buffer_size;
		cursor->get_pos_fp = ina_file_cursor_mmap_get_pos;
		cursor->set_pos_fp = ina_file_cursor_mmap_set_pos;
		cursor->set_bof_fp = ina_file_cursor_mmap_set_bof;
		cursor->set_eof_fp = ina_file_cursor_mmap_set_eof;
		cursor->text_read_line_fp = ina_file_cursor_mmap_text_read_line;
		cursor->bin_read_chunk_fp = ina_file_cursor_mmap_binary_read_chunk;
		cursor->bin_readwrite_fp = ina_file_cursor_mmap_binary_readwrite_chunk;
		cursor->text_read_chunk_fp = ina_file_cursor_mmap_text_read_chunk;
	}
	else {
        cursor->ext.f.len = flen;
        cursor->ext.f.buffer_size = buffer_size;
        cursor->ext.f.eof = 0;
        cursor->ext.f.position = 0;
        if (cursor->mpref == NULL) {
            cursor->ext.f.buffer = (unsigned char*)ina_mem_alloc(sizeof(unsigned char)*(size_t)buffer_size);
        }
        else {
            cursor->ext.f.buffer = (unsigned char*)ina_mempool_dalloc(cursor->mpref, sizeof(unsigned char)*(size_t)buffer_size);
        }
		cursor->free_fp = ina_file_cursor_fileio_free;
		cursor->get_buffer_size_fp = ina_file_cursor_fileio_get_buffer_size;
        cursor->get_pos_fp = ina_file_cursor_fileio_get_pos;
        cursor->set_pos_fp = ina_file_cursor_fileio_set_pos;
        cursor->set_bof_fp = ina_file_cursor_fileio_set_bof;
        cursor->set_eof_fp = ina_file_cursor_fileio_set_eof;
        if (cursor->mpref == NULL) {
            cursor->text_read_line_fp = ina_file_cursor_fileio_text_read_line;
        }
        else {
            cursor->text_read_line_fp = ina_file_cursor_fileio_text_read_line_mp;
        }
        cursor->bin_read_chunk_fp = ina_file_cursor_fileio_binary_read_chunk;
        cursor->bin_readwrite_fp = ina_file_cursor_fileio_binary_readwrite_chunk;
        cursor->text_read_chunk_fp = ina_file_cursor_fileio_text_read_chunk;
	}
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_file_cursor_new_using_pool(ina_file_t *file, 
                                                 ina_file_cursor_type_t cursor_type,
                                                 ina_file_cursor_mode_t mode, uint64_t buffer_size,
                                                 ina_file_cursor_t **cursor,
                                                 ina_mmap_ctx_t *mmap_ctx,
                                                 ina_mempool_t *pool)
{
	INA_VERIFY_NOT_NULL(file);
    INA_VERIFY_NOT_NULL(cursor);
	INA_VERIFY_NOT_NULL(mmap_ctx);
	INA_VERIFY_NOT_NULL(pool);
    *cursor = (ina_file_cursor_t*)ina_mempool_dalloc(pool, sizeof(ina_file_cursor_t));
    INA_RETURN_IF_NULL(*cursor);
    (*cursor)->mpref = pool;
    return ina_file_cursor_init_internal(file, cursor_type, mode, buffer_size, *cursor, mmap_ctx);
}


INA_API(ina_rc_t) ina_file_cursor_new(ina_file_t *file, 
									  ina_file_cursor_type_t cursor_type,
									  ina_file_cursor_mode_t mode, uint64_t buffer_size,
									  ina_file_cursor_t **cursor,
                                      ina_mmap_ctx_t *mmap_ctx)
{
    INA_VERIFY_NOT_NULL(file);
    INA_VERIFY_NOT_NULL(cursor);
    INA_VERIFY_NOT_NULL(mmap_ctx);
    *cursor = (ina_file_cursor_t*)ina_mem_alloc(sizeof(ina_file_cursor_t));
    (*cursor)->mpref = NULL;
	return ina_file_cursor_init_internal(file, cursor_type, mode, buffer_size, *cursor, mmap_ctx);
}

INA_API(ina_rc_t) ina_file_cursor_free(ina_file_cursor_t **cursor)
{
	ina_file_cursor_t *c = *cursor;
	return c->free_fp(cursor);
}

INA_API(ina_rc_t) ina_file_cursor_get_file(const ina_file_cursor_t *cursor, ina_file_t **file)
{
	INA_VERIFY_NOT_NULL(cursor);
	INA_VERIFY_NOT_NULL(file);
	*file = cursor->file;
	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_file_cursor_get_buffer_size(const ina_file_cursor_t *cursor, uint64_t *buffer_size)
{
	INA_VERIFY_NOT_NULL(cursor);
	INA_VERIFY_NOT_NULL(buffer_size);
	return cursor->get_buffer_size_fp(cursor, buffer_size);
}

INA_API(ina_rc_t) ina_file_cursor_get_mode(const ina_file_cursor_t *cursor, ina_file_cursor_mode_t *mode)
{
	INA_VERIFY_NOT_NULL(cursor);
	INA_VERIFY_NOT_NULL(mode);
	*mode = cursor->mode;
	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_file_cursor_get_type(const ina_file_cursor_t *cursor, ina_file_cursor_type_t *type)
{
	INA_VERIFY_NOT_NULL(cursor);
	INA_VERIFY_NOT_NULL(type);
	*type = cursor->cur_type;
	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_file_cursor_get_pos(const ina_file_cursor_t *cursor, uint64_t *position)
{
	INA_VERIFY_NOT_NULL(cursor);
	INA_VERIFY_NOT_NULL(position);
	return cursor->get_pos_fp(cursor, position);
}

INA_API(ina_rc_t) ina_file_cursor_set_pos(ina_file_cursor_t *cursor, uint64_t position)
{
    INA_VERIFY_NOT_NULL(cursor);
	return cursor->set_pos_fp(cursor, position);
}

INA_API(ina_rc_t) ina_file_cursor_set_bof(ina_file_cursor_t *cursor)
{
	return cursor->set_bof_fp(cursor);
}

INA_API(ina_rc_t) ina_file_cursor_set_eof(ina_file_cursor_t *cursor)
{
    INA_VERIFY_NOT_NULL(cursor);
	return cursor->set_eof_fp(cursor);
}

INA_API(ina_rc_t) ina_file_cursor_text_read_line(ina_file_cursor_t *cursor, const char **begin_line, size_t *len)
{
	return cursor->text_read_line_fp(cursor, begin_line, len);
}

INA_API(ina_rc_t) ina_file_cursor_binary_read_chunk(ina_file_cursor_t *cursor, size_t requested,
												  size_t *read, const unsigned char **chunk)
{
	return cursor->bin_read_chunk_fp(cursor, requested, read, chunk);
}

INA_API(ina_rc_t) ina_file_cursor_binary_readwrite_chunk(ina_file_cursor_t *cursor, size_t requested,
												       size_t *actual, unsigned char **chunk)
{
	return cursor->bin_readwrite_fp(cursor, requested, actual, chunk);
}

INA_API(ina_rc_t) ina_file_cursor_text_read_chunk(ina_file_cursor_t *cursor, size_t requested,
												size_t *read, const char **chunk)
{
	return cursor->text_read_chunk_fp(cursor, requested, read, chunk);
}

