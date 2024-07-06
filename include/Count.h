#pragma once
#include "abstract.h"
template<uint32_t DATA_LEN>
class Count : public Abstract<DATA_LEN> {
public:
	Count(uint32_t MemoryInByte, uint32_t _hash_num)
		: hash_num(_hash_num)
	{
		this->name = "Count";
		length = MemoryInByte / sizeof(uint32_t) / _hash_num;
		counters = new int* [hash_num];
		for (size_t i = 0; i < hash_num; i++)
		{
			counters[i] = new count_type[length];
			memset(counters[i], 0, sizeof(count_type) * length);
		}
	}
	~Count()
	{
		for (size_t i = 0; i < hash_num; i++)
		{
			delete[] counters[i];
		}
		delete[] counters;
	}
	void Insert(const Data<DATA_LEN>& item)
	{
		for (uint32_t i = 0; i < hash_num; ++i) {
			uint32_t position = item.Hash(i) % length;
			int coef = item.Hash(i + hash_num) & 1;
			counters[i][position] += 2 * coef - 1;
		}
	}
	int Query(const Data<DATA_LEN>& item)
	{
		int* alt = new int[hash_num];
		for (uint32_t i = 0; i < hash_num; ++i) {
			uint32_t position = item.Hash(i) % length;
			int coef = item.Hash(i + hash_num) & 1;
			alt[i] = counters[i][position] * (2 * coef - 1);
		}
		/*sort(alt.begin(), alt.end());
		uint32_t pos = hash_num / 2;
		if (hash_num % 2)
			return alt[pos];
		else
			return (alt[pos] + alt[pos - 1]) / 2;*/
		return Get_Median(alt, hash_num);
	}
private:
	int** counters;
	uint32_t length, hash_num;
};