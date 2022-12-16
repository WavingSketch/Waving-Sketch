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

#define DATA_LEN 4 //sizeof(Data)

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

class Stream{
public:
    uchar str[DATA_LEN << 1];

    Stream(const Data& an, const Data& bn){
        memcpy(str, an.str, DATA_LEN);
        memcpy(str + DATA_LEN, bn.str, DATA_LEN);
    }

    Stream& operator = (const Stream& an){
        memcpy(str, an.str, DATA_LEN << 1);
        return *this;
    }

    bool operator < (const Stream& an) const{
        return memcmp(str, an.str, DATA_LEN << 1) < 0;
    }

    bool operator == (const Stream& an) const{
        return memcmp(str, an.str, DATA_LEN << 1) == 0;
    }

    uint Hash(uint num = 0) const{
        return Hash::BOBHash32(str, DATA_LEN << 1, num);
    } //the hash num of the data
};

//defined for unordered_map

class Data_Hash{
public:
    uint operator()(const Data& dat) const{
        return dat.Hash();
    }
};

class Stream_Hash{
public:
    uint operator()(const Stream& dat) const{
        return dat.Hash();
    }
};

typedef unordered_map<Data, int, Data_Hash> HashMap;
typedef unordered_map<Stream, int, Stream_Hash> StreamMap;
typedef pair<int, Data> KV;

#endif // DATA_H
