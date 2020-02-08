#pragma once

#include "abstract.h"

#define factor 1

template<uint32_t slot_num>
class WavingSketch : public Abstract{
public:

    struct Bucket{
        data_type items[slot_num];
        count_type counters[slot_num];
        count_type incast;

        void Insert(const data_type item){
            uint32_t choice = hash_(item, 17) & 1;
            count_type min_num = INT_MAX;
            uint32_t min_pos = -1;
            
            for(uint32_t i = 0;i < slot_num;++i){
                if(counters[i] == 0){
                    items[i] = item;
                    counters[i] = -1;
                    return;
                }
                else if(items[i] == item){
                    if(counters[i] < 0)
                        counters[i]--;
                    else{
                        counters[i]++;
                        incast += COUNT[choice];
                    }
                    return;
                }

                count_type counter_val = std::abs(counters[i]);
                if(counter_val < min_num){
                    min_num = counter_val;
                    min_pos = i;
                }
            }

            if(incast * COUNT[choice] >= int(min_num * factor)){
                if(counters[min_pos] < 0){
                    uint32_t min_choice = hash_(items[min_pos], 17) & 1;
                    incast -= COUNT[min_choice] * counters[min_pos];
                }
                items[min_pos] = item;
                counters[min_pos] = min_num + 1;
            }
            incast += COUNT[choice];
        }

        count_type Query(const data_type item){
            uint32_t choice = hash_(item, 17) & 1;

            for(uint32_t i = 0;i < slot_num;++i){
                if(items[i] == item){
                    return std::abs(counters[i]);
                }
            }

            return 0;//incast * COUNT[choice];
        }
    };

    WavingSketch(uint32_t _BUCKET_NUM):Abstract((char *)"WavingSketch"), BUCKET_NUM(_BUCKET_NUM){
        buckets = new Bucket[BUCKET_NUM];
        memset(buckets, 0, BUCKET_NUM * sizeof(Bucket));
    }
    ~WavingSketch(){
        delete [] buckets;
    }

    void Insert(const data_type item){
        uint32_t bucket_pos = hash_(item) % BUCKET_NUM;
        buckets[bucket_pos].Insert(item);
    }

    count_type Query(const data_type item){
        uint32_t bucket_pos = hash_(item) % BUCKET_NUM;
        return buckets[bucket_pos].Query(item);
    }

private:
    Bucket* buckets;
    const uint32_t BUCKET_NUM;

};