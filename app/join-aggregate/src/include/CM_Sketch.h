#include "MurmurHash.h"
# include "params.h"
# include <iostream>
# include <string.h>
using namespace std;
class CM_Sketch:public Sketch{
public:
	int w, d;
	int* counter[MAX_HASH_NUM];
	int COUNTER_SIZE_MAX_CNT = (1LL << (COUNTER_SIZE - 1)) - 1;
	int hash_seed;
	int index[MAX_HASH_NUM];    //index of each d

public:
	CM_Sketch(int _w, int _d, int _hash_seed = 1000){
		w = _w/4/_d, d = _d;
		hash_seed=_hash_seed;
		for (int i = 0; i < d; i++)counter[i] = new int[w]();
	}
	void Insert(const char* str){
		int temp = 0, min_value = COUNTER_SIZE_MAX_CNT;
		for (int i = 0; i < d; i++){
			index[i] = MurmurHash32(str, KEY_LEN, hash_seed + i) % w;
			counter[i][index[i]]++;
		}
	}
	int Query(const char* str)const{
		int temp = 0, min_value = COUNTER_SIZE_MAX_CNT;
		int index[MAX_HASH_NUM];
		for (int i = 0; i < d; i++){
			index[i] =  MurmurHash32(str, KEY_LEN, hash_seed + i) % w;
			temp = counter[i][index[i]];
			min_value = temp < min_value ? temp : min_value;
		}
		return min_value;
	}

	void Delete(const char* str){
		for (int i = 0; i < d; i++){
			index[i] =  MurmurHash32(str, KEY_LEN, hash_seed) % w;
			counter[i][index[i]] --;
		}
	}
	long double Join(Sketch*_other){
		const CM_Sketch* other=(CM_Sketch*)_other;
		long double re=0;
		for (int i = 0; i < d; i++){
			long double k=0;
			for (int j = 0; j < w; j++)
				k+=1.0*counter[i][j]*other->counter[i][j];
			if(i==0||k<re)re=k;
		}
		return re;
	}
	~CM_Sketch()
	{
		for (int i = 0; i < d; i++){
			delete[]counter[i];
		}
	}
};
