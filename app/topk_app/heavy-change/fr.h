#ifndef FR_H
#define FR_H

#include "bitset.h"
#include "abstract.h"

class FR : public Abstract{
public:
    FR(int num, int _HIT):HIT(_HIT){
        SIZE = num / 17; 
        //SIZE = 118750 + 30000 * (num - 1);
        LENGTH = 8 * SIZE; 
        //LENGTH = 800000 + 160000 * (num - 1);
        flow = 0;
        bitset = new BitSet(LENGTH);
        counter = new Counter[SIZE];
        memset(counter, 0, sizeof(Counter) * SIZE);
        name = "FR";
        sep="\t\t\t";
    }
    ~FR(){
        delete bitset;
        delete []counter;
    }

    void Init(const Data& data){
        bool init = true;
        for(int i = 0;i < 3;++i){
            uint position = data.Hash(i) % LENGTH;
            if(!bitset->Get(position)){
                init = false;
                bitset->Set(position);
            }
        }

        for(int i = 0;i < 2;++i){
            uint position = data.Hash(i) % SIZE;
            if(!init){
                flow++;
                counter[position].data ^= data;
                counter[position].count += 1;
            }
            counter[position].packet += 1;
       }

    }

    int Query(const Data &data, HashMap& mp){
        while(true){
            bool flag = true;
            for(int i = 0;i < SIZE;++i){
                if(counter[i].count == 1){
                    Data data = counter[i].data;
                    int count = counter[i].packet;
                    mp[data] = count;

                    for(int j = 0;j < 2;++j){
                        uint position = data.Hash(j) % SIZE;
                        counter[position].count -= 1;
                        counter[position].data ^= data;
                        counter[position].packet -= count;
                    }
                    flag = false;
                }
            }
            if(flag) return 0;
        }
        return 0;
    }

    void Check(HashMap mp, Abstract* another){
        Data temp;
        HashMap first, second;
        Query(temp, first); another->Query(temp, second);
        HashMap::iterator it;
        int value = 0, all = 0, hit = 0, size = 0;
        for(it = mp.begin();it != mp.end();++it){
            value = abs(first[it->first] - second[it->first]);
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
    struct Counter{
        Data data;
        int count;
        int packet;
    };
    BitSet* bitset;
    Counter* counter;
    int flow;
    int LENGTH;
    int SIZE;
    int HIT;
};

#endif // FR_H
