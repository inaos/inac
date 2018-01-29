/*
 * Copyright (c) 2016, INAOS GmbH
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

#include <contribs/xxhash/xxhash.h>
#include <contribs/falkhash/falkhash.h>
#include <contribs/memhash/memhash.h>

/* intrinsics */
#ifdef INA_OS_WIN32
#include <nmmintrin.h>
#else
#include <x86intrin.h>
#endif

#ifdef INA_OS_WIN32
#define __INA_HASH_ROTL32(x,y)    _rotl(x,y)
#else
INA_INLINE uint32_t __ina_hash_rotl32(uint32_t x, int8_t r)
{
    return (x << r) | (x >> (32 - r));
}
#define __INA_HASH_ROTL32(x,y)    __ina_hash_rotl32(x,y)
#endif

#define __INA_HASH_BIG_CONSTANT(x) (x)

/* BEGIN LOOKUP3 support code, credits to tommyds */

#define __INA_HASH_LOOKUP3_ROT(x, k)      \
	(((x) << (k)) | ((x) >> (32 - (k))))

#define __INA_HASH_LOOKUP3_MIX(a, b, c) do {                \
	a -= c;  a ^= __INA_HASH_LOOKUP3_ROT(c, 4);  c += b;    \
	b -= a;  b ^= __INA_HASH_LOOKUP3_ROT(a, 6);  a += c;    \
	c -= b;  c ^= __INA_HASH_LOOKUP3_ROT(b, 8);  b += a;    \
	a -= c;  a ^= __INA_HASH_LOOKUP3_ROT(c, 16); c += b;    \
	b -= a;  b ^= __INA_HASH_LOOKUP3_ROT(a, 19); a += c;    \
	c -= b;  c ^= __INA_HASH_LOOKUP3_ROT(b, 4);  b += a;    \
} while (0)

#define __INA_HASH_LOOKUP3_FINAL(a, b, c) do {              \
	c ^= b; c -= __INA_HASH_LOOKUP3_ROT(b, 14);             \
	a ^= c; a -= __INA_HASH_LOOKUP3_ROT(c, 11);             \
	b ^= a; b -= __INA_HASH_LOOKUP3_ROT(a, 25);             \
	c ^= b; c -= __INA_HASH_LOOKUP3_ROT(b, 16);             \
	a ^= c; a -= __INA_HASH_LOOKUP3_ROT(c, 4);              \
	b ^= a; b -= __INA_HASH_LOOKUP3_ROT(a, 14);             \
	c ^= b; c -= __INA_HASH_LOOKUP3_ROT(b, 24);             \
} while (0)

INA_INLINE uint32_t __ina_hash_le_uint32_read(const void* ptr)
{
	/* allow unaligned read on Intel x86 and x86_64 platforms */
#if defined(__i386__) || defined(_M_IX86) || defined(_X86_) || defined(__x86_64__) || defined(_M_X64)
	/* defines from http://predef.sourceforge.net/ */
	return *(const uint32_t*)ptr;
#else
	const unsigned char* ptr8 = (const unsigned char*)ptr;
	return ptr8[0] + ((uint32_t)ptr8[1] << 8) + ((uint32_t)ptr8[2] << 16) + ((uint32_t)ptr8[3] << 24);
#endif
}

/* END LOOKUP3 support code */

INA_API(uint32_t) ina_hash_32_lookup3(uint32_t hash, const void *data, size_t size)
{
	const unsigned char* key = (const unsigned char*)(data);
	uint32_t a, b, c;

	a = b = c = 0xdeadbeef + ((uint32_t)size) + hash;

	while (size > 12) {
		a += __ina_hash_le_uint32_read(key + 0);
		b += __ina_hash_le_uint32_read(key + 4);
		c += __ina_hash_le_uint32_read(key + 8);

		__INA_HASH_LOOKUP3_MIX(a, b, c);

		size -= 12;
		key += 12;
	}

	switch (size) {
	case 0 :
		return c; /* used only when called with a zero length */
	case 12 :
		c += __ina_hash_le_uint32_read(key + 8);
		b += __ina_hash_le_uint32_read(key + 4);
		a += __ina_hash_le_uint32_read(key + 0);
		break;
	case 11 : c += ((uint32_t)key[10]) << 16;
	case 10 : c += ((uint32_t)key[9]) << 8;
	case 9 : c += key[8];
	case 8 :
		b += __ina_hash_le_uint32_read(key + 4);
		a += __ina_hash_le_uint32_read(key + 0);
		break;
	case 7 : b += ((uint32_t)key[6]) << 16;
	case 6 : b += ((uint32_t)key[5]) << 8;
	case 5 : b += key[4];
	case 4 :
		a += __ina_hash_le_uint32_read(key + 0);
		break;
	case 3 : a += ((uint32_t)key[2]) << 16;
	case 2 : a += ((uint32_t)key[1]) << 8;
	case 1 : a += key[0];
	}

	__INA_HASH_LOOKUP3_FINAL(a, b, c);

	return c;
}

