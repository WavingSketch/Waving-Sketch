#ifndef PIE_H
#define PIE_H

#include <string.h>
#include <algorithm>
#include "data.h"
#include "abstract.h"
#include "definition.h"

#define RAPTOR_SIZE 1
#define PRINT_SIZE 14
#define MASK_PRINT 0x3fff

class PIE : public Abstract{
public:
    PIE(int num, int _HIT, uint _CYCLE):CYCLE(_CYCLE){
        HIT = _HIT;
        LENGTH = num / _CYCLE / 6;
        counter = new Counter[LENGTH * CYCLE];
        memset(counter, 0, sizeof(Counter) * LENGTH * CYCLE);
        aae = are = pr = cr = 0;
        sep="\t\t\t";
        name = "PIE";
    }
    ~PIE(){
        delete []counter;
    }
    void Init(const Data& data, uint time){
        Counter temp = Encode(data, time);
        uint position = data.Hash() % LENGTH;
        position += (time - 1) * LENGTH;
        if(counter[position].empty()){
            counter[position] = temp;
        }
        else if(!(counter[position] == temp)){
            counter[position].set_collision();
        }
    }

    int Query(const Data& data){
        int ret = 0;
        uint position = data.Hash() % LENGTH;
        for(uint time = 0;time < CYCLE;++time){
            uint pos = position + time * LENGTH;
            if(counter[pos].print == (data.Hash(1) & MASK_PRINT))
                ret += 1;
        }
        return ret;
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
    struct Counter{
        short flag : 1;
        short raptor : RAPTOR_SIZE;
        short print : PRINT_SIZE;
        bool empty(){
            return (flag == 0 && raptor == 0 && print == 0);
        }
        void set_collision(){
            flag = 1;
            raptor = -1;
            print = -1;
        }
        bool operator == (Counter count){
            return (raptor == count.raptor && print == count.print);
        }
    };

    uint LENGTH;
    const uint CYCLE;

    Counter* counter;

    Counter Encode(const Data& data, uint time){
        Counter temp;
        temp.flag = 1;
        temp.print = (data.Hash(1) & MASK_PRINT);
        srand(time);
        int len = DATA_LEN * 8;
        bool coeff[len];
        bool id[len];

        for(int i = 0;i < len;++i)
            coeff[i] = (rand() & 1);
        for(int i = 0;i < DATA_LEN;++i){
            for(int j = 0;j < 8;++j)
                id[i * 8 + j] = (data.str[i] & (7 - j));
        }
        temp.raptor = 0;
        for(int i = 0;i < len;++i){
                temp.raptor ^= (coeff[i] & id[i]);
        }
        return temp;
    }
};

#endif // PIE_H
