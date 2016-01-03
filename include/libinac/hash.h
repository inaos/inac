/*
 * Copyright (c) 2014-2016, INAOS GmbH
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
#ifndef _LIBINAC_HASH_H_
#define _LIBINAC_HASH_H_

#include <libinac/lib.h>

#ifdef __cplusplus
extern "C" {
#endif

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
 * 1. Create a test-suite; I believe we should extract the sanitiy-tests from smasher.
 *    Port it to C and add it as dependency to inac, then use them from the test-suite.
 *    To test all the hashes. All hashes should at least pass these two santiy tests.
 *
 * 2. Provide a refactor lua-script which finds all usages of the old hashes located in util.h (sdbm)
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
 * Return Value
 *  Hash
 */
INA_API(uint32_t) INA_INLINE ina_hash_32_wang_int8(uint8_t key8)
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
 * Return Value
 *  Hash
 */
INA_API(uint32_t) INA_INLINE ina_hash_32_wang_int16(uint16_t key16)
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
 * Return Value
 *  Hash
 */
INA_API(uint32_t) INA_INLINE ina_hash_32_wang_int32(uint32_t key)
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
 * Return Value
 *  Hash
 */
INA_API(uint32_t) INA_INLINE ina_hash_32_jenkins_int32(uint32_t key)
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
 * Return Value
 *  Hash
 */
INA_API(uint64_t) INA_INLINE ina_hash_64_wang_int64(uint64_t key)
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
 * Return Value
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
 * Return Value
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
 * Return Value
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
 * Return Value
 *  Hash
 */
INA_API(uint32_t) ina_hash_32_jenkins_ooat(uint32_t hash, const void *data, size_t size);
/*
 * Calculate 32bit FNV (Fowler–Noll–Vo) hash
 *
 * Source: Wikipedia
 *
 * Parameters
 *  hash   starting hash
 *  data   data to hash
 *  size   size of buffer to hash
 *
 * Return Value
 *  Hash
 */
INA_API(uint32_t) ina_hash_32_fnv(uint32_t hash, const void *data, size_t size);
/*
 * Calculate 64bit FNV (Fowler–Noll–Vo) hash
 *
 * Source: Wikipedia
 *
 * Parameters
 *  hash   starting hash
 *  data   data to hash
 *  size   size of buffer to hash
 *
 * Return Value
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
 * Return Value
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
 * Return Value
 *  Hash
 */
INA_API(uint32_t) ina_hash_32_sdbm(uint32_t hash, const void *data, size_t size);
/*
 * Calculate 32bit FNV hash derivate from sanmayce
 *
 * Source: www.sanmayce.com/Fastest_Hash/index.html
 *
 * Parameters
 *  hash   starting hash
 *  data   data to hash
 *  size   size of buffer to hash
 *
 * Return Value
 *  Hash
 */
INA_API(uint32_t) ina_hash_32_fnv_yoshimitsu(uint32_t hash, const void *data, size_t size);
/*
 * Calculate 32bit Murmur3 hash from Austin Appleby
 *
 * Source: C port by Shane Day
 *
 * Parameters
 *  hash   starting hash
 *  data   data to hash
 *  size   size of buffer to hash
 *
 * Return Value
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
 * Return Value
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
 * Return Value
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
 * Return Value
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
 * Return Value
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
 * Return Value
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
 * Return Value
 *  Hash
 */
INA_API(uint64_t) ina_hash_64_crc_hw(uint64_t hash, const void *data, size_t size);
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
 * Return Value
 *  Hash
 */
INA_API(uint64_t) ina_hash_64_memhash(uint64_t hash, const void *data, size_t size);
/*
 * Calculate 64bit falkhash
 *
 * Source: https://github.com/gamozolabs/falkhash
 *
 * Parameters
 *  hash   starting hash
 *  data   data to hash
 *  size   size of buffer to hash
 *
 * Return Value
 *  Hash
 */
INA_API(uint64_t) ina_hash_64_falkhash(uint64_t hash, const void *data, size_t size);

#ifdef __cplusplus
}
#endif 

#endif