INA_API(uint64_t) ina_hash_64_lookup3(uint64_t hash, const void *data, size_t size)
{
	const unsigned char* key = (const unsigned char*)data;
	uint32_t a, b, c;

	a = b = c = 0xdeadbeef + ((uint32_t)size) + (hash & 0xffffffff);
	c += hash >> 32;

	while (size > 12) {
		a += __ina_hash_le_uint32_read(key + 0);
		b += __ina_hash_le_uint32_read(key + 4);
		c += __ina_hash_le_uint32_read(key + 8);

		__INA_HASH_LOOKUP3_MIX(a, b, c);

		size -= 12;
		key += 12;
	}

	switch (size) {
	case 0 :
		return c + ((uint64_t)b << 32); /* used only when called with a zero length */
	case 12 :
		c += __ina_hash_le_uint32_read(key + 8);
		b += __ina_hash_le_uint32_read(key + 4);
		a += __ina_hash_le_uint32_read(key + 0);
		break;
	case 11 : c += ((uint32_t)key[10]) << 16;
	case 10 : c += ((uint32_t)key[9]) << 8;
	case 9 : c += key[8];
	case 8 :
		b += __ina_hash_le_uint32_read(key + 4);
		a += __ina_hash_le_uint32_read(key + 0);
		break;
	case 7 : b += ((uint32_t)key[6]) << 16;
	case 6 : b += ((uint32_t)key[5]) << 8;
	case 5 : b += key[4];
	case 4 :
		a += __ina_hash_le_uint32_read(key + 0);
		break;
	case 3 : a += ((uint32_t)key[2]) << 16;
	case 2 : a += ((uint32_t)key[1]) << 8;
	case 1 : a += key[0];
	}

	__INA_HASH_LOOKUP3_FINAL(a, b, c);

	return c + ((uint64_t)b << 32);
}

INA_API(uint32_t) ina_hash_32_djb(uint32_t hash, const void *data, size_t size)
{
    const uint8_t *d = (const uint8_t*)data;
    size_t i;

    for(i = 0; i < size; ++i)  {
        hash = 33 * hash + d[i];
    }

    return hash;
}

INA_API(uint32_t) ina_hash_32_jenkins_ooat(uint32_t hash, const void *data, size_t size)
{
    unsigned char  *str = (unsigned char *)data;
    const unsigned char *const end = (const unsigned char *)str + size;
    
    while (str < end) {
        hash += *str++;
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash = hash + (hash << 15);

    return hash;
}

INA_API(uint32_t) ina_hash_32_fnv(uint32_t hash, const void *data, size_t size)
{
    uint32_t h = hash;
    const uint8_t *d = (const uint8_t*)data;
    size_t i;

    h ^= __INA_HASH_BIG_CONSTANT(2166136261);
    
    for (i = 0; i < size; i++) {
        h ^= d[i];
        h *= 16777619;
    }
    
    return h;
}

INA_API(uint64_t) ina_hash_64_fnv(uint64_t hash, const void *data, size_t size)
{
    uint64_t h = (uint64_t)hash;
    const uint8_t *d = (const uint8_t*)data;
    size_t i;

    h ^= __INA_HASH_BIG_CONSTANT(0xcbf29ce484222325);
    
    for (i = 0; i < size; i++) {
        h ^= d[i];
        h *= 0x100000001b3ULL;
    }

    return h;
}

/* By Paul Hsieh (C) 2004, 2005.  Covered under the Paul Hsieh derivative 
   license. See: 
   http://www.azillionmonkeys.com/qed/weblicense.html for license details.

   http://www.azillionmonkeys.com/qed/hash.html */

#ifndef get16bits
INA_INLINE uint16_t get16bits(const void * p)
{
  return *(const uint16_t*)p;
}
#endif

INA_API(uint32_t) ina_hash_32_superfast(uint32_t seed, const void *key, size_t size)
{
    const signed char* data = (const signed char*)key;
    uint32_t hash = 0, tmp;
    int rem;
    
    if (size <= 0 || data == NULL) {
        return 0;
    }
    
    rem = size & 3;
    size >>= 2;
    
    /* Main loop */
    for (;size > 0; size--) {
        hash  += get16bits (data);
        tmp    = (get16bits (data+2) << 11) ^ hash;
        hash   = (hash << 16) ^ tmp;
        data  += 2*sizeof (uint16_t);
        hash  += hash >> 11;
    }
    
    /* Handle end cases */
    switch (rem) {
        case 3:	hash += get16bits (data);
            hash ^= hash << 16;
            hash ^= data[sizeof (uint16_t)] << 18;
            hash += hash >> 11;
            break;
        case 2:	hash += get16bits (data);
            hash ^= hash << 11;
            hash += hash >> 17;
            break;
        case 1: hash += *data;
            hash ^= hash << 10;
            hash += hash >> 1;
    }

    /* Force "avalanching" of final 127 bits */
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;

    return hash;
}

INA_API(uint32_t) ina_hash_32_sdbm(uint32_t hash, const void *data, size_t size)
{
    unsigned char *str = (unsigned char*)data;
    const unsigned char *const end = (const unsigned char*)str + size;
    while (str < end) {
        hash = (hash << 6) + (hash << 16) - hash + *str++;
    }
    return hash;
}

INA_API(uint32_t) ina_hash_32_fnv_yoshimitsu(uint32_t hash, const void *data, size_t size)
{
    const uint8_t  *p = (const uint8_t*)data;
    const uint32_t  PRIME = 709607;
    uint32_t hash32A = hash ^ 2166136261;
    uint32_t hash32B = 2166136261 + size;
    uint32_t hash32C = 2166136261;
    
    for (; size >= 3 * 2 * sizeof(uint32_t); size -= 3 * 2 * sizeof(uint32_t), p += 3 * 2 * sizeof(uint32_t)) {
        hash32A = (hash32A ^ (__INA_HASH_ROTL32(*(uint32_t *) (p + 0), 5)  ^ *(uint32_t *) (p + 4)))  * PRIME;
        hash32B = (hash32B ^ (__INA_HASH_ROTL32(*(uint32_t *) (p + 8), 5)  ^ *(uint32_t *) (p + 12))) * PRIME;
        hash32C = (hash32C ^ (__INA_HASH_ROTL32(*(uint32_t *) (p + 16), 5) ^ *(uint32_t *) (p + 20))) * PRIME;
    }
    if (p != data) {
        hash32A = (hash32A ^ __INA_HASH_ROTL32(hash32C, 5)) * PRIME;
    }
    /* Cases 0. .31 */
    if (size & 4 * sizeof(uint32_t)) {
        hash32A = (hash32A ^ (__INA_HASH_ROTL32(*(uint32_t *) (p + 0), 5) ^ *(uint32_t *) (p + 4))) * PRIME;
        hash32B = (hash32B ^ (__INA_HASH_ROTL32(*(uint32_t *) (p + 8), 5) ^ *(uint32_t *) (p + 12))) * PRIME;
        p += 8 * sizeof(uint16_t);
    }
    /* Cases 0. .15 */
    if (size & 2 * sizeof(uint32_t)) {
        hash32A = (hash32A ^ *(uint32_t *) (p + 0)) * PRIME;
        hash32B = (hash32B ^ *(uint32_t *) (p + 4)) * PRIME;
        p += 4 * sizeof(uint16_t);
    }
    /* Cases:0. .7 */
    if (size & sizeof(uint32_t)) {
        hash32A = (hash32A ^ *(uint16_t *) (p + 0)) * PRIME;
        hash32B = (hash32B ^ *(uint16_t *) (p + 2)) * PRIME;
        p += 2 * sizeof(uint16_t);
    }
    /* Cases:0. .3 */
    if (size & sizeof(uint16_t)) {
        hash32A = (hash32A ^ *(uint16_t *) p) * PRIME;
        p += sizeof(uint16_t);
    }
    if (size & 1) {
        hash32A = (hash32A ^ *p) * PRIME;
    }
    hash32A = (hash32A ^ __INA_HASH_ROTL32(hash32B, 5)) * PRIME;
    return hash32A ^ (hash32A >> 16);
}

/* BEGIN MURMUR3 support code */

#if INA_LITTLE_ENDIAN == 1
  #define __INA_HASH_MURMUR_UNALIGNED_SAFE
  /* CPU endian matches murmurhash algorithm, so read 32-bit word directly */
  #define __INA_HASH_MURMUR_READ_UINT32(ptr)   (*((uint32_t*)(ptr)))
#elif INA_BIG_ENDIAN == 1
  /* TODO: Add additional cases below where a compiler provided bswap32 is available */
  #if defined(__GNUC__) && (__GNUC__>4 || (__GNUC__==4 && __GNUC_MINOR__>=3))
    #define __INA_HASH_MURMUR_READ_UINT32(ptr)   (__builtin_bswap32(*((uint32_t*)(ptr))))
  #else
    /* Without a known fast bswap32 we're just as well off doing this */
    #define __INA_HASH_MURMUR_READ_UINT32(ptr)   (ptr[0]|ptr[1]<<8|ptr[2]<<16|ptr[3]<<24)
    #define __INA_HASH_MURMUR_UNALIGNED_SAFE
  #endif
#else
  /* Unknown endianess so last resort is to read individual bytes */
  #define __INA_HASH_MURMUR_READ_UINT32(ptr)   (ptr[0]|ptr[1]<<8|ptr[2]<<16|ptr[3]<<24)
  /* Since we're not doing word-reads we can skip the messing about with realignment */
  #define __INA_HASH_MURMUR_UNALIGNED_SAFE
#endif

/* Core murmurhash algorithm macros */

#define __INA_HASH_MURMUR_C1  (0xcc9e2d51)
#define __INA_HASH_MURMUR_C2  (0x1b873593)

/* This is the main processing body of the algorithm. It operates
 * on each full 32-bits of input. */
#define __INA_HASH_MURMUR_DOBLOCK(h1, k1) do {     \
    k1 *= __INA_HASH_MURMUR_C1;                    \
    k1 = __INA_HASH_ROTL32(k1,15);                 \
    k1 *= __INA_HASH_MURMUR_C2;                    \
                                                   \
    h1 ^= k1;                                      \
    h1 = __INA_HASH_ROTL32(h1,13);                 \
    h1 = h1*5+0xe6546b64;                          \
} while(0)

