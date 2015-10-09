/*
 * Copyright (c) 2015, INAOS GmbH
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

/* GZIP format - RFC 1952 */

#define _INA_GZIP_HEADER_ID1 0x1f
#define _INA_GZIP_HEADER_ID2 0x8b

/*
 * Header
 *
 * +---+---+---+---+---+---+---+---+---+---+
 * |ID1|ID2|CM |FLG|     MTIME     |XFL|OS | (more-->)
 * +---+---+---+---+---+---+---+---+---+---+
 *
 */
 
INA_VS_BEGIN_PACK
typedef struct INA_PACKED _ina_gzip_header_s {
	unsigned char id1;
	unsigned char id2;
	char cm;
	char flg;
	uint32_t mtime;
	char xfl;
	char os;
} _ina_gzip_header_t;
INA_VS_END_PACK
 
/* 
 * (if FLG.FEXTRA set)
 *
 * +---+---+=================================+
 * | XLEN  |...XLEN bytes of "extra field"...| (more-->)
 * +---+---+=================================+
 *
 * (if FLG.FNAME set)
 *
 * +=========================================+
 * |...original file name, zero-terminated...| (more-->)
 * +=========================================+
 *
 * (if FLG.FCOMMENT set)
 *
 * +===================================+
 * |...file comment, zero-terminated...| (more-->)
 * +===================================+
 *
 * (if FLG.FHCRC set)
 *
 * +---+---+
 * | CRC16 |
 * +---+---+
 *
 * +=======================+
 * |...compressed blocks...| (more-->)
 * +=======================+
 *
 * 0   1   2   3   4   5   6   7
 * +---+---+---+---+---+---+---+---+
 * |     CRC32     |     ISIZE     |
 * +---+---+---+---+---+---+---+---+
 *
 */

struct ina_gzip_file_s {
    ina_file_ctx_t *file_ctx;
    ina_file_t *fgzip;
    ina_file_cursor_t *fcur;
    _ina_gzip_header_t hdr;
	ina_str_t name;
	ina_str_t comment;
    int is_text;
    int initial;
    unsigned char *buffer;
    unsigned char *bufpos;
    size_t buffer_size;
    size_t nbread;
    ina_compression_state_t *gzip_cstate;
};

#define _INA_GZIP_FLAG_TEXT      (1 << 0)
#define _INA_GZIP_FLAG_HCRD      (1 << 1)
#define _INA_GZIP_FLAG_EXTRA     (1 << 2)
#define _INA_GZIP_FLAG_NAME      (1 << 3)
#define _INA_GZIP_FLAG_COMMENT   (1 << 4)

#define _INA_GZIP_BUF_SIZE 1024
#define _INA_GZIP_BUF_REMAIN	(file->nbread - pos)
#define _INA_GZIP_DO_HEADER_CRC(crc_len) head_crc32 = ina_util_hash_crc32(head_crc32, file->buffer, crc_len)

