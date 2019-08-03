/*
 * Copyright INAOS GmbH, Thalwil, 2016-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <libinac/lib.h>

typedef struct __ina_test_hash32_info_s {
    ina_str_t name;
    ina_hash_func_32_t hash;
    int test1_expected;
    int test2_expected;
} __ina_test_hash32_info_t;

typedef struct __ina_test_hash64_info_s {
    ina_str_t name;
    ina_hash_func_64_t hash;
} __ina_test_hash64_info_t;

typedef struct __ina_hash_rand_s {
    uint32_t x;
    uint32_t y;
    uint32_t z;
    uint32_t w;
} __ina_hash_rand_t;

typedef struct __ina_hash_test_call_wrapper_s {
    int which;
    union {
        ina_hash_func_32_t h32;
        ina_hash_func_64_t h64;
    };
} __ina_hash_test_call_wrapper_t;

static __ina_hash_rand_t __ina_hash_test_rand;
static __ina_test_hash32_info_t __hash32_all[15];
static __ina_test_hash64_info_t __hash64_all[9];

static void __ina_hash_test_mix()
{
    uint32_t t = __ina_hash_test_rand.x ^ (__ina_hash_test_rand.x << 11);
    __ina_hash_test_rand.x = __ina_hash_test_rand.y;
    __ina_hash_test_rand.y = __ina_hash_test_rand.z;
    __ina_hash_test_rand.z = __ina_hash_test_rand.w;
    __ina_hash_test_rand.w = __ina_hash_test_rand.w ^ (__ina_hash_test_rand.w >> 19) ^ t ^ (t >> 8);
}

static void __ina_hash_test_reseed(uint32_t seed)
{
    int i;

    __ina_hash_test_rand.x = 0x498b3bc5 ^ seed;
    __ina_hash_test_rand.y = 0;
    __ina_hash_test_rand.z = 0;
    __ina_hash_test_rand.w = 0;

    for (i = 0; i < 10; i++) {
        __ina_hash_test_mix();
    }
}

static uint32_t __ina_hash_test_rand_u32()
{
    __ina_hash_test_mix();
    return __ina_hash_test_rand.x;
}

static void __ina_hash_rand_p(void * blob, int bytes)
{
    uint32_t * blocks = (uint32_t*)blob;
    uint8_t * tail;
    int i;

    while (bytes >= 4) {
        blocks[0] = __ina_hash_test_rand_u32();
        blocks++;
        bytes -= 4;
    }

    tail = (uint8_t*)blocks;

    for (i = 0; i < bytes; i++) {
        tail[i] = (uint8_t)__ina_hash_test_rand_u32();
    }
}

static void __ina_hash_flipbit(void *block, int len, uint32_t bit)
{
    uint8_t * b = (uint8_t*)block;
    
    int byte = bit >> 3;
    bit = bit & 0x7;
 
    if (byte < len) {
        b[byte] ^= (1 << bit);
    }
}

static void __ina_hash_wrapper32(ina_hash_func_32_t hash, uint32_t seed, const void *key, size_t len, const void *out)
{
    *(uint32_t*)out = hash(seed, key, len);
}

static void __ina_hash_wrapper64(ina_hash_func_64_t hash, uint64_t seed, const void *key, size_t len, const void *out)
{
    *(uint64_t*)out = hash(seed, key, len);
}

static void __ina_hash_test_sanity_test(ina_str_t name, __ina_hash_test_call_wrapper_t *w, const int hashbits, int expected)
{
    int result = 1;
    int hashbytes = hashbits/8;
    int reps = 2;
    int rep;
    int keymax = 256;
    int pad = 16;
    int buflen = keymax + pad*3;
    int len;
    int offset;
    int bit;
 
    uint8_t *buffer1 = (uint8_t*)ina_mem_alloc(buflen*sizeof(uint8_t));
    uint8_t *buffer2 = (uint8_t*)ina_mem_alloc(buflen*sizeof(uint8_t));
    uint8_t *hash1 = (uint8_t*)ina_mem_alloc(hashbytes*sizeof(uint8_t));
    uint8_t *hash2 = (uint8_t*)ina_mem_alloc(hashbytes*sizeof(uint8_t));

    __ina_hash_test_reseed(883741);

    INA_TEST_MSG("Running sanity check 1 for: %s", ina_str_cstr(name));

    for (rep = 0; rep < reps; rep++) {
        for (len = 4; len <= keymax; len++) {
            for (offset = pad; offset < pad*2; offset++) {
                uint8_t *key1 = &buffer1[pad];
                uint8_t *key2 = &buffer2[pad+offset];
            
                __ina_hash_rand_p(buffer1,buflen);
                __ina_hash_rand_p(buffer2,buflen);
            
                memcpy(key2, key1, len);
            
                if (w->which == 0) {
                    __ina_hash_wrapper32(w->h32, 0, key1, len, hash1);
                }
                else {
                    __ina_hash_wrapper64(w->h64, 0, key1, len, hash1);
                }
            
                for(bit = 0; bit < (len * 8); bit++) {
                    /* Flip a bit, hash the key -> we should get a different result. */
                
                    __ina_hash_flipbit(key2, len, bit);
                    if (w->which == 0) {
                        __ina_hash_wrapper32(w->h32, 0, key2, len, hash2);
                    }
                    else {
                        __ina_hash_wrapper64(w->h64, 0, key2, len, hash2);
                    }
                
                    if (memcmp(hash1, hash2, hashbytes) == 0) {
                        result = 0;
                    }
                
                    /* Flip it back, hash again -> we should get the original result. */
                    __ina_hash_flipbit(key2,len,bit);
                    if (w->which == 0) {
                        __ina_hash_wrapper32(w->h32, 0, key2, len, hash2);
                    }
                    else {
                        __ina_hash_wrapper64(w->h64, 0, key2, len, hash2);
                    }
                
                    if (memcmp(hash1, hash2, hashbytes) != 0) {
                        result = 0;
                    }
                }
            }
        }
    }
    
    if (result) {
        INA_TEST_MSG("PASSED sanity check 1 for %s", ina_str_cstr(name));
    }
    else {
        INA_TEST_MSG("FAILED sanity check 1 for %s", ina_str_cstr(name));
    }

    if (expected) {
        INA_TEST_ASSERT_TRUE(result);
    }
    
    ina_mem_free(buffer1);
    ina_mem_free(buffer2);
    
    ina_mem_free(hash1);
    ina_mem_free(hash2);
}

