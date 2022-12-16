#include "MurmurHash.h"
# include "params.h"
# include <iostream>
# include <string.h>
#include <algorithm>
#include <unordered_map>
#include <map>
#include <immintrin.h>
#include <stdexcept>
#ifdef UNIX
#include <x86intrin.h>
#else
#include <emmintrin.h>
#endif
using namespace std;
extern int Heavy_Thes;
extern int BILI;
inline int SIMD_match_8(uint32_t *ID,uint32_t key){
	const __m128i item = _mm_set1_epi32((int)key);
	int matched;
	__m128i *keys_p = (__m128i *)ID;
	__m128i a_comp = _mm_cmpeq_epi32(item, keys_p[0]);
	__m128i b_comp = _mm_cmpeq_epi32(item, keys_p[1]);
	a_comp = _mm_packs_epi32(a_comp, b_comp);
	matched = _mm_movemask_epi8(a_comp);
	if (matched != 0) {
		int matched_index = __builtin_ctz((uint32_t)matched)>>1;
		return matched_index;
    }else return -1;
}
struct Hash_table{
	int n,hash_seed;const int m=8;
	unsigned **Id,**Key;
	Hash_table(int bucket_sum,int _hash_seed=1000){
		n=bucket_sum;
		Id=new unsigned*[n]();
		for(int i=0;i<n;i++)
			Id[i]=new unsigned[m]();
		Key=new unsigned*[n]();
		for(int i=0;i<n;i++)
			Key[i]=new unsigned[m]();
		for(int i=0;i<n;i++)
		for(int j=0;j<m;j++)
		Id[i][j]=Key[i][j]=0;
	}
	bool count(unsigned id)const{
		unsigned i=MurmurHash32(&id, 4, hash_seed)%n;
		if(SIMD_match_8(Id[i],id)>=0)return 1;
		i=MurmurHash32(&id, 4, hash_seed*2)%n;
		if(SIMD_match_8(Id[i],id)>=0)return 1;
		return 0;
	}
	int query(unsigned id)const{
		unsigned i=MurmurHash32(&id, 4, hash_seed)%n;
		int j=SIMD_match_8(Id[i],id);
		if(j>=0)return Key[i][j];
		i=MurmurHash32(&id, 4, hash_seed*2)%n;
		j=SIMD_match_8(Id[i],id);
		if(j>=0)return Key[i][j];
		return -1;
	}
	void insert(unsigned id,unsigned cnt=1){
		unsigned i=MurmurHash32(&id, 4, hash_seed)%n;
		int j;
		j=SIMD_match_8(Id[i],id);
		if(j>=0){
			Key[i][j]+=cnt;
			return;
		}
		i=MurmurHash32(&id, 4, hash_seed*2)%n;
		j=SIMD_match_8(Id[i],id);
		if(j>=0){
			Key[i][j]+=cnt;
			return;
		}
		for(int j=0;j<m;j++)
		if(!solid(i,j)){
			Id[i][j]=id;
			Key[i][j]=cnt;
			return;
		}
		i=MurmurHash32(&id, 4, hash_seed)%n;
		for(int j=0;j<m;j++)
		if(!solid(i,j)){
			Id[i][j]=id;
			Key[i][j]=cnt;
			return;
		}
		moretime:Expansion();
		i=MurmurHash32(&id, 4, hash_seed)%n;
		for(int j=0;j<m;j++)
		if(!solid(i,j)){
			Id[i][j]=id;
			Key[i][j]=cnt;
			return;
		}
		i=MurmurHash32(&id, 4, hash_seed*2)%n;
		for(int j=0;j<m;j++)
		if(!solid(i,j)){
			Id[i][j]=id;
			Key[i][j]=cnt;
			return;
		}
		goto moretime;
		exit(1);
	}
	bool solid(unsigned i,unsigned j){
		if(Key[i][j]==0)return 0;
		unsigned id=Id[i][j];
		unsigned ii=MurmurHash32(&id, 4, hash_seed)%n;
		if(ii==i)return 1;
		ii=MurmurHash32(&id, 4, hash_seed*2)%n;
		if(ii==i)return 1;
		return 0;
	}
	void Expansion(){
		unsigned **nwid;
		unsigned **nwkey;
		nwid=new unsigned*[n+n]();
		nwkey=new unsigned*[n+n]();
		for(int i=0;i<n;i++){
			nwid[i]=Id[i];
			nwkey[i]=Key[i];
		}
		for(int i=0;i<n;i++){
			nwid[n+i]=new unsigned[m]();
			nwkey[n+i]=new unsigned[m]();
			memmove(nwid[n+i],nwid[i],m*sizeof(unsigned));
			memmove(nwkey[n+i],nwkey[i],m*sizeof(unsigned));
		}
		delete[]Id;
		delete[]Key;
		Id=nwid;
		Key=nwkey;
		n=n+n;
	}
	~Hash_table()
	{
		for (int i = 0; i < n; i++){
			delete[]Id[i];
			delete[]Key[i];
		}
		delete[]Id;
		delete[]Key;
	}
};