/* Append unaligned bytes to carry, forcing hash churn if we have 4 bytes */
/* cnt=bytes to process, h1=name of h1 var, c=carry, n=bytes in c, ptr/len=payload */
#define __INA_HASH_MURMUR_DOBYTES(cnt, h1, c, n, ptr, len) do {    \
    int _i = cnt;                                                  \
    while(_i--) {                                                  \
        c = c>>8 | *ptr++<<24;                                     \
        n++; len--;                                                \
        if(n==4) {                                                 \
            __INA_HASH_MURMUR_DOBLOCK(h1, c);                      \
            n = 0;                                                 \
        }                                                          \
    }                                                              \
} while(0)

/* Main hashing function. Initialise carry to 0 and h1 to 0 or an initial seed
 * if wanted. Both ph1 and pcarry are required arguments. */
static void __ina_hash_PMurHash32_Process(uint32_t *ph1, uint32_t *pcarry, const void *key, int len)
{
  uint32_t h1 = *ph1;
  uint32_t c = *pcarry;

  const uint8_t *ptr = (uint8_t*)key;
  const uint8_t *end;

  /* Extract carry count from low 2 bits of c value */
  int n = c & 3;

#if defined(__INA_HASH_MURMUR_UNALIGNED_SAFE)
  /* This CPU handles unaligned word access */

  /* Consume any carry bytes */
  int i = (4-n) & 3;
  if(i && i <= len) {
    __INA_HASH_MURMUR_DOBYTES(i, h1, c, n, ptr, len);
  }

  /* Process 32-bit chunks */
  end = ptr + len/4*4;
  for( ; ptr < end ; ptr+=4) {
    uint32_t k1 = __INA_HASH_MURMUR_READ_UINT32(ptr);
    __INA_HASH_MURMUR_DOBLOCK(h1, k1);
  }

#else /*UNALIGNED_SAFE*/
  /* This CPU does not handle unaligned word access */

  /* Consume enough so that the next data byte is word aligned */
  int i = -(long)ptr & 3;
  if(i && i <= len) {
      __INA_HASH_MURMUR_DOBYTES(i, h1, c, n, ptr, len);
  }

  /* We're now aligned. Process in aligned blocks. Specialise for each possible carry count */
  end = ptr + len/4*4;
  switch(n) { /* how many bytes in c */
  case 0: /* c=[----]  w=[3210]  b=[3210]=w            c'=[----] */
    for( ; ptr < end ; ptr+=4) {
      uint32_t k1 = __INA_HASH_MURMUR_READ_UINT32(ptr);
      __INA_HASH_MURMUR_DOBLOCK(h1, k1);
    }
    break;
  case 1: /* c=[0---]  w=[4321]  b=[3210]=c>>24|w<<8   c'=[4---] */
    for( ; ptr < end ; ptr+=4) {
      uint32_t k1 = c>>24;
      c = __INA_HASH_MURMUR_READ_UINT32(ptr);
      k1 |= c<<8;
      __INA_HASH_MURMUR_DOBLOCK(h1, k1);
    }
    break;
  case 2: /* c=[10--]  w=[5432]  b=[3210]=c>>16|w<<16  c'=[54--] */
    for( ; ptr < end ; ptr+=4) {
      uint32_t k1 = c>>16;
      c = __INA_HASH_MURMUR_READ_UINT32(ptr);
      k1 |= c<<16;
      __INA_HASH_MURMUR_DOBLOCK(h1, k1);
    }
    break;
  case 3: /* c=[210-]  w=[6543]  b=[3210]=c>>8|w<<24   c'=[654-] */
    for( ; ptr < end ; ptr+=4) {
      uint32_t k1 = c>>8;
      c = __INA_HASH_MURMUR_READ_UINT32(ptr);
      k1 |= c<<24;
      __INA_HASH_MURMUR_DOBLOCK(h1, k1);
    }
  }
#endif /*__INA_HASH_MURMUR_UNALIGNED_SAFE*/

  /* Advance over whole 32-bit chunks, possibly leaving 1..3 bytes */
  len -= len/4*4;

  /* Append any remaining bytes into carry */
  __INA_HASH_MURMUR_DOBYTES(len, h1, c, n, ptr, len);

  /* Copy out new running hash and carry */
  *ph1 = h1;
  *pcarry = (c & ~0xff) | n;
} 

