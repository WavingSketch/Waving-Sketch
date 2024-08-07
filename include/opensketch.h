#ifndef OPENSKETCH_H
#define OPENSKETCH_H

#include "abstract.h"
#include "bitset.h"
using namespace std;
template<uint32_t DATA_LEN>
class OpenSketch : public Abstract<DATA_LEN> {
public:
	OpenSketch(int num, int _HIT) {
		this->HIT = _HIT;
		LENGTH = 128;
		CAPACITY = 250;
		COLUMN = (num - 6 * CAPACITY * sizeof(KV<DATA_LEN>)) / 20;
		heap_num = 0;
		bitset = new BitSet[COLUMN];
		counter = new int[COLUMN];
		memset(counter, 0, sizeof(int) * COLUMN);
		heap = new KV<DATA_LEN>[CAPACITY];
		memset(heap, 0, sizeof(KV<DATA_LEN>) * CAPACITY);
		this->aae = this->are = this->pr = this->cr = 0;
		this->sep = "\t\t";
		this->name = "OpenSketch";
	}
	~OpenSketch() {
		delete[] heap;
		delete[] bitset;
		delete[] counter;
	}

	void Init(const Data<DATA_LEN>& from, const Data<DATA_LEN>& to) {
		Stream<DATA_LEN> stream(from, to);

		int min_num = INT_MAX;
		uint index = Hash::BOBHash32(stream.str + DATA_LEN, DATA_LEN) % LENGTH;
		for (int i = 0; i < 2; ++i) {
			uint pos = from.Hash(i) % COLUMN;
			if (!bitset[pos].Get(index)) {
				counter[pos] += 1;
				bitset[pos].Set(index);
			}
			min_num = MIN(min_num, counter[pos]);
		}

		if (mp.find(from) != mp.end()) {
			if (heap[mp[from]].first != min_num) {
				heap[mp[from]].first = min_num;
				heap_down(mp[from]);
			}
		}
		else if (heap_num < CAPACITY) {
			heap[heap_num].second = from;
			heap[heap_num].first = min_num;
			mp[from] = heap_num++;
			heap_up(heap_num - 1);
		}
		else if (min_num > heap[0].first) {
			KV<DATA_LEN>& kv = heap[0];
			mp.erase(kv.second);
			kv.second = from;
			kv.first = min_num;
			mp[from] = 0;
			heap_down(0);
		}
	}

	int Query(const Data<DATA_LEN>& data) {
		if (mp.find(data) == mp.end())
			return -1;
		else
			return heap[mp[data]].first;
	}

	void Check(HashMap<DATA_LEN> mp, const std::vector<std::ostream*>& outs) {
		typename HashMap<DATA_LEN>::iterator it;
		int value = 0, all = 0, hit = 0, size = 0;
		for (it = mp.begin(); it != mp.end(); ++it) {
			value = Query(it->first);
			if (it->second > this->HIT) {
				all++;
				if (value > this->HIT) {
					// std::cout << value << " " << it->second << std::endl;
					hit += 1;
					this->aae += abs(it->second - value);
					this->are += abs(it->second - value) / (double)it->second;
				}
			}
			if (value > this->HIT) size += 1;
		}
		// std::cout << size << " " << hit << std::endl;
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
	int LENGTH;
	int COLUMN;
	int CAPACITY;
	int heap_num;

	BitSet* bitset;
	int* counter;
	HashMap<DATA_LEN> mp;
	KV<DATA_LEN>* heap;

	void heap_down(int i) {
		while (i < heap_num / 2) {
			int l_child = 2 * i + 1;
			int r_child = 2 * i + 2;
			int larger_one = i;
			if (l_child < heap_num && heap[l_child] < heap[larger_one]) {
				larger_one = l_child;
			}
			if (r_child < heap_num && heap[r_child] < heap[larger_one]) {
				larger_one = r_child;
			}
			if (larger_one != i) {
				swap(heap[i], heap[larger_one]);
				swap(mp[heap[i].second], mp[heap[larger_one].second]);
				heap_down(larger_one);
			}
			else {
				break;
			}
		}
	}

	void heap_up(int i) {
		while (i > 1) {
			int parent = (i - 1) / 2;
			if (heap[parent] <= heap[i]) {
				break;
			}
			swap(heap[i], heap[parent]);
			swap(mp[heap[i].second], mp[heap[parent].second]);
			i = parent;
		}
	}
};

#endif  // OPENSKETCH_H
