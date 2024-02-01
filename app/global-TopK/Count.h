#pragma once
class Count : public Abstract {
public:
	Count(uint32_t MemoryInByte, uint32_t _hash_num)
		:Abstract((char*)"Count"), hash_num(_hash_num)
	{
		length = MemoryInByte / sizeof(uint32_t) / _hash_num;
		counters = new count_type * [hash_num];
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
	void Insert(const data_type item)
	{
		for (uint32_t i = 0; i < hash_num; ++i) {
			uint32_t position = hash32(item, i) % length;
			int coef = hash32(item, i + hash_num) & 1;
			counters[i][position] += 2 * coef - 1;
		}
	}
	count_type Query(const data_type item)
	{
		std::vector<count_type> alt;
		for (uint32_t i = 0; i < hash_num; ++i) {
			uint32_t position = hash32(item, i) % length;
			int coef = hash32(item, i + hash_num) & 1;
			alt.push_back(counters[i][position] * (2 * coef - 1));
		}
		sort(alt.begin(), alt.end());
		uint32_t pos = hash_num / 2;
		if (hash_num % 2)
			return alt[pos];
		else
			return (alt[pos] + alt[pos - 1]) / 2;
	}
private:
	count_type** counters;
	uint32_t length, hash_num;
};