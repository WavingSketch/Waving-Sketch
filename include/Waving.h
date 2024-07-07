#ifndef WAVING_H
#define WAVING_H

#include <time.h>
//#include <unistd.h>
#include <bitset>
#include <algorithm>
#include <random>

#include "abstract.h"
#include "./bitset.h"
using namespace std;
#define factor 1
template <uint32_t slot_num, uint32_t counter_num, uint32_t DATA_LEN>
class WavingSketch : public Abstract<DATA_LEN> {
public:
	struct node  // Defined as temporary data structure used in shrink
	{
		count_type counter;
		Data<DATA_LEN> item;
		node() : counter(0) {};
		bool operator<(const node& tp) const {
			if (counter < 0)  // true
			{
				if (tp.counter < 0)
					return std::abs(counter) > std::abs(tp.counter);
				else
					return 1;
			}
			else {
				if (tp.counter < 0)
					return 0;
				else
					return std::abs(counter) > std::abs(tp.counter);
			}
		}
	};
	struct Bucket {
		Data<DATA_LEN> items[slot_num];
		int counters[slot_num];
		int16_t incast[counter_num];

		void Insert(const Data<DATA_LEN>& item, uint32_t seed_s, uint32_t seed_incast, bool& is_empty) {
			uint32_t choice = item.Hash(seed_s) & 1;
			uint32_t whichcast = item.Hash(seed_incast) % counter_num;
			count_type min_num = INT_MAX;
			uint32_t min_pos = -1;
			is_empty = false;
			for (uint32_t i = 0; i < slot_num; ++i) {
				if (counters[i] == 0) {
					// The error free item's counter is negative, which is a trick to
					// be differentiated from items which are not error free.
					items[i] = item;
					counters[i] = -1;
					is_empty = true;
					return;
				}
				else if (items[i] == item) {
					if (counters[i] < 0)
						counters[i]--;
					else {
						counters[i]++;
						incast[whichcast] += COUNT[choice];
					}
					return;
				}

				count_type counter_val = std::abs(counters[i]);
				if (counter_val < min_num) {
					min_num = counter_val;
					min_pos = i;
				}
			}

			if (incast[whichcast] * COUNT[choice] >= int(min_num * factor)) {
				if (counters[min_pos] < 0) {
					uint32_t min_choice = items[min_pos].Hash(seed_s) & 1;
					incast[items[min_pos].Hash(seed_incast) % counter_num] -=
						COUNT[min_choice] * counters[min_pos];
				}
				items[min_pos] = item;
				counters[min_pos] = min_num + 1;
			}
			incast[whichcast] += COUNT[choice];
		}

		int Query(const Data<DATA_LEN>& item, uint32_t seed_s, uint32_t seed_incast, bool only_heavy = false) {
			uint32_t choice = item.Hash(seed_s) & 1;
			uint32_t whichcast = item.Hash(seed_incast) % counter_num;
			count_type retv = (only_heavy == true) ? 0 : std::max(incast[whichcast] * COUNT[choice], 0);

			for (uint32_t i = 0; i < slot_num; ++i) {
				if (items[i] == item) {
					return std::abs(counters[i]);
				}
			}
			return retv;
		}
	};

	WavingSketch(int mem, int _HIT, uint32_t _HASH_NUM = 3)
		: BUCKET_NUM(mem / sizeof(Bucket)),
		HASH_NUM(_HASH_NUM),
		LENGTH(mem * 8 * 1 / 2),
		cnt(0)
	{
		this->HIT = _HIT;
		this->name = (char*)"WavingSketch";
		record = 0;
		bitset = new BitSet(LENGTH);
		buckets = new Bucket[BUCKET_NUM];
		memset(buckets, 0, BUCKET_NUM * sizeof(Bucket));
		this->rename(int(slot_num), int(counter_num));
		this->sep = "\t";
		seed_choice = std::clock();
		sleep(1);
		seed_incast = std::clock();
		sleep(1);
		seed_s = std::clock();
		// std::printf("seed_choice: %d, seed_incast: %d, seed_s: %d\n",
		// seed_choice, seed_incast, seed_s);
	}
	WavingSketch(uint32_t _BUCKET_NUM)
		: BUCKET_NUM(_BUCKET_NUM), HASH_NUM(3),
		cnt(0),
		LENGTH(0) {
		buckets = new Bucket[BUCKET_NUM];
		this->name = (char*)"WavingSketch";
		memset(buckets, 0, BUCKET_NUM * sizeof(Bucket));
		this->rename(int(slot_num), int(counter_num));
		seed_choice = std::clock();
		sleep(1);  // need to sleep for a while, or seed_choice and seed_incast
				   // might get the same seed!
		seed_incast = std::clock();
		sleep(1);  // need to sleep for a while, or seed_incast and seed_s might get
				   // the same seed!
		seed_s = std::clock();
	}
	~WavingSketch() { delete[] buckets; }

