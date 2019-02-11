

---

```C
#ifndef _LIBINAC_HASH_H_
#define _LIBINAC_HASH_H_

```

Copyright INAOS GmbH, Thalwil, 2014-2018. All rights reserved

This software is the confidential and proprietary information of INAOS GmbH
("Confidential Information"). You shall not disclose such Confidential
Information and shall use it only in accordance with the terms of the
license agreement you entered into with INAOS GmbH.


---

```C
INA_API(uint32_t) ina_hash_crc32(uint32_t hashh, const void *data, size_t size);
```

Calculate 32bit CRC hash


**Parameters**
 - `hash`: starting hash
 - `data`: data to hash
 - `size`: size of buffer to hash


Return Value
Hash


---

```C
INA_API(uint32_t) ina_hash_sdbm(uint32_t hash, const void *data, size_t size);
```

Calculate 32bit SDBM hash


**Parameters**
 - `hash`: starting hash
 - `data`: data to hash
 - `size`: size of buffer to hash


Return Value
Hash


---

```C
typedef uint32_t (*ina_hash_func_32_t)(uint32_t hash, const void *data, size_t size);
```

DESIGN:
-------

Hash functions are used for the following purpose:
- Symbol tables      => 32bit                 keylen
- Hash tables        => 32bit                 keylen
- Databases          => 64bit or 128bit       keylen
- File-Systems       => 64bit or 128bit       keylen
- File checksums     => 64bit or 128bit       keylen
- Crypto             => starting with 256bit  keylen

For now we only care about 32bit and 64bit keylen

TODO:
-----

1. Provide a refactor lua-script which finds all usages of the old hashes located in util.h (sdbm)
2. Clean-up the github issues related to hash-functions ;)



---

```C
INA_INLINE uint32_t ina_hash_32_wang_int8(uint8_t key8)
{
    uint32_t key = key8;
```

Calculate 32bit integer hash from 8-bit key

Source: Thomas Wang http://www.cris.com/~Ttwang/tech/inthash.htm


**Parameters**
 - `key8`: 8-bit integer key



**Return**

Hash



---

```C
INA_INLINE uint32_t ina_hash_32_wang_int16(uint16_t key16)
{
    uint32_t key = key16;
```

Calculate 32bit integer hash from 16-bit key

Source: Thomas Wang http://www.cris.com/~Ttwang/tech/inthash.htm


**Parameters**
 - `key16`: 16-bit integer key



**Return**

Hash



---

```C
INA_INLINE uint32_t ina_hash_32_wang_int32(uint32_t key)
{
    key += ~(key << 15);
```

Calculate 32bit integer hash from 32-bit key

Source: Thomas Wang http://www.cris.com/~Ttwang/tech/inthash.htm


**Parameters**
 - `key`: 32-bit integer key



**Return**

Hash



---

```C
INA_INLINE uint32_t ina_hash_32_jenkins_int32(uint32_t key)
{
    key -= key << 6;
```

Calculate 32bit integer hash from 32-bit key

Source: Bob Jenkins http://burtleburtle.net/bob/hash/integer.html


**Parameters**
 - `key`: 32-bit integer key



**Return**

Hash



---

```C
INA_INLINE uint64_t ina_hash_64_wang_int64(uint64_t key)
{
    key = ~key + (key << 21);
```

Calculate 64bit integer hash from 64-bit key

Source: Thomas Wang http://www.cris.com/~Ttwang/tech/inthash.htm


**Parameters**
 - `key`: 64-bit integer key



**Return**

Hash



---

```C
INA_API(uint32_t) ina_hash_32_lookup3(uint32_t hash, const void *data, size_t size);
```

Calculate 32bit lookup3 (Bob Jenkins) hash

Source: http://www.burtleburtle.net/bob/hash/doobs.html


**Parameters**
 - `hash`: starting hash
 - `data`: data to hash
 - `size`: size of buffer to hash



**Return**

Hash



---

```C
INA_API(uint64_t) ina_hash_64_lookup3(uint64_t hash, const void *data, size_t size);
```

Calculate 64bit lookup3 (Bob Jenkins) hash

Source: http://www.burtleburtle.net/bob/hash/doobs.html


**Parameters**
 - `hash`: starting hash
 - `data`: data to hash
 - `size`: size of buffer to hash



**Return**

