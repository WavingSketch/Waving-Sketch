#ifndef DATA_H
#define DATA_H

/*
 * consistent data structure for all Sketch
 */

#include <limits.h>
#include <unordered_map>
#include <cstring>
#include "hash.h"
#include "definition.h"

#define DATA_LEN 8 //sizeof(Data)

using namespace std;

class Data{
public:
    uchar str[DATA_LEN];

    Data& operator = (const Data& an){
        memcpy(str, an.str, DATA_LEN);
        return *this;
    }

    bool operator < (const Data& an) const{
        return memcmp(str, an.str, DATA_LEN) < 0;
    }

    bool operator == (const Data& an) const{
        return memcmp(str, an.str, DATA_LEN) == 0;
    }

    uint Hash(uint num = 0) const{
        return Hash::BOBHash32(str, DATA_LEN, num);
    } //the hash num of the data
};

//defined for unordered_map
class My_Hash{
public:
    uint operator()(const Data& dat) const{
        return dat.Hash();
    }
};

typedef unordered_map<Data, int, My_Hash> HashMap;

#endif // DATA_H
