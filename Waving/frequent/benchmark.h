#pragma once

#include <chrono>
#include <climits>
#include <vector>
#include <iostream>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "SS.h"
#include "USS.h"
#include "Count_Heap.h"
#include "WavingSketch.h"

typedef std::chrono::high_resolution_clock::time_point TP;

inline TP now() { return std::chrono::high_resolution_clock::now(); }

data_type *read_data(const char *PATH, const int size, int *cnt) 
{
	struct stat buf;
	// printf("Opening file...\n");
	int fd=open(PATH,O_RDONLY);
	fstat(fd,&buf);
	*cnt=buf.st_size/size;
	// printf("\tcnt=%d\n",*cnt);
	// printf("Mmap...\n");
	void* addr=mmap(NULL,buf.st_size,PROT_READ,MAP_PRIVATE,fd,0);
	close(fd);
	if (addr==MAP_FAILED)
	{
		printf("[ERROR] MMAP FAILED!\n");
		exit(-1);
	}
	return (data_type *)addr;
}

data_type *read_skipping_data(const char *PATH, int *cnt) 
{
	struct stat buf;
	// printf("Opening file...\n");
	int fd=open(PATH,O_RDONLY);
	fstat(fd,&buf);
	*cnt=buf.st_size/8;
	// printf("\tcnt=%d\n",*cnt);
	// printf("Mmap...\n");
	void* raw_addr=mmap(NULL,buf.st_size,PROT_READ,MAP_PRIVATE,fd,0);
	close(fd);
	if (raw_addr==MAP_FAILED)
	{
		printf("[ERROR] MMAP FAILED!\n");
		exit(-1);
	}
	data_type* addr=reinterpret_cast<data_type*>(raw_addr);
	void* raw_data=malloc(buf.st_size);
	if (raw_data==NULL)
	{
		printf("[ERROR] MALLOC FAILED!\n");
		exit(-1);
	}
	int num=*cnt;
	data_type* data=reinterpret_cast<data_type*>(raw_data);
	for (int i=0;i<num;i++)
		data[i]=addr[2*i+1];
	munmap(raw_addr,buf.st_size);
	return data;
}

uint32_t Get_TopK(HashMap mp, uint32_t k) {
	uint32_t size = mp.size();
	uint32_t *num = new uint32_t[size];

	uint32_t pos = 0;
	HashMap::iterator it;
	for (it = mp.begin(); it != mp.end(); ++it) {
		num[pos++] = it->second;
	}
	std::nth_element(num, num + size - k, num + size);
	uint32_t ret = num[size - k];

	delete[] num;
	return ret;
}

// Evaluation on Dataset Distributions. (skewness of the data)
void BenchSkew()
{
	printf("\033[0m\033[1;4;33m# Testing function: BenchSkew\n\033[0m");
	count_type cnt;
	data_type *items;
	items=read_data("../dataset/new_zipf/030.dat", 4, &cnt);
	constexpr int32_t mem_base = 0;
	constexpr int32_t mem_inc = 20000;
	// constexpr int32_t num_inc = 10;
	int nshrink = 0;
	constexpr int32_t mem_var = 3;
	constexpr int32_t cmp_num = 1;

	Abstract *sketches[mem_var][cmp_num];
	rst result[mem_var][cmp_num];

	// Ground truth
	HashMap mp;
	for (int l = 0; l < cnt; ++l) {
		if (mp.find(items[l]) == mp.end())
			mp[items[l]] = 1;
		else
			mp[items[l]] += 1;
	}
	uint32_t topK = Get_TopK(mp, 2000);
	for (int i = 0; i < mem_var; ++i) 
	{
		sketches[i][0] = new WavingSketch<16,32>((i + 1) * mem_inc / (16*8+32*2));
	}

	for (int i = 0; i < mem_var; ++i) {
		printf("\033[0m\033[1;4;36m> Memory size: %dKB\n\033[0m", (mem_base + mem_inc * (i + 1)) / 1000);
		for (int l = 0; l < cnt; ++l) {
			for (int j = 0; j < cmp_num; ++j) {
				sketches[i][j]->Insert(items[l]);
			}
		}

		for (int j = 0; j < cmp_num; ++j) {
			sketches[i][j]->Check(mp, topK);
			delete sketches[i][j];
		}
	}

	delete items;
}

