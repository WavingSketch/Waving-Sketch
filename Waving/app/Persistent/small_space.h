#ifndef SMALL_SPACE_H
#define SMALL_SPACE_H

#include <string.h>
#include <algorithm>
#include "data.h"
#include "abstract.h"
#include "definition.h"

struct Info{
    uint time;
    int count;
    Info(uint _time = 0, int _count = 0){
        time = _time;
        count = _count;
    }
};

typedef unordered_map<Data, Info, My_Hash> SS_Map;

class Small_Space : public Abstract{
public:
    Small_Space(int num, int _HIT){
        uint Max = 0xffffffff;
        EPSILON = 0.00027 * num / 200000;
        HIT = _HIT;
        map.clear();
        aae = are = pr = cr = 0;
        sep="\t\t";
        name = "Small-Space";
    }
    ~Small_Space() = default;

    int Size(){
        return map.size();
    }

    void Init(const Data& data, uint time){
        if(map.find(data) != map.end()){
            if(map[data].time < time){
                map[data].time = time;
                map[data].count += 1;
            }
        }
        else{
            uchar str[DATA_LEN + 4] = {0};
            memcpy(str, data.str, DATA_LEN);
            memcpy(str + DATA_LEN, &time, sizeof(time));
            uint Max = 0xffffffff;
            if(Hash::BOBHash32(str, DATA_LEN + 4) / (double)Max < EPSILON){
                map[data] = Info(time, 1);
            }
        }
    }

    void Check(HashMap mp){
        int value = 0, all = 0, hit = 0, size = 0;
        for(HashMap::iterator it = mp.begin();it != mp.end();++it){
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

    int Query(const Data& data){
        if(map.find(data) == map.end())
            return -1;
        return map[data].count + 1 / EPSILON;
    }

private:
    double EPSILON;
    SS_Map map;
};

#endif // SMALL_SPACE_H
