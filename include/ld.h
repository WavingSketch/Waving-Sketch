#pragma once

#include "LD-Sketch/LDSketch.hpp"
#include "abstract.h"
template<uint32_t DATA_LEN>
class LdSketchWrapper : public Abstract<DATA_LEN> {
public:
	LdSketchWrapper(uint32_t memory_size, int _HIT) :up_key("up"), low_key("low") {
		this->HIT = _HIT;
		// 4 rows
		// 40 bytes for LD Sketch
		// 104 bytes for each bucket
		int w = (memory_size - 40) / 4 / 104;
		summary = LDSketch_init(w, 4, 1, DATA_LEN * 8, this->HIT, 0);
		this->name = "LD Sketch";
		this->sep = "\t";
	}
	~LdSketchWrapper() { delete summary; }
	void Init(const Data<DATA_LEN>& data) {
		unsigned char* key = (unsigned char*)(&(data.str[0]));
		LDSketch_update(summary, key, 1);
	}

	int Query(const Data<DATA_LEN>& data, HashMap<DATA_LEN>& mp) {
		// estimate the frequency of data and store the estimations in mp
		unsigned char* key = (unsigned char*)(&(data.str[0]));
		mp[up_key] = LDSketch_up_estimate(summary, key);
		mp[low_key] = LDSketch_low_estimate(summary, key);
		return 0;
	}
	int Query(const Data<DATA_LEN>& item) {
		unsigned char* key = (unsigned char*)(&(item.str[0]));
		return LDSketch_up_estimate(summary, key);
	}
	void Check(HashMap<DATA_LEN> mp, Abstract<DATA_LEN>* another, const std::vector<std::ostream*>& outs) {
		HashMap<DATA_LEN> est1, est2;

		typename HashMap<DATA_LEN>::iterator it;
		int value = 0, all = 0, hit = 0, size = 0;
		for (it = mp.begin(); it != mp.end(); ++it) {
			Query(it->first, est1);
			another->Query(it->first, est2);

			value = max(abs(est1[up_key] - est2[low_key]),
				abs(est2[up_key] - est1[low_key]));
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

	double average_l() {
		int l_sum = 0;
		int count = (summary->h) * (summary->w);
		for (int i = 0; i < count; ++i) {
			l_sum += summary->tbl[i]->max_len;
		}
		return l_sum / (double)count;
	}

private:
	LDSketch_t* summary;
	const Data<DATA_LEN> up_key, low_key;
};