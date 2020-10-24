/*
 * Copyright 2014-2020 INAOS GmbH, Thalwil
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _LIBINAC_HASH_H_
#define _LIBINAC_HASH_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <libinac/lib.h>

#define INA_HASH_CSTR_TO_CRC32(s) ina_hash_crc32(0, s, strlen(s))
#define INA_HASH_CSTR_TO_SDBM(s)  ina_hash_sdbm(0, s, strlen(s))
#define INA_HASH_STR_TO_CRC32(s) ina_hash_crc32(0, ina_str_cstr(s), ina_str_len(s))
#define INA_HASH_STR_TO_SDBM(s)  ina_hash_sdbm(0, ina_str_cstr(s), ina_str_len(s))

typedef enum ina_hash_type_e {
    INA_HASH_DEFAULT = -1,
    INA_HASH32_CRC,
    INA_HASH32_LOOKUP3,
    INA_HASH32_DJB,
    INA_HASH32_JENKINS_OOAT,
    INA_HASH32_FNV,
    INA_HASH32_SUPERFAST,
    INA_HASH32_SDBM,
    INA_HASH32_FNV_YOSHIMITSU,
    INA_HASH32_MURMUR3,
    INA_HASH32_SPOOKY,
    INA_HASH32_XXHASH,
    INA_HASH32_CRC_HW,
    INA_HASH32_MEMMASH,
    INA_HASH32_FALKHASH,
    INA_HASH32_T1HA0,
    INA_HASH32_T1HA1,
#ifdef INA_CPU_X86_64
    INA_HASH64_LOCKUP3,
    INA_HASH64_FNV,
    INA_HASH64_SPOOKY,
    INA_HASH64_XXHASH,
    INA_HASH64_CRC_HW,
    INA_HASH64_MEMMASH,
    INA_HASH64_FALKHASH,
    INA_HASH64_T1HA0,
    INA_HASH64_T1HA1
#endif
} ina_hash_type_t;

INA_API(const char*) ina_hash_name(ina_hash_type_t hash_type);

INA_API(ina_rc_t) ina_hash_type(const char *hash_name, ina_hash_type_t *hash_type);

/*
 * Calculate 32bit CRC hash
 *
 * Parameters
 *  hash   starting hash
 *  data   data to hash
 *  size   size of buffer to hash
 *
 * Return Value
 *  Hash
 */
INA_API(uint32_t) ina_hash_crc32(uint32_t hashh, const void *data, size_t size);

/*
 * Calculate 32bit SDBM hash
 *
 * Parameters
 *  hash   starting hash
 *  data   data to hash
 *  size   size of buffer to hash
 *
 * Return Value
 *  Hash
 */
INA_API(uint32_t) ina_hash_sdbm(uint32_t hash, const void *data, size_t size);

/*
 * DESIGN:
 * -------
 * 
 * Hash functions are used for the following purpose: 
 * - Symbol tables      => 32bit                 keylen
 * - Hash tables        => 32bit                 keylen
 * - Databases          => 64bit or 128bit       keylen
 * - File-Systems       => 64bit or 128bit       keylen
 * - File checksums     => 64bit or 128bit       keylen
 * - Crypto             => starting with 256bit  keylen
 *
 * For now we only care about 32bit and 64bit keylen
 *
 * TODO:
 * -----
 *
 * 1. Provide a refactor lua-script which finds all usages of the old hashes located in util.h (sdbm)
 * 2. Clean-up the github issues related to hash-functions ;)
 *
 */

typedef uint32_t (*ina_hash_func_32_t)(uint32_t hash, const void *data, size_t size);
typedef uint64_t (*ina_hash_func_64_t)(uint64_t hash, const void *data, size_t size);

typedef uint32_t (*ina_hash_int8_func_32_t)(uint8_t key);
typedef uint32_t (*ina_hash_int16_func_32_t)(uint16_t key);
typedef uint32_t (*ina_hash_int32_func_32_t)(uint32_t key);
typedef uint64_t (*ina_hash_int64_func_64_t)(uint64_t key);

/*
 * Calculate 32bit integer hash from 8-bit key
 *
 * Source: Thomas Wang http://www.cris.com/~Ttwang/tech/inthash.htm
 *
 * Parameters
 *  key8   8-bit integer key
 *
 * Return
 *  Hash
 */
