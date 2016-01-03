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
 
#ifndef _LIBINAC_HASHTABLE_H_
#define _LIBINAC_HASHTABLE_H_

#include <libinac/lib.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * DESIGN:
 * -------
 * 
 * Libraries to consider with regard to features or design:
 * - tommyds (https://github.com/amadvance/tommyds)
 * - khash (https://github.com/attractivechaos/klib/blob/master/khash.h)
 * - gcc hashtable libiberty (https://gcc.gnu.org/svn/gcc/trunk/libiberty/hashtab.c)
 * - judy ?
 * - google dense hash (https://github.com/sparsehash/sparsehash)
 * - libdynamic (https://github.com/fredrikwidlund/libdynamic)
 * - ulib (https://github.com/stefanocasazza/ULib)
 * - uthash
 *
 * From an API perspective we should only support to have, integer and string keys.
 *
 * Must have feature -  as seen in uthash (keystats):
 * 
 * fcn  ideal%     #items   #buckets  dup%  fl   add_usec  find_usec  del-all usec
 * ---  ------ ---------- ---------- -----  -- ---------- ----------  ------------
 * SFH   91.6%       1219        256    0%  ok         92        131            25
 * FNV   90.3%       1219        512    0%  ok        107         97            31
 * SAX   88.7%       1219        512    0%  ok        111        109            32
 * OAT   87.2%       1219        256    0%  ok         99        138            26
 * JEN   86.7%       1219        256    0%  ok         87        130            27
 * BER   86.2%       1219        256    0%  ok        121        129            27
 *
 * + This feature in uthash is compile-time, it would be great if we could make it a runtime
 *   feature with zero performance impact - e.g. by using ULLC.
 *   Either this means that we have to store the entire hash-map in shared-memory or 
 *   push off all the keys to an ULLC ring? Or we do something similar then uthash with their
 *   hashscan utility where the memory of the process is scanned by a different process, this 
 *   process looks for uthash structures and reads its content to analyze stats? can we do this 
 *   efficiently without impacting the running process?
 *
 * + Maybe we could also record the collisions or is that the dup%?
 *
 * Thoughs about refactoring:
 * - Currently in some places we do double hashing by first hashing the string with sdbm and then adding it to uthash 
 *   which involves a lookup3 hash
 *
 * TODO:
 * -----
 *
 * 1. Investigate open questions:
 *    - Should we use chaining or open addressing? or both by choice and use-case?
 *      Here a post which contains some input in that regard: http://preshing.com/20110603/hash-table-performance-tests/
 *    - What should be a macro and what can be typed c-code?
 *    - We should probably have some fixed size variants and dynamic ones.. if dynamic how to grow:
 *      Quadratic probing, linear probing etc.
 *
 * 2. How to benchmark
 *    - must be simple because the real benchmark is alwayls the application
 *    - benchmark can be kept outside if it grows too big.. ideally we want to compare our 
 *      implementation against the above contenders.
 *
 *
 */

#ifdef __cplusplus
}
#endif

#endif
