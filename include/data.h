#ifndef DATA_H
#define DATA_H

/*
 * consistent data structure for all Sketch
 */

#include <limits.h>

#include <cstring>
#include <unordered_map>

#include "definition.h"
#include "hash.h"

using namespace std;

template<uint32_t DATA_LEN>
class Data {
public:
	uchar str[DATA_LEN];
	Data() { memset(str, 0, DATA_LEN); }
	Data(const char* s) {
		memset(str, 0, DATA_LEN);
		if (s != NULL)
		{
			copy(s, s + min(DATA_LEN, (uint32_t)strlen(s)), str);
		}
	}
	Data<DATA_LEN>& operator=(const Data<DATA_LEN>& an) {
		memcpy(str, an.str, DATA_LEN);
		return *this;
	}

	bool operator<(const Data<DATA_LEN>& an) const {
		return memcmp(str, an.str, DATA_LEN) < 0;
	}

	bool operator==(const Data<DATA_LEN>& an) const {
		return memcmp(str, an.str, DATA_LEN) == 0;
	}

	void operator^=(const Data<DATA_LEN>& an) {
		for (int i = 0; i < DATA_LEN; ++i) {
			str[i] ^= an.str[i];
		}
	}

	uint Hash(uint num = 0) const {
		return MurmurHash3_x86_32(str, DATA_LEN, num);
	}  // the hash num of the data
};
template<uint32_t DATA_LEN>
class Stream {
public:
	uchar str[DATA_LEN << 1];

	Stream(const Data<DATA_LEN>& an, const Data<DATA_LEN>& bn) {
		memcpy(str, an.str, DATA_LEN);
		memcpy(str + DATA_LEN, bn.str, DATA_LEN);
	}

	Stream<DATA_LEN>& operator=(const Stream<DATA_LEN>& an) {
		memcpy(str, an.str, DATA_LEN << 1);
		return *this;
	}

	bool operator<(const Stream<DATA_LEN>& an) const {
		return memcmp(str, an.str, DATA_LEN << 1) < 0;
	}

	bool operator==(const Stream<DATA_LEN>& an) const {
		return memcmp(str, an.str, DATA_LEN << 1) == 0;
	}

	uint Hash(uint num = 0) const {
		return Hash::BOBHash32(str, DATA_LEN << 1, num);
	}  // the hash num of the data
};
// defined for unordered_map
template<uint32_t DATA_LEN>
class My_Hash {
public:
	uint operator()(const Data<DATA_LEN>& dat) const { return dat.Hash(); }
};
template<uint32_t DATA_LEN>
class Stream_Hash {
public:
	uint operator()(const Stream<DATA_LEN>& dat) const { return dat.Hash(); }
};

template<uint32_t DATA_LEN>
using KV = pair<int, Data<DATA_LEN>>;

template<uint32_t DATA_LEN>
using HashMap = unordered_map<Data<DATA_LEN>, int, My_Hash<DATA_LEN>>;
template<uint32_t DATA_LEN>
using  StreamMap = unordered_map<Stream<DATA_LEN>, int, Stream_Hash<DATA_LEN>>;
#endif  // DATA_H
