/*
 * Copyright INAOS GmbH, Thalwil, 2014-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <libinac/lib.h>
#include "config.h"

#ifndef INA_OS_OSX
#include <contribs/cpu-topology/cputopology.h>
#endif

struct ina_cpu_ctx_s {
    int package_count;
    int core_count;
    int thread_count;
    int logical_count;
    int running_on_vm;
    uint8_t family;
    uint8_t model;
    uint8_t stepping;
    ina_cpu_feature_t features;
    ina_str_t brand;
    ina_str_t vendor;
	unsigned long l1_data_bytes;
	unsigned long l2_bytes;
	unsigned long l3_bytes;
    size_t cache_line;
    int ipc_sp;
    int ipc_dp;
    int frequency_os;
};

static ina_cpu_ctx_t *__ina_cpu_ctx = NULL;

#ifdef INA_OS_WIN32
static ina_rc_t __ina_cpu_clock_by_os(int *result_mhz)
{
	HKEY key;
	DWORD result;
	DWORD size = 4;
	
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0"), 0, KEY_READ, &key) != ERROR_SUCCESS)
        return INA_ERROR(INA_ES_OPERATION|INA_ERR_FAILED);
	
	if (RegQueryValueEx(key, TEXT("~MHz"), NULL, NULL, (LPBYTE) &result, (LPDWORD) &size) != ERROR_SUCCESS) {
		RegCloseKey(key);
        return INA_ERROR(INA_ES_OPERATION|INA_ERR_FAILED);;
	}
	RegCloseKey(key);
	
	*result_mhz = (int)result;
    return INA_SUCCESS;
}
static ina_rc_t __ina_cpu_cache_line_size(size_t *bytes)
{
    size_t lineSize = 0;
	DWORD bufferSize = 0;
	DWORD i = 0;
	SYSTEM_LOGICAL_PROCESSOR_INFORMATION * buffer = 0;

	GetLogicalProcessorInformation(0, &bufferSize);
	buffer = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION*)ina_mem_alloc(bufferSize);
	GetLogicalProcessorInformation(&buffer[0], &bufferSize);

	for (i = 0; i != bufferSize / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION); ++i) {
		if (buffer[i].Relationship == RelationCache && buffer[i].Cache.Level == 1) {
			lineSize = buffer[i].Cache.LineSize;
			break;
		}
	}

	ina_mem_free(buffer);
    *bytes = lineSize;

	return INA_SUCCESS;
}
#else
#ifdef INA_OS_OSX
#include <sys/types.h>
#include <sys/sysctl.h>
static ina_rc_t __ina_cpu_cache_line_size(size_t *bytes)
{
    size_t lineSize = 0;
	size_t sizeOfLineSize = sizeof(lineSize);
	sysctlbyname("hw.cachelinesize", &lineSize, &sizeOfLineSize, 0, 0);
    *bytes = lineSize;
	return INA_SUCCESS;
}
/* Assuming Mac OS X with hw.cpufrequency sysctl */
static ina_rc_t __ina_cpu_clock_by_os(int *result_mhz)
{
	long long result = -1;
	size_t size = sizeof(result);
	if (sysctlbyname("hw.cpufrequency", &result, &size, NULL, 0)) {
		return INA_OS_ERROR(INA_ES_OPERATION|INA_ERR_FAILED);
    }
	*result_mhz = (int) (result / (long long) 1000000);
    return INA_SUCCESS;
}
#else
static ina_rc_t __ina_cpu_cache_line_size(size_t *bytes)
{
    FILE *p = 0;
	p = fopen("/sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size", "r");
	unsigned int lineSize = 0;
	if (p) {
		fscanf(p, "%d", &lineSize);
		fclose(p);
	}
	*bytes = lineSize;
    return INA_SUCCESS;
}
/* Assuming Linux with /proc/cpuinfo */
static ina_rc_t __ina_cpu_clock_by_os(int *result_mhz)
{
	FILE *f;
	char line[1024], *s;
	int result;
	
	f = fopen("/proc/cpuinfo", "rt");
	if (!f) {
        return INA_OS_ERROR(INA_ES_OPERATION|INA_ERR_FAILED);
    }
	
	while (fgets(line, sizeof(line), f)) {
		if (!strncmp(line, "cpu MHz", 7)) {
			s = strchr(line, ':');
			if (s && 1 == sscanf(s, ":%d.", &result)) {
				fclose(f);
                *result_mhz = result;
				return INA_SUCCESS;
			}
		}
	}
	fclose(f);

	return INA_ERROR(INA_ES_PATTERN|INA_ERR_NOT_FOUND);
}
#endif
#endif

