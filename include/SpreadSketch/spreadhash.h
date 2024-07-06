#ifndef __SS_HASH_H__
#define __SS_HASH_H__

#include <stdint.h>
#include <stdlib.h>

#if defined (__cplusplus)
extern "C" {
#endif

	uint64_t SS_AwareHash(unsigned char* data, uint64_t n,
		uint64_t hash, uint64_t scale, uint64_t hardener);
	uint64_t SS_AwareHash_debug(unsigned char* data, uint64_t n,
		uint64_t hash, uint64_t scale, uint64_t hardener);
	uint64_t SS_GenHashSeed(int index);
	uint64_t MurmurHash64A(const void* key, int len, uint64_t seed);
	uint32_t MurmurHash2(const void* key, int len, uint32_t seed);

	void MurmurHash3_x86_128(const void* key, const int len,
		uint32_t seed, void* out);
	void MurmurHash3_x64_128(const void* key, const int len,
		uint32_t seed, void* out);
	void MurmurHash3_x86_32(const void* key, int len, uint32_t seed, void* out);

	void SS_mangle(const unsigned char* key, unsigned char* ret_key,
		int nbytes);

#if defined (__cplusplus)
}
#endif

#endif
