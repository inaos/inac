/*
 * Copyright INAOS GmbH, Thalwil, 2014-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <libinac/lib.h>


INA_TEST(mmap, test_init_destroy)
{
	ina_mmap_ctx_t *ctx = NULL;

	INA_TEST_ASSERT_SUCCEED(ina_mmap_ctx_new(&ctx));
	INA_TEST_ASSERT_NOT_NULL(ctx);
	INA_TEST_ASSERT_SUCCEED(ina_mmap_ctx_free(&ctx));
	INA_TEST_ASSERT_NULL(ctx);
}

INA_TEST_SKIP(mmap, test_new_free)
{
	ina_mmap_ctx_t *ctx = NULL;
	ina_mmap_mapping_t *m = NULL;
	ina_file_t *file;
	ina_file_ctx_t *file_ctx;
	INA_TEST_ASSERT_SUCCEED(ina_file_ctx_new(&file_ctx, 0));
	INA_TEST_ASSERT_SUCCEED(ina_file_new(file_ctx,
			"tests.mem",
			INA_FILE_ACCESS_MODE_READWRITE,
			INA_FILE_CREATE_MODE_CREATE,
			INA_FILE_SHARE_MODE_EXCLUSIVE,
			0,
			&file));
	INA_TEST_ASSERT_NOT_NULL(file);
	INA_TEST_ASSERT_SUCCEED(ina_mmap_ctx_new(&ctx));
	INA_TEST_ASSERT_NOT_NULL(ctx);
	INA_TEST_ASSERT_SUCCEED(ina_mmap_new(ctx, file,
			INA_MMAP_MEM_PROT_READ,
			INA_MMAP_MEM_SHARE_SHARED,
			INA_MMAP_MAP_TYPE_MEMORY,
			1024*1024*1024,
			0, &m));
	INA_TEST_ASSERT_NOT_NULL(m);
	INA_TEST_ASSERT_SUCCEED(ina_mmap_free(ctx, &m));
	INA_TEST_ASSERT_NULL(m);
	INA_TEST_ASSERT_SUCCEED(ina_mmap_ctx_free(&ctx));
	INA_TEST_ASSERT_NULL(ctx);
	ina_file_free(&file);
	ina_file_ctx_free(&file_ctx);
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

