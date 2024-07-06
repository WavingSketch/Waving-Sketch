#ifndef TLF_H
#define TLF_H

#include <random>

#include "abstract.h"

using namespace std;

template<uint32_t DATA_LEN>
class TLF : public Abstract<DATA_LEN> {
public:
	TLF(int num, int _HIT) {
		this->HIT = _HIT;
		this->aae = this->are = this->pr = this->cr = 0;
		this->name = "TLF";
		this->sep = "\t\t\t";
		epsilon = num / 100000 * 0.001;
	}
	~TLF() {
		// cout << name << ":" << endl;
		// cout << 60 * sp.size() + 40 * (mp.size() + ip.size()) << endl;
	}

	void Init(const Data<DATA_LEN>& from, const Data<DATA_LEN>& to) {
		uint Max = 0xffffffff;
		Stream<DATA_LEN> stream(from, to);
		if (stream.Hash() / (double)Max < epsilon) {
			if (ip.find(from) != ip.end()) {
				if (sp.find(stream) == sp.end()) {
					sp[stream] = 1;
					if (mp.find(from) == mp.end())
						mp[from] = 1;
					else
						mp[from] += 1;
				}
			}
			else {
				ip[from] = 1;
			}
		}
	}
	int Query(const Data<DATA_LEN>& data) {
		if (mp.find(data) == mp.end()) return -1;
		return (int)(mp[data] / epsilon);
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
		this->cr = hit / (double)all;
		this->pr = hit / (double)size;
		*outs[0] << this->are << ",";
		*outs[1] << this->cr << ",";
		*outs[2] << this->pr << ",";
		*outs[3] << 2 * this->pr * this->cr / ((this->cr + this->pr < 1e-6) ? 1 : this->cr + this->pr) << ",";
	}

private:
	double epsilon;
	HashMap<DATA_LEN> mp;
	HashMap<DATA_LEN> ip;
	StreamMap<DATA_LEN> sp;
};

#endif  // TLF_H