Hash



---

```C
INA_API(uint32_t) ina_hash_32_djb(uint32_t hash, const void *data, size_t size);
```

Calculate 32bit djb (Dan Bernstein) hash

Source: http://www.cse.yorku.ca/~oz/hash.html


**Parameters**
 - `hash`: starting hash
 - `data`: data to hash
 - `size`: size of buffer to hash



**Return**

Hash



---

```C
INA_API(uint32_t) ina_hash_32_jenkins_ooat(uint32_t hash, const void *data, size_t size);
```

Calculate 32bit Jenking OAAT (one-at-a-time) hash

Source: http://www.cse.yorku.ca/~oz/hash.html


**Parameters**
 - `hash`: starting hash
 - `data`: data to hash
 - `size`: size of buffer to hash



**Return**

Hash



---

```C
INA_API(uint32_t) ina_hash_32_fnv(uint32_t hash, const void *data, size_t size);
```

Calculate 32bit FNV (Fowler-Noll-Vo) hash

Source: Wikipedia


**Parameters**
 - `hash`: starting hash
 - `data`: data to hash
 - `size`: size of buffer to hash



**Return**

Hash



---

```C
INA_API(uint64_t) ina_hash_64_fnv(uint64_t hash, const void *data, size_t size);
```

Calculate 64bit FNV (Fowler-Noll-Vo) hash

Source: Wikipedia


**Parameters**
 - `hash`: starting hash
 - `data`: data to hash
 - `size`: size of buffer to hash



**Return**

Hash



---

```C
INA_API(uint32_t) ina_hash_32_superfast(uint32_t hash, const void *data, size_t size);
```

Calculate 32bit SuperFastHash (Paul Hsieh) hash

Source: http://www.azillionmonkeys.com/qed/hash.html


**Parameters**
 - `hash`: starting hash
 - `data`: data to hash
 - `size`: size of buffer to hash



**Return**

Hash



---

```C
INA_API(uint32_t) ina_hash_32_sdbm(uint32_t hash, const void *data, size_t size);
```

Calculate 32bit sdbm hash

Source: Perl5


**Parameters**
 - `hash`: starting hash
 - `data`: data to hash
 - `size`: size of buffer to hash



**Return**

Hash



---

```C
INA_API(uint32_t) ina_hash_32_fnv_yoshimitsu(uint32_t hash, const void *data, size_t size);
```

Calculate 32bit FNV hash derivate from sanmayce

Source: www.sanmayce.com/Fastest_Hash/index.html

Limitations: This hash function could behave undefined in case 'size' is > 32bit


**Parameters**
 - `hash`: starting hash
 - `data`: data to hash
 - `size`: size of buffer to hash



**Return**

Hash



---

```C
INA_API(uint32_t) ina_hash_32_murmur3(uint32_t hash, const void *data, size_t size);
```

Calculate 32bit Murmur3 hash from Austin Appleby

Source: C port by Shane Day

Limitations: This hash function could behave undefined in case 'size' is > 32bit


**Parameters**
 - `hash`: starting hash
 - `data`: data to hash
 - `size`: size of buffer to hash



**Return**

Hash



---

```C
INA_API(uint32_t) ina_hash_32_spooky(uint32_t hash, const void *data, size_t size);
```

Calculate 32bit Spooky hash from Bob Jenkins (c version from Andi Kleen)

Source: https://github.com/andikleen/spooky-c; BSD licensed


**Parameters**
 - `hash`: starting hash
 - `data`: data to hash
 - `size`: size of buffer to hash



**Return**

Hash



---

```C
INA_API(uint64_t) ina_hash_64_spooky(uint64_t hash, const void *data, size_t size);
```

Calculate 64bit Spooky hash from Bob Jenkins (c version from Andi Kleen)

Source: https://github.com/andikleen/spooky-c; BSD licensed


**Parameters**
 - `hash`: starting hash
 - `data`: data to hash
 - `size`: size of buffer to hash



**Return**

Hash



---

```C
INA_API(uint32_t) ina_hash_32_xxhash(uint32_t hash, const void *data, size_t size);
```

Calculate 32bit xxhash from Yann Collet

Source: https://github.com/Cyan4973/xxHash; BSD licensed


**Parameters**
 - `hash`: starting hash
 - `data`: data to hash
 - `size`: size of buffer to hash



**Return**

Hash



---

```C
INA_API(uint64_t) ina_hash_64_xxhash(uint64_t hash, const void *data, size_t size);
```

Calculate 64bit xxhash from Yann Collet

Source: https://github.com/Cyan4973/xxHash; BSD licensed


**Parameters**
 - `hash`: starting hash
 - `data`: data to hash
 - `size`: size of buffer to hash



**Return**

Hash



---

```C
INA_API(uint32_t) ina_hash_32_crc_hw(uint32_t hash, const void *data, size_t size);
```

Calculate 32bit CRC checksum, using the Intel hardware instruction

Source: n/a


**Parameters**
 - `hash`: starting hash
 - `data`: data to hash
 - `size`: size of buffer to hash



**Return**

Hash



---

```C
INA_API(uint64_t) ina_hash_64_crc_hw(uint64_t hash, const void *data, size_t size);
```

Calculate 64bit CRC checksum, using the Intel hardware instruction

Source: n/a


**Parameters**
 - `hash`: starting hash
 - `data`: data to hash
 - `size`: size of buffer to hash



**Return**

Hash



---

```C
INA_API(uint32_t) ina_hash_32_memhash(uint32_t hash, const void *data, size_t size);
```

Calculate 32bit Jesse W. Towner's memhash, adopted from 64bit version

Source: see 64bit version


**Parameters**
 - `hash`: starting hash
 - `data`: data to hash
 - `size`: size of buffer to hash



**Return**

Hash



---

```C
INA_API(uint64_t) ina_hash_64_memhash(uint64_t hash, const void *data, size_t size);
```

Calculate 64bit Jesse W. Towner's memhash, inpired by http://locklessinc.com/articles/fast_hash/

Source: https://github.com/upcaste/upcaste/blob/master/src/upcore/src/cstring/gas/x86_64/memhash.s


**Parameters**
 - `hash`: starting hash
 - `data`: data to hash
 - `size`: size of buffer to hash



**Return**

Hash



---

```C
INA_API(uint32_t) ina_hash_32_falkhash(uint32_t hash, const void *data, size_t size);
```

Calculate 32bit falkhash

Source: see 64bit version

Limitations: If CPU does not support AES instruction hash always returns 0


**Parameters**
 - `hash`: starting hash
 - `data`: data to hash
 - `size`: size of buffer to hash



**Return**

Hash



---

```C
INA_API(uint64_t) ina_hash_64_falkhash(uint64_t hash, const void *data, size_t size);
```

Calculate 64bit falkhash

Source: https://github.com/gamozolabs/falkhash

Limitations: If CPU does not support AES instruction hash always returns 0


**Parameters**
 - `hash`: starting hash
 - `data`: data to hash
 - `size`: size of buffer to hash



**Return**

Hash



---

```C
INA_API(uint32_t) ina_hash_32_t1ha0(uint32_t hash, const void *data, size_t size);
```

Calculate 32bit t1ha - 0, fast version, not portable

Source: https://github.com/leo-yuriev/t1ha


**Parameters**
 - `hash`: starting hash
 - `data`: data to hash
 - `size`: size of buffer to hash



**Return**

Hash



---

```C
INA_API(uint32_t) ina_hash_32_t1ha1(uint32_t hash, const void *data, size_t size);
```

Calculate 32bit t1ha - 1, portable/stable

Source: https://github.com/leo-yuriev/t1ha


**Parameters**
 - `hash`: starting hash
 - `data`: data to hash
 - `size`: size of buffer to hash



**Return**

Hash



---

```C
INA_API(uint64_t) ina_hash_64_t1ha0(uint64_t hash, const void *data, size_t size);
```

Calculate 64bit t1ha - 0, fast version, not portable

Source: https://github.com/leo-yuriev/t1ha


**Parameters**
 - `hash`: starting hash
 - `data`: data to hash
 - `size`: size of buffer to hash



**Return**

Hash



---

```C
INA_API(uint64_t) ina_hash_64_t1ha1(uint64_t hash, const void *data, size_t size);
```

Calculate 64bit t1ha - 1, portable/stable

Source: https://github.com/leo-yuriev/t1ha


**Parameters**
 - `hash`: starting hash
 - `data`: data to hash
 - `size`: size of buffer to hash



**Return**

Hash

