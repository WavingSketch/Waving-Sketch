#ifndef WAVING_H
#define WAVING_H

#include <random>
#include "abstract.h"
#include <unistd.h>
#include <time.h>
#include <algorithm>

using namespace std;

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

	WavingSketch(int mem, int _HIT)
	    : BUCKET_NUM(mem/sizeof(Bucket)), HIT(_HIT)
    {
        name=(char *)"WavingSketch";
		buckets = new Bucket[BUCKET_NUM];
		memset(buckets, 0, BUCKET_NUM * sizeof(Bucket));
		rename(int(slot_num),int(counter_num));
        sep="\t";
		seed_choice=std::clock();
		sleep(1);
		seed_incast=std::clock();
		sleep(1);
		seed_s=std::clock();
		// std::printf("seed_choice: %d, seed_incast: %d, seed_s: %d\n", seed_choice, seed_incast, seed_s);
	}
	~WavingSketch() { delete[] buckets; }

	void Init(const Data& item) {
		uint32_t bucket_pos = item.Hash(seed_choice) % BUCKET_NUM;
		buckets[bucket_pos].Insert(item,seed_s,seed_incast);
	}
    

	int Query(const Data& item, HashMap &mp) {
		uint32_t bucket_pos = item.Hash(seed_choice) % BUCKET_NUM;
		return buckets[bucket_pos].Query(item,seed_s,seed_incast);
	}


    void Check(HashMap mp, Abstract* another){
        HashMap::iterator it;
        int value = 0, all = 0, hit = 0, size = 0;
        for(it = mp.begin();it != mp.end();++it){
            value = abs((Query(it->first, mp) - another->Query(it->first, mp)));
            if(abs(it->second) > HIT){
                all++;
                if(value > HIT){
                    hit += 1;
                }
            }
            if(value > HIT)
                size += 1;
        }
        cr = hit / (double)all;
        pr = hit / (double)size;
        // cout << all << " " << size << endl;
        // cout << cr << " " << pr << endl;
    }


private:
	Bucket *buckets;
	uint32_t BUCKET_NUM;
	// uint32_t prev_BUCKET_NUM;
	uint32_t seed_choice;
	uint32_t seed_s;
	uint32_t seed_incast;
    int HIT;
};

class Interest : public Abstract{
public:
    Interest(int mem, int _HIT):HIT(_HIT){
        BUCKET_NUM = mem/(8*sizeof(Counter)+16*4);
        SIZE = BUCKET_NUM*8;
        counter = new Counter[SIZE];
        incast = (int(*) [16])malloc(BUCKET_NUM*32);
        memset(counter, 0, SIZE*sizeof(Counter));
        memset(incast, 0, BUCKET_NUM*sizeof(int));
        pr = cr = 0;
        name = "IttSketch(memory/20)";
    }
    ~Interest(){
        delete [] counter;
        delete [] incast;
    }

    void Init(const Data& data){
        uint bucket_pos = data.Hash() % BUCKET_NUM;
        uint incast_pos = data.Hash(17) % 16; 
        uint position = (bucket_pos << 3);

        int min_num = INT_MAX;
        uint min_pos = -1;
        for(uint i = 0;i < 8;++i){
            if(counter[position + i].count == 0){
                counter[position + i].Set(data, 1);
                return;
            }
            else if(counter[position + i].data == data){
                counter[position + i].count += 1;
                return;
            }

            if(counter[position + i].count < min_num){
                min_num = counter[position + i].count;
                min_pos = position + i;
            }
        }

        if(rd() % ( (min_num << 1) - incast[bucket_pos][incast_pos] + 1) == 0){
            counter[min_pos].Set(data, min_num + (incast[bucket_pos][incast_pos] / min_num));
            incast[bucket_pos][incast_pos] = 0;
        }
        else
            incast[bucket_pos][incast_pos] += 1;
    }

    int Query(const Data &data, HashMap &mp){
        uint position = ((data.Hash() % BUCKET_NUM) << 3);
        int min_num = INT_MAX;
        for(uint i = 0;i < 8;++i){
            if(counter[position + i].data == data)
                return counter[position + i].count;
            min_num = MIN(min_num, counter[position + i].count);
        }
        return 0;
    }

    void Check(HashMap mp, Abstract* another){
        HashMap::iterator it;
        int value = 0, all = 0, hit = 0, size = 0;
        for(it = mp.begin();it != mp.end();++it){
            value = abs((Query(it->first, mp) - another->Query(it->first, mp)));
            if(abs(it->second) > HIT){
                all++;
                if(value > HIT){
                    hit += 1;
                }
            }
            if(value > HIT)
                size += 1;
        }
        cr = hit / (double)all;
        pr = hit / (double)size;
    }

private:
    struct Counter{
        Data data;
        int count;
        void Set(const Data& dat, int num){
            data = dat;
            count = num;
        }
    };

    uint BUCKET_NUM;
    uint SIZE;
    random_device rd;

    Counter* counter;
    int (*incast)[16];
    int HIT;
};

#endif // WAVING_H