/*---------------------------------------------------------------------------*/

/* Finalize a hash. To match the original Murmur3A the total_length must be provided */
uint32_t __ina_hash_PMurHash32_Result(uint32_t h, uint32_t carry, uint32_t total_length)
{
  uint32_t k1;
  int n = carry & 3;
  if(n) {
    k1 = carry >> (4-n)*8;
    k1 *= __INA_HASH_MURMUR_C1; k1 = __INA_HASH_ROTL32(k1,15); k1 *= __INA_HASH_MURMUR_C2; h ^= k1;
  }
  h ^= total_length;

  /* fmix */
  h ^= h >> 16;
  h *= 0x85ebca6b;
  h ^= h >> 13;
  h *= 0xc2b2ae35;
  h ^= h >> 16;

  return h;
}

/*---------------------------------------------------------------------------*/

/* Murmur3A compatable all-at-once */
uint32_t __ina_hash_PMurHash32(uint32_t seed, const void *key, int len)
{
  uint32_t h1=seed, carry=0;
  __ina_hash_PMurHash32_Process(&h1, &carry, key, len);
  return __ina_hash_PMurHash32_Result(h1, carry, len);
}

/* END MURMUR3 support code */

INA_API(uint32_t) ina_hash_32_murmur3(uint32_t hash, const void *data, size_t size)
{
    return __ina_hash_PMurHash32(hash, data, size);
}

/* BEGIN SPOOKY hash support code */

/* A C version of Bob Jenkins' spooky hash
 * Spooky Hash
 * A 128-bit noncryptographic hash, for checksums and table lookup
 * By Bob Jenkins. Bob's version was under Public Domain
 * The C version is under the BSD license
 *  Copyright (c) 2014, Spooky Contributors
 *  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * Oct 31 2010: published framework, disclaimer ShortHash isn't right
 * Nov 7 2010: disabled ShortHash
 * Oct 11 2011: C version ported by Andi Kleen (andikleen@github)
 * Oct 31 2011: replace End, ShortMix, ShortEnd, enable ShortHash again
 * Apr 10 2012: buffer overflow on platforms without unaligned reads
 * Apr 27 2012: C version updated by Ziga Zupanec ziga.zupanec@gmail.com (agiz@github)
 * Jan 03 2015: Adopted for INAC by Christian Steiner
 *
 *  Assumes little endian ness. Caller has to check this case.
 */

#if INA_LITTLE_ENDIAN == 1
    #define __INA_HASH_SPOOKY_ALLOW_UNALIGNED_READS 1
#else
    #define __INA_HASH_SPOOKY_ALLOW_UNALIGNED_READS 0
#endif

#define __INA_HASH_SPOOKY_SC_NUMVARS      12
#define __INA_HASH_SPOOKY_SC_BLOCKSIZE    (8 * __INA_HASH_SPOOKY_SC_NUMVARS)
#define __INA_HASH_SPOOKY_SC_BUFSIZE      (2 * __INA_HASH_SPOOKY_SC_BLOCKSIZE)

struct __ina_hash_spooky_state
{
	uint64_t m_data[2 * __INA_HASH_SPOOKY_SC_NUMVARS];
	uint64_t m_state[__INA_HASH_SPOOKY_SC_NUMVARS];
	size_t m_length;
	unsigned char m_remainder;
};

/* SC_CONST: a constant which:
 * is not zero
 * is odd
 * is a not-very-regular mix of 1's and 0's
 * does not need any other special mathematical properties
 */
