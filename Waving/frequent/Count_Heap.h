#pragma once

#include "Heap.h"

class Count_Heap : public Abstract {
public:
	Count_Heap(uint32_t _SIZE, uint32_t _length, uint32_t _hash_num)
	    : Abstract((char *)"Count_Heap"), length(_length), hash_num(_hash_num) {
		heap = new Heap(_SIZE);
		counter = new count_type *[hash_num];
		for (uint32_t i = 0; i < hash_num; ++i) {
			counter[i] = new count_type[length];
			memset(counter[i], 0, sizeof(count_type) * length);
		}
	}
	~Count_Heap() {
		for (uint32_t i = 0; i < hash_num; ++i)
			delete[] counter[i];
		delete[] counter;
		delete heap;
	}

	void Insert(const data_type item) {
		count_type *result = new count_type[hash_num];

		for (uint32_t i = 0; i < hash_num; ++i) {
			uint32_t position = hash(item, i) % length;
			uint32_t choice = hash(item, i + 101) & 1;
			counter[i][position] += COUNT[choice];
			result[i] = counter[i][position] * COUNT[choice];
		}

		heap->Insert(item, Get_Median(result, hash_num));
		delete[] result;
	}

	count_type Query(const data_type item) { return heap->Query(item); }

private:
	Heap *heap;
	count_type **counter;
	uint32_t length;
	uint32_t hash_num;
};