static void __ina_hash_test_appended_zeroes_test(ina_str_t name, __ina_hash_test_call_wrapper_t *w, const int hashbits, int expected)
{
    int hashbytes = hashbits/8;
    int rep;
    int i;
    
    INA_TEST_MSG("Running sanity check 2 for %s", ina_str_cstr(name));
    
    __ina_hash_test_reseed(173994);
    
    for(rep = 0; rep < 100; rep++) {
        uint32_t h1[16];
        uint32_t h2[16];
        unsigned char key[256];

        memset(key, 0, sizeof(key));
        memset(h1, 0, hashbytes);
        memset(h2, 0, hashbytes);

        __ina_hash_rand_p(key, 32);

        for(i = 0; i < 32; i++) {
            if (w->which == 0) {
                __ina_hash_wrapper32(w->h32, 0, key, 32+i, h1);
            }
            else {
                __ina_hash_wrapper64(w->h64, 0, key, 32+i, h1);
            }
        
            if (memcmp(h1, h2, hashbytes) == 0) {
                INA_TEST_MSG("FAILED sanity check 2 for %s", ina_str_cstr(name));
                if (expected) {
                    INA_TEST_ASSERT_TRUE(0);
                }
                else {
                    return;
                }
            }
            memcpy(h2, h1, hashbytes);
        }
    }
    INA_TEST_MSG("PASSED sanity check 2 for %s", ina_str_cstr(name));
}