	void Init(const Data<DATA_LEN>& item) {
		uint32_t bucket_pos = item.Hash(seed_choice) % BUCKET_NUM;
		bool is_empty = 0;
		buckets[bucket_pos].Insert(item, seed_s, seed_incast, is_empty);
		cnt += is_empty;
	}
	void Init(const Data<DATA_LEN>& from, const Data<DATA_LEN>& to) {
		Stream<DATA_LEN> stream(from, to);

		bool init = true;
		for (uint i = 0; i < HASH_NUM; ++i) {
			uint position = stream.Hash(i) % LENGTH;
			if (!bitset->Get(position)) {
				init = false;
				bitset->Set(position);
			}
		}

		if (!init) Init(from);
	}
	void Init(const Data<DATA_LEN>& item, uint time) {
		if (time > record) {
			record = time;
			bitset->Clear();
		}

		bool init = true;
		for (uint i = 0; i < HASH_NUM; ++i) {
			uint position = item.Hash(i) % LENGTH;
			if (!bitset->Get(position)) {
				init = false;
				bitset->Set(position);
			}
		}

		if (!init) Init(item);
	}
	int Query(const Data<DATA_LEN>& item) {
		uint32_t bucket_pos = item.Hash(seed_choice) % BUCKET_NUM;
		return buckets[bucket_pos].Query(item, seed_s, seed_incast);
	}
	int Query(const Data<DATA_LEN>& item, HashMap<DATA_LEN>& mp) {
		return Query(item);
	}
	virtual int QueryTopK(const Data<DATA_LEN>& item)
	{
		uint32_t bucket_pos = item.Hash(seed_choice) % BUCKET_NUM;
		return buckets[bucket_pos].Query(item, seed_s, seed_incast, true);
	}
	virtual void Check(HashMap<DATA_LEN> mp, Abstract<DATA_LEN>* another, const std::vector<std::ostream*>& outs) {
		typename HashMap<DATA_LEN>::iterator it;
		int value = 0, all = 0, hit = 0, size = 0;
		for (it = mp.begin(); it != mp.end(); ++it) {
			value = abs((Query(it->first, mp) - another->Query(it->first, mp)));
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
		// cout << this->cr << " " << this->pr << endl;
	}
	virtual void Check(HashMap<DATA_LEN> mp, const std::vector<std::ostream*>& outs) {
		typename HashMap<DATA_LEN>::iterator it;
		int value = 0, all = 0, hit = 0, size = 0;
		for (it = mp.begin(); it != mp.end(); ++it) {
			value = QueryTopK(it->first);
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
	void Check(const HashMap<DATA_LEN>& mp, count_type HIT, const std::vector<std::ostream*>& outs) {
		typename HashMap<DATA_LEN>::const_iterator it;
		count_type value = 0, all = 0, hit = 0, size = 0, flagtrue = 0;
		double aae = 0, are = 0, cr = 0, pr = 0, rt = 0;
		for (it = mp.begin(); it != mp.end(); ++it) {
			value = QueryTopK(it->first);
			if (it->second > HIT) {
				all++;
				if (value < 0) {
					value = -value;
					flagtrue += 1;
				}
				if (value > HIT) {
					hit += 1;
					aae += abs(it->second - value);
					are += abs(it->second - value) / (double)it->second;
				}
			}
			if (value > HIT)
			{
				size += 1;
			}
		}

		aae /= hit;
		are /= hit;
		cr = hit / (double)all;
		pr = hit / (double)size;

		printf(
			"%s:\tARE: %f\tCR: %f\tPR: %f\tF1: %f\n",
			this->name.c_str(), are, cr, pr, 2 * pr * cr / ((cr + pr < 1e-6) ? 1 : cr + pr));
		*outs[0] << are << ",";
		*outs[1] << cr << ",";
		*outs[2] << pr << ",";
		*outs[3] << 2 * pr * cr / ((cr + pr < 1e-6) ? 1 : cr + pr) << ",";
	}
	bool isFull(double threshold) const
	{
		return cnt >= BUCKET_NUM * slot_num * threshold;
	}
	uint32_t getMemSize() const
	{
		return BUCKET_NUM * (slot_num * 8 + 2 * counter_num);
	}

	void shrink() {
		// shrink to half of total buckets
		if (BUCKET_NUM % 2) {
			printf("Bucket num %d is odd, can NOT shrink!\n", BUCKET_NUM);
			printf("FATAL ERROR!\n");
			exit(-1);
		}
		uint32_t num = (BUCKET_NUM + 1) >> 1;
		Bucket* tpbuck = new Bucket[num];
		for (uint32_t i = 0; i < num; i++) {
			if (i + num < BUCKET_NUM)
				mergebuck(&buckets[i], &buckets[i + num], &tpbuck[i]);
			else
				copybuck(&buckets[i], &tpbuck[i]);
		}
		BUCKET_NUM = num;
		delete[] buckets;
		buckets = tpbuck;
	}
	void expand() {
		uint32_t num = BUCKET_NUM << 1;
		Bucket* tpbuck = new Bucket[num];
		for (uint32_t i = 0; i < BUCKET_NUM; i++) {
			copybuck(&buckets[i], &tpbuck[i]);
			copybuck(&buckets[i], &tpbuck[BUCKET_NUM + i]);
		}
		for (uint32_t i = 0; i < num; i++) {
			for (uint32_t j = 0; j < slot_num; j++) {
				if (tpbuck[i].items[j].Hash(seed_choice) % num != i) {
					if (tpbuck[i].counters[j] > 0)  // flag=False
					{
						tpbuck[i].incast[tpbuck[i].items[j].Hash(seed_incast) %
							counter_num] -=
							COUNT[tpbuck[i].items[j].Hash(seed_s) & 1] *
							tpbuck[i].counters[j];
					}
					tpbuck[i].counters[j] = 0;
					tpbuck[i].items[j] = 0;
				}
			}
			for (uint32_t j = 0; j < slot_num; j++) {
				if (tpbuck[i].counters[j] == 0) {
					uint32_t k = j + 1;
					for (; k < slot_num && tpbuck[i].counters[k] == 0; k++)
						;
					if (k == slot_num) break;
					tpbuck[i].counters[j] = tpbuck[i].counters[k];
					tpbuck[i].items[j] = tpbuck[i].items[k];
					tpbuck[i].counters[k] = 0;
				}
			}
			for (int j = 0; j < counter_num; j++)
				tpbuck[i].incast[j] = tpbuck[i].incast[j] >> 1;
		}
		BUCKET_NUM = num;
		delete[] buckets;
		buckets = tpbuck;
	}
private:

	void copybuck(Bucket* src, Bucket* dest) {
		for (int i = 0; i < counter_num; i++) dest->incast[i] = src->incast[i];
		for (int i = 0; i < slot_num; i++) {
			dest->items[i] = src->items[i];
			dest->counters[i] = src->counters[i];
		}
	}

	void mergebuck(Bucket* src1, Bucket* src2, Bucket* dest) {
		for (int i = 0; i < counter_num; i++)
			dest->incast[i] = src1->incast[i] + src2->incast[i];
		node* tp1 = new node[slot_num << 1];
		node* p = tp1;
		for (uint32_t i = 0; i < slot_num; i++) {
			p->item = src1->items[i];
			p->counter = src1->counters[i];
			p++;
			cnt -= src1->counters[i] != 0;
		}
		for (uint32_t i = 0; i < slot_num; i++) {
			p->item = src2->items[i];
			p->counter = src2->counters[i];
			cnt -= src2->counters[i] != 0;
			p++;
		}
		std::sort(tp1, p, [](node x, node y)-> bool {return abs(x.counter) > abs(y.counter); });
		uint32_t i = 0;
		for (i = 0; i < slot_num; i++) {
			dest->items[i] = tp1[i].item;
			dest->counters[i] = tp1[i].counter;
			cnt += tp1[i].counter != 0;
		}
		uint32_t end = slot_num << 1;
		while (i < end && tp1[i].counter < 0)  // flag=true
		{
			uint32_t min_choice = tp1[i].item.Hash(seed_s) & 1;
			uint32_t whichcast = tp1[i].item.Hash(seed_incast) % counter_num;
			dest->incast[whichcast] -= COUNT[min_choice] * tp1[i].counter;
			i++;
		}
		delete[] tp1;
	}

	Bucket* buckets;
	uint32_t BUCKET_NUM;
	const uint HASH_NUM;
	const uint LENGTH;
	BitSet* bitset;
	uint32_t record;
	// uint32_t prev_BUCKET_NUM;
	uint32_t seed_choice;
	uint32_t seed_s;
	uint32_t seed_incast;
	int cnt;
};
template<uint32_t DATA_LEN>
class Interest : public Abstract<DATA_LEN> {
public:
	Interest(int mem, int _HIT) {
		this->HIT = _HIT;
		BUCKET_NUM = mem / (8 * sizeof(Counter) + 16 * 4);
		SIZE = BUCKET_NUM * 8;
		counter = new Counter[SIZE];
		incast = (int(*)[16])malloc(BUCKET_NUM * 32);
		memset(counter, 0, SIZE * sizeof(Counter));
		memset(incast, 0, BUCKET_NUM * sizeof(int));
		this->pr = this->cr = 0;
		this->name = "IttSketch(memory/20)";
	}
	~Interest() {
		delete[] counter;
		delete[] incast;
	}

	void Init(const Data<DATA_LEN>& data) {
		uint bucket_pos = data.Hash() % BUCKET_NUM;
		uint incast_pos = data.Hash(17) % 16;
		uint position = (bucket_pos << 3);

		int min_num = INT_MAX;
		uint min_pos = -1;
		for (uint i = 0; i < 8; ++i) {
			if (counter[position + i].count == 0) {
				counter[position + i].Set(data, 1);
				return;
			}
			else if (counter[position + i].data == data) {
				counter[position + i].count += 1;
				return;
			}

			if (counter[position + i].count < min_num) {
				min_num = counter[position + i].count;
				min_pos = position + i;
			}
		}

		if (rd() % ((min_num << 1) - incast[bucket_pos][incast_pos] + 1) == 0) {
			counter[min_pos].Set(
				data, min_num + (incast[bucket_pos][incast_pos] / min_num));
			incast[bucket_pos][incast_pos] = 0;
		}
		else
			incast[bucket_pos][incast_pos] += 1;
	}

	int Query(const Data<DATA_LEN>& data, HashMap<DATA_LEN>& mp) {
		uint position = ((data.Hash() % BUCKET_NUM) << 3);
		int min_num = INT_MAX;
		for (uint i = 0; i < 8; ++i) {
			if (counter[position + i].data == data)
				return counter[position + i].count;
			min_num = MIN(min_num, counter[position + i].count);
		}
		return 0;
	}

	void Check(HashMap<DATA_LEN> mp, Abstract<DATA_LEN>* another, const std::vector<std::ostream*>& outs) {
		typename HashMap<DATA_LEN>::iterator it;
		int value = 0, all = 0, hit = 0, size = 0;
		for (it = mp.begin(); it != mp.end(); ++it) {
			value = abs((Query(it->first, mp) - another->Query(it->first, mp)));
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
		void Set(const Data<DATA_LEN>& dat, int num) {
			data = dat;
			count = num;
		}
	};

	uint BUCKET_NUM;
	uint SIZE;
	random_device rd;
	Counter* counter;
	int(*incast)[16];
};

#endif  // WAVING_H
