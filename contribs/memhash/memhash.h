#pragma once

#if defined (__cplusplus)
extern "C" {
#endif

uint64_t memhash(void const* key, size_t n, uint64_t seed);

#if defined (__cplusplus)
}
#endif
