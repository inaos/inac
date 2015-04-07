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

struct ina_file_cursor_s {
	ina_file_t *file;
	ina_file_cursor_type_t cur_type;
	ina_file_cursor_mode_t mode;
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
			size_t buffer_size;
			int buffer_idx;
		} m;
		struct {
			int eof;
		} f;
	} ext;
	ina_file_cursor_free_fp free_fp;
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
	ina_mem_free(*cursor);
        *cursor = NULL;
        return INA_SUCCESS;
}

static ina_rc_t ina_file_cursor_fileio_set_pos(ina_file_cursor_t *cursor, uint64_t position)
{
	return ina_file_set_pos(cursor->file, position);
}

static ina_rc_t ina_file_cursor_fileio_set_eof(ina_file_cursor_t *cursor)
{
	return ina_file_set_eof(cursor->file);
}

static ina_rc_t ina_file_cursor_fileio_set_bof(ina_file_cursor_t *cursor)
{
	return ina_file_set_bof(cursor->file);
}

static ina_rc_t ina_file_cursor_fileio_text_read_line(ina_file_cursor_t *cursor, const char **begin_line, size_t *len)
{
	return INA_SUCCESS;
}

static ina_rc_t ina_file_cursor_fileio_binary_read_chunk(ina_file_cursor_t *cursor, size_t requested,
                                                         size_t *read, const unsigned char **chunk)
{
        
	return INA_SUCCESS;
}

static ina_rc_t ina_file_cursor_fileio_text_read_chunk(ina_file_cursor_t *cursor, size_t requested,
                                                       size_t *read, const char **chunk)
{
	return INA_SUCCESS;
}


static ina_rc_t ina_file_cursor_fileio_binary_readwrite_chunk(ina_file_cursor_t *cursor, size_t requested,
                                                              size_t *actual, unsigned char **chunk)
{
        return INA_FAILURE;
}

/* MMAP */

static ina_rc_t ina_file_cursor_mmap_free(ina_file_cursor_t **cursor)
{
	ina_mmap_free((*cursor)->ext.m.mmap_ctx, &(*cursor)->ext.m.fm);
	ina_mem_free(*cursor);
	*cursor = NULL;
	return INA_SUCCESS;
}

static ina_rc_t ina_file_cursor_mmap_set_pos(ina_file_cursor_t *cursor, uint64_t position)
{
	void *head;
	int buffer_idx = (int)floor((double)(position/cursor->ext.m.buffer_size));
    uint64_t tmp = (uint64_t)buffer_idx * (uint64_t)cursor->ext.m.buffer_size;
	uint64_t offset = tmp - cursor->ext.m.carry;

	if (position > cursor->ext.m.len) {
		return INA_FAILURE;
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
		ina_mmap_free(cursor->ext.m.mmap_ctx, &cursor->ext.m.fm);
		ina_mmap_new(cursor->ext.m.mmap_ctx, cursor->file, cursor->ext.m.mmap_flags, 
			INA_MMAP_MEM_SHARE_SHARED, offset, len, &cursor->ext.m.fm);
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
	return INA_FAILURE;
}


INA_API(ina_rc_t) ina_file_cursor_new(ina_file_t *file, 
									  ina_file_cursor_type_t cursor_type,
									  ina_file_cursor_mode_t mode, size_t buffer_size,
									  ina_file_cursor_t **cursor,
                                      ina_mmap_ctx_t *mmap_ctx)
{
	ina_file_stat_t *fstat = NULL;
	uint64_t flen = 0;

	ina_file_stat_new(file, &fstat);
	ina_file_stat_file_size(fstat, &flen);
	ina_file_stat_free(file, &fstat);

	*cursor = (ina_file_cursor_t*)ina_mem_alloc(sizeof(ina_file_cursor_t));
	(*cursor)->file = file;
        (*cursor)->cur_type = cursor_type;
        (*cursor)->mode = mode;

	if (cursor_type == INA_FILE_CURSOR_TYPE_MMAP) {
		int proto_flags = INA_MMAP_MEM_PROT_READ;
		uint64_t map_len = 0;
		void *head = NULL;

		if (mode == INA_FILE_CURSOR_MODE_READWRITE_BINARY
                	|| mode == INA_FILE_CURSOR_MODE_READWRITE_TEXT_CHUNK
                	|| mode == INA_FILE_CURSOR_MODE_READWRITE_TEXT_LINE) {
                	proto_flags |= INA_MMAP_MEM_PROT_WRITE;
        	}
		(*cursor)->ext.m.mmap_ctx = mmap_ctx;
        	if (flen <= buffer_size) {
                	map_len = flen;
                }
        	else {
                	map_len = buffer_size;
        	}

        	(*cursor)->ext.m.fm = NULL;

        	ina_mmap_new(mmap_ctx, file, proto_flags,
                	INA_MMAP_MEM_SHARE_SHARED, 0, map_len, &(*cursor)->ext.m.fm);

        	ina_mmap_memory_head((*cursor)->ext.m.fm, &head);

        	(*cursor)->ext.m.mmap_flags = proto_flags;
        	(*cursor)->ext.m.position = 0;
        	(*cursor)->ext.m.mem_pos = head;
        	(*cursor)->ext.m.eof = 0;
        	(*cursor)->ext.m.buffer_idx = 0;
        	(*cursor)->ext.m.buffer_size = buffer_size;
        	(*cursor)->ext.m.len = flen;
		(*cursor)->ext.m.carry = 0;
		(*cursor)->free_fp = ina_file_cursor_mmap_free;
		(*cursor)->set_pos_fp = ina_file_cursor_mmap_set_pos;
		(*cursor)->set_bof_fp = ina_file_cursor_mmap_set_bof;
		(*cursor)->set_eof_fp = ina_file_cursor_mmap_set_eof;
		(*cursor)->text_read_line_fp = ina_file_cursor_mmap_text_read_line;
		(*cursor)->bin_read_chunk_fp = ina_file_cursor_mmap_binary_read_chunk;
		(*cursor)->bin_readwrite_fp = ina_file_cursor_mmap_binary_readwrite_chunk;
		(*cursor)->text_read_chunk_fp = ina_file_cursor_mmap_text_read_chunk;
	}
	else {
		(*cursor)->free_fp = ina_file_cursor_fileio_free;
                (*cursor)->set_pos_fp = ina_file_cursor_fileio_set_pos;
                (*cursor)->set_bof_fp = ina_file_cursor_fileio_set_bof;
                (*cursor)->set_eof_fp = ina_file_cursor_fileio_set_eof;
                (*cursor)->text_read_line_fp = ina_file_cursor_fileio_text_read_line;
                (*cursor)->bin_read_chunk_fp = ina_file_cursor_fileio_binary_read_chunk;
                (*cursor)->bin_readwrite_fp = ina_file_cursor_fileio_binary_readwrite_chunk;
                (*cursor)->text_read_chunk_fp = ina_file_cursor_fileio_text_read_chunk;
	}

	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_file_cursor_free(ina_file_cursor_t **cursor)
{
	ina_file_cursor_t *c = *cursor;
	return c->free_fp(cursor);
}

INA_API(ina_rc_t) ina_file_cursor_set_pos(ina_file_cursor_t *cursor, uint64_t position)
{
	return cursor->set_pos_fp(cursor, position);
}

INA_API(ina_rc_t) ina_file_cursor_set_bof(ina_file_cursor_t *cursor)
{
	return cursor->set_bof_fp(cursor);
}

INA_API(ina_rc_t) ina_file_cursor_set_eof(ina_file_cursor_t *cursor)
{
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

