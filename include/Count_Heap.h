#pragma once

#include "Heap.h"
#include "abstract.h"
template<uint32_t DATA_LEN>
class Count_Heap : public Abstract<DATA_LEN> {
public:
	Count_Heap(uint32_t _SIZE, uint32_t _length, uint32_t _hash_num)
		: length(_length), hash_num(_hash_num) {
		this->name = "Count_Heap";
		heap = new Heap<DATA_LEN>(_SIZE);
		counter = new int* [hash_num];
		for (uint32_t i = 0; i < hash_num; ++i) {
			counter[i] = new int[length];
			memset(counter[i], 0, sizeof(int) * length);
		}
	}
	~Count_Heap() {
		for (uint32_t i = 0; i < hash_num; ++i) delete[] counter[i];
		delete[] counter;
		delete heap;
	}

	void Init(const Data<DATA_LEN>& item) {
		int* result = new int[hash_num];

		for (uint32_t i = 0; i < hash_num; ++i) {
			uint32_t position = item.Hash(i) % length;
			uint32_t choice = item.Hash(i + 101) & 1;
			counter[i][position] += COUNT[choice];
			result[i] = counter[i][position] * COUNT[choice];
		}

		heap->Insert(item, Get_Median(result, hash_num));
		delete[] result;
	}

	int Query(const Data<DATA_LEN>& item) { return heap->Query(item); }

private:
	Heap<DATA_LEN>* heap;
	int** counter;
	uint32_t length;
	uint32_t hash_num;
};