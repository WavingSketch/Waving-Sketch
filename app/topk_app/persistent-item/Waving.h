#ifndef WAVING_H
#define WAVING_H

#include <bitset>
#include <unistd.h>
#include "abstract.h"
#include "bitset.h"

#define hash hash32
#define data_type Data
#define count_type int

inline uint32_t hash32(data_type item, uint32_t seed = 0){
    return Hash::BOBHash64((uint8_t*)&item, sizeof(data_type), seed);
}

static const count_type COUNT[2] = {1, -1};
static const int slot_num = 16;
static const int factor = 1;

template < uint32_t slot_num, uint32_t counter_num >
class WavingSketch : public Abstract {
public:
	struct Bucket {
		Data items[slot_num];
		int counters[slot_num];
		int16_t incast[counter_num];

		void Insert(const Data& item,uint32_t seed_s,uint32_t seed_incast) {
			uint32_t choice = item.Hash(seed_s) & 1;
			uint32_t whichcast = item.Hash(seed_incast) % counter_num;
			int min_num = INT_MAX;
			uint32_t min_pos = -1;

			for (uint32_t i = 0; i < slot_num; ++i) {
				if (counters[i] == 0) {
					items[i] = item;
					counters[i] = -1;
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

				int counter_val = std::abs(counters[i]);
				if (counter_val < min_num) {
					min_num = counter_val;
					min_pos = i;
				}
			}

			if (incast[whichcast] * COUNT[choice] >= int(min_num)) {
				if (counters[min_pos] < 0) {
					uint32_t min_choice = items[min_pos].Hash(seed_s) & 1;
					incast[items[min_pos].Hash(seed_incast) % counter_num] -= COUNT[min_choice] * counters[min_pos];
				}
				items[min_pos] = item;
				counters[min_pos] = min_num + 1;
			}
			incast[whichcast] += COUNT[choice];
		}

		int Query(const Data& item,uint32_t seed_s,uint32_t seed_incast)
		{
			uint32_t choice = item.Hash(seed_s) & 1;
			uint32_t whichcast = item.Hash(seed_incast) % counter_num;
			int retv = 0;

			for (uint32_t i = 0; i < slot_num; ++i) {
				if (items[i] == item) {
					return std::abs(counters[i]);
				}
			}
			return retv;
		}
	};

	WavingSketch(int mem, int _HIT, uint32_t _HASH_NUM = 3)
	    : BUCKET_NUM(mem/sizeof(Bucket)), HIT(_HIT), HASH_NUM(_HASH_NUM), LENGTH(mem * 8 * 1 / 2)
    {
        sep="\t";
        name=(char *)"WavingSketch";
        record = 0;
        bitset = new BitSet(LENGTH);
		buckets = new Bucket[BUCKET_NUM];
		memset(buckets, 0, BUCKET_NUM * sizeof(Bucket));
		rename(int(slot_num),int(counter_num));
        aae = are = pr = cr = 0;
		seed_choice=std::clock();
		sleep(1);
		seed_incast=std::clock();
		sleep(1);
		seed_s=std::clock();
		// std::printf("seed_choice: %d, seed_incast: %d, seed_s: %d\n", seed_choice, seed_incast, seed_s);
	}
	~WavingSketch() { delete[] buckets; }

	void Init(const Data& item, uint time) 
    {
        if(time > record)
        {
            record = time;
            bitset->Clear();
        }

        bool init = true;
        for(uint i = 0;i < HASH_NUM;++i)
        {
            uint position = item.Hash(i) % LENGTH;
            if(!bitset->Get(position)){
                init = false;
                bitset->Set(position);
            }
        }

        if (!init)
		{
            uint32_t bucket_pos = item.Hash(seed_choice) % BUCKET_NUM;
            buckets[bucket_pos].Insert(item,seed_s,seed_incast);
        }
	}
    

	int Query(const Data& item) {
		uint32_t bucket_pos = item.Hash(seed_choice) % BUCKET_NUM;
		return buckets[bucket_pos].Query(item,seed_s,seed_incast);
	}


    void Check(HashMap mp){
        HashMap::iterator it;
        int value = 0, all = 0, hit = 0, size = 0;
        for(it = mp.begin();it != mp.end();++it){
            value = Query(it->first);
            if(it->second > HIT){
                all++;
                if(value > HIT){
                    hit += 1;
                    aae += abs(it->second - value);
                    are += abs(it->second - value) / (double)it->second;
                }
            }
            if(value > HIT)
                size += 1;
        }
        aae /= hit; are /= hit; cr = hit / (double)all;
        pr = hit / (double)size;
    }


private:
	Bucket *buckets;
	uint32_t BUCKET_NUM;
    uint record;
    BitSet* bitset;
    const uint HASH_NUM;
    const uint LENGTH;
	uint32_t seed_choice;
	uint32_t seed_s;
	uint32_t seed_incast;
    int HIT;
};

class Count_Bucket : public Abstract{
public:

    struct Bucket{
        data_type items[slot_num];
        count_type counters[slot_num];
        std::bitset<slot_num> real;
        count_type incast;

        void Insert(const data_type item){
            uint32_t choice = hash(item, 17) & 1;
            count_type min_num = INT_MAX;
            uint32_t min_pos = -1;
            
            for(uint32_t i = 0;i < slot_num;++i){
                if(counters[i] == 0){
                    items[i] = item;
                    counters[i] = 1;
                    real[i] = 1;
                    return;
                }
                else if(items[i] == item){
                    if(real[i])            
                        counters[i]++;
                    else{
                        counters[i]++;
                        incast += COUNT[choice];
                    }
                    return;
                }

                if(counters[i] < min_num){
                    min_num = counters[i];
                    min_pos = i;
                }
            }

            if(incast * COUNT[choice] >= int(min_num * factor)){
                if(real[min_pos]){
                    uint32_t min_choice = hash(items[min_pos], 17) & 1;
                    incast += COUNT[min_choice] * counters[min_pos];
                }
                items[min_pos] = item;
                counters[min_pos] += 1;
                real[min_pos] = 0;
            }
            incast += COUNT[choice];
        }

        count_type Query(const data_type item){
            uint32_t choice = hash(item, 17) & 1;

            for(uint32_t i = 0;i < slot_num;++i){
                if(items[i] == item){
                    return counters[i];
                }
            }

            return 0;
        }
    };

    Count_Bucket(uint32_t _MEM, int _HIT, uint32_t _HASH_NUM = 3):
    BUCKET_NUM(_MEM / (slot_num * (sizeof(data_type) + sizeof(count_type) + 1) + sizeof(count_type)) * 1 / 2), HASH_NUM(_HASH_NUM), LENGTH(_MEM * 8 * 1 / 2)
    {
        name = "Count_Bucket";
        HIT = _HIT;   
        record = 0;

        bitset = new BitSet(LENGTH);
        buckets = new Bucket[BUCKET_NUM];
        memset(buckets, 0, BUCKET_NUM * sizeof(Bucket));
        aae = are = pr = cr = 0;

    }
    ~Count_Bucket(){
        delete [] buckets;
    }

    void Init(const Data& data, uint time){
        if(time > record){
            record = time;
            bitset->Clear();
        }

        bool init = true;
        for(uint i = 0;i < HASH_NUM;++i){
            uint position = data.Hash(i) % LENGTH;
            if(!bitset->Get(position)){
                init = false;
                bitset->Set(position);
            }
        }

        if(!init)
        {
            uint32_t bucket_pos = hash(data) % BUCKET_NUM;
            buckets[bucket_pos].Insert(data);
        }
    }

    int Query(const data_type &item){
        uint32_t bucket_pos = hash(item) % BUCKET_NUM;
        return buckets[bucket_pos].Query(item);
    }

    void Check(HashMap mp){
        HashMap::iterator it;
        int value = 0, all = 0, hit = 0, size = 0;
        for(it = mp.begin();it != mp.end();++it){
            value = Query(it->first);
            if(it->second > HIT){
                all++;
                if(value > HIT){
                    hit += 1;
                    aae += abs(it->second - value);
                    are += abs(it->second - value) / (double)it->second;
                }
            }
            if(value > HIT)
                size += 1;
        }
        aae /= hit; are /= hit; cr = hit / (double)all;
        pr = hit / (double)size;
    }

private:
    Bucket* buckets;
    const uint32_t BUCKET_NUM;
    uint record;


    BitSet* bitset;
    const uint HASH_NUM;
    const uint LENGTH;
};

#endif // WAVING_H