INA_INLINE uint32_t ina_hash_32_wang_int8(uint8_t key8)
{
    uint32_t key = key8;
    key += ~(key << 15);
    key ^= (key >> 10);
    key += (key << 3);
    key ^= (key >> 6);
    key += ~(key << 11);
    key ^= (key >> 16);
    return key;
}
/*
 * Calculate 32bit integer hash from 16-bit key
 *
 * Source: Thomas Wang http://www.cris.com/~Ttwang/tech/inthash.htm
 *
 * Parameters
 *  key16   16-bit integer key
 *
 * Return
 *  Hash
 */
INA_INLINE uint32_t ina_hash_32_wang_int16(uint16_t key16)
{
    uint32_t key = key16;
    key += ~(key << 15);
    key ^= (key >> 10);
    key += (key << 3);
    key ^= (key >> 6);
    key += ~(key << 11);
    key ^= (key >> 16);
    return key;
}
/*
 * Calculate 32bit integer hash from 32-bit key
 *
 * Source: Thomas Wang http://www.cris.com/~Ttwang/tech/inthash.htm
 *
 * Parameters
 *  key   32-bit integer key
 *
 * Return
 *  Hash
 */
INA_INLINE uint32_t ina_hash_32_wang_int32(uint32_t key)
{
    key += ~(key << 15);
    key ^= (key >> 10);
    key += (key << 3);
    key ^= (key >> 6);
    key += ~(key << 11);
    key ^= (key >> 16);
    return key;
}
/*
 * Calculate 32bit integer hash from 32-bit key
 *
 * Source: Bob Jenkins http://burtleburtle.net/bob/hash/integer.html
 *
 * Parameters
 *  key   32-bit integer key
 *
 * Return
 *  Hash
 */
INA_INLINE uint32_t ina_hash_32_jenkins_int32(uint32_t key)
{
    key -= key << 6;
	key ^= key >> 17;
	key -= key << 9;
	key ^= key << 4;
	key -= key << 3;
	key ^= key << 10;
	key ^= key >> 15;
	return key;
}
/*
 * Calculate 64bit integer hash from 64-bit key
 *
 * Source: Thomas Wang http://www.cris.com/~Ttwang/tech/inthash.htm
 *
 * Parameters
 *  key   64-bit integer key
 *
 * Return
 *  Hash
 */
INA_INLINE uint64_t ina_hash_64_wang_int64(uint64_t key)
{
    key = ~key + (key << 21);
	key = key ^ (key >> 24);
	key = key + (key << 3) + (key << 8);
	key = key ^ (key >> 14);
	key = key + (key << 2) + (key << 4);
	key = key ^ (key >> 28);
	key = key + (key << 31);
	return key;
}
/*
 * Calculate 32bit lookup3 (Bob Jenkins) hash
 *
 * Source: http://www.burtleburtle.net/bob/hash/doobs.html
 *
 * Parameters
 *  hash   starting hash
 *  data   data to hash
 *  size   size of buffer to hash
 *
 * Return
 *  Hash
 */
INA_API(uint32_t) ina_hash_32_lookup3(uint32_t hash, const void *data, size_t size);
/*
 * Calculate 64bit lookup3 (Bob Jenkins) hash
 *
 * Source: http://www.burtleburtle.net/bob/hash/doobs.html
 *
 * Parameters
 *  hash   starting hash
 *  data   data to hash
 *  size   size of buffer to hash
 *
 * Return
 *  Hash
 */
INA_API(uint64_t) ina_hash_64_lookup3(uint64_t hash, const void *data, size_t size);
/*
 * Calculate 32bit djb (Dan Bernstein) hash
 *
 * Source: http://www.cse.yorku.ca/~oz/hash.html
 *
 * Parameters
 *  hash   starting hash
 *  data   data to hash
 *  size   size of buffer to hash
 *
 * Return
 *  Hash
 */
INA_API(uint32_t) ina_hash_32_djb(uint32_t hash, const void *data, size_t size);
/*
 * Calculate 32bit Jenking OAAT (one-at-a-time) hash
 *
 * Source: http://www.cse.yorku.ca/~oz/hash.html
 *
 * Parameters
 *  hash   starting hash
 *  data   data to hash
 *  size   size of buffer to hash
 *
 * Return
 *  Hash
 */
