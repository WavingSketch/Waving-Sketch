#pragma once

#include "abstract.h"
#include <unistd.h>
#include <time.h>
#include <algorithm>

#define factor 1

template < uint32_t slot_num, uint32_t counter_num >
class WavingSketch : public Abstract {
public:
	struct node // Defined as temporary data structure used in shrink
	{
		count_type counter;
		data_type item;
		node(): counter(0), item(0){};
		bool operator<(const node& tp) const
		{
			if (counter<0) // true
			{
				if (tp.counter<0)
					return std::abs(counter)>std::abs(tp.counter);
				else
					return 1;
			}
			else
			{
				if (tp.counter<0)
					return 0;
				else
					return std::abs(counter)>std::abs(tp.counter);
			}
		}
	};
	
	struct Bucket {
		data_type items[slot_num];
		count_type counters[slot_num];
		int16_t incast[counter_num];

		void Insert(const data_type item,uint32_t seed_s,uint32_t seed_incast) {
			uint32_t choice = hash(item, seed_s) & 1;
			uint32_t whichcast = hash(item, seed_incast) % counter_num;
			count_type min_num = INT_MAX;
			uint32_t min_pos = -1;

			for (uint32_t i = 0; i < slot_num; ++i) {
				if (counters[i] == 0) {
					// The error free item's counter is negative, which is a trick to 
					// be differentiated from items which are not error free.
					items[i] = item;
					counters[i] = -1;
					return;
				}
				else if (items[i] == item) {
					if (counters[i] < 0)
						counters[i]--;
					else {
						counters[i]++;
						incast[whichcast] += COUNT[choice];
					}
					return;
				}

				count_type counter_val = std::abs(counters[i]);
				if (counter_val < min_num) {
					min_num = counter_val;
					min_pos = i;
				}
			}

			if (incast[whichcast] * COUNT[choice] >= int(min_num * factor)) {
				if (counters[min_pos] < 0) {
					uint32_t min_choice = hash(items[min_pos], seed_s) & 1;
					incast[hash(items[min_pos], seed_incast) % counter_num] -= COUNT[min_choice] * counters[min_pos];
				}
				items[min_pos] = item;
				counters[min_pos] = min_num + 1;
			}
			incast[whichcast] += COUNT[choice];
		}

		count_type Query(const data_type item,uint32_t seed_s,uint32_t seed_incast)
		{
			uint32_t choice = hash(item, seed_s) & 1;
			uint32_t whichcast = hash(item, seed_incast) % counter_num;
			count_type retv = 0;

			for (uint32_t i = 0; i < slot_num; ++i) {
				if (items[i] == item) {
					return std::abs(counters[i]);
				}
			}
			return retv;
		}
	};

	WavingSketch(uint32_t _BUCKET_NUM)
	    : Abstract((char *)"WavingSketch"), BUCKET_NUM(_BUCKET_NUM) {
		buckets = new Bucket[BUCKET_NUM];
		memset(buckets, 0, BUCKET_NUM * sizeof(Bucket));
		rename(int(slot_num),int(counter_num));
		seed_choice=std::clock();
		sleep(1); // need to sleep for a while, or seed_choice and seed_incast might get the same seed!
		seed_incast=std::clock();
		sleep(1); // need to sleep for a while, or seed_incast and seed_s might get the same seed!
		seed_s=std::clock();
	}
	~WavingSketch() { delete[] buckets; }

	void Insert(const data_type item) {
		uint32_t bucket_pos = hash(item,seed_choice) % BUCKET_NUM;
		buckets[bucket_pos].Insert(item,seed_s,seed_incast);
	}

	void copybuck(Bucket* src, Bucket* dest)
	{
		for (int i = 0; i < counter_num;i++)
			dest->incast[i] = src->incast[i];
		for (int i=0;i<slot_num;i++)
		{
			dest->items[i]=src->items[i];
			dest->counters[i]=src->counters[i];
		}
	}