// test ARE, CR, PR multiple times with different hash functions
void BenchCmpRound(int fid)
{
	printf("\033[0m\033[1;4;33m# Testing function: BenchCmpRound\n\033[0m");
	count_type cnt;
	data_type *items;
	if (fid==0)
		items=read_skipping_data("../dataset/demo.dat", &cnt);
	else if (fid==1)
		items=read_data("../dataset/syn.dat", 4, &cnt);
	else if (fid==2)
		items=read_data("../dataset/CAIDA2018.dat", 4, &cnt);
	else if (fid==3)
		items=read_data("../dataset/webpage.dat", 4, &cnt);
	else if (fid==4)
		items=read_data("../dataset/campus.dat", 4, &cnt);
	constexpr int32_t mem_base = 0;
	constexpr int32_t mem_inc = 50000;
	// constexpr int32_t num_inc = 10;
	int nshrink = 0;
	constexpr int32_t mem_var = 6;
	constexpr int32_t cmp_num = 5;
	constexpr int32_t round = 100;

	Abstract *sketches[mem_var][cmp_num];
	rst result[mem_var][cmp_num];

	// Ground truth
	HashMap mp;
	for (int l = 0; l < cnt; ++l) {
		if (mp.find(items[l]) == mp.end())
			mp[items[l]] = 1;
		else
			mp[items[l]] += 1;
	}
	uint32_t topK = Get_TopK(mp, 2000);
	// =============Original test code===============
	for (int i = 0; i < mem_var; ++i) 
	{
		printf("\033[0m\033[1;4;36m> Memory size: %dKB\n\033[0m", (mem_base + mem_inc * (i + 1)) / 1000);
		for (int r = 0; r < round;r++)
		{
			// ============================Build up sketches===============================
			sketches[i][0] = new WavingSketch<8,1>((i + 1) * mem_inc / (8*8+1*2));
			sketches[i][1] = new WavingSketch<8,16>((i + 1) * mem_inc / (8*8+16*2));
			sketches[i][2] = new SS((i + 1) * mem_inc / 100);
			sketches[i][3] = new USS((i + 1) * mem_inc / 100);
			sketches[i][4] =
				new Count_Heap(2500, ((i + 1) * mem_inc - 20000) / 12, 3);
			// ============================================================================
			for (int l = 0; l < cnt; ++l)
			{
				for (int j = 0; j < cmp_num; ++j)
				{
					sketches[i][j]->Insert(items[l]);
				}
			}
			for (int j = 0; j < cmp_num; ++j) 
			{
				result[i][j]=result[i][j]+sketches[i][j]->QuietCheck(mp, topK);
				if (r<round-1)
					delete sketches[i][j];
			}
		}
		for (int j = 0; j < cmp_num;j++)
		{
			printf("\033[0m\033[1;36m|\033[0m%21s:\tARE: %lf\tCR: %lf\tPR: %lf\n", sketches[i][j]->name, result[i][j].are/round, result[i][j].cr/round, result[i][j].pr/round);
		}
	}
	std::cout << std::endl;
	delete items;
}

void BenchElastic(int fid) {
	printf("\033[0m\033[1;4;33m# Testing function: BenchElastic\n\033[0m");
	count_type cnt;
	data_type *items;
	if (fid==0)
		items=read_skipping_data("../dataset/demo.dat", &cnt);
	else if (fid==1)
		items=read_data("../dataset/syn.dat", 4, &cnt);
	else if (fid==2)
		items=read_data("../dataset/CAIDA2018.dat", 4, &cnt);
	else if (fid==3)
		items=read_data("../dataset/webpage.dat", 4, &cnt);
	else if (fid==4)
		items=read_data("../dataset/campus.dat", 4, &cnt);
	constexpr int32_t mem_base = 320000;
	// constexpr int32_t mem_inc = 50000;
	// constexpr int32_t num_inc = 10;
	int nshrink = 5;
	// constexpr int32_t mem_var = 6;
	constexpr int32_t cmp_num = 3;

	Abstract *sketches[cmp_num];

	sketches[0] = new WavingSketch<8,1>((mem_base / (8*8+1*2))&(INT32_MAX<<nshrink));
	sketches[1] = new WavingSketch<8,16>((mem_base / (8*8+16*2))&(INT32_MAX<<nshrink));
	sketches[2] = new WavingSketch<16,32>((mem_base / (16*8+32*2))&(INT32_MAX<<nshrink));

	// Ground truth
	HashMap mp;
	for (int l = 0; l < cnt; ++l) {
		if (mp.find(items[l]) == mp.end())
			mp[items[l]] = 1;
		else
			mp[items[l]] += 1;
	}
	uint32_t topK = Get_TopK(mp, 2000);
	for (int l = 0; l < cnt; ++l) {
		for (int j = 0; j < cmp_num; ++j) {
			sketches[j]->Insert(items[l]);
		}
	}
	for (int i = 0; i <= nshrink; ++i) {
		printf("\033[0m\033[1;4;36m> Memory size: %fKB\n\033[0m", (mem_base/pow(2,i)) / 1000);

		for (int j = 0; j < cmp_num; ++j) {
			sketches[j]->Check(mp, topK);
			if (i<nshrink)
				sketches[j]->shrink();
		}
		std::cout << std::endl;
	}

	delete items;
}