static ina_rc_t __ina_gzip_process_header(ina_gzip_file_t *file)
{
	_ina_gzip_header_t *hdr;
	ina_str_t text_buf = NULL;
	size_t skip = 0;
	uint16_t header_crc = 0;
	int proc_extra = 0;
	int proc_name = 0;
	int proc_comment = 0;
	int proc_crc = 0;
	uint32_t head_crc32 = 0;
	uint16_t head_crc16 = 0;
    size_t pos;
	
	hdr = &file->hdr;
	
	if (hdr->id1 != _INA_GZIP_HEADER_ID1 || hdr->id2 != _INA_GZIP_HEADER_ID2) {
	    /* FIMXE: not a gzip file */
	    return INA_FAILURE;
	}
    INA_ASSERT_EQUAL(hdr->cm, 8);
	
	if (hdr->flg & _INA_GZIP_FLAG_TEXT) {
		file->is_text = 1;
	}
	
	/* read until data starts */
	for (;;) {
		pos = 0;
		/* read next buffer */
        if (!INA_SUCCEED(ina_file_cursor_binary_read_chunk(file->fcur, file->buffer_size, &file->nbread, &file->buffer))) {
            return INA_ERR_PUSH_LAST;
        }
		if (file->nbread == 0) {
			break;
		}
		if (skip > 0) {
			if (skip > _INA_GZIP_BUF_REMAIN) {
				skip = skip - _INA_GZIP_BUF_REMAIN;
				_INA_GZIP_DO_HEADER_CRC(file->nbread);
				continue;
			}
			pos += skip;
			skip = 0;
		}
		if ( (hdr->flg & _INA_GZIP_FLAG_EXTRA) && !proc_extra ) {
			uint16_t xlen = file->buffer[pos++] << 8 | file->buffer[pos++];
			/* skip over the extra flag - we do not need it */
			if (pos + xlen > _INA_GZIP_BUF_REMAIN) {
				skip = (pos + xlen) - _INA_GZIP_BUF_REMAIN;
				_INA_GZIP_DO_HEADER_CRC(file->nbread);
				continue;
			}
			pos += xlen;
			proc_extra = 1;
		}
		if ( (hdr->flg & _INA_GZIP_FLAG_NAME) && !proc_name ) {
			int end = 0;
			int text_start = pos;
			while (pos < _INA_GZIP_BUF_REMAIN) {
				char c = file->buffer[pos++];
				if (c == '\0') {
					end = pos;
					break;
				}
			}
			if (end) {
				if (text_buf == NULL) {
					file->name = ina_str_new_fromblk(&file->buffer[text_start], end - text_start);
				}
				else {
					file->name = ina_str_dup(text_buf);
					file->name = ina_str_ncatcstr(file->name, (const char*)file->buffer[text_start], end);
				}
				proc_name = 1;
			}
			else {
				end = pos;
				if (text_buf == NULL) {
					text_buf = ina_str_new_fromblk(&file->buffer[text_start], end - text_start);
				}
				else {
					text_buf = ina_str_ncatcstr(text_buf, (const char*)file->buffer[text_start], end);
				}
				continue;
				_INA_GZIP_DO_HEADER_CRC(file->nbread);
			}
		}
		if ( (hdr->flg & _INA_GZIP_FLAG_COMMENT) && !proc_comment ) {
			/* FIXME: once the text reading works create a macro and use the same logic as for name */
		}
		_INA_GZIP_DO_HEADER_CRC(_INA_GZIP_BUF_REMAIN);
		if ( (hdr->flg & _INA_GZIP_FLAG_HCRD) && !proc_crc ) {
			if (header_crc != 0) {
				header_crc |= file->buffer[pos++];
			}
			if (_INA_GZIP_BUF_REMAIN < 2) {
				header_crc = file->buffer[pos++] << 8;
				continue;
			}
			else {
				header_crc = file->buffer[pos++] << 8 | file->buffer[pos++];
				proc_crc = 1;
			}
		}
		break;
	}
	
	if (text_buf != NULL) {
		ina_str_free(text_buf);
	}
	
	if (hdr->flg & _INA_GZIP_FLAG_HCRD) {
		/* The CRC16 consists
		   of the two least significant bytes of the CRC32 for all
           bytes of the gzip header up to and not including the CRC16. */
	   
		/* Do header CRC16 check */
		head_crc16 = (head_crc32 & 0xFF) | ((head_crc32 >> 8) & 0xFF);
		if (header_crc != head_crc16) {
			/* FIXME: proper error handling */
			return INA_FAILURE;
		}
	}
	file->bufpos = file->buffer+pos;
	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_gzip_open(const char *gzip_file, size_t buffer_size, ina_gzip_file_t **gzf)
{
    *gzf = (ina_gzip_file_t*)ina_mem_alloc(sizeof(ina_gzip_file_t));
    (*gzf)->buffer_size = buffer_size;
  	(*gzf)->buffer = NULL;
  	(*gzf)->bufpos = NULL;
  	(*gzf)->nbread = 0;
    (*gzf)->comment = NULL;
    (*gzf)->is_text = 0;
    (*gzf)->name = NULL;

    if (!INA_SUCCEED(ina_file_init(&(*gzf)->file_ctx))) {
        return INA_ERR_PUSH_LAST;
    }
    if (!INA_SUCCEED(ina_file_new((*gzf)->file_ctx, gzip_file, 
        INA_FILE_ACCESS_MODE_READ, INA_FILE_CREATE_MODE_OPEN, 
        INA_FILE_SHARE_MODE_READ, INA_FILE_FLAG_SEQUENTIAL_ACCESS,
        &(*gzf)->fgzip))) {
            return INA_ERR_PUSH_LAST;
    }
    if (!INA_SUCCEED(ina_file_cursor_new((*gzf)->fgzip, INA_FILE_CURSOR_TYPE_FILEIO, 
        INA_FILE_CURSOR_MODE_READ_BINARY, (uint64_t)buffer_size, &(*gzf)->fcur, NULL))) {
            return INA_ERR_PUSH_LAST;
    }

    /* read gzip header */
    if (!INA_SUCCEED(ina_file_cursor_binary_read_chunk((*gzf)->fcur, sizeof(_ina_gzip_header_t), 
        &(*gzf)->nbread, &(*gzf)->buffer))) {
            return INA_ERR_PUSH_LAST;
    }

    ina_mem_cpy(&(*gzf)->hdr, (*gzf)->buffer, (*gzf)->nbread);

    /* validate and process header, read until data starts */
    if (!INA_SUCCEED(__ina_gzip_process_header(*gzf))) {
        return INA_ERR_PUSH_LAST;
    }
    if (!INA_SUCCEED(ina_compression_new(&(*gzf)->gzip_cstate, INA_COMPRESSION_TYPE_DEFLATE_RAW, INA_COMPRESSION_MODE_TRUSTED_FAST))) {
        return INA_ERR_PUSH_LAST;
    }

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_gzip_read_next_block(ina_gzip_file_t *gzf, size_t requested, size_t *read, unsigned char **chunk)
{
    size_t consumed = 0;
    int more = INA_NO;

    /*printf("%s buffer %p, bufpos %p len: %ld read: %d\n", "NEXT", gzf->buffer, gzf->bufpos, gzf->bufpos-gzf->buffer, gzf->nbread);
    printf("DIFF %d\n", gzf->nbread - (gzf->bufpos-gzf->buffer));*/
    /* Buffer fully consumed, read again from file */
    if (gzf->bufpos == gzf->buffer+gzf->nbread) {
    	if (!INA_SUCCEED(ina_file_cursor_binary_read_chunk(gzf->fcur, 
    							gzf->buffer_size, 
    							&gzf->nbread, 
    							&gzf->buffer))) {
    		return INA_ERR_PUSH_LAST;
    	}
    	gzf->bufpos = gzf->buffer;
    }

    /*more = ((gzf->nbread - (gzf->bufpos-gzf->buffer)) > requested);*/
    /*more = gzf->nbread == gzf->buffer_size;*/
    if (gzf->nbread == gzf->buffer_size) {
        more = (gzf->nbread - (gzf->bufpos-gzf->buffer) > 0);
    } else {
        more = (gzf->nbread - (gzf->bufpos-gzf->buffer) > 8);
    }

    /*printf("more: %d\n", more);*/
    if (!INA_SUCCEED(ina_compression_decompress_chunk(gzf->gzip_cstate, 
    								gzf->bufpos, 
                                   	(gzf->nbread - (gzf->bufpos-gzf->buffer)), 
                                    (unsigned char*)(*chunk), 
                                    requested, 
                                    read,
                                    &consumed,
                                    more))) {
    	/*printf("consumed: %ld\n", consumed);
    	printf("%s\n", "ERROR");*/
   		return INA_ERR_PUSH_LAST;
    }
    gzf->bufpos = gzf->bufpos+consumed;
    /*printf("requested: %ld\n", requested);
    printf("read: %ld\n", *read);
    printf("consumed: %ld\n", consumed);*/


    /* this is the last block - we need to chop-off the 4-byte crc and 4-byte input len */
    /*if (*read < requested) {
    	 gzf->bufpos = gzf->bufpos+8;
    	*read -= 8;
    }*/
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_gzip_close(ina_gzip_file_t **gzf)
{
    if ((*gzf)->gzip_cstate != NULL) {
        ina_compression_free(&(*gzf)->gzip_cstate);
    }
    ina_file_cursor_free(&(*gzf)->fcur);
    ina_file_free((*gzf)->file_ctx, &(*gzf)->fgzip);
    ina_file_destroy(&(*gzf)->file_ctx);
    ina_mem_free(*gzf);
    return INA_SUCCESS;
}