#define __INA_HASH_SPOOKY_SC_CONST 0xdeadbeefdeadbeefLL

#if defined(INA_OS_WIN32) && defined(__rotl64)
#define __INA_HASH_SPOOKY_ROT64(x,y) __rotl64(x,y)
#else
INA_INLINE uint64_t __ina_hash_spooky_rot64(uint64_t x, int k)
{
	return (x << k) | (x >> (64 - k));
}
#define __INA_HASH_SPOOKY_ROT64(x,y) __ina_hash_spooky_rot64(x,y)
#endif

/*
 * This is used if the input is 96 bytes long or longer.
 *
 * The internal state is fully overwritten every 96 bytes.
 * Every input bit appears to cause at least 128 bits of entropy
 * before 96 other bytes are combined, when run forward or backward
 *   For every input bit,
 *   Two inputs differing in just that input bit
 *   Where "differ" means xor or subtraction
 *   And the base value is random
 *   When run forward or backwards one Mix
 * I tried 3 pairs of each; they all differed by at least 212 bits.
 */
INA_INLINE void __ina_hash_spooky_mix
(
	const uint64_t *data,
	uint64_t *s0, uint64_t *s1, uint64_t *s2,  uint64_t *s3,
	uint64_t *s4, uint64_t *s5, uint64_t *s6,  uint64_t *s7,
	uint64_t *s8, uint64_t *s9, uint64_t *s10, uint64_t *s11
)
{
	*s0 += data[0];		*s2 ^= *s10;	*s11 ^= *s0;	*s0 = __INA_HASH_SPOOKY_ROT64(*s0, 11);	*s11 += *s1;
	*s1 += data[1];		*s3 ^= *s11;	*s0 ^= *s1;		*s1 = __INA_HASH_SPOOKY_ROT64(*s1, 32);	*s0 += *s2;
	*s2 += data[2];		*s4 ^= *s0;		*s1 ^= *s2;		*s2 = __INA_HASH_SPOOKY_ROT64(*s2, 43);	*s1 += *s3;
	*s3 += data[3];		*s5 ^= *s1;		*s2 ^= *s3;		*s3 = __INA_HASH_SPOOKY_ROT64(*s3, 31);	*s2 += *s4;
	*s4 += data[4];		*s6 ^= *s2;		*s3 ^= *s4;		*s4 = __INA_HASH_SPOOKY_ROT64(*s4, 17);	*s3 += *s5;
	*s5 += data[5];		*s7 ^= *s3;		*s4 ^= *s5;		*s5 = __INA_HASH_SPOOKY_ROT64(*s5, 28);	*s4 += *s6;
	*s6 += data[6];		*s8 ^= *s4;		*s5 ^= *s6;		*s6 = __INA_HASH_SPOOKY_ROT64(*s6, 39);	*s5 += *s7;
	*s7 += data[7];		*s9 ^= *s5;		*s6 ^= *s7;		*s7 = __INA_HASH_SPOOKY_ROT64(*s7, 57);	*s6 += *s8;
	*s8 += data[8];		*s10 ^= *s6;	*s7 ^= *s8;		*s8 = __INA_HASH_SPOOKY_ROT64(*s8, 55);	*s7 += *s9;
	*s9 += data[9];		*s11 ^= *s7;	*s8 ^= *s9;		*s9 = __INA_HASH_SPOOKY_ROT64(*s9, 54);	*s8 += *s10;
	*s10 += data[10];	*s0 ^= *s8;		*s9 ^= *s10;	*s10 = __INA_HASH_SPOOKY_ROT64(*s10, 22);	*s9 += *s11;
	*s11 += data[11];	*s1 ^= *s9;		*s10 ^= *s11;	*s11 = __INA_HASH_SPOOKY_ROT64(*s11, 46);	*s10 += *s0;
}

/*
 * Mix all 12 inputs together so that h0, h1 are a hash of them all.
 *
 * For two inputs differing in just the input bits
 * Where "differ" means xor or subtraction
 * And the base value is random, or a counting value starting at that bit
 * The final result will have each bit of h0, h1 flip
 * For every input bit,
 * with probability 50 +- .3%
 * For every pair of input bits,
 * with probability 50 +- 3%
 *
 * This does not rely on the last Mix() call having already mixed some.
 * Two iterations was almost good enough for a 64-bit result, but a
 * 128-bit result is reported, so End() does three iterations.
 */
INA_INLINE void __ina_hash_spooky_endPartial
(
	uint64_t *h0, uint64_t *h1, uint64_t *h2,  uint64_t *h3,
	uint64_t *h4, uint64_t *h5, uint64_t *h6,  uint64_t *h7,
	uint64_t *h8, uint64_t *h9, uint64_t *h10, uint64_t *h11
)
{
	*h11+= *h1;		*h2 ^= *h11;	*h1 = __INA_HASH_SPOOKY_ROT64(*h1, 44);
	*h0 += *h2;		*h3 ^= *h0;		*h2 = __INA_HASH_SPOOKY_ROT64(*h2, 15);
	*h1 += *h3;		*h4 ^= *h1;		*h3 = __INA_HASH_SPOOKY_ROT64(*h3, 34);
	*h2 += *h4;		*h5 ^= *h2;		*h4 = __INA_HASH_SPOOKY_ROT64(*h4, 21);
	*h3 += *h5;		*h6 ^= *h3;		*h5 = __INA_HASH_SPOOKY_ROT64(*h5, 38);
	*h4 += *h6;		*h7 ^= *h4;		*h6 = __INA_HASH_SPOOKY_ROT64(*h6, 33);
	*h5 += *h7;		*h8 ^= *h5;		*h7 = __INA_HASH_SPOOKY_ROT64(*h7, 10);
	*h6 += *h8;		*h9 ^= *h6;		*h8 = __INA_HASH_SPOOKY_ROT64(*h8, 13);
	*h7 += *h9;		*h10^= *h7;		*h9 = __INA_HASH_SPOOKY_ROT64(*h9, 38);
	*h8 += *h10;	*h11^= *h8;		*h10= __INA_HASH_SPOOKY_ROT64(*h10, 53);
	*h9 += *h11;	*h0 ^= *h9;		*h11= __INA_HASH_SPOOKY_ROT64(*h11, 42);
	*h10+= *h0;		*h1 ^= *h10;	*h0 = __INA_HASH_SPOOKY_ROT64(*h0, 54);
}

