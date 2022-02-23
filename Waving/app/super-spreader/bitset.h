#ifndef BITSET_H
#define BITSET_H

/*
 * My bitset
 */

#include <string.h>
#include "definition.h"

class BitSet
{
public:
    BitSet(uint _LENGTH = 128):LENGTH(_LENGTH){
        uint size = (_LENGTH >> 3) + 1;
        bitset = new uchar[size];
        memset(bitset, 0, size * sizeof(uchar));
    }
    ~BitSet(){
        delete [] bitset;
    }
    void Set(uint index){
        uint position = (index >> 3);
        uint offset = (index & 0x7);
        bitset[position] |= (1 << offset);
    } //Set the index bit to 1
    void Clear(){
        uint size = (LENGTH >> 3) + 1;
        memset(bitset, 0, size * sizeof(uchar));
    } //Set all bit to 0
    void Clear(uint index){
        uint position = (index >> 3);
        uint offset = (index & 0x7);
        uchar mask = 0xff;
        mask ^= (1 << offset);
        bitset[position] &= mask;
    } //Set the index bit to 0
    bool Get(uint index){
        uint position = (index >> 3);
        uint offset = (index & 0x7);
        if((bitset[position] & (1 << offset)) != 0)
            return true;
        return false;
    } //return the index bit

private:
    const uint LENGTH;
    uchar* bitset;
};

#endif // BITSET_H
