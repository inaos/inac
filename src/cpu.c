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
#include "config.h"

#include <contribs/cpu-topology/cputopology.h>

struct ina_cpu_ctx_s {
    int package_count;
    int core_count;
    int thread_count;
    int logical_count;
    int family;
    int model;
    int stepping;
    ina_cpu_feature_t features;
    ina_str_t brand;
    ina_str_t vendor;
};

static ina_cpu_ctx_t *__ina_cpu_ctx = NULL;

INA_API(ina_rc_t) ina_cpu_init()
{
    char vendor[16];
	int packages = 0;
	int cores = 0;
	int threads = 0;
    int logical = 0;
	CPUIDinfo info;
	int stepping, model, family, extmodel, extfam;
	char brandstr[49];
	char cpubrand[49];
	ina_cpu_feature_t cpufeatures = 0;

    __ina_cpu_ctx = (ina_cpu_ctx_t*)ina_mem_alloc(sizeof(struct ina_cpu_ctx_s));

	/* cpus physical layout */
	get_cpu_hw_info(&packages, &cores, &threads, &logical);
    __ina_cpu_ctx->package_count = packages;
    __ina_cpu_ctx->core_count = cores;
    __ina_cpu_ctx->thread_count = threads;
    __ina_cpu_ctx->logical_count = logical;
	
	get_cpuid_info(&info, 0, 0);
	memcpy(vendor + 0, &info.EBX, 4);
	memcpy(vendor + 4, &info.EDX, 4);
	memcpy(vendor + 8, &info.ECX, 4);
	vendor[12] = 0;
    __ina_cpu_ctx->vendor = ina_str_new_fromcstr(vendor);

	/* brand-string if supported */
	get_cpuid_info(&info, 0x80000000, 0);
	if (info.EAX >= 0x80000004) {
		int i,j;
		unsigned __int32 base = 0x80000002;
		for (i = 0; i < 3; i++) {
			get_cpuid_info(&info, base+i, 0);
			for (j = 0; j < 4; j++) {
				switch (j) {
					case 0:
						memcpy(brandstr + i * 16 + j * 4, &info.EAX, 4);
						break;
					case 1:
						memcpy(brandstr + i * 16 + j * 4, &info.EBX, 4);
						break;
					case 2:
						memcpy(brandstr + i * 16 + j * 4, &info.ECX, 4);
						break;
					case 3:
						memcpy(brandstr + i * 16 + j * 4, &info.EDX, 4);
						break;
				}
			}
		}
		brandstr[48] = '\0';
		i = 0;
		while (brandstr[i] == ' ') i++;
		strncpy(cpubrand, brandstr + i, sizeof(cpubrand));
		cpubrand[48] = '\0';
        __ina_cpu_ctx->brand = ina_str_new_fromcstr(cpubrand);
	}

	/* model info */
	get_cpuid_info(&info, 1, 0);
	family = (info.EAX >> 8) & 0xf;
	model = (info.EAX >> 4) & 0xf;
	stepping = info.EAX & 0xf;
	extmodel = (info.EAX >> 16) & 0xf;
	extfam = (info.EAX >> 20) & 0xff;	
	family = family + extfam;
	model = model + (extmodel << 4);
    __ina_cpu_ctx->family = family;
    __ina_cpu_ctx->model = model;
    __ina_cpu_ctx->stepping = stepping;
	
    /* load features */
	{
		int i;
		for (i = 0; i < 32; i++) {
			if (i == 10 || i == 20) {
				continue;
			}
			switch (i) {
				case 0:
					if (info.EDX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_FPU;
					}
					if (info.ECX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_SSE3;
					}
					break;
				case 1:
					if (info.EDX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_VME;
					}
					if (info.ECX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_PCLMULQDQ;
					}
					break;
				case 2:
					if (info.EDX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_DE;
					}
					if (info.ECX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_DTES64;
					}
					break;
				case 3:
					if (info.EDX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_PSE;
					}
					if (info.ECX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_MONITOR;
					}
					break;
				case 4:
					if (info.EDX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_TSC;
					}
					if (info.ECX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_DSCPL;
					}
					break;
				case 5:
					if (info.EDX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_MSR;
					}
					if (info.ECX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_VMX;
					}
					break;
				case 6:
					if (info.EDX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_PAE;
					}
					if (info.ECX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_SMX;
					}
					break;
				case 7:
					if (info.EDX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_MCE;
					}
					if (info.ECX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_EST;
					}
					break;
				case 8:
					if (info.EDX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_CX8;
					}
					if (info.ECX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_TM2;
					}
					break;
				case 9:
					if (info.EDX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_APIC;
					}
					if (info.ECX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_SSSE3;
					}
					break;
				case 10:
					if (info.ECX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_CNXTID;
					}
					break;
				case 11:
					if (info.EDX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_SEP;
					}
					break;
				case 12:
					if (info.EDX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_MTRR;
					}
					if (info.ECX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_FMA;
					}
					break;
				case 13:
					if (info.EDX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_PGE;
					}
					if (info.ECX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_CX16;
					}
					break;
				case 14:
					if (info.EDX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_MCA;
					}
					if (info.ECX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_XTPR;
					}
					break;
				case 15:
					if (info.EDX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_CMOV;
					}
					if (info.ECX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_PDCM;
					}
					break;
				case 16:
					if (info.EDX & (1 << i)) {
						cpufeatures |= INA_CPU_FETUARE_PAT;
					}
					break;
				case 17:
					if (info.EDX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_PSE36;
					}
					if (info.ECX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_PCID;
					}
					break;
				case 18:
					if (info.EDX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_PSN;
					}
					if (info.ECX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_DCA;
					}
					break;
				case 19:
					if (info.EDX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_CLFSH;
					}
					if (info.ECX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_SSE41;
					}
					break;
				case 20:
					if (info.ECX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_SSE42;
					}
					break;
				case 21:
					if (info.EDX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_DS;
					}
					if (info.ECX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_X2APIC;
					}
					break;
				case 22:
					if (info.EDX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_ACPI;
					}
					if (info.ECX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_MOVBE;
					}
					break;
				case 23:
					if (info.EDX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_MMX;
					}
					if (info.ECX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_POPCNT;
					}
					break;
				case 24:
					if (info.EDX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_FXSR;
					}
					if (info.ECX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_TSCDL;
					}
					break;
				case 25:
					if (info.EDX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_SSE;
					}
					if (info.ECX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_AES;
					}
					break;
				case 26:
					if (info.EDX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_SSE2;
					}
					if (info.ECX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_XSAVE;
					}
					break;
				case 27:
					if (info.EDX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_SS;
					}
					if (info.ECX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_OSXSAVE;
					}
					break;
				case 28:
					if (info.EDX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_HTT;
					}
					if (info.ECX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_AVX;
					}
					break;
				case 29:
					if (info.EDX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_TM;
					}
					if (info.ECX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_F16C;
					}
					break;
				case 30:
					if (info.EDX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_IA64;
					}
					if (info.ECX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_RDRND;
					}
					break;
				case 31:
					if (info.EDX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_PBE;
					}
					if (info.ECX & (1 << i)) {
						cpufeatures |= INA_CPU_FEATURE_HYPERVISOR;
					}
					break;
			}
		} 
	}
    __ina_cpu_ctx->features = cpufeatures;

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_cpu_destroy()
{
    if (__ina_cpu_ctx != NULL) {
        if (__ina_cpu_ctx->vendor != NULL) {
            ina_str_free(__ina_cpu_ctx->vendor);
        }
        if (__ina_cpu_ctx->brand != NULL) {
            ina_str_free(__ina_cpu_ctx->brand);
        }
        ina_mem_free(__ina_cpu_ctx);
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_cpu_get_package_count(int *package_count)
{
    INA_ASSERT_NOTNULL(__ina_cpu_ctx);
    *package_count = __ina_cpu_ctx->package_count;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_cpu_get_core_count(int *core_count)
{
    INA_ASSERT_NOTNULL(__ina_cpu_ctx);
    *core_count = __ina_cpu_ctx->core_count;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_cpu_get_thread_count(int *thread_count)
{
    INA_ASSERT_NOTNULL(__ina_cpu_ctx);
    *thread_count = __ina_cpu_ctx->thread_count;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_cpu_get_total_logical_count(int *logical_count)
{
    INA_ASSERT_NOTNULL(__ina_cpu_ctx);
    *logical_count = __ina_cpu_ctx->logical_count;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_cpu_get_features(ina_cpu_feature_t *features)
{
    INA_ASSERT_NOTNULL(__ina_cpu_ctx);
    *features = __ina_cpu_ctx->features;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_cpu_get_brand_string(ina_str_t *brand)
{
    INA_ASSERT_NOTNULL(__ina_cpu_ctx);
    *brand = ina_str_dup(__ina_cpu_ctx->brand);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_cpu_is_supported(int *supported)
{
    INA_ASSERT_NOTNULL(__ina_cpu_ctx);
    if (strcmp(INA_CPU_SUPPORTED_VENDOR, ina_str_cstr(__ina_cpu_ctx->vendor)) == 0) {
        *supported = 1;
    }
    else {
        *supported = 0;
    }
    return INA_SUCCESS;
}