class Short_C_Sketch
{
public:
	int w, d;
	int index[MAX_HASH_NUM];
	int* counter[MAX_HASH_NUM];
	int MAX_CNT, MIN_CNT, hash_seed;
public:
	Short_C_Sketch(int _w, int _d, int _hash_seed = 1000)
	{
		d = _d, w = _w/_d/2;
		hash_seed = _hash_seed;
		for (int i = 0; i < d; i++)
		{
			counter[i] = new int[w]();
			memset(counter[i], 0, sizeof(int) * w);
		}

		MAX_CNT = (1 << (32 - 1)) - 1;
		MIN_CNT = (-(1 << (32 - 1)));
	}

	void Insert(unsigned id,int k=1)
	{
		int g = 0;
		for (int i = 0; i < d; i++)
		{
			index[i] = ((unsigned)MurmurHash32(&id, 4, hash_seed + i)) % w;
			g =((unsigned) MurmurHash32(&id, 4, hash_seed + i + d)) % 2;
			if (g == 0)
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
	long double Join(Short_C_Sketch*other){
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

	int Query(unsigned id)const
	{
		int temp;
		int res[MAX_HASH_NUM];
		int index[MAX_HASH_NUM];
		int g;
		for (int i = 0; i < d; i++)
		{
			index[i] = ((unsigned)MurmurHash32(&id, 4, hash_seed + i)) % w;
			temp = counter[i][index[i]];
			g = ((unsigned)MurmurHash32(&id, 4, hash_seed + i + d)) % 2;

			res[i] = (g == 0 ? temp : -temp);
		}

		sort(res, res + d);
		if (d % 2 == 0)
			return ((res[d / 2] + res[d / 2 - 1]) / 2);
		else
			return (res[d / 2]);
	}

	~Short_C_Sketch()
	{
		for (int i = 0; i < d; i++)
			delete[]counter[i];
	}
};

class Classifier:public Sketch
{
public:
	int n;const int m=8;
	unsigned **Mid;
	int hash_seed;
	Short_C_Sketch *Light;
	Hash_table *Heavy;
public:
	Classifier(int _w, int _d, int _hash_seed = 1000)
	{
		int heavysize=_w/8;
		heavysize=heavysize/64;
		Heavy=new Hash_table(heavysize,_hash_seed*2);
		_w-=heavysize*8*8;
		int midsize=_w/2;
		midsize/=4;
		n=midsize/m;
		midsize=n*m;
		Mid=new unsigned*[n];
		for(int i=0;i<n;i++)
		Mid[i]=new unsigned[m]();
		_w-=midsize*4;
		hash_seed=_hash_seed*3;
		
		Light=new Short_C_Sketch(_w,_d,_hash_seed);
	}
	#define Mid_Id(x) (x>>10)
	#define Mid_Key(x) (x&1023)
	void Insert(const unsigned id){
		if(Heavy->count(id)){Heavy->insert(id);return;}
		int MdId=Mid_Id(id)<<10;
		unsigned k=MurmurHash32(&MdId, 4, hash_seed)%n;
		unsigned*bucket=Mid[k];
		for(int i=0;i<m;i++)
		if(Mid_Id(bucket[i])==Mid_Id(id)||Mid_Key(bucket[i])==0){
			bucket[i]=(bucket[i]+1)&1023;
			bucket[i]|=MdId;
			if(Mid_Key(bucket[i])==Heavy_Thes){
				Heavy->insert(id,Mid_Key(bucket[i]));
				bucket[i]=0;
			}
			return;
		}
		int to=0,mn=Mid_Key(bucket[0]);
		for(int i=1;i<m;i++)
		if(Mid_Key(bucket[i])<mn){
			mn=Mid_Key(bucket[i]);
			to=i;
		}
		Light->Insert(Mid_Id(bucket[to]),Mid_Key(bucket[to]));
		bucket[to]=1;
		bucket[to]|=MdId;
		return;
	}
	void Insert(const char* str)
	{
		unsigned id=((unsigned*)str)[0];
		Insert(id);
		return;
	}
	bool CheckHeavy(const char* str){
		unsigned id=((unsigned*)str)[0];
		return Heavy->count(id);
	}
	long double Join(Sketch*_other){
		Classifier* other=(Classifier*)_other;
		long double re=Light->Join(other->Light);
		for(int i=0;i<Heavy->n;i++)
		for(int j=0;j<Heavy->m;j++)
		if(Heavy->solid(i,j)){
			re+=1.0*Heavy->Key[i][j]*other->Query(Heavy->Id[i][j]);
		}
		for(int i=0;i<other->Heavy->n;i++)
		for(int j=0;j<other->Heavy->m;j++)
		if(other->Heavy->solid(i,j)){
			re+=1.0*QueryMid(other->Heavy->Id[i][j])*other->Heavy->Key[i][j];
		}
		for(int i=0;i<n;i++)
		for(int j=0;j<m;j++)
		if(Mid[i][j]!=0){
			re+=1.0*Mid_Key(Mid[i][j])*other->QueryMid(Mid[i][j]);
		}
		for(int i=0;i<n;i++)
		for(int j=0;j<m;j++)
		if(other->Mid[i][j]!=0){
			re+=1.0*Mid_Key(other->Mid[i][j])*Light->Query(Mid_Id(other->Mid[i][j]));
		}
		return re;
	}
	int QueryMid(const unsigned id){
		int re=Light->Query(Mid_Id(id));
		int MdId=Mid_Id(id)<<10;
		unsigned k=MurmurHash32(&MdId, 4, hash_seed)%n;
		unsigned*bucket=Mid[k];
		for(int i=0;i<m;i++)
		if(Mid_Id(bucket[i])==Mid_Id(id))re+=Mid_Key(bucket[i]);
		return re;
	}
	int Query(const unsigned id)const{
		int re=Light->Query(Mid_Id(id));
		if(Heavy->count(id))re+=Heavy->query(id);
		int MdId=Mid_Id(id)<<10;
		unsigned k=MurmurHash32(&MdId, 4, hash_seed)%n;
		unsigned*bucket=Mid[k];
		for(int i=0;i<m;i++)
		if(Mid_Id(bucket[i])==Mid_Id(id))re+=Mid_Key(bucket[i]);
		return re;
	}
	int Query(const char* str)const
	{
		// unsigned id= (unsigned)MurmurHash32(str, KEY_LEN, fingerprint);
		unsigned id=((unsigned*)str)[0];
		return Query(id);
	}
	~Classifier()
	{
		for (int i = 0; i < n; i++)
			delete[]Mid[i];
		delete[]Mid;
		Heavy->~Hash_table();
		Light->~Short_C_Sketch();
	}
};