static void __ina_hash_test_init32(int has_aes_support)
{
    ina_mem_set(__hash32_all, 0, sizeof(__hash32_all));

    __hash32_all[0].hash = ina_hash_32_lookup3;
    __hash32_all[0].name = ina_str_new_fromcstr("lookup3");
    __hash32_all[0].test1_expected = 1;
    __hash32_all[0].test2_expected = 1;

    __hash32_all[1].hash = ina_hash_32_djb;
    __hash32_all[1].name = ina_str_new_fromcstr("djb");
    __hash32_all[1].test1_expected = 1;
    __hash32_all[1].test2_expected = 1;
    
    __hash32_all[2].hash = ina_hash_32_jenkins_ooat;
    __hash32_all[2].name = ina_str_new_fromcstr("jenkins_ooat");
    __hash32_all[2].test1_expected = 1;
    __hash32_all[2].test2_expected = 1;

    __hash32_all[3].hash = ina_hash_32_fnv;
    __hash32_all[3].name = ina_str_new_fromcstr("fnv");
    __hash32_all[3].test1_expected = 1;
    __hash32_all[3].test2_expected = 1;

    __hash32_all[4].hash = ina_hash_32_superfast;
    __hash32_all[4].name = ina_str_new_fromcstr("superfast");
    __hash32_all[4].test1_expected = 1;
    __hash32_all[4].test2_expected = 0;

    __hash32_all[5].hash = ina_hash_32_sdbm;
    __hash32_all[5].name = ina_str_new_fromcstr("sdbm");
    __hash32_all[5].test1_expected = 1;
    __hash32_all[5].test2_expected = 1;

    __hash32_all[6].hash = ina_hash_32_fnv_yoshimitsu;
    __hash32_all[6].name = ina_str_new_fromcstr("fnv_yoshimitsu");
    __hash32_all[6].test1_expected = 1;
    __hash32_all[6].test2_expected = 1;
    
    __hash32_all[7].hash = ina_hash_32_murmur3;
    __hash32_all[7].name = ina_str_new_fromcstr("murmur3");
    __hash32_all[7].test1_expected = 1;
    __hash32_all[7].test2_expected = 1;

    __hash32_all[8].hash = ina_hash_32_spooky;
    __hash32_all[8].name = ina_str_new_fromcstr("spooky");
    __hash32_all[8].test1_expected = 1;
    __hash32_all[8].test2_expected = 1;

    __hash32_all[9].hash = ina_hash_32_xxhash;
    __hash32_all[9].name = ina_str_new_fromcstr("xxhash");
    __hash32_all[9].test1_expected = 1;
    __hash32_all[9].test2_expected = 1;
    
    __hash32_all[10].hash = ina_hash_32_crc_hw;
    __hash32_all[10].name = ina_str_new_fromcstr("crc_hw");
    __hash32_all[10].test1_expected = 1;
    __hash32_all[10].test2_expected = 1;

    __hash32_all[11].hash = ina_hash_32_memhash;
    __hash32_all[11].name = ina_str_new_fromcstr("memhash");
    __hash32_all[11].test1_expected = 1;
    __hash32_all[11].test2_expected = 1;

    if (has_aes_support) {
        __hash32_all[12].hash = ina_hash_32_falkhash;
        __hash32_all[12].name = ina_str_new_fromcstr("falkhash");
        __hash32_all[12].test1_expected = 1;
        __hash32_all[12].test2_expected = 1;
    }

	__hash32_all[13].hash = ina_hash_32_t1ha1;
	__hash32_all[13].name = ina_str_new_fromcstr("t1ha1");
	__hash32_all[13].test1_expected = 1;
	__hash32_all[13].test2_expected = 1;

	__hash32_all[14].hash = ina_hash_32_t1ha0;
	__hash32_all[14].name = ina_str_new_fromcstr("t1ha0");
	__hash32_all[14].test1_expected = 1;
	__hash32_all[14].test2_expected = 1;
}

INA_TEST_DATA(hash) {};
INA_TEST_SKIP(hash, all_32_bit) {
    int i;
    int aes_hw_support = 0;
    ina_cpu_feature_t cpu_features;
    __ina_hash_test_call_wrapper_t w;
    INA_UNUSED(data);

    INA_TEST_ASSERT_SUCCEED(ina_cpu_get_features(&cpu_features));
    if (cpu_features & INA_CPU_FEATURE_AES) {
        aes_hw_support = 1;
    }
    else {
        INA_TEST_MSG("CPU has no AES hardware support, feature: %d", aes_hw_support);
    }

    w.which = 0;
    __ina_hash_test_init32(aes_hw_support);

    for (i = 0; i < sizeof(__hash32_all)/sizeof(__ina_test_hash32_info_t); i++) {
        __ina_test_hash32_info_t *hi = &__hash32_all[i];
        if (hi->hash != NULL) {
            INA_TEST_MSG("Testing 32bit hash function: %s", ina_str_cstr(hi->name));
            w.h32 = hi->hash;
            __ina_hash_test_sanity_test(hi->name, &w, 32, hi->test1_expected);
            __ina_hash_test_appended_zeroes_test(hi->name, &w, 32, hi->test2_expected);
        }
    }
}