INA_INLINE void __ina_hash_spooky_end
(
	uint64_t *h0,	uint64_t *h1,	uint64_t *h2,	uint64_t *h3,
	uint64_t *h4,	uint64_t *h5,	uint64_t *h6,	uint64_t *h7,
	uint64_t *h8,	uint64_t *h9,	uint64_t *h10,	uint64_t *h11
)
{
	__ina_hash_spooky_endPartial(h0, h1, h2, h3, h4, h5, h6, h7, h8, h9, h10, h11);
	__ina_hash_spooky_endPartial(h0, h1, h2, h3, h4, h5, h6, h7, h8, h9, h10, h11);
	__ina_hash_spooky_endPartial(h0, h1, h2, h3, h4, h5, h6, h7, h8, h9, h10, h11);
}

/*
 * The goal is for each bit of the input to expand into 128 bits of
 *   apparent entropy before it is fully overwritten.
 * n trials both set and cleared at least m bits of h0 h1 h2 h3
 *   n: 2   m: 29
 *   n: 3   m: 46
 *   n: 4   m: 57
 *   n: 5   m: 107
 *   n: 6   m: 146
 *   n: 7   m: 152
 * when run forwards or backwards
 * for all 1-bit and 2-bit diffs
 * with diffs defined by either xor or subtraction
 * with a base of all zeros plus a counter, or plus another bit, or random
 */
INA_INLINE void __ina_hash_spooky_short_mix
(
	uint64_t *h0,
	uint64_t *h1,
	uint64_t *h2,
	uint64_t *h3
)
{
	*h2 = __INA_HASH_SPOOKY_ROT64(*h2, 50);	*h2 += *h3;  *h0 ^= *h2;
	*h3 = __INA_HASH_SPOOKY_ROT64(*h3, 52);	*h3 += *h0;  *h1 ^= *h3;
	*h0 = __INA_HASH_SPOOKY_ROT64(*h0, 30);	*h0 += *h1;  *h2 ^= *h0;
	*h1 = __INA_HASH_SPOOKY_ROT64(*h1, 41);	*h1 += *h2;  *h3 ^= *h1;
	*h2 = __INA_HASH_SPOOKY_ROT64(*h2, 54);	*h2 += *h3;  *h0 ^= *h2;
	*h3 = __INA_HASH_SPOOKY_ROT64(*h3, 48);	*h3 += *h0;  *h1 ^= *h3;
	*h0 = __INA_HASH_SPOOKY_ROT64(*h0, 38);	*h0 += *h1;  *h2 ^= *h0;
	*h1 = __INA_HASH_SPOOKY_ROT64(*h1, 37);	*h1 += *h2;  *h3 ^= *h1;
	*h2 = __INA_HASH_SPOOKY_ROT64(*h2, 62);	*h2 += *h3;  *h0 ^= *h2;
	*h3 = __INA_HASH_SPOOKY_ROT64(*h3, 34);	*h3 += *h0;  *h1 ^= *h3;
	*h0 = __INA_HASH_SPOOKY_ROT64(*h0, 5);	*h0 += *h1;  *h2 ^= *h0;
	*h1 = __INA_HASH_SPOOKY_ROT64(*h1, 36);	*h1 += *h2;  *h3 ^= *h1;
}

/*
 * Mix all 4 inputs together so that h0, h1 are a hash of them all.
 *
 * For two inputs differing in just the input bits
 * Where "differ" means xor or subtraction
 * And the base value is random, or a counting value starting at that bit
 * The final result will have each bit of h0, h1 flip
 * For every input bit,
 * with probability 50 +- .3% (it is probably better than that)
 * For every pair of input bits,
 * with probability 50 +- .75% (the worst case is approximately that)
 */
INA_INLINE void __ina_hash_spooky_short_end
(
	uint64_t *h0,
	uint64_t *h1,
	uint64_t *h2,
	uint64_t *h3
)
{
	*h3 ^= *h2;  *h2 = __INA_HASH_SPOOKY_ROT64(*h2, 15);  *h3 += *h2;
	*h0 ^= *h3;  *h3 = __INA_HASH_SPOOKY_ROT64(*h3, 52);  *h0 += *h3;
	*h1 ^= *h0;  *h0 = __INA_HASH_SPOOKY_ROT64(*h0, 26);  *h1 += *h0;
	*h2 ^= *h1;  *h1 = __INA_HASH_SPOOKY_ROT64(*h1, 51);  *h2 += *h1;
	*h3 ^= *h2;  *h2 = __INA_HASH_SPOOKY_ROT64(*h2, 28);  *h3 += *h2;
	*h0 ^= *h3;  *h3 = __INA_HASH_SPOOKY_ROT64(*h3, 9);   *h0 += *h3;
	*h1 ^= *h0;  *h0 = __INA_HASH_SPOOKY_ROT64(*h0, 47);  *h1 += *h0;
	*h2 ^= *h1;  *h1 = __INA_HASH_SPOOKY_ROT64(*h1, 54);  *h2 += *h1;
	*h3 ^= *h2;  *h2 = __INA_HASH_SPOOKY_ROT64(*h2, 32);  *h3 += *h2;
	*h0 ^= *h3;  *h3 = __INA_HASH_SPOOKY_ROT64(*h3, 25);  *h0 += *h3;
	*h1 ^= *h0;  *h0 = __INA_HASH_SPOOKY_ROT64(*h0, 63);  *h1 += *h0;
}

static void __ina_hash_spooky_shorthash
(
	const void *message,
	size_t length,
	uint64_t *hash1,
	uint64_t *hash2
)
{
	uint64_t buf[2 * __INA_HASH_SPOOKY_SC_NUMVARS];
	union
	{
		const uint8_t *p8;
		uint32_t *p32;
		uint64_t *p64;
		size_t i;
	} u;
	size_t remainder;
	uint64_t a, b, c, d;
	u.p8 = (const uint8_t *)message;

	if (!__INA_HASH_SPOOKY_ALLOW_UNALIGNED_READS && (u.i & 0x7))
	{
		memcpy(buf, message, length);
		u.p64 = buf;
	}

	remainder = length % 32;
	a = *hash1;
	b = *hash2;
	c = __INA_HASH_SPOOKY_SC_CONST;
	d = __INA_HASH_SPOOKY_SC_CONST;

	if (length > 15)
	{
		const uint64_t *endp = u.p64 + (length/32)*4;

		/* handle all complete sets of 32 bytes */
		for (; u.p64 < endp; u.p64 += 4)
		{
			c += u.p64[0];
			d += u.p64[1];
			__ina_hash_spooky_short_mix(&a, &b, &c, &d);
			a += u.p64[2];
			b += u.p64[3];
		}

		/* Handle the case of 16+ remaining bytes. */
		if (remainder >= 16)
		{
			c += u.p64[0];
			d += u.p64[1];
			__ina_hash_spooky_short_mix(&a, &b, &c, &d);
			u.p64 += 2;
			remainder -= 16;
		}
	}

	/* Handle the last 0..15 bytes, and its length */
	d = ((uint64_t)length) << 56;
	switch (remainder)
	{
		case 15:
			d += ((uint64_t)u.p8[14]) << 48;
		case 14:
			d += ((uint64_t)u.p8[13]) << 40;
		case 13:
			d += ((uint64_t)u.p8[12]) << 32;
		case 12:
			d += u.p32[2];
			c += u.p64[0];
			break;
		case 11:
			d += ((uint64_t)u.p8[10]) << 16;
		case 10:
			d += ((uint64_t)u.p8[9]) << 8;
		case 9:
			d += (uint64_t)u.p8[8];
		case 8:
			c += u.p64[0];
			break;
		case 7:
			c += ((uint64_t)u.p8[6]) << 48;
		case 6:
			c += ((uint64_t)u.p8[5]) << 40;
		case 5:
			c += ((uint64_t)u.p8[4]) << 32;
		case 4:
			c += u.p32[0];
			break;
		case 3:
			c += ((uint64_t)u.p8[2]) << 16;
		case 2:
			c += ((uint64_t)u.p8[1]) << 8;
		case 1:
			c += (uint64_t)u.p8[0];
			break;
		case 0:
			c += __INA_HASH_SPOOKY_SC_CONST;
			d += __INA_HASH_SPOOKY_SC_CONST;
	}
	__ina_hash_spooky_short_end(&a, &b, &c, &d);
	*hash1 = a;
	*hash2 = b;
}


static void __ina_hash_spooky_hash128
(
	const void *message,
	size_t length,
	uint64_t *hash1,
	uint64_t *hash2
)
{
	uint64_t h0, h1, h2, h3, h4, h5, h6, h7, h8, h9, h10, h11;
	uint64_t buf[__INA_HASH_SPOOKY_SC_NUMVARS];
	uint64_t *endp;
	union
	{
		const uint8_t *p8;
		uint64_t *p64;
		uintptr_t i;
	} u;
	size_t remainder;

	if (length < __INA_HASH_SPOOKY_SC_BUFSIZE)
	{
		__ina_hash_spooky_shorthash(message, length, hash1, hash2);
		return;
	}

	h0 = h3 = h6 = h9  = *hash1;
	h1 = h4 = h7 = h10 = *hash2;
	h2 = h5 = h8 = h11 = __INA_HASH_SPOOKY_SC_CONST;

	u.p8 = (const uint8_t *)message;
	endp = u.p64 + (length/__INA_HASH_SPOOKY_SC_BLOCKSIZE)*__INA_HASH_SPOOKY_SC_NUMVARS;

	/* handle all whole blocks of SC_BLOCKSIZE bytes */
	if (__INA_HASH_SPOOKY_ALLOW_UNALIGNED_READS || (u.i & 0x7) == 0)
	{
		while (u.p64 < endp)
		{
			__ina_hash_spooky_mix(u.p64, &h0, &h1, &h2, &h3, &h4, &h5, &h6, &h7, &h8, &h9, &h10, &h11);
			u.p64 += __INA_HASH_SPOOKY_SC_NUMVARS;
		}
	}
	else
	{
		while (u.p64 < endp)
		{
			memcpy(buf, u.p64, __INA_HASH_SPOOKY_SC_BLOCKSIZE);
			__ina_hash_spooky_mix(buf, &h0, &h1, &h2, &h3, &h4, &h5, &h6, &h7, &h8, &h9, &h10, &h11);
			u.p64 += __INA_HASH_SPOOKY_SC_NUMVARS;
		}
	}

	/* handle the last partial block of SC_BLOCKSIZE bytes */
	remainder = (length - ((const uint8_t *)endp-(const uint8_t *)message));
	memcpy(buf, endp, remainder);
	memset(((uint8_t *)buf)+remainder, 0, __INA_HASH_SPOOKY_SC_BLOCKSIZE-remainder);
	((uint8_t *)buf)[__INA_HASH_SPOOKY_SC_BLOCKSIZE-1] = (uint8_t)remainder;
	__ina_hash_spooky_mix(buf, &h0 , &h1, &h2, &h3, &h4, &h5, &h6, &h7, &h8, &h9, &h10, &h11);

	/* do some final mixing */
	__ina_hash_spooky_end(&h0, &h1, &h2, &h3, &h4, &h5, &h6, &h7, &h8, &h9, &h10, &h11);
	*hash1 = h0;
	*hash2 = h1;
}