INA_API(ina_rc_t) ina_cpu_init()
{
	INA_INIT_GUARD();
#ifndef INA_OS_OSX
    char vendor[16];
    int packages = 0;
    int cores = 0;
    int threads = 0;
    int logical = 0;
    uint8_t stepping, model, family, extmodel, extfam;
    char brandstr[49];
    char cpubrand[49];
    char hyper_vendor_id[13];
    ina_cpu_feature_t cpufeatures = 0;
    CPUIDinfo info;
#endif

    __ina_cpu_ctx = (ina_cpu_ctx_t*)ina_mem_alloc(sizeof(struct ina_cpu_ctx_s));
    INA_RETURN_IF(__ina_cpu_ctx == NULL);

#ifdef INA_OS_OSX
    return INA_SUCCESS;
#else
    /* check hypervisor bit - test if hypervisor is present */
    get_cpuid_info(&info, 1, 0);
    if (info.ECX & (1U << 31)) {
        get_cpuid_info(&info, 0x40000000, 0);
        memcpy(hyper_vendor_id + 0, &info.EBX, 4);
        memcpy(hyper_vendor_id + 4, &info.ECX, 4);
        memcpy(hyper_vendor_id + 8, &info.EDX, 4);
        hyper_vendor_id[12] = '\0';
        if (!strcmp(hyper_vendor_id, "VMwareVMware")) {
            /* FIXME: save hypervisor info */
        }
		__ina_cpu_ctx->running_on_vm = 1; 
    }

	/* only if hypervisor bit is not set */
	if (!__ina_cpu_ctx->running_on_vm) {
		/* Retrieve CPU physical layout */
		if (get_cpu_hw_info(&packages, &cores, &threads, &logical) > 0) {
			/* FIXME: error handling get_last_error_cputopo() */
		}
		__ina_cpu_ctx->package_count = packages;
		__ina_cpu_ctx->core_count = cores;
		__ina_cpu_ctx->thread_count = threads;
		__ina_cpu_ctx->logical_count = logical;

		/* Retrieve CPU cache info */
		get_cache_info(&__ina_cpu_ctx->l1_data_bytes,
			&__ina_cpu_ctx->l2_bytes,
			&__ina_cpu_ctx->l3_bytes
		);
	}
	else {
		__ina_cpu_ctx->logical_count = GetMaxCPUSupportedByOS();
	}
	
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
		strncpy(cpubrand, brandstr + i, sizeof(cpubrand) - 1);
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

    __ina_cpu_cache_line_size(&__ina_cpu_ctx->cache_line);

    /* CPU instructions per cycle: 
     * ---------------------------
     *
     * Core         = 4,8
     * Nehalem      = 4,8
     * Sandy-Bridge = 8,16
     * Haswell      = 16,32
     * Skylake      = 16,32
     *
     */
    switch (__ina_cpu_ctx->model) {
        case 0x0F: /* Merom, Core */
        case 0x17: /* Penryn, Core */
        case 0x2E: /* Nehalem */
        case 0x1A: /* Nehalem */
        case 0x1E: /* Nehalem */
            __ina_cpu_ctx->ipc_dp = 4;
            __ina_cpu_ctx->ipc_sp = 8;
            break;
        case 0x2F: /* Westmere, Sandy-Bridge */
        case 0x2C: /* Westmere, Sandy-Bridge */
        case 0x25: /* Westmere, Sandy-Bridge */
        case 0x2D: /* Sandy-Bridge */
        case 0x2A: /* Sandy-Bridge */
        case 0x3A: /* Ivy Bridge, Sandy-Bridge */
            __ina_cpu_ctx->ipc_dp = 8;
            __ina_cpu_ctx->ipc_sp = 16; 
            break;
        case 0x3C: /* Haswell */
        case 0x3D: /* Broadwell, Haswell */
        case 0x5E: /* Skylake */
            __ina_cpu_ctx->ipc_dp = 16;
            __ina_cpu_ctx->ipc_sp = 32;
            break;
    }

    /* CPU frequency */
    INA_MUST_SUCCEED(__ina_cpu_clock_by_os(&__ina_cpu_ctx->frequency_os));

    return INA_SUCCESS;
#endif
}

INA_API(void) ina_cpu_destroy()
{
	INA_DESTROY_GUARD();
	INA_FREE_CHECK(&__ina_cpu_ctx);
    INA_STR_FREE_SAFE(__ina_cpu_ctx->vendor);
	INA_STR_FREE_SAFE(__ina_cpu_ctx->brand);
	INA_MEM_FREE_SAFE(__ina_cpu_ctx);
}