static void __ina_hash_test_init64(int has_aes_support)
{
    ina_mem_set(__hash64_all, 0, sizeof(__hash64_all));

    __hash64_all[0].hash = ina_hash_64_lookup3;
    __hash64_all[0].name = ina_str_new_fromcstr("lookup3");

    __hash64_all[1].hash = ina_hash_64_fnv;
    __hash64_all[1].name = ina_str_new_fromcstr("fnv");

    __hash64_all[2].hash = ina_hash_64_spooky;
    __hash64_all[2].name = ina_str_new_fromcstr("spooky");

    __hash64_all[3].hash = ina_hash_64_xxhash;
    __hash64_all[3].name = ina_str_new_fromcstr("xxhash");

    __hash64_all[4].hash = ina_hash_64_crc_hw;
    __hash64_all[4].name = ina_str_new_fromcstr("crc_hw");

    __hash64_all[5].hash = ina_hash_64_memhash;
    __hash64_all[5].name = ina_str_new_fromcstr("memhash");

    if (has_aes_support) {
        __hash64_all[6].hash = ina_hash_64_falkhash;
        __hash64_all[6].name = ina_str_new_fromcstr("falkhash");
    }

	__hash64_all[7].hash = ina_hash_64_t1ha1;
	__hash64_all[7].name = ina_str_new_fromcstr("t1ha1");

	__hash64_all[8].hash = ina_hash_64_t1ha0;
	__hash64_all[8].name = ina_str_new_fromcstr("t1ha0");
}
                            
INA_TEST_SKIP(hash, all_64_bit)
{
    int i;
    int aes_hw_support = 0;
    ina_cpu_feature_t cpu_features;
    __ina_hash_test_call_wrapper_t w;
    INA_UNUSED(data);

    INA_TEST_ASSERT_SUCCEED(ina_cpu_get_features(&cpu_features));
    if (cpu_features & INA_CPU_FEATURE_AES) {
        aes_hw_support = 1;
    }
    else {
        INA_TEST_MSG("CPU has no AES hardware support, feature: %d", aes_hw_support);
    }

    w.which = 1;
    __ina_hash_test_init64(aes_hw_support);

    for (i = 0; i < sizeof(__hash64_all)/sizeof(__ina_test_hash64_info_t); i++) {
        __ina_test_hash64_info_t *hi = &__hash64_all[i];
        if (hi->hash != NULL) {
            INA_TEST_MSG("Testing 64bit hash function: %s", ina_str_cstr(hi->name));
            w.h64 = hi->hash;
            __ina_hash_test_sanity_test(hi->name, &w, 64, 1);
            __ina_hash_test_appended_zeroes_test(hi->name, &w, 64, 1);
        }
    }
}


INA_TEST_SKIP(hash, sdbm_macro)
{
    ina_str_t str = NULL;
    str = ina_str_new_fromcstr("test");
    INA_UNUSED(data);

    INA_TEST_ASSERT_NOT_NULL(str);
    INA_TEST_ASSERT_EQUAL_FLOATING(1195757874, INA_HASH_CSTR_TO_SDBM(ina_str_cstr(str)));
    INA_TEST_ASSERT_NOT_EQUAL_FLOATING(3632233, INA_HASH_CSTR_TO_SDBM(ina_str_cstr(str)));
}

INA_TEST_SKIP(hash, sdbm)
{
    ina_str_t str = NULL;
    str = ina_str_new_fromcstr("test");
    INA_UNUSED(data);

    INA_TEST_ASSERT_NOT_NULL(str);
    INA_TEST_ASSERT_EQUAL_FLOATING(1195757874, ina_hash_sdbm(0, str, ina_str_len(str)));
    INA_TEST_ASSERT_NOT_EQUAL_FLOATING(3632233, ina_hash_sdbm(0, str, ina_str_len(str)));
    INA_TEST_ASSERT_EQUAL_FLOATING(1732587620, ina_hash_sdbm(1195757874, str, ina_str_len(str)));
}

INA_TEST_SKIP(hash, crc32_macro)
{
    ina_str_t str = NULL;
    str = ina_str_new_fromcstr("test");
    INA_UNUSED(data);

    INA_TEST_ASSERT_NOT_NULL(str);
    INA_TEST_ASSERT_EQUAL_FLOATING(3632233996, INA_HASH_CSTR_TO_CRC32(ina_str_cstr(str)));
    INA_TEST_ASSERT_NOT_EQUAL_FLOATING(3632233, INA_HASH_CSTR_TO_CRC32(ina_str_cstr(str)));
}

INA_TEST_SKIP(hash, crc32)
{
    ina_str_t str = NULL;
    str = ina_str_new_fromcstr("test");
    INA_UNUSED(data);

    INA_TEST_ASSERT_NOT_NULL(str);
    INA_TEST_ASSERT_EQUAL_FLOATING(3632233996, ina_hash_crc32(0, str, ina_str_len(str)));
    INA_TEST_ASSERT_NOT_EQUAL_FLOATING(3632233, ina_hash_crc32(0, str, ina_str_len(str)));
    INA_TEST_ASSERT_EQUAL_FLOATING(3966352177, ina_hash_crc32(3632233996, str, ina_str_len(str)));
}