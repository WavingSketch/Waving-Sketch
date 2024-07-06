#ifndef __LD_HASH_H__
#define __LD_HASH_H__

#include <stdint.h>
#include <stdlib.h>

/**
 * hash function
 * @param data the binary to be hashed
 * @param n the length of binary to be hashed
 */
static unsigned int LD_AwareHash1(const unsigned char* data, unsigned int n) {
	unsigned int hash = 388650253;
	unsigned int scale = 388650319;
	unsigned int hardener = 1176845762;
	while (n) {
		hash *= scale;
		hash += *data++;
		n--;
	}
	return hash ^ hardener;
}

uint64_t LD_AwareHash(unsigned char* data, uint64_t n,
	uint64_t hash, uint64_t scale, uint64_t hardener);
uint64_t LD_AwareHash_debug(unsigned char* data, uint64_t n,
	uint64_t hash, uint64_t scale, uint64_t hardener);

uint64_t LD_GenHashSeed(int index);

void LD_mangle(const unsigned char* key, unsigned char* ret_key,
	int nbytes);

#endif
