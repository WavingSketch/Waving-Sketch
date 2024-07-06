#ifndef SS_H
#define SS_H

#include "SpreadSketch/spreadsketch.hpp"
#include "abstract.h"
template<uint32_t DATA_LEN>
class SpreadSketchWrapper : public Abstract<DATA_LEN> {
public:
	SpreadSketchWrapper(int memory, int _HIT) {
		this->HIT = _HIT;
		// total_mem = cmdepth*cmwidth*(memory+lgn+8)/8
		int width = memory * 8 / 4 / (438 + 32 + 8);
		sketch = new DetectorSS(4, width, 32, 79, 3, 438);
		this->name = "SpreadSketch";
		this->sep = "\t";
	}
	~SpreadSketchWrapper() { delete sketch; }

	void Init(const Data<DATA_LEN>& from, const Data<DATA_LEN>& to) {
		key_tp src_ip = *(key_tp*)from.str;
		key_tp dst_ip = *(key_tp*)to.str;
		sketch->Update(src_ip, dst_ip, 1);
	};
	void Check(HashMap<DATA_LEN> mp, const std::vector<std::ostream*>& outs) {
		typename HashMap<DATA_LEN>::iterator it;
		vector<pair<key_tp, val_tp>> result;
		sketch->Query(this->HIT, result);
		HashMap<DATA_LEN> our_result;
		for (auto const& p : result) {
			Data<DATA_LEN> key;
			memcpy(key.str, &(p.first), sizeof(Data<DATA_LEN>));
			our_result[key] = p.second;
		}

		int value = 0, all = 0, hit = 0, size = 0;
		for (it = mp.begin(); it != mp.end(); ++it) {
			value = our_result[it->first];
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
		this->cr = hit / (double)all;
		this->pr = hit / (double)size;
		*outs[0] << this->are << ",";
		*outs[1] << this->cr << ",";
		*outs[2] << this->pr << ",";
		*outs[3] << 2 * this->pr * this->cr / ((this->cr + this->pr < 1e-6) ? 1 : this->cr + this->pr) << ",";
	}

private:
	DetectorSS* sketch;
};

#endif  // SS_H