static uint64_t __ina_hash_spooky_hash64
(
	const void *message,
	size_t length,
	uint64_t seed
)
{
	uint64_t hash1 = seed;
	__ina_hash_spooky_hash128(message, length, &hash1, &seed);
	return hash1;
}

static uint32_t __ina_hash_spooky_hash32
(
	const void *message,
	size_t length,
	uint32_t seed
)
{
	uint64_t hash1 = seed, hash2 = seed;
	__ina_hash_spooky_hash128(message, length, &hash1, &hash2);
	return (uint32_t)hash1;
}

INA_API(uint32_t) ina_hash_32_spooky(uint32_t hash, const void *data, size_t size)
{
    return __ina_hash_spooky_hash32(data, size, hash);
}

INA_API(uint64_t) ina_hash_64_spooky(uint64_t hash, const void *data, size_t size)
{
    return __ina_hash_spooky_hash64(data, size, hash);
}

INA_API(uint32_t) ina_hash_32_xxhash(uint32_t hash, const void *data, size_t size)
{
    return XXH32(data, size, hash);
}

INA_API(uint64_t) ina_hash_64_xxhash(uint64_t hash, const void *data, size_t size)
{
    return XXH64(data, size, hash);
}

/* Byte-boundary alignment issues */
#define __INA_HASH_CRC_ALIGN_SIZE      0x08UL
#define __INA_HASH_CRC_ALIGN_MASK      (__INA_HASH_CRC_ALIGN_SIZE - 1)
#define __INA_HASH_CRC_CALC_CRC(op, crc, type, buf, len) do {                          \
    for (; (len) >= sizeof (type); (len) -= sizeof(type), buf += sizeof (type)) {      \
      /*(crc) = op((crc), *(type *) (buf)); */                                             \
    }                                                                                  \
} while(0)

INA_API(uint32_t) ina_hash_32_crc_hw(uint32_t hash, const void *data, size_t size)
{
    uint32_t crc = hash;
    const char* buf = (const char*)data;

    /* XOR the initial CRC with INT_MAX */
    crc ^= 0xFFFFFFFF;

    /* Align the input to the word boundary */
    for (; (size > 0) && ((size_t)buf & __INA_HASH_CRC_ALIGN_MASK); size--, buf++) {
        /*crc = _mm_crc32_u8(crc, *buf);*/
    }

    /* Blast off the CRC32 calculation */
#ifdef INA_CPU_X86_64
    __INA_HASH_CRC_CALC_CRC(_mm_crc32_u64, crc, uint64_t, buf, size);
#endif
    __INA_HASH_CRC_CALC_CRC(_mm_crc32_u32, crc, uint32_t, buf, size);
    __INA_HASH_CRC_CALC_CRC(_mm_crc32_u16, crc, uint16_t, buf, size);
    __INA_HASH_CRC_CALC_CRC(_mm_crc32_u8,  crc, uint8_t, buf, size);

    // Post-process the crc
    return (crc ^ 0xFFFFFFFF);
}

INA_API(uint64_t) ina_hash_64_crc_hw(uint64_t hash, const void *data, size_t size)
{
    const char* buf = (const char*)data;
    uint64_t crc = (uint64_t)hash;

    /* Align the input to the word boundary */
    for (; (size > 0) && ((size_t)buf & __INA_HASH_CRC_ALIGN_MASK); size--, buf++) {
        /*crc = _mm_crc32_u8((unsigned int)crc, *buf);*/
    }

    /* Blast off the CRC32 calculation */
#ifdef INA_CPU_X86_64
    __INA_HASH_CRC_CALC_CRC(_mm_crc32_u64, crc, uint64_t, buf, size);
#endif
#ifdef INA_OS_WIN32
    __INA_HASH_CRC_CALC_CRC(_mm_crc32_u32, (unsigned int)crc, uint32_t, buf, size);
    __INA_HASH_CRC_CALC_CRC(_mm_crc32_u16, (unsigned int)crc, uint16_t, buf, size);
    __INA_HASH_CRC_CALC_CRC(_mm_crc32_u8, (unsigned int)crc, uint8_t, buf, size);
#else
    __INA_HASH_CRC_CALC_CRC(_mm_crc32_u32, crc, uint32_t, buf, size);
    __INA_HASH_CRC_CALC_CRC(_mm_crc32_u16, crc, uint16_t, buf, size);
    __INA_HASH_CRC_CALC_CRC(_mm_crc32_u8, crc, uint8_t, buf, size);
#endif

    /* Post-process the crc */
    return crc;
}

INA_API(uint32_t) ina_hash_32_memhash(uint32_t hash, const void *data, size_t size)
{
    /* Note: tested with smasher the hash quality as well as the speed does not seem to change due to the cast */
    uint64_t h = ina_hash_64_memhash(hash, data, size);
    return (uint32_t)h;
}

INA_API(uint64_t) ina_hash_64_memhash(uint64_t hash, const void *data, size_t size)
{
    return memhash(data, size, hash);
}

INA_API(uint32_t) ina_hash_32_falkhash(uint32_t hash, const void *data, size_t size)
{
    /* Note: tested with smasher the hash quality as well as the speed does not seem to change due to the cast */
    uint64_t h = ina_hash_64_falkhash(hash, data, size);
    return (uint32_t)h;
}

INA_API(uint64_t) ina_hash_64_falkhash(uint64_t hash, const void *data, size_t size)
{
    return falkhash64(data, size, hash);
}
