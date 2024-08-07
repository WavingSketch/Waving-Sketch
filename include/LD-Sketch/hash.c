#include <stdio.h>
#include "hash.h"

uint64_t LD_AwareHash(unsigned char* data, uint64_t n,
	uint64_t hash, uint64_t scale, uint64_t hardener) {
	while (n) {
		hash *= scale;
		hash += *data++;
		n--;
	}
	return hash ^ hardener;
}

uint64_t LD_AwareHash_debug(unsigned char* data, uint64_t n,
	uint64_t hash, uint64_t scale, uint64_t hardener) {
	while (n) {
		fprintf(stderr, "    %lu %lu %lu %u\n", n, hash, scale, *data);
		hash *= scale;
		hash += *data++;
		n--;
		fprintf(stderr, "        internal %lu\n", hash);
	}
	return hash ^ hardener;
}

/*
 * mangle
 */
void LD_mangle(const unsigned char* key, unsigned char* ret_key,
	int nbytes) {
	unsigned long long new_key = 0;
	int i;
	for (i = 0; i < nbytes; ++i) {
		new_key |= key[nbytes - i - 1] << (i * 8);
	}
	new_key = (new_key * 2083697005) & (0xffffffff);
	for (i = 0; i < nbytes; ++i) {
		ret_key[i] = (new_key >> (i * 8)) & 0xff;
	}
}

uint64_t LD_seed = 0;
uint64_t LD_GenHashSeed(int index) {
	/*
	if (index == 0) {
		srand(0);
	}
	*/
	if (LD_seed == 0) {
		LD_seed = rand();
	}
	uint64_t x, y = LD_seed + index;
	LD_mangle((const unsigned char*)&y, (unsigned char*)&x, 8);
	return LD_AwareHash((uint8_t*)&y, 8, 388650253, 388650319, 1176845762);
}

int is_prime(int num) {
	int i;
	for (i = 2; i < num; i++) {
		if ((num % i) == 0) {
			break;
		}
	}
	if (i == num) {
		return 1;
	}
	return 0;
}