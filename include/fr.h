#ifndef FR_H
#define FR_H

#include "abstract.h"
#include "bitset.h"
template<uint32_t DATA_LEN>
class FR : public Abstract<DATA_LEN> {
public:
	FR(int num, int _HIT) {
		this->HIT = _HIT;
		SIZE = num / 17;
		// SIZE = 118750 + 30000 * (num - 1);
		LENGTH = 8 * SIZE;
		// LENGTH = 800000 + 160000 * (num - 1);
		flow = 0;
		bitset = new BitSet(LENGTH);
		counter = new Counter[SIZE];
		memset(counter, 0, sizeof(Counter) * SIZE);
		this->name = "FR";
		this->sep = "\t\t\t";
	}
	~FR() {
		delete bitset;
		delete[] counter;
	}
	void Init(const Data<DATA_LEN>& data) {
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

	int Query(const Data<DATA_LEN>& data, HashMap<DATA_LEN>& mp) {
		while (true) {
			bool flag = true;
			for (int i = 0; i < SIZE; ++i) {
				if (counter[i].count == 1) {
					Data<DATA_LEN> data = counter[i].data;
					int count = counter[i].packet;
					mp[data] = count;

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
		Data<DATA_LEN> temp;
		HashMap<DATA_LEN> first, second;
		Query(temp, first);
		another->Query(temp, second);
		typename HashMap<DATA_LEN>::iterator it;
		int value = 0, all = 0, hit = 0, size = 0;
		for (it = mp.begin(); it != mp.end(); ++it) {
			value = abs(first[it->first] - second[it->first]);
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
		// cout << all << " " << size << endl;
		// cout <<cr << " " << this->pr << endl;
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

#endif  // FR_H
