#ifndef INTEREST_H
#define INTEREST_H

#include <random>
#include "abstract.h"

using namespace std;

class Interest : public Abstract{
public:
    Interest(int num, int _HIT):HIT(_HIT){
        BUCKET_NUM = 1000 + 250 * (num - 1);
        SIZE = (BUCKET_NUM << 3);
        counter = new Counter[SIZE];
        incast = new int [BUCKET_NUM];
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

        if(rd() % ( (min_num << 1) - incast[bucket_pos] + 1) == 0){
            counter[min_pos].Set(data, min_num + (incast[bucket_pos] / min_num));
            incast[bucket_pos] = 0;
        }
        else
            incast[bucket_pos] += 1;
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
    int* incast;
    int HIT;
};

#endif // INTEREST_H
