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


INA_TEST(mmap, test_init_destroy)
{
	ina_mmap_ctx_t *ctx = NULL;

	INA_TEST_ASSERT_SUCCEED(ina_mmap_init(&ctx));
	INA_TEST_ASSERT_NOT_NULL(ctx);
	INA_TEST_ASSERT_SUCCEED(ina_mmap_destroy(&ctx));
	INA_TEST_ASSERT_NULL(ctx);
}

INA_TEST(mmap, test_new_free)
{
	ina_mmap_ctx_t *ctx = NULL;
	ina_mmap_mapping_t *m = NULL;
	ina_file_t *file;
	ina_file_ctx_t *file_ctx;
	INA_TEST_ASSERT_SUCCEED(ina_file_init(&file_ctx, 0));
	INA_TEST_ASSERT_SUCCEED(ina_file_new(file_ctx, "tests.mem", INA_FILE_ACCESS_MODE_READWRITE, INA_FILE_CREATE_MODE_CREATE, INA_FILE_SHARE_MODE_EXCLUSIVE, 0, &file));
	INA_TEST_ASSERT_SUCCEED(ina_mmap_init(&ctx));
	INA_TEST_ASSERT_NOT_NULL(ctx);
	INA_TEST_ASSERT_SUCCEED(ina_mmap_new(ctx, file, INA_MMAP_MEM_PROT_READ, INA_MMAP_MEM_SHARE_SHARED, INA_MMAP_MAP_TYPE_MEMORY, 0, 1024*1024*1024, &m));
	INA_TEST_ASSERT_NOT_NULL(m);
	INA_TEST_ASSERT_SUCCEED(ina_mmap_free(ctx, &m));
	INA_TEST_ASSERT_NULL(m);
	INA_TEST_ASSERT_SUCCEED(ina_mmap_destroy(&ctx));
	INA_TEST_ASSERT_NULL(ctx);
	INA_TEST_ASSERT_SUCCEED(ina_file_free(file_ctx, &file));
	INA_TEST_ASSERT_SUCCEED(ina_file_destroy(&file_ctx));
}

INA_TEST_SKIP(mmap, synch)
{
	/*
	INA_API(ina_rc_t) ina_mmap_sync(ina_mmap_mapping_t *mapping);
	*/
}

INA_TEST_SKIP(mmap, memory_head_tail)
{
	/*
	INA_API(ina_rc_t) ina_mmap_memory_head(ina_mmap_mapping_t *mapping, void **memory);
	INA_API(ina_rc_t) ina_mmap_memory_tail(ina_mmap_mapping_t *mapping, void **memory);
	*/
}

INA_TEST_SKIP(mmap, advice) 
{
	/*
	INA_API(ina_rc_t) ina_mmap_advice(ina_mmap_mapping_t *mapping, size_t length, ina_mmap_mem_advice_t advice);
	*/
}

