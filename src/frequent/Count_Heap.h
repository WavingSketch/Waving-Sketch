#ifndef SKETCH_COUNT_HEAP_H
#define SKETCH_COUNT_HEAP_H

#include "Heap.h"

template<uint32_t length, uint32_t hash_num>
class Count_Heap : public Abstract{
public:

    Count_Heap(uint32_t _SIZE):Abstract((char *)"Count_Heap"){
        heap = new Heap(_SIZE);
        counter = new count_type*[hash_num];
        for(uint32_t i = 0;i < hash_num;++i){
            counter[i] = new count_type[length];
            memset(counter[i], 0, sizeof(count_type) * length);
        }
    }
    ~Count_Heap(){
        for(uint32_t i = 0;i < hash_num;++i)
            delete [] counter[i];
        delete [] counter;
        delete heap;
    }

    void Insert(const data_type item){
        count_type result[hash_num];

        for(uint32_t i = 0;i < hash_num;++i){
            uint32_t position = hash_(item, i) % length;
            uint32_t choice = hash_(item, i + 101) & 1;
            counter[i][position] += COUNT[choice];
            result[i] = counter[i][position] * COUNT[choice];
        }

        heap->Insert(item, Get_Median(result, hash_num));
    }

    count_type Query(const data_type item){
        return heap->Query(item);
    }

private:
    Heap* heap;
    count_type** counter;
};

#endif //SKETCH_COUNT_HEAP_H
