#ifndef PIE_H
#define PIE_H

#include <string.h>

#include <algorithm>

#include "abstract.h"
#include "data.h"
#include "definition.h"

#define RAPTOR_SIZE 1
#define PRINT_SIZE 14
#define MASK_PRINT 0x3fff
template<uint32_t DATA_LEN>
class PIE : public Abstract<DATA_LEN> {
public:
	PIE(int num, int _HIT, uint _CYCLE) : CYCLE(_CYCLE) {
		this->HIT = _HIT;
		LENGTH = num / _CYCLE / 6;
		counter = new Counter[LENGTH * CYCLE];
		memset(counter, 0, sizeof(Counter) * LENGTH * CYCLE);
		this->aae = this->are = this->pr = this->cr = 0;
		this->sep = "\t\t\t";
		this->name = "PIE";
	}
	~PIE() { delete[] counter; }
	void Init(const Data<DATA_LEN>& data, uint time) {
		Counter temp = Encode(data, time);
		uint position = data.Hash() % LENGTH;
		position += (time - 1) * LENGTH;
		if (counter[position].empty()) {
			counter[position] = temp;
		}
		else if (!(counter[position] == temp)) {
			counter[position].set_collision();
		}
	}

	int Query(const Data<DATA_LEN>& data) {
		int ret = 0;
		uint position = data.Hash() % LENGTH;
		for (uint time = 0; time < CYCLE; ++time) {
			uint pos = position + time * LENGTH;
			if (counter[pos].print == (data.Hash(1) & MASK_PRINT)) ret += 1;
		}
		return ret;
	}

	void Check(HashMap<DATA_LEN> mp, const std::vector<std::ostream*>& outs) {
		typename HashMap<DATA_LEN>::iterator it;
		int value = 0, all = 0, hit = 0, size = 0;
		for (it = mp.begin(); it != mp.end(); ++it) {
			value = Query(it->first);
			if (it->second > this->HIT) {
				all++;
				if (value > this->HIT) {
					hit += 1;
					this->aae += abs(it->second - value);
					this->are += abs(it->second - value) / (double)it->second;
				}
			}
			if (value > this->HIT) size += 1;
		}
		this->aae /= hit;
		this->are /= hit;
		this->cr = hit / (double)std::max(1, all);
		this->pr = hit / (double)std::max(1, size);
		*outs[0] << this->cr << ",";
		*outs[1] << this->pr << ",";
		*outs[2] << 2 * this->pr * this->cr / ((this->pr + this->cr < 1e-9) ? 1.0 : this->pr + this->cr) << ",";
		*outs[3] << this->are << ",";
	}

private:
	struct Counter {
		short flag : 1;
		short raptor : RAPTOR_SIZE;
		short print : PRINT_SIZE;
		bool empty() { return (flag == 0 && raptor == 0 && print == 0); }
		void set_collision() {
			flag = 1;
			raptor = -1;
			print = -1;
		}
		bool operator==(Counter count) {
			return (raptor == count.raptor && print == count.print);
		}
	};

	uint LENGTH;
	const uint CYCLE;

	Counter* counter;

	Counter Encode(const Data<DATA_LEN>& data, uint time) {
		Counter temp;
		temp.flag = 1;
		temp.print = (data.Hash(1) & MASK_PRINT);
		srand(time);
		int len = DATA_LEN * 8;
		bool coeff[len];
		bool id[len];

		for (int i = 0; i < len; ++i) coeff[i] = (rand() & 1);
		for (int i = 0; i < DATA_LEN; ++i) {
			for (int j = 0; j < 8; ++j) id[i * 8 + j] = (data.str[i] & (7 - j));
		}
		temp.raptor = 0;
		for (int i = 0; i < len; ++i) {
			temp.raptor ^= (coeff[i] & id[i]);
		}
		return temp;
	}
};

#endif  // PIE_H
