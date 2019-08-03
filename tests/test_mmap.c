/*
 * Copyright INAOS GmbH, Thalwil, 2014-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <libinac/lib.h>

INA_TEST_DATA(mmap){};
INA_TEST(mmap, test_init_destroy)
{
	ina_mmap_ctx_t *ctx = NULL;
    INA_UNUSED(data);

	INA_TEST_ASSERT_SUCCEED(ina_mmap_ctx_new(&ctx));
	INA_TEST_ASSERT_NOT_NULL(ctx);
	ina_mmap_ctx_free(&ctx);
	INA_TEST_ASSERT_NULL(ctx);
}

INA_TEST_SKIP(mmap, test_new_free)
{
	ina_mmap_ctx_t *ctx = NULL;
	ina_mmap_mapping_t *m = NULL;
	ina_file_t *file;
	ina_file_ctx_t *file_ctx;
    INA_UNUSED(data);

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
	ina_mmap_free(&m);
	INA_TEST_ASSERT_NULL(m);
	ina_mmap_ctx_free(&ctx);
	INA_TEST_ASSERT_NULL(ctx);
	ina_file_free(&file);
	ina_file_ctx_free(&file_ctx);
}

INA_TEST_SKIP(mmap, synch)
{
    INA_UNUSED(data);

    /*
    INA_API(ina_rc_t) ina_mmap_sync(ina_mmap_mapping_t *mapping);
    */
}

INA_TEST_SKIP(mmap, memory_head_tail)
{
    INA_UNUSED(data);

    /*
    INA_API(ina_rc_t) ina_mmap_memory_head(ina_mmap_mapping_t *mapping, void **memory);
    INA_API(ina_rc_t) ina_mmap_memory_tail(ina_mmap_mapping_t *mapping, void **memory);
    */
}

INA_TEST_SKIP(mmap, advice) 
{
    INA_UNUSED(data);

    /*
    INA_API(ina_rc_t) ina_mmap_advice(ina_mmap_mapping_t *mapping, size_t length, ina_mmap_mem_advice_t advice);
    */
}

INA_TEST(mmap, invalid_arguments)
{
    ina_mmap_ctx_t *ctx = NULL;
    ina_mmap_mapping_t *mapping = NULL;
    ina_file_t *fd = NULL;
    int fake = 0;
    void *mem = NULL;
    INA_UNUSED(data);


    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_mmap_ctx_new(NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_mmap_new(NULL, fd,
            INA_MMAP_MEM_PROT_EXEC,
            INA_MMAP_MEM_SHARE_PRIVATE,
            INA_MMAP_MAP_TYPE_FILE,
            0,
            1024,
            &mapping
    ));

    INA_TEST_ASSERT_SUCCEED(ina_mmap_ctx_new(&ctx));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_mmap_new(ctx, NULL,
                                                                  INA_MMAP_MEM_PROT_EXEC,
                                                                  INA_MMAP_MEM_SHARE_PRIVATE,
                                                                  INA_MMAP_MAP_TYPE_FILE,
                                                                  0,
                                                                  1024,
                                                                  &mapping
    ));

    INA_DISABLE_WARNING(int-to-pointer-cast, int-to-pointer-cast,4312)
    fd = (ina_file_t*)fake;
    INA_ENABLE_WARNING(int-to-pointer-cast, int-to-pointer-cast,4312)
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_mmap_new(ctx, fd,
                                                                  INA_MMAP_MEM_PROT_EXEC,
                                                                  INA_MMAP_MEM_SHARE_PRIVATE,
                                                                  INA_MMAP_MAP_TYPE_FILE,
                                                                  0,
                                                                  1024,
                                                                  NULL
    ));


    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_mmap_sync(NULL));

    INA_DISABLE_WARNING(int-to-pointer-cast, int-to-pointer-cast,4312)
    mapping = (ina_mmap_mapping_t*)fake;
    INA_ENABLE_WARNING(int-to-pointer-cast, int-to-pointer-cast,4312)

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_mmap_memory_head(NULL, &mem));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_mmap_memory_head(mapping, NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_mmap_memory_tail(NULL, &mem));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_mmap_memory_tail(mapping, NULL));

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_mmap_advice(NULL, 0, INA_MMAP_MEM_ADVICE_RANDOM));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_mmap_advice(mapping, 0, 100));

    ina_mmap_ctx_free(&ctx);
}

