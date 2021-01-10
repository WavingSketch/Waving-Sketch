#pragma once

#include <chrono>
#include <climits>
#include <vector>
#include <iostream>

#include "SS.h"
#include "USS.h"
#include "Count_Heap.h"
#include "WavingSketch.h"

typedef std::chrono::high_resolution_clock::time_point TP;

inline TP now() { return std::chrono::high_resolution_clock::now(); }

data_type *read_data(const char *PATH, const count_type length,
                     count_type *cnt) {
	data_type *items = new data_type[length];
	data_type *it = items;

	FILE *data = fopen(PATH, "rb");

	*cnt = 0;
	while (fread(it++, sizeof(data_type), 1, data) > 0) {
		(*cnt)++;
	}

	fclose(data);

	return items;
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

void BenchCmp(const char *PATH) {
	std::cout << "Comparison with SS, USS, Count+Heap on ARE, CR and PR"
	          << std::endl
	          << std::endl;
	count_type cnt;
	data_type *items = read_data(PATH, 100000000, &cnt);

	constexpr int32_t mem_base = 0;
	constexpr int32_t mem_inc = 200000;
	constexpr int32_t mem_var = 5;
	constexpr int32_t cmp_num = 4;

	Abstract *sketches[mem_var][cmp_num];

	for (int i = 0; i < mem_var; ++i) {
		sketches[i][0] = new WavingSketch< 8 >((i + 1) * mem_inc / 66);
		sketches[i][1] = new SS((i + 1) * mem_inc / 100);
		sketches[i][2] = new USS((i + 1) * mem_inc / 100);
		sketches[i][3] =
		    new Count_Heap(2500, ((i + 1) * mem_inc - 120000) / 12, 3);
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
		std::cout << "Memory size: " << (mem_base + mem_inc * (i + 1)) / 1000
		          << "KB" << std::endl
		          << std::endl;
		for (int l = 0; l < cnt; ++l) {
			for (int j = 0; j < cmp_num; ++j) {
				sketches[i][j]->Insert(items[l]);
			}
		}

		for (int j = 0; j < cmp_num; ++j) {
			sketches[i][j]->Check(mp, topK);
			delete sketches[i][j];
		}
		std::cout << std::endl;
	}

	delete items;
}

void BenchThp(const char *PATH) {
	std::cout << "Comparison with SS, USS, Count+Heap on Throughput"
	          << std::endl
	          << std::endl;

	count_type cnt;
	data_type *items = read_data(PATH, 100000000, &cnt);

	constexpr int32_t mem_base = 0;
	constexpr int32_t mem_inc = 200000;
	constexpr int32_t mem_var = 5;
	constexpr int32_t cmp_num = 4;
	constexpr int32_t round = 5;

	Abstract *sketches[mem_var][cmp_num];

	int progress = 0;

	for (int i = 0; i < mem_var; ++i) {
		std::cout << "Memory size: " << (mem_base + mem_inc * (i + 1)) / 1000
		          << "KB" << std::endl
		          << std::endl;

		double thp[round][cmp_num] = {};
		double avg_thp[cmp_num] = {};

		for (int j = 0; j < round; ++j) {
			sketches[i][0] =
			    new WavingSketch< 8 >((mem_base + (i + 1) * mem_inc) / 66);
			sketches[i][1] = new SS((mem_base + mem_inc * (i + 1)) / 100);
			sketches[i][2] = new USS((mem_base + mem_inc * (i + 1)) / 100);
			sketches[i][3] =
			    new Count_Heap(2500, ((i + 1) * mem_inc - 120000) / 12, 3);

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
			printf("%12s:\tthp = %lf\n", sketches[i][l]->name,
			       avg_thp[l] / round);
			delete sketches[i][l];
		}
		std::cout << std::endl;
	}

	delete items;
}
