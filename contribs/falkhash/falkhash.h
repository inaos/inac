#pragma once

#if defined (__cplusplus)
extern "C" {
#endif

uint64_t falkhash64(const void * key, size_t len, uint64_t seed);

#if defined (__cplusplus)
}
#endif
