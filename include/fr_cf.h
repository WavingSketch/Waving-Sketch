#ifndef FR_CF_H
#define FR_CF_H

#include "abstract.h"
#include "bitset.h"
template<uint32_t DATA_LEN>
class CF {
public:
	const int THRESHOLD;

	CF(uint _LENGTH, uint _HASH_NUM = 2, int _THRESHOLD = 15)
		: HASH_NUM(_HASH_NUM), LENGTH(_LENGTH), THRESHOLD(_THRESHOLD) {
		counter = new uchar[LENGTH];
		memset(counter, 0, sizeof(uchar) * LENGTH);
	}
	~CF() { delete[] counter; }
	int Query(const Data<DATA_LEN>& data) {
		int min_num = INT_MAX;
		for (uint i = 0; i < HASH_NUM; ++i) {
			uint position = data.Hash(i + 1) % LENGTH;
			min_num = MIN(counter[position], min_num);
		}
		return min_num;
	}

	bool Init(const Data<DATA_LEN>& data) {
		if (LENGTH == 0) return true;

		int min_num = INT_MAX;
		uint* position = new uint[HASH_NUM];
		for (uint i = 0; i < HASH_NUM; ++i) {
			position[i] = data.Hash(i + 1) % LENGTH;
			min_num = MIN(counter[position[i]], min_num);
		}

		if (min_num >= THRESHOLD) {
			delete[] position;
			return true;
		}

		for (uint i = 0; i < HASH_NUM; ++i) {
			if (counter[position[i]] == min_num) {
				counter[position[i]] += 1;
			}
		}

		delete[] position;
		return false;
	}

private:
	const uint HASH_NUM;
	const uint LENGTH;

	uchar* counter;
};
template<uint32_t DATA_LEN>
class FR_CF : public Abstract<DATA_LEN> {
public:
	CF<DATA_LEN>* filter;

	FR_CF(int num, int _HIT) {
		this->HIT = _HIT;
		SIZE = num * 4 / 69;
		// SIZE = 118000 + 30000 * (num - 1);
		LENGTH = SIZE * 8;
		// LENGTH = 800000 + 150000 * (num - 1);
		flow = 0;
		bitset = new BitSet(LENGTH);
		counter = new Counter[SIZE];
		filter = new CF<DATA_LEN>(SIZE / 4);
		// filter = new CF(24000 + 2500 * (num - 1));
		memset(counter, 0, sizeof(Counter) * SIZE);
		this->name = "FR+CF";
		this->sep = "\t\t\t";
	}
	~FR_CF() {
		delete filter;
		delete bitset;
		delete[] counter;
	}

	void Init(const Data<DATA_LEN>& data) {
		if (filter->Init(data)) {
			bool init = true;
			for (int i = 0; i < 3; ++i) {
				uint position = data.Hash(i) % LENGTH;
				if (!bitset->Get(position)) {
					init = false;
					bitset->Set(position);
				}
			}

			for (int i = 0; i < 2; ++i) {
				uint position = data.Hash(i) % SIZE;
				if (!init) {
					flow++;
					counter[position].data ^= data;
					counter[position].count += 1;
				}
				counter[position].packet += 1;
			}
		}
	}

	int Query(const Data<DATA_LEN>& data, HashMap<DATA_LEN>& mp) {
		while (true) {
			bool flag = true;
			for (int i = 0; i < SIZE; ++i) {
				if (counter[i].count == 1) {
					Data<DATA_LEN>data = counter[i].data;
					int count = counter[i].packet;
					mp[data] = count + 15;

					for (int j = 0; j < 2; ++j) {
						uint position = data.Hash(j) % SIZE;
						counter[position].count -= 1;
						counter[position].data ^= data;
						counter[position].packet -= count;
					}
					flag = false;
				}
			}
			if (flag) return 0;
		}
		return 0;
	}

	void Check(HashMap<DATA_LEN> mp, Abstract<DATA_LEN>* another, const std::vector<std::ostream*>& outs) {
		FR_CF* an = (FR_CF*)another;
		Data<DATA_LEN> temp;
		HashMap<DATA_LEN> first, second;
		Query(temp, first);
		an->Query(temp, second);
		typename HashMap<DATA_LEN>::iterator it;
		int value = 0, all = 0, hit = 0, size = 0;
		for (it = mp.begin(); it != mp.end(); ++it) {
			int num_first = 0, num_second = 0;
			if (first[it->first] == 0)
				num_first = filter->Query(it->first);
			else
				num_first = first[it->first];

			if (second[it->first] == 0)
				num_second = an->filter->Query(it->first);
			else
				num_second = second[it->first];

			value = abs(num_first - num_second);
			if (abs(it->second) > this->HIT) {
				all++;
				if (value > this->HIT) {
					hit += 1;
				}
			}
			if (value > this->HIT) size += 1;
		}

		this->cr = hit / (double)all;
		this->pr = hit / (double)size;
		*outs[0] << this->cr << ",";
		*outs[1] << this->pr << ",";
		*outs[2] << 2 * this->pr * this->cr / ((this->pr + this->cr < 1e-9) ? 1.0 : this->pr + this->cr) << ",";
	}

private:
	struct Counter {
		Data<DATA_LEN> data;
		int count;
		int packet;
	};
	BitSet* bitset;
	Counter* counter;
	int flow;
	int LENGTH;
	int SIZE;
};
#endif  // FR_CF_H
