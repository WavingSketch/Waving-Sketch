#pragma once

#include"../../include/common.h"
#include "../../include/Count_Heap.h"
#include "../../include/SS.h"
#include "../../include/USS.h"
#include "../../include/Waving.h"
#include "../../include/ld.h"
#include "../../include/CM.h"
#include "../../include/Count.h"
// Basic test function, to test ARE, CR, PR
template<uint32_t DATA_LEN>
void Frequent_BenchCmp(std::string PATH) {
	printf("\033[0m\033[1;4;33m# Testing function: BenchCmp\n\033[0m");
	count_type cnt = 0;
	Data<DATA_LEN>* items = NULL;
	std::string file = "caida";

	items = read_standard_data<DATA_LEN>(PATH.c_str(), record_length, &cnt);
	std::cout << "Total " << cnt << " items" << std::endl;
	int32_t mem_base = 0;
	int32_t mem_inc = 50000;
	constexpr int32_t mem_var = 6;
	constexpr int32_t cmp_num = 6;
	int heap_size = 2500, K = 2000;
	// constexpr int32_t num_inc = 10;
	int nshrink = 0;
	// Ground truth
	HashMap<DATA_LEN> mp;
	for (int l = 0; l < cnt; ++l) {
		if (mp.find(items[l]) == mp.end())
			mp[items[l]] = 1;
		else
			mp[items[l]] += 1;
	}
	uint32_t topK = Get_TopK(mp, K);

	Abstract<DATA_LEN>* sketches[mem_var][cmp_num];

	for (int i = 0; i < mem_var; ++i) {
		int memory = (mem_base + mem_inc * (i + 1));
		sketches[i][0] = new WavingSketch<8, 1, DATA_LEN>(memory / (8 * 8 + 1 * 2));
		sketches[i][1] = new WavingSketch<8, 16, DATA_LEN>(memory / (8 * 8 + 16 * 2));
		sketches[i][2] = new SS<DATA_LEN>(memory / 100);
		sketches[i][3] = new USS<DATA_LEN>(memory / 100);
		sketches[i][4] = new Count_Heap<DATA_LEN>(heap_size, (memory - heap_size * (sizeof(Data<DATA_LEN>) + 4)) / 12, 3);
		sketches[i][5] = new LdSketchWrapper<DATA_LEN>(memory, topK);
	}

	std::vector<std::ostream*> outs(4);
	outs[0] = new std::ofstream(RESULT_FOLDER + "frequent_" + file + "_ARE.csv");
	outs[1] = new std::ofstream(RESULT_FOLDER + "frequent_" + file + "_Recall Rate (RR).csv");
	outs[2] = new std::ofstream(RESULT_FOLDER + "frequent_" + file + "_Precision Rate (PR).csv");
	outs[3] = new std::ofstream(RESULT_FOLDER + "frequent_" + file + "_F1.csv");

	for (auto& out : outs)
	{
		*out << "MEM(KB)" << ",";
		for (uint32_t i = 0; i < cmp_num; i++)
		{
			*out << sketches[0][i]->name << ",";
		}
		*out << std::endl;
	}

	for (int i = 0; i < mem_var; ++i) {
		printf("\033[0m\033[1;4;36m> Memory size: %dKB\n\033[0m",
			(mem_base + mem_inc * (i + 1)) / 1000);
		for (auto& out : outs)
		{
			*out << (mem_base + mem_inc * (i + 1)) / 1000 << ",";
		}
		printf("Init average l: %lf\n",
			((LdSketchWrapper<DATA_LEN>*)sketches[i][5])->average_l());
		for (int l = 0; l < cnt; ++l) {
			for (int j = 0; j < cmp_num; ++j) {
				sketches[i][j]->Init(items[l]);
			}
		}
		printf("After average l: %lf\n",
			((LdSketchWrapper<DATA_LEN>*)sketches[i][5])->average_l());

		for (int j = 0; j < cmp_num; ++j) {
			sketches[i][j]->Check(mp, topK, outs);
			delete sketches[i][j];
		}
		for (auto& out : outs)
		{
			*out << std::endl;
		}
	}

	delete items;
}

// test the throughput of different algorithms
template<uint32_t DATA_LEN>
void Frequent_BenchThp(std::string PATH) {
	printf("\033[0m\033[1;4;33m# Testing function: BenchThp\n\033[0m");
	count_type cnt = 0;
	Data<DATA_LEN>* items = NULL;
	std::string file = "caida";

	items = read_standard_data<DATA_LEN>(PATH.c_str(), record_length, &cnt);
	std::cout << "Total " << cnt << " items" << std::endl;
	int32_t mem_base = 0;
	int32_t mem_inc = 50000;
	constexpr int32_t mem_var = 6;
	constexpr int32_t cmp_num = 6;
	constexpr int32_t round = 5;
	int heap_size = 2500, K = 2000;
	std::ofstream out(RESULT_FOLDER + "frequent_" + file + "_Throughput(Mops).csv");
	HashMap<DATA_LEN> mp;
	for (int l = 0; l < cnt; ++l) {
		if (mp.find(items[l]) == mp.end())
			mp[items[l]] = 1;
		else
			mp[items[l]] += 1;
	}
	uint32_t topK = Get_TopK(mp, K);

	Abstract<DATA_LEN>* sketches[mem_var][cmp_num];

	int progress = 0;

	for (int i = 0; i < mem_var; ++i) {
		printf("\033[0m\033[1;4;36m> Memory size: %dKB\n\033[0m",
			(mem_base + mem_inc * (i + 1)) / 1000);
		double thp[round][cmp_num] = {};
		double avg_thp[cmp_num] = {};
		for (int j = 0; j < round; ++j) {
			int memory = (mem_base + mem_inc * (i + 1));
			sketches[i][0] = new WavingSketch<8, 1, DATA_LEN>(memory / (8 * 8 + 1 * 2));
			sketches[i][1] = new WavingSketch<8, 16, DATA_LEN>(memory / (8 * 8 + 16 * 2));
			sketches[i][2] = new SS<DATA_LEN>(memory / 100);
			sketches[i][3] = new USS<DATA_LEN>(memory / 100);
			sketches[i][4] = new Count_Heap<DATA_LEN>(heap_size, (memory - heap_size * (sizeof(Data<DATA_LEN>) + 4)) / 12, 3);
			sketches[i][5] = new LdSketchWrapper<DATA_LEN>(memory, topK);

			for (int l = 0; l < cmp_num; ++l) {
				TP start, finish;

				start = now();
				for (int m = 0; m < cnt; ++m) {
					sketches[i][l]->Init(items[m]);
				}
				finish = now();

				thp[j][l] =
					(double)cnt /
					std::chrono::duration_cast<
					std::chrono::duration<double, std::ratio<1, 1000000> >>(
						finish - start)
					.count();
				avg_thp[l] += thp[j][l];

				if (j != round - 1) {
					delete sketches[i][l];
				}
			}
		}

		if (i == 0)
		{
			out << "MEM(KB)" << ",";
			for (uint32_t j = 0; j < cmp_num; j++)
			{
				out << sketches[0][j]->name << ",";
			}
			out << std::endl;
		}
		out << (mem_base + mem_inc * (i + 1)) / 1000 << ",";

		for (int l = 0; l < cmp_num; ++l) {
			printf("\033[0m\033[1;36m|\033[0m%21s:\tthp = %lf\n",
				sketches[i][l]->name.c_str(), avg_thp[l] / round);
			out << avg_thp[l] / round << ",";
			delete sketches[i][l];
		}
		out << std::endl;
	}

	delete items;
	out.close();
}