INA_API(uint32_t) ina_hash_32_jenkins_ooat(uint32_t hash, const void *data, size_t size);
/*
 * Calculate 32bit FNV (Fowler-Noll-Vo) hash
 *
 * Source: Wikipedia
 *
 * Parameters
 *  hash   starting hash
 *  data   data to hash
 *  size   size of buffer to hash
 *
 * Return
 *  Hash
 */
INA_API(uint32_t) ina_hash_32_fnv(uint32_t hash, const void *data, size_t size);
/*
 * Calculate 64bit FNV (Fowler-Noll-Vo) hash
 *
 * Source: Wikipedia
 *
 * Parameters
 *  hash   starting hash
 *  data   data to hash
 *  size   size of buffer to hash
 *
 * Return
 *  Hash
 */
INA_API(uint64_t) ina_hash_64_fnv(uint64_t hash, const void *data, size_t size);
/*
 * Calculate 32bit SuperFastHash (Paul Hsieh) hash
 *
 * Source: http://www.azillionmonkeys.com/qed/hash.html
 *
 * Parameters
 *  hash   starting hash
 *  data   data to hash
 *  size   size of buffer to hash
 *
 * Return
 *  Hash
 */
INA_API(uint32_t) ina_hash_32_superfast(uint32_t hash, const void *data, size_t size);
/*
 * Calculate 32bit sdbm hash
 *
 * Source: Perl5
 *
 * Parameters
 *  hash   starting hash
 *  data   data to hash
 *  size   size of buffer to hash
 *
 * Return
 *  Hash
 */
INA_API(uint32_t) ina_hash_32_sdbm(uint32_t hash, const void *data, size_t size);
/*
 * Calculate 32bit FNV hash derivate from sanmayce
 *
 * Source: www.sanmayce.com/Fastest_Hash/index.html
 *
 * Limitations: This hash function could behave undefined in case 'size' is > 32bit
 *
 * Parameters
 *  hash   starting hash
 *  data   data to hash
 *  size   size of buffer to hash
 *
 * Return
 *  Hash
 */
INA_API(uint32_t) ina_hash_32_fnv_yoshimitsu(uint32_t hash, const void *data, size_t size);
/*
 * Calculate 32bit Murmur3 hash from Austin Appleby
 *
 * Source: C port by Shane Day
 *
 * Limitations: This hash function could behave undefined in case 'size' is > 32bit
 *
 * Parameters
 *  hash   starting hash
 *  data   data to hash
 *  size   size of buffer to hash
 *
 * Return
 *  Hash
 */
INA_API(uint32_t) ina_hash_32_murmur3(uint32_t hash, const void *data, size_t size);
/*
 * Calculate 32bit Spooky hash from Bob Jenkins (c version from Andi Kleen)
 *
 * Source: https://github.com/andikleen/spooky-c; BSD licensed
 *
 * Parameters
 *  hash   starting hash
 *  data   data to hash
 *  size   size of buffer to hash
 *
 * Return
 *  Hash
 */
INA_API(uint32_t) ina_hash_32_spooky(uint32_t hash, const void *data, size_t size);
/*
 * Calculate 64bit Spooky hash from Bob Jenkins (c version from Andi Kleen)
 *
 * Source: https://github.com/andikleen/spooky-c; BSD licensed
 *
 * Parameters
 *  hash   starting hash
 *  data   data to hash
 *  size   size of buffer to hash
 *
 * Return
 *  Hash
 */
INA_API(uint64_t) ina_hash_64_spooky(uint64_t hash, const void *data, size_t size);
/*
 * Calculate 32bit xxhash from Yann Collet
 *
 * Source: https://github.com/Cyan4973/xxHash; BSD licensed
 *
 * Parameters
 *  hash   starting hash
 *  data   data to hash
 *  size   size of buffer to hash
 *
 * Return
 *  Hash
 */
INA_API(uint32_t) ina_hash_32_xxhash(uint32_t hash, const void *data, size_t size);
/*
 * Calculate 64bit xxhash from Yann Collet
 *
 * Source: https://github.com/Cyan4973/xxHash; BSD licensed
 *
 * Parameters
 *  hash   starting hash
 *  data   data to hash
 *  size   size of buffer to hash
 *
 * Return
 *  Hash
 */