	void mergebuck(Bucket* src1, Bucket* src2, Bucket* dest)
	{
		for (int i = 0; i < counter_num;i++)
			dest->incast[i] = src1->incast[i] + src2->incast[i];
		node* tp1=new node[slot_num<<1];
		node* p=tp1;
		for (uint32_t i=0;i<slot_num;i++)
		{
			p->item=src1->items[i];
			p->counter=src1->counters[i];
			p++;
		}
		for (uint32_t i=0;i<slot_num;i++)
		{
			p->item=src2->items[i];
			p->counter=src2->counters[i];
			p++;
		}
		std::sort(tp1,p);
		uint32_t i=0;
		for (i=0;i<slot_num;i++)
		{
			dest->items[i]=tp1[i].item;
			dest->counters[i]=tp1[i].counter;
		}
		uint32_t end=slot_num<<1;
		while (i<end && tp1[i].counter<0) // flag=true
		{
			uint32_t min_choice = hash(tp1[i].item, seed_s) & 1;
			uint32_t whichcast = hash(tp1[i].item, seed_incast) % counter_num;
			dest->incast[whichcast] -= COUNT[min_choice] * tp1[i].counter;
			i++;
		}
		delete[] tp1;
	}

	void shrink()
	{
		// shrink to half of total buckets
		if (BUCKET_NUM % 2)
		{
			printf("Bucket num %d is odd, can NOT shrink!\n", BUCKET_NUM);
			printf("FATAL ERROR!\n");
			exit(-1);
		}
		uint32_t num=(BUCKET_NUM+1)>>1;
		Bucket* tpbuck=new Bucket[num];
		for (uint32_t i=0;i<num;i++)
		{
			if (i+num<BUCKET_NUM)
				mergebuck(&buckets[i],&buckets[i+num],&tpbuck[i]);
			else
				copybuck(&buckets[i],&tpbuck[i]);
		}
		BUCKET_NUM=num;
		delete[] buckets;
		buckets=tpbuck;
	}

	count_type Query(const data_type item) {
		uint32_t bucket_pos = hash(item,seed_choice) % BUCKET_NUM;
		return buckets[bucket_pos].Query(item,seed_s,seed_incast);
	}

	void expand()
	{
		uint32_t num=BUCKET_NUM<<1;
		Bucket* tpbuck=new Bucket[num];
		for (uint32_t i=0;i<BUCKET_NUM;i++)
		{
			copybuck(&buckets[i],&tpbuck[i]);
			copybuck(&buckets[i],&tpbuck[BUCKET_NUM+i]);
		}
		for (uint32_t i=0;i<num;i++)
		{
			for (uint32_t j=0;j<slot_num;j++)
			{
				if (hash(tpbuck[i].items[j],seed_choice)%num != i)
				{
					if (tpbuck[i].counters[j] > 0) // flag=False
					{
						tpbuck[i].incast[hash(tpbuck[i].items[j],seed_incast)%counter_num]-=COUNT[hash(tpbuck[i].items[j],seed_s)&1]*tpbuck[i].counters[j];
					}
					tpbuck[i].counters[j]=0;
					tpbuck[i].items[j]=0;
				}
			}
			for (uint32_t j=0;j<slot_num;j++)
			{
				if (tpbuck[i].counters[j]==0)
				{
					uint32_t k=j+1;
					for (;k<slot_num && tpbuck[i].counters[k]==0;k++);
					if (k==slot_num)
						break;
					tpbuck[i].counters[j]=tpbuck[i].counters[k];
					tpbuck[i].items[j]=tpbuck[i].items[k];
					tpbuck[i].counters[k]=0;
				}
			}
			for (int j = 0; j < counter_num;j++)
				tpbuck[i].incast[j] = tpbuck[i].incast[j] >> 1;
		}
		BUCKET_NUM=num;
		delete[] buckets;
		buckets=tpbuck;
	}

private:
	Bucket *buckets;
	uint32_t BUCKET_NUM;
	uint32_t seed_choice;
	uint32_t seed_s;
	uint32_t seed_incast;
};