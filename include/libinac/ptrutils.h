/*
 * Copyright 2013-2020 INAOS GmbH, Thalwil
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
#ifndef _LIBINAC_PTRUTILS_H_
#define _LIBINAC_PTRUTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <libinac/lib.h>

// Raw pointer manipulators. Should use typeof() instead of T, but this operator is a part of C23 standard.
#define INA_PTR_OFFSET(T, p, offs) ((T*)(((char*) (p)) + (offs)))
#define INA_CONST_PTR_OFFSET(T, p, offs) ((const T*)(((const char*) (p)) + (offs)))

INA_ALWAYS_INLINE static size_t align_down(size_t x, size_t alignment) {
    assert(alignment > 0);
    return (x / alignment) * alignment;
}

INA_ALWAYS_INLINE static size_t align_up(size_t x, size_t alignment) {
    assert(alignment > 0);
    return align_down(x+alignment-1, alignment);
}

#ifdef __cplusplus
}
#endif
#endif
