#ifndef SMALL_SPACE_H
#define SMALL_SPACE_H

#include <string.h>

#include <algorithm>

#include "abstract.h"
#include "data.h"
#include "definition.h"

struct Info {
	uint time;
	int count;
	Info(uint _time = 0, int _count = 0) {
		time = _time;
		count = _count;
	}
};
template<uint32_t DATA_LEN>
using SS_Map = unordered_map<Data<DATA_LEN>, Info, My_Hash<DATA_LEN>>;

template<uint32_t DATA_LEN>
class Small_Space : public Abstract<DATA_LEN> {
public:
	Small_Space(int num, int _HIT) {
		uint Max = 0xffffffff;
		EPSILON = 0.00027 * num / 200000;
		this->HIT = _HIT;
		map.clear();
		this->aae = this->are = this->pr = this->cr = 0;
		this->sep = "\t\t";
		this->name = "Small-Space";
	}
	~Small_Space() = default;

	int Size() { return map.size(); }
	void Init(const Data<DATA_LEN>& data, uint time) {
		if (map.find(data) != map.end()) {
			if (map[data].time < time) {
				map[data].time = time;
				map[data].count += 1;
			}
		}
		else {
			uchar str[DATA_LEN + 4] = { 0 };
			memcpy(str, data.str, DATA_LEN);
			memcpy(str + DATA_LEN, &time, sizeof(time));
			uint Max = 0xffffffff;
			if (Hash::BOBHash32(str, DATA_LEN + 4) / (double)Max < EPSILON) {
				map[data] = Info(time, 1);
			}
		}
	}

	void Check(HashMap<DATA_LEN> mp, const std::vector<std::ostream*>& outs) {
		int value = 0, all = 0, hit = 0, size = 0;
		for (typename HashMap<DATA_LEN>::iterator it = mp.begin(); it != mp.end(); ++it) {
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

	int Query(const Data<DATA_LEN>& data) {
		if (map.find(data) == map.end()) return -1;
		return map[data].count + 1 / EPSILON;
	}

private:
	double EPSILON;
	SS_Map<DATA_LEN> map;
};

#endif  // SMALL_SPACE_H