INA_API(ina_rc_t) ina_cpu_get_package_count(int *package_count)
{
    INA_ASSERT_NOT_NULL(__ina_cpu_ctx);
    INA_VERIFY_NOT_NULL(package_count);
	if (__ina_cpu_ctx->running_on_vm) {
		*package_count = 0;
		return INA_ERROR(INA_ES_STATE|INA_ERR_ILLEGAL);
	}
    *package_count = __ina_cpu_ctx->package_count;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_cpu_get_core_count(int *core_count)
{
    INA_ASSERT_NOT_NULL(__ina_cpu_ctx);
    INA_VERIFY_NOT_NULL(core_count);
	if (__ina_cpu_ctx->running_on_vm) {
		*core_count = 0;
        return INA_ERROR(INA_ES_STATE|INA_ERR_ILLEGAL);
	}
    *core_count = __ina_cpu_ctx->core_count;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_cpu_get_thread_count(int *thread_count)
{
    INA_ASSERT_NOT_NULL(__ina_cpu_ctx);
    INA_VERIFY_NOT_NULL(thread_count);
	if (__ina_cpu_ctx->running_on_vm) {
		*thread_count = 0;
		return INA_ERROR(INA_ES_STATE|INA_ERR_ILLEGAL);
	}
    *thread_count = __ina_cpu_ctx->thread_count;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_cpu_get_total_logical_count(int *logical_count)
{
    INA_ASSERT_NOT_NULL(__ina_cpu_ctx);
    INA_VERIFY_NOT_NULL(logical_count);
    *logical_count = __ina_cpu_ctx->logical_count;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_cpu_get_features(ina_cpu_feature_t *features)
{
    INA_ASSERT_NOT_NULL(__ina_cpu_ctx);
    INA_VERIFY_NOT_NULL(features);
    *features = __ina_cpu_ctx->features;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_cpu_get_brand_string(ina_str_t *brand)
{
    INA_ASSERT_NOT_NULL(__ina_cpu_ctx);
    INA_VERIFY_NOT_NULL(brand);
    *brand = ina_str_dup(__ina_cpu_ctx->brand);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_cpu_is_supported(int *supported)
{
    INA_ASSERT_NOT_NULL(__ina_cpu_ctx);
    INA_VERIFY_NOT_NULL(supported);
#ifdef INA_OS_OSX
    *supported = 0;
#else
    if (__ina_cpu_ctx->running_on_vm) {
        *supported = 0;
        return INA_SUCCESS;
    }
    if (strcmp(INA_CPU_SUPPORTED_VENDOR, ina_str_cstr(__ina_cpu_ctx->vendor)) == 0) {
        *supported = 1;
    }
    else {
        *supported = 0;
    }
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_cpu_pin_to_core(int cpuid)
{
#ifndef INA_OS_OSX
#ifdef INA_OS_WIN32
    HANDLE pid = GetCurrentProcess();
#ifdef INA_CPU_X86_64
    DWORD_PTR processAffinityMask = 1ULL << cpuid;
#else
	DWORD_PTR processAffinityMask = 1UL << cpuid;
#endif
    /* Set Affinity */
    if (!SetProcessAffinityMask(pid, processAffinityMask)) {
        return INA_OS_ERROR(INA_ES_OPERATION|INA_ERR_FAILED);
    }
#else
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(cpuid, &mask);
    int ret = sched_setaffinity(0, sizeof(mask), &mask);
    if (ret != 0) {
        return INA_OS_ERROR(INA_ES_OPERATION|INA_ERR_FAILED);
    }
#endif
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_cpu_get_signature(uint8_t *family, uint8_t *model, uint8_t *stepping)
{
    INA_ASSERT_NOT_NULL(__ina_cpu_ctx);
    INA_VERIFY_NOT_NULL(family);
    INA_VERIFY_NOT_NULL(model);
    INA_VERIFY_NOT_NULL(stepping);
    *family = __ina_cpu_ctx->family;
    *model = __ina_cpu_ctx->model;
    *stepping = __ina_cpu_ctx->stepping;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_cpu_get_ipc_sp(int *ipc)
{
    INA_ASSERT_NOT_NULL(__ina_cpu_ctx);
    INA_VERIFY_NOT_NULL(ipc);
    *ipc = __ina_cpu_ctx->ipc_sp;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_cpu_get_ipc_dp(int *ipc)
{
    INA_ASSERT_NOT_NULL(__ina_cpu_ctx);
    INA_VERIFY_NOT_NULL(ipc);
    *ipc = __ina_cpu_ctx->ipc_dp;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_cpu_get_l1_cache_size(size_t *bytes)
{
    INA_ASSERT_NOT_NULL(__ina_cpu_ctx);
    INA_VERIFY_NOT_NULL(bytes);
    *bytes = __ina_cpu_ctx->l1_data_bytes;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_cpu_get_l2_cache_size(size_t *bytes)
{
    INA_ASSERT_NOT_NULL(__ina_cpu_ctx);
    INA_VERIFY_NOT_NULL(bytes);
    *bytes = __ina_cpu_ctx->l2_bytes;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_cpu_get_l3_cache_size(size_t *bytes)
{
    INA_ASSERT_NOT_NULL(__ina_cpu_ctx);
    INA_VERIFY_NOT_NULL(bytes);
    *bytes = __ina_cpu_ctx->l3_bytes;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_cpu_get_frequency_os(int *mHz)
{
    INA_ASSERT_NOT_NULL(__ina_cpu_ctx);
    INA_VERIFY_NOT_NULL(mHz);
    *mHz = __ina_cpu_ctx->frequency_os;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_cpu_get_gflops_dp(double *gflops)
{
    int mHz = 0;
    int cores = 0;
    int ipc = 0;
    INA_VERIFY_NOT_NULL(gflops);
    INA_MUST_SUCCEED(ina_cpu_get_frequency_os(&mHz));
    INA_MUST_SUCCEED(ina_cpu_get_core_count(&cores));
    INA_MUST_SUCCEED(ina_cpu_get_ipc_dp(&ipc));

    *gflops = (mHz/1024.0)*cores*ipc;

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_cpu_get_gflops_sp(double *gflops)
{
    int mHz = 0;
    int cores = 0;
    int ipc = 0;
    INA_VERIFY_NOT_NULL(gflops);
    INA_MUST_SUCCEED(ina_cpu_get_frequency_os(&mHz));
    INA_MUST_SUCCEED(ina_cpu_get_core_count(&cores));
    INA_MUST_SUCCEED(ina_cpu_get_ipc_dp(&ipc));

    *gflops = (mHz/1024.0)*cores*ipc;

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_cpu_process_promote()
{
#ifndef INA_OS_OSX
#ifdef INA_OS_WIN32
    HANDLE pid = GetCurrentProcess();

    /* Set Priority */
	if(!SetPriorityClass(pid, HIGH_PRIORITY_CLASS)) {
		return INA_OS_ERROR(INA_ES_OPERATION|INA_ERR_FAILED);
	}
	if(!SetThreadPriority(GetCurrentThread(), HIGH_PRIORITY_CLASS)) {
		return INA_OS_ERROR(INA_ES_OPERATION|INA_ERR_FAILED);
	}
#else
    pid_t pid = getpid();
    struct sched_param param;
    int max_prio = sched_get_priority_max(SCHED_FIFO);
    if (max_prio == -1) {
        return INA_OS_ERROR(INA_ES_OPERATION|INA_ERR_FAILED);
    }
    param.sched_priority = max_prio;
    int ret = sched_setscheduler(pid, SCHED_FIFO, &param);
    if (ret != 0) {
        return INA_OS_ERROR(INA_ES_OPERATION|INA_ERR_FAILED);
    }
    ret = mlockall(MCL_CURRENT | MCL_FUTURE);
    if (ret != 0) {
        return INA_OS_ERROR(INA_ES_OPERATION|INA_ERR_FAILED);
    }
#endif
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_cpu_process_query_core(int *core)
{
    INA_VERIFY_NOT_NULL(core);
#ifndef INA_OS_OSX
#ifdef INA_OS_WIN32
    *core = GetCurrentProcessorNumber();
#else
    *core = sched_getcpu();
#endif
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_cpu_hyperthreading_enabled(int *enabled)
{
    INA_ASSERT_NOT_NULL(__ina_cpu_ctx);
    INA_VERIFY_NOT_NULL(enabled);

    if (__ina_cpu_ctx->package_count*__ina_cpu_ctx->core_count 
        != __ina_cpu_ctx->logical_count) {
            *enabled = 1;
    }
    else {
        *enabled = 0;
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_cpu_get_cache_line_size(size_t *bytes)
{
    INA_ASSERT_NOT_NULL(__ina_cpu_ctx);
    INA_VERIFY_NOT_NULL(bytes);
    *bytes = __ina_cpu_ctx->cache_line;
    return INA_SUCCESS;
}
