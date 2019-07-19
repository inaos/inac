/*
 * Copyright INAOS GmbH, Thalwil, 2014-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <libinac/lib.h>

#ifndef INA_OS_OSX
INA_TEST(cpu, test_brand_string)
{
    ina_str_t brand = NULL;

    INA_TEST_ASSERT_SUCCEED(ina_cpu_get_brand_string(&brand));

    INA_TEST_MSG("CPU: %s\n", ina_str_cstr(brand));

    ina_str_free(brand);
}

INA_TEST(cpu, test_features)
{
    ina_cpu_feature_t cpufeatures;
    
    INA_TEST_ASSERT_SUCCEED(ina_cpu_get_features(&cpufeatures));

    /* every modern CPU should have these */
    INA_TEST_ASSERT_TRUE(cpufeatures & INA_CPU_FEATURE_TSC);
    INA_TEST_ASSERT_TRUE(cpufeatures & INA_CPU_FEATURE_SSE3);
}

INA_TEST(cpu, test_package_count)
{
    int package_count = 0;
	int supported = 0;

	INA_TEST_ASSERT_SUCCEED(ina_cpu_is_supported(&supported));

    ina_cpu_get_package_count(&package_count);

	if (supported) {
		INA_TEST_ASSERT_TRUE(package_count > 0);
		INA_TEST_MSG("CPU package-count: %d\n", package_count);
	}
	else {
		INA_TEST_MSG("%s\n", "CPU not supported or hypervisor bit set");
	}
}

INA_TEST(cpu, test_core_count)
{
    int core_count = 0;
	int supported = 0;

	INA_TEST_ASSERT_SUCCEED(ina_cpu_is_supported(&supported));

    ina_cpu_get_core_count(&core_count);

	if (supported) {
		INA_TEST_ASSERT_TRUE(core_count > 0);
		INA_TEST_MSG("CPU core-count: %d\n", core_count);
	}
	else {
		INA_TEST_MSG("%s/n","CPU not supported or hypervisor bit set");
	}
}

INA_TEST(cpu, test_thread_count)
{
    int thread_count = 0;
	int supported = 0;

	INA_TEST_ASSERT_SUCCEED(ina_cpu_is_supported(&supported));

    ina_cpu_get_thread_count(&thread_count);

	if (supported) {
		INA_TEST_ASSERT_TRUE(thread_count > 0);
		INA_TEST_MSG("CPU thread-count: %d\n", thread_count);
	}
	else {
		INA_TEST_MSG("%s\n", "CPU not supported or hypervisor bit set");
	}
}

INA_TEST(cpu, test_logical_count)
{
    int logical_count = 0;

    INA_TEST_ASSERT_SUCCEED(ina_cpu_get_total_logical_count(&logical_count));

    INA_TEST_ASSERT_TRUE(logical_count > 0);

    INA_TEST_MSG("CPU logical-count: %d\n", logical_count);
}
#endif

INA_TEST(cpu, test_supported)
{
    int supported = 0;

    INA_TEST_ASSERT_SUCCEED(ina_cpu_is_supported(&supported));

    if (supported) {
        INA_TEST_MSG("CPU and OS are supported by INAC\n", NULL);
    }
    else {
        INA_TEST_MSG("CPU and OS are NOT supported by INAC\n", NULL);
    }
}

INA_TEST(cpu, invalid_arguments)
{
    uint8_t family = 0;
    uint8_t  stepping = 0;
    uint8_t model = 0;


    INA_TEST_ASSERT_SUCCEED(ina_cpu_init());

    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cpu_get_package_count(NULL));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cpu_get_core_count(NULL));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cpu_get_thread_count(NULL));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cpu_get_total_logical_count(NULL));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cpu_get_features(NULL));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cpu_get_features(NULL));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cpu_get_brand_string(NULL));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cpu_is_supported(NULL));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cpu_get_signature(NULL, &model, &stepping));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cpu_get_signature(&family, NULL, &stepping));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cpu_get_signature(&family, &model, NULL));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cpu_get_ipc_sp(NULL));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cpu_get_ipc_dp(NULL));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cpu_get_l1_cache_size(NULL));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cpu_get_l2_cache_size(NULL));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cpu_get_l3_cache_size(NULL));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cpu_get_frequency_os(NULL));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cpu_get_gflops_dp(NULL));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cpu_get_gflops_sp(NULL));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cpu_process_query_core(NULL));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cpu_hyperthreading_enabled(NULL));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cpu_get_cache_line_size(NULL));

}


