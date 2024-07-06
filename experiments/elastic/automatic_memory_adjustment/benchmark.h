#pragma once
#include"../../../include/common.h"
#include "../../../include/Count_Heap.h"
#include "../../../include/SS.h"
#include "../../../include/USS.h"
#include "../../../include/Waving.h"
#include "../../../include/ld.h"
#include "../../../include/CM.h"
#include "../../../include/Count.h"
template<uint32_t DATA_LEN>
void BenchElastic(std::string PATH) {
	printf("\033[0m\033[1;4;33m# Testing function: BenchElastic\n\033[0m");
	count_type cnt = 0;
	Data<DATA_LEN>* items = NULL;
	std::string file = "caida";

	items = read_standard_data<DATA_LEN>(PATH.c_str(), record_length, &cnt);

	constexpr int32_t memory_upper_bound = 1081344;
	constexpr int32_t memory_lower_bound = 16896 * 2;
	constexpr double hit_upper_bound = 0.768;
	constexpr double hit_lower_bound = 0.732;
	int32_t checkpoint = 0.005 * cnt;
	constexpr int32_t K = 1000;
	int output_pointer = 0;
	// constexpr int32_t num_inc = 10;
	int nshrink = 5;
	// constexpr int32_t mem_var = 6;
	constexpr int32_t cmp_num = 1;

	WavingSketch<8, 1, DATA_LEN>* sketches[cmp_num];

	sketches[0] = new WavingSketch<8, 1, DATA_LEN>(memory_upper_bound / (8 * 8 + 1 * 2));

	std::vector<std::ostream*> outs(6);
	outs[0] = new std::ofstream(RESULT_FOLDER + "elasticExpand_" + file + "_ARE.csv");
	outs[1] = new std::ofstream(RESULT_FOLDER + "elasticExpand_" + file + "_CR.csv");
	outs[2] = new std::ofstream(RESULT_FOLDER + "elasticExpand_" + file + "_PR.csv");
	outs[3] = new std::ofstream(RESULT_FOLDER + "elasticExpand_" + file + "_F1.csv");
	outs[4] = new std::ofstream(RESULT_FOLDER + "elasticExpand_" + file + "_HR.csv");
	outs[5] = new std::ofstream(RESULT_FOLDER + "elasticExpand_" + file + "_MEM.csv");
	for (auto& out : outs)
	{
		*out << "MEM(KB)" << "," << sketches[0]->name << ",";
		*out << std::endl;
	}
	std::queue<bool> hits[cmp_num];
	std::vector<uint32_t> total(cmp_num, 0);
	HashMap<DATA_LEN> mp;
	std::multiset<std::pair<uint32_t, Data<DATA_LEN>>> topk_items;
	for (int l = 0; l < cnt; ++l) {
		std::pair<uint32_t, Data<DATA_LEN>> p = std::make_pair(mp[items[l]], items[l]);
		if (topk_items.find(p) != topk_items.end())
		{
			topk_items.erase(p);
		}
		mp[items[l]] += 1;
		p = std::make_pair(mp[items[l]], items[l]);
		topk_items.insert(p);
		if (topk_items.size() > K)
		{
			topk_items.erase(topk_items.begin());
		}
		for (int j = 0; j < cmp_num; ++j) {
			bool is_hit = sketches[j]->QueryTopK(items[l]) != 0;
			hits[j].push(is_hit);
			total[j] += is_hit;
			sketches[j]->Init(items[l]);
			if (l >= checkpoint)
			{
				total[j] -= hits[j].front();
				hits[j].pop();
			}
		}
		if (l % checkpoint == 0 && l)// checkpoint=1 when slide window
		{
			std::cout << "checkpoint= " << l * 1.0 / cnt * 100 << " %" << std::endl;
			for (auto& out : outs)
			{
				*out << l * 1.0 / cnt << ",";
			}
			for (int j = 0; j < cmp_num; j++)
			{
				uint32_t topK = topk_items.begin()->first;
				sketches[j]->Check(mp, topK, outs);
				*outs[4] << total[j] * 1.0 / checkpoint << ",";
				*outs[5] << pow(2, floor(log2(sketches[j]->getMemSize() / 1000))) * 1000 << ",";
				std::cout << "Hit Rate: " << total[j] * 1.0 / checkpoint
					<< " Mem: " << pow(2, floor(log2(sketches[j]->getMemSize() / 1000))) << " KB" << std::endl;

				if (sketches[j]->getMemSize() < memory_upper_bound
					&& total[j] < hit_lower_bound * checkpoint)
				{
					sketches[j]->expand();
					std::cout << "ID=" << j << " expand to " << pow(2, floor(log2(sketches[j]->getMemSize() / 1000))) << "KB" << std::endl;
				}
				if (sketches[j]->getMemSize() > memory_lower_bound
					&& total[j] > hit_upper_bound * checkpoint)
				{
					sketches[j]->shrink();
					std::cout << "ID=" << j << " shrink to " << pow(2, floor(log2(sketches[j]->getMemSize() / 1000))) << "KB" << std::endl;
				}
			}
			for (auto& out : outs)
			{
				*out << std::endl;
			}
			std::cout << std::endl;
		}
	}
	delete items;
}