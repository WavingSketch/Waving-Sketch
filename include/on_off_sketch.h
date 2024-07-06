#ifndef OO_FPI_H
#define OO_FPI_H

/*
 * On-Off sketch on finding persistent items
 */

#include "abstract.h"
#include "bitset.h"
const static int SLOT_NUM = 8;
template<uint32_t DATA_LEN>
class OO_FPI : public Abstract<DATA_LEN> {
public:
	struct Bucket {
		Data<DATA_LEN> items[SLOT_NUM];
		int counters[SLOT_NUM];

		inline int Query(const Data<DATA_LEN> item) {
			for (uint32_t i = 0; i < SLOT_NUM; ++i) {
				if (items[i] == item) return counters[i];
			}
			return 0;
		}
	};

	OO_FPI(uint64_t memory, int _HIT)
		: length((double)memory /
			(sizeof(Bucket) + sizeof(int) + (SLOT_NUM + 1) * BITSIZE)) {
		this->HIT = _HIT;
		this->name = "On Off Sketch";
		this->sep = "\t";
		buckets = new Bucket[length];
		sketch = new int[length];

		memset(buckets, 0, length * sizeof(Bucket));
		memset(sketch, 0, length * sizeof(int));

		bucketBitsets = new BitSet(SLOT_NUM * length);
		sketchBitsets = new BitSet(length);
	}

	~OO_FPI() {
		delete[] buckets;
		delete[] sketch;
		delete bucketBitsets;
		delete sketchBitsets;
	}

	void Init(const Data<DATA_LEN>& item, uint window) {
		if (window > last_window) {
			last_window = window;
			NewWindow();
		}

		uint32_t pos = item.Hash(17) % length;
		uint32_t bucketBitPos = pos * SLOT_NUM;

		for (uint32_t i = 0; i < SLOT_NUM; ++i) {
			if (buckets[pos].items[i] == item) {
				buckets[pos].counters[i] += (!bucketBitsets->SetNGet(bucketBitPos + i));
				return;
			}
		}

		if (!sketchBitsets->Get(pos)) {
			for (uint32_t i = 0; i < SLOT_NUM; ++i) {
				if (buckets[pos].counters[i] == sketch[pos]) {
					buckets[pos].items[i] = item;
					buckets[pos].counters[i] += 1;
					bucketBitsets->Set(bucketBitPos + i);
					return;
				}
			}

			sketch[pos] += 1;
			sketchBitsets->Set(pos);
		}
	}

	int Query(const Data<DATA_LEN> item) {
		return buckets[item.Hash(17) % length].Query(item);
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

	void NewWindow() {
		bucketBitsets->Clear();
		sketchBitsets->Clear();
	}

private:
	const uint32_t length;

	BitSet* bucketBitsets;
	Bucket* buckets;

	BitSet* sketchBitsets;
	int* sketch;
	int HIT;
	int last_window = 0;
};

#endif  // OO_FPI_H