#pragma once

#include "../abstract.h"

template<uint32_t slot_num>
class Heap_Counter : public Abstract{
public:

    struct Bucket{
        data_type items[slot_num];
        count_type counters[slot_num];
        bool real[slot_num];
        count_type incast;

        void Insert(const data_type item){
            uint32_t choice = hash(item, 17) & 1;
            count_type min_num = INT_MAX;
            uint32_t min_pos = -1;
            
            for(uint32_t i = 0;i < slot_num;++i){
                if(counters[i] == 0){
                    items[i] = item;
                    counters[i] = 1;
                    real[i] = true;
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

            if(incast * COUNT[choice] >= min_num){
                //count_type pre_incast = incast;
                if(real[min_pos]){
                    uint32_t min_choice = hash(items[min_pos], 17) & 1;
                    incast += COUNT[min_choice] * counters[min_pos];
                }
                items[min_pos] = item;
                counters[min_pos] += 1;
                real[min_pos] = false;
            }
            incast += COUNT[choice];

            /*
            if(incast * COUNT[choice] >= min_num * 4){
                //count_type pre_incast = incast;
                if(real[min_pos]){
                    uint32_t min_choice = hash(items[min_pos], 17) & 1;
                    incast += COUNT[min_choice] * counters[min_pos];
                }
                items[min_pos] = item;
                counters[min_pos] += 1;
                real[min_pos] = false;
            }
            incast += COUNT[choice];
            */
        }

        count_type Query(const data_type item){
            uint32_t choice = hash(item, 17) & 1;

            for(uint32_t i = 0;i < slot_num;++i){
                if(items[i] == item){
                    return counters[i];
                }
            }

            return 0;//incast * COUNT[choice];
        }
    };

    Heap_Counter(uint32_t _BUCKET_NUM):Abstract((char *)"Heap_Counter"), BUCKET_NUM(_BUCKET_NUM){
        buckets = new Bucket[BUCKET_NUM];
        memset(buckets, 0, BUCKET_NUM * sizeof(Bucket));
    }
    ~Heap_Counter(){
        delete [] buckets;
    }

    void Insert(const data_type item){
        uint32_t bucket_pos = hash(item) % BUCKET_NUM;
        buckets[bucket_pos].Insert(item);
    }

    count_type Query(const data_type item){
        uint32_t bucket_pos = hash(item) % BUCKET_NUM;
        return buckets[bucket_pos].Query(item);
    }

private:
    Bucket* buckets;
    const uint32_t BUCKET_NUM;

};