// Basic test function, to test ARE, CR, PR
void BenchCmp(int fid) {
	printf("\033[0m\033[1;4;33m# Testing function: BenchCmp\n\033[0m");
	count_type cnt;
	data_type *items;
	if (fid==0)
		items=read_skipping_data("../dataset/demo.dat", &cnt);
	else if (fid==1)
		items=read_data("../dataset/syn.dat", 4, &cnt);
	else if (fid==2)
		items=read_data("../dataset/CAIDA2018.dat", 4, &cnt);
	else if (fid==3)
		items=read_data("../dataset/webpage.dat", 4, &cnt);
	else if (fid==4)
		items=read_data("../dataset/campus.dat", 4, &cnt);
	constexpr int32_t mem_base = 0;
	constexpr int32_t mem_inc = 50000;
	// constexpr int32_t num_inc = 10;
	int nshrink = 0;
	constexpr int32_t mem_var = 6;
	constexpr int32_t cmp_num = 5;

	Abstract *sketches[mem_var][cmp_num];

	for (int i = 0; i < mem_var; ++i) {
		sketches[i][0] = new WavingSketch<8,1>((i + 1) * mem_inc / (8*8+1*2));
		sketches[i][1] = new WavingSketch<8,16>((i + 1) * mem_inc / (8*8+16*2));
		sketches[i][2] = new SS((i + 1) * mem_inc / 100);
		sketches[i][3] = new USS((i + 1) * mem_inc / 100);
		sketches[i][4] =
		    new Count_Heap(2500, ((i + 1) * mem_inc - 20000) / 12, 3);
	}

	// Ground truth
	HashMap mp;
	for (int l = 0; l < cnt; ++l) {
		if (mp.find(items[l]) == mp.end())
			mp[items[l]] = 1;
		else
			mp[items[l]] += 1;
	}
	uint32_t topK = Get_TopK(mp, 2000);

	for (int i = 0; i < mem_var; ++i) {
		printf("\033[0m\033[1;4;36m> Memory size: %dKB\n\033[0m", (mem_base + mem_inc * (i + 1)) / 1000);
		for (int l = 0; l < cnt; ++l) {
			for (int j = 0; j < cmp_num; ++j) {
				sketches[i][j]->Insert(items[l]);
			}
		}

		for (int j = 0; j < cmp_num; ++j) {
			sketches[i][j]->Check(mp, topK);
			delete sketches[i][j];
		}
	}

	delete items;
}

// test the throughput of different algorithms
void BenchThp(int fid) {
	printf("\033[0m\033[1;4;33m# Testing function: BenchThp\n\033[0m");
	count_type cnt;
	data_type *items;
	if (fid==0)
		items=read_skipping_data("../dataset/demo.dat", &cnt);
	else if (fid==1)
		items=read_data("../dataset/syn.dat", 4, &cnt);
	else if (fid==2)
		items=read_data("../dataset/CAIDA2018.dat", 4, &cnt);
	else if (fid==3)
		items=read_data("../dataset/webpage.dat", 4, &cnt);
	else if (fid==4)
		items=read_data("../dataset/campus.dat", 4, &cnt);

	constexpr int32_t mem_base = 0;
	constexpr int32_t mem_inc = 50000;
	constexpr int32_t mem_var = 6;
	constexpr int32_t cmp_num = 5;
	constexpr int32_t round = 5;

	Abstract *sketches[mem_var][cmp_num];

	int progress = 0;

	for (int i = 0; i < mem_var; ++i) {
		printf("\033[0m\033[1;4;36m> Memory size: %dKB\n\033[0m", (mem_base + mem_inc * (i + 1)) / 1000);
		double thp[round][cmp_num] = {};
		double avg_thp[cmp_num] = {};

		for (int j = 0; j < round; ++j) {
			sketches[i][0] = new WavingSketch<8,1>((i + 1) * mem_inc / (8*8+1*2));
			sketches[i][1] = new WavingSketch<8,16>((i + 1) * mem_inc / (8*8+16*2));
			sketches[i][2] = new SS((i + 1) * mem_inc / 100);
			sketches[i][3] = new USS((i + 1) * mem_inc / 100);
			sketches[i][4] =
				new Count_Heap(2500, ((i + 1) * mem_inc - 20000) / 12, 3);

			for (int l = 0; l < cmp_num; ++l) {
				TP start, finish;

				start = now();
				for (int m = 0; m < cnt; ++m) {
					sketches[i][l]->Insert(items[m]);
				}
				finish = now();

				thp[j][l] =
				    (double)cnt /
				    std::chrono::duration_cast< std::chrono::duration<
				        double, std::ratio< 1, 1000000 > > >(finish - start)
				        .count();
				avg_thp[l] += thp[j][l];

				if (j != round - 1) {
					delete sketches[i][l];
				}
			}
		}

		for (int l = 0; l < cmp_num; ++l) {
			printf("\033[0m\033[1;36m|\033[0m%21s:\tthp = %lf\n", sketches[i][l]->name,
			       avg_thp[l] / round);
			delete sketches[i][l];
		}
	}

	delete items;
}
