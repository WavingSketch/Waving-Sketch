#ifndef _CSKETCH_H
#define _CSKETCH_H
#include "params.h"
#include <string.h>
#include <iostream>
#include "MurmurHash.h"
#include <random>
#include <mmintrin.h>
#include <algorithm>
using namespace std;

class C_Sketch:public Sketch
{
public:
	int w, d;
	int index[MAX_HASH_NUM];
	int* counter[MAX_HASH_NUM];
	int MAX_CNT, MIN_CNT, hash_seed;

public:
	C_Sketch(int _w, int _d, int _hash_seed = 1000)
	{
		d = _d, w = _w/4/_d;
		// printf("%d\n",w);
		hash_seed = _hash_seed;
		for (int i = 0; i < d; i++)
		{
			counter[i] = new int[w];
			memset(counter[i], 0, sizeof(int) * w);
		}

		MAX_CNT = (1 << (COUNTER_SIZE - 1)) - 1;
		MIN_CNT = (-(1 << (COUNTER_SIZE - 1)));
	}

	void Insert(const char* str)
	{
		int g = 0;
		for (int i = 0; i < d; i++)
		{
			index[i] = ((unsigned)MurmurHash32(str, KEY_LEN, hash_seed + i)) % w;
			g =((unsigned) MurmurHash32(str, KEY_LEN, hash_seed + i + d)) % 2;

			if (g == 0)
			{
				if (counter[i][index[i]] != MAX_CNT)
				{
					counter[i][index[i]]++;
				}
			}
			else
			{
				if (counter[i][index[i]] != MIN_CNT)
				{
					counter[i][index[i]]--;
				}
			}
		}
	}

	void Delete(const char* str) {
		int g = 0;
		for (int i = 0; i < d; i++)
		{
			index[i] = ((unsigned)MurmurHash32(str, KEY_LEN, hash_seed + i)) % w;
			g = ((unsigned)MurmurHash32(str, KEY_LEN, hash_seed + i + d)) % 2;

			if (g == 1)
			{
				if (counter[i][index[i]] != MAX_CNT)
				{
					counter[i][index[i]]++;
				}
			}
			else
			{
				if (counter[i][index[i]] != MIN_CNT)
				{
					counter[i][index[i]]--;
				}
			}
		}
	}
	long double Join(Sketch*_other){
		const C_Sketch* other=(C_Sketch*)_other;
		long double res[MAX_HASH_NUM];
		for (int i = 0; i < d; i++){
			long double k=0;
			for (int j = 0; j < w; j++)
				k+=1ll*counter[i][j]*other->counter[i][j];
			res[i]=k;
		}
		sort(res, res + d);
		if (d % 2 == 0)
			return ((res[d / 2] + res[d / 2 - 1]) / 2);
		else
			return (res[d / 2]);
	}

	int Query(const char* str)const
	{
		int temp;
		int res[MAX_HASH_NUM];
		int index[MAX_HASH_NUM];
		int g;
		for (int i = 0; i < d; i++)
		{
			index[i] = ((unsigned)MurmurHash32(str, KEY_LEN, hash_seed + i)) % w;
			temp = counter[i][index[i]];
			g = ((unsigned)MurmurHash32(str, KEY_LEN, hash_seed + i + d)) % 2;

			res[i] = (g == 0 ? temp : -temp);
		}

		sort(res, res + d);
		if (d % 2 == 0)
			return ((res[d / 2] + res[d / 2 - 1]) / 2);
		else
			return (res[d / 2]);
	}

	~C_Sketch()
	{
		for (int i = 0; i < d; i++)
			delete[]counter[i];
	}
};




#endif
