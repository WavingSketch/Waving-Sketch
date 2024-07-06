#pragma once


#include "../../../include/common.h"
#include "../../../include/Count_Heap.h"
#include "../../../include/SS.h"
#include "../../../include/USS.h"
#include "../../../include/Waving.h"
#include "../../../include/ld.h"
#include "../../../include/CM.h"
#include "../../../include/Count.h"
template<uint32_t DATA_LEN>
void BenchTopKSubset(std::string PATH)
{
	printf("\033[0m\033[1;4;33m# Testing function: BenchTopKSubset\n\033[0m");
	count_type cnt = 0;
	Data<DATA_LEN>* items = NULL;
	std::string file = "caida";

	items = read_standard_data<DATA_LEN>(PATH.c_str(), record_length, &cnt);
	std::cout << "Total " << cnt << " items" << std::endl;
	int32_t mem_base = 0;
	int32_t mem_inc = 50000;
	constexpr int32_t mem_var = 5;
	constexpr int32_t cmp_num = 5;
	constexpr int32_t epoch_num = 100;
	constexpr int32_t batch_size = 1000;
	int heap_size = 2500;
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
	uint32_t topK = Get_TopK<DATA_LEN>(mp, 2000);

	Abstract<DATA_LEN>* sketches[mem_var][cmp_num];

	for (int i = 0; i < mem_var; ++i) {
		int memory = (mem_base + mem_inc * (i + 1));
		sketches[i][0] = new WavingSketch<8, 1, DATA_LEN>(memory / (8 * 8 + 1 * 2));
		sketches[i][1] = new SS<DATA_LEN>(memory / 100);
		sketches[i][2] = new USS<DATA_LEN>(memory / 100);
		sketches[i][3] = new Count_Heap<DATA_LEN>(heap_size, (memory - heap_size * (sizeof(Data<DATA_LEN>) + 4)) / 12, 3);
		sketches[i][4] = new LdSketchWrapper<DATA_LEN>(memory, topK);
	}

	std::vector<std::ostream*> outs(4);
	outs[0] = new std::ofstream(RESULT_FOLDER + "TopKSubset_" + file + "_ARE.csv"); // subset sum
	outs[1] = new std::ofstream(RESULT_FOLDER + "TopKSubset_" + file + "_AAE.csv");
	outs[2] = new std::ofstream(RESULT_FOLDER + "TopKSubset_" + file + "_ARE1.csv"); //subset average
	outs[3] = new std::ofstream(RESULT_FOLDER + "TopKSubset_" + file + "_AAE1.csv");
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
			((LdSketchWrapper<DATA_LEN>*)sketches[i][4])->average_l());
		for (int l = 0; l < cnt; ++l) {
			for (int j = 0; j < cmp_num; ++j) {
				sketches[i][j]->Init(items[l]);
			}
		}
		printf("After average l: %lf\n",
			((LdSketchWrapper<DATA_LEN>*)sketches[i][4])->average_l());

		for (int j = 0; j < cmp_num; ++j) {
			sketches[i][j]->CheckSubset(mp, topK, epoch_num, batch_size, outs);
			delete sketches[i][j];
		}
		for (auto& out : outs)
		{
			*out << std::endl;
		}
	}

	delete items;
}