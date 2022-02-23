#ifndef OLF_H
#define OLF_H

#include <random>
#include "abstract.h"

using namespace std;

class OLF : public Abstract{
public:
    OLF(int num, int _HIT){
        HIT = _HIT;
        aae = are = pr = cr = 0;
        name = "OLF";
        sep="\t\t\t";
        epsilon = num / 100000 * 0.0011;
    }
    ~OLF(){
        // cout << name << ":" << endl;
        // cout << 40 * mp.size() + 60 * sp.size() << endl;
    }

    void Init(const Data& from, const Data& to){
        uint Max = 0xffffffff;
        Stream stream(from, to);
        if(stream.Hash() / (double)Max < epsilon && sp.find(stream) == sp.end()){
            sp[stream] = 1;
            if(mp.find(from) == mp.end())
                mp[from] = 1;
            else
                mp[from] += 1;
        }
    }

    int Query(const Data &data){
        if(mp.find(data) == mp.end())
            return -1;
        return (int)(mp[data] / epsilon);
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
    double epsilon;
    HashMap mp;
    StreamMap sp;
};

#endif // OLF_H
