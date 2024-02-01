#pragma once
class CM : public Abstract {
public:
	CM(uint32_t MemoryInByte, uint32_t _hash_num)
		:Abstract((char*)"CM"), hash_num(_hash_num)
	{
		length = MemoryInByte / sizeof(uint32_t) / _hash_num;
		counters = new count_type * [hash_num];
		for (size_t i = 0; i < hash_num; i++)
		{
			counters[i] = new count_type[length];
			memset(counters[i], 0, sizeof(count_type) * length);
		}
	}
	~CM()
	{
		for (size_t i = 0; i < hash_num; i++)
		{
			delete[] counters[i];
		}
		delete[] counters;
	}
	void Insert(const data_type item)
	{
		for (uint32_t i = 0; i < hash_num; ++i) {
			uint32_t position = hash32(item, i) % length;
			counters[i][position] ++;
		}
	}
	count_type Query(const data_type item)
	{
		count_type ret = 0x7FFFFFFF;
		for (uint32_t i = 0; i < hash_num; ++i) {
			uint32_t position = hash32(item, i) % length;
			ret = std::min(ret, counters[i][position]);
		}
		return ret;
	}
private:
	count_type** counters;
	uint32_t length, hash_num;
};