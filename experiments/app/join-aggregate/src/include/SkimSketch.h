#include "params.h"
#include <string.h>
#include <iostream>
#include "MurmurHash.h"
#include <random>
#include <mmintrin.h>
#include <algorithm>
#include <unordered_map>
using namespace std;

extern int Heavy_Thes;
class SkimSketch:public Sketch
{
public:
	int w, d;
	int index[MAX_HASH_NUM];
	int* counter[MAX_HASH_NUM];
	int MAX_CNT, MIN_CNT, hash_seed;
	
	unordered_map<string, int>id,heavy;
public:
	SkimSketch(int _w, int _d, int _hash_seed = 1000)
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
		id[string(str, KEY_LEN)]=1;
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

	void Delete(const char* str,int k=1) {
		int g = 0;
		for (int i = 0; i < d; i++)
		{
			index[i] = ((unsigned)MurmurHash32(str, KEY_LEN, hash_seed + i)) % w;
			g = ((unsigned)MurmurHash32(str, KEY_LEN, hash_seed + i + d)) % 2;

			if (g == 1)
			{
				if (counter[i][index[i]] != MAX_CNT)
				{
					counter[i][index[i]]+=k;
				}
			}
			else
			{
				if (counter[i][index[i]] != MIN_CNT)
				{
					counter[i][index[i]]-=k;
				}
			}
		}
	}
	void fin(){
		for(auto x:id){
			int k=Query(x.first.c_str());
			if(k>=10*Heavy_Thes){
				Delete(x.first.c_str(),k);
				heavy[x.first]=k;
			}
		}
	}
	long double Join(Sketch*_other){
		SkimSketch* other=(SkimSketch*)_other;
		fin();
		other->fin();
		long double hyre=0;
		for(auto x:heavy){
			if(other->heavy.count(x.first)){
				hyre+=x.second*other->heavy[x.first];
			}else{
				Delete(x.first.c_str(),-x.second);
				// hyre+=x.second*other->Query(x.first.c_str());
			}
		}
		for(auto x:other->heavy)
		if(!heavy.count(x.first)){
			other->Delete(x.first.c_str(),-x.second);
			// hyre+=x.second*Query(x.first.c_str());
		}
		long double res[MAX_HASH_NUM];
		for (int i = 0; i < d; i++){
			long double k=0;
			for (int j = 0; j < w; j++)
				k+=1ll*counter[i][j]*other->counter[i][j];
			res[i]=k;
		}
		sort(res, res + d);
		if (d % 2 == 0)
			return hyre+((res[d / 2] + res[d / 2 - 1]) / 2);
		else
			return hyre+(res[d / 2]);
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

	~SkimSketch()
	{
		for (int i = 0; i < d; i++)
			delete[]counter[i];
	}
};
