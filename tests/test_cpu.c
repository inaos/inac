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