INA_API(uint64_t) ina_hash_64_xxhash(uint64_t hash, const void *data, size_t size);
/*
 * Calculate 32bit CRC checksum, using the Intel hardware instruction
 *
 * Source: n/a
 *
 * Parameters
 *  hash   starting hash
 *  data   data to hash
 *  size   size of buffer to hash
 *
 * Return
 *  Hash
 */
INA_API(uint32_t) ina_hash_32_crc_hw(uint32_t hash, const void *data, size_t size);
/*
 * Calculate 64bit CRC checksum, using the Intel hardware instruction
 *
 * Source: n/a
 *
 * Parameters
 *  hash   starting hash
 *  data   data to hash
 *  size   size of buffer to hash
 *
 * Return
 *  Hash
 */
INA_API(uint64_t) ina_hash_64_crc_hw(uint64_t hash, const void *data, size_t size);
/*
 * Calculate 32bit Jesse W. Towner's memhash, adopted from 64bit version
 *
 * Source: see 64bit version
 *
 * Parameters
 *  hash   starting hash
 *  data   data to hash
 *  size   size of buffer to hash
 *
 * Return
 *  Hash
 */
INA_API(uint32_t) ina_hash_32_memhash(uint32_t hash, const void *data, size_t size);
/*
 * Calculate 64bit Jesse W. Towner's memhash, inpired by http://locklessinc.com/articles/fast_hash/
 *
 * Source: https://github.com/upcaste/upcaste/blob/master/src/upcore/src/cstring/gas/x86_64/memhash.s
 *
 * Parameters
 *  hash   starting hash
 *  data   data to hash
 *  size   size of buffer to hash
 *
 * Return
 *  Hash
 */
INA_API(uint64_t) ina_hash_64_memhash(uint64_t hash, const void *data, size_t size);
/*
 * Calculate 32bit falkhash
 *
 * Source: see 64bit version
 *
 * Limitations: If CPU does not support AES instruction hash always returns 0
 *
 * Parameters
 *  hash   starting hash
 *  data   data to hash
 *  size   size of buffer to hash
 *
 * Return
 *  Hash
 */
INA_API(uint32_t) ina_hash_32_falkhash(uint32_t hash, const void *data, size_t size);
/*
 * Calculate 64bit falkhash
 *
 * Source: https://github.com/gamozolabs/falkhash
 *
 * Limitations: If CPU does not support AES instruction hash always returns 0
 *
 * Parameters
 *  hash   starting hash
 *  data   data to hash
 *  size   size of buffer to hash
 *
 * Return
 *  Hash
 */
INA_API(uint64_t) ina_hash_64_falkhash(uint64_t hash, const void *data, size_t size);
/*
 * Calculate 32bit t1ha - 0, fast version, not portable
 *
 * Source: https://github.com/leo-yuriev/t1ha
 *
 * Parameters
 *  hash   starting hash
 *  data   data to hash
 *  size   size of buffer to hash
 *
 * Return
 *  Hash
 */
INA_API(uint32_t) ina_hash_32_t1ha0(uint32_t hash, const void *data, size_t size);
/*
 * Calculate 32bit t1ha - 1, portable/stable
 *
 * Source: https://github.com/leo-yuriev/t1ha
 *
 * Parameters
 *  hash   starting hash
 *  data   data to hash
 *  size   size of buffer to hash
 *
 * Return
 *  Hash
 */
INA_API(uint32_t) ina_hash_32_t1ha1(uint32_t hash, const void *data, size_t size);
/*
 * Calculate 64bit t1ha - 0, fast version, not portable
 *
 * Source: https://github.com/leo-yuriev/t1ha
 *
 * Parameters
 *  hash   starting hash
 *  data   data to hash
 *  size   size of buffer to hash
 *
 * Return
 *  Hash
 */
INA_API(uint64_t) ina_hash_64_t1ha0(uint64_t hash, const void *data, size_t size);
/*
 * Calculate 64bit t1ha - 1, portable/stable
 *
 * Source: https://github.com/leo-yuriev/t1ha
 *
 * Parameters
 *  hash   starting hash
 *  data   data to hash
 *  size   size of buffer to hash
 *
 * Return
 *  Hash
 */
INA_API(uint64_t) ina_hash_64_t1ha1(uint64_t hash, const void *data, size_t size);


#ifdef __cplusplus
}
#endif 

#endif
