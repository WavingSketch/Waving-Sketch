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
void SplitData(const HashMap<DATA_LEN>& mp, double skewness, uint32_t total, uint32_t blocks, HashMap<DATA_LEN>& data2block)
{
	std::vector<std::pair<count_type, Data<DATA_LEN>>> lst;
	for (auto& k_v : mp)
	{
		lst.push_back(std::make_pair(k_v.second, k_v.first));
	}
	data2block.clear();
	uint32_t primary_cnt = 0;
	random_shuffle(lst.begin(), lst.end());
	while (primary_cnt < skewness * total)
	{
		auto obj = *lst.rbegin();
		data2block[obj.second] = 0;
		primary_cnt += obj.first;
		lst.pop_back();
	}
	sort(lst.begin(), lst.end());
	auto target = 1;
	while (!lst.empty())
	{
		auto k_v = *lst.rbegin();
		data2block[k_v.second] = target;
		target = (target + 1) % (blocks - 1) + 1;
		lst.pop_back();
	}
}
template<uint32_t DATA_LEN>
void BenchSkewGlobalTopK(std::string PATH)
{
	printf("\033[0m\033[1;4;33m# Testing function: BenchSkewGlobalTopK\n\033[0m");
	count_type cnt = 0;
	Data<DATA_LEN>* items = NULL;
	std::string file = "caida";

	items = read_standard_data<DATA_LEN>(PATH.c_str(), record_length, &cnt);
	std::cout << "Total " << cnt << " items" << std::endl;
	// Ground truth
	HashMap<DATA_LEN> mp;
	for (int l = 0; l < cnt; ++l) {
		if (mp.find(items[l]) == mp.end())
			mp[items[l]] = 1;
		else
			mp[items[l]] += 1;
	}
	uint32_t topK = Get_TopK<DATA_LEN>(mp, 2000);
	constexpr uint32_t cmp_num = 4;
	constexpr uint32_t memory = 10000;
	constexpr uint32_t blocks = 100, epochs = 2;
	uint32_t heap_size = 1000;
	std::vector<double> skewnesses{ 0.01,0.02,0.03,0.05,0.1,0.2,0.3,0.4,0.5 };
	uint32_t skew_var = skewnesses.size();
	Abstract<DATA_LEN>* sketches[cmp_num][blocks];

	std::vector<std::ostream*> outs(4);
	outs[0] = new std::ofstream(RESULT_FOLDER + "skewGlobalTopK_" + file + "_ARE.csv");
	outs[1] = new std::ofstream(RESULT_FOLDER + "skewGlobalTopK_" + file + "_Recall Rate (RR).csv");
	outs[2] = new std::ofstream(RESULT_FOLDER + "skewGlobalTopK_" + file + "_Precision Rate (PR).csv");
	outs[3] = new std::ofstream(RESULT_FOLDER + "skewGlobalTopK_" + file + "_F1.csv");

	for (auto& skewness : skewnesses)
	{
		printf("\033[0m\033[1;4;36m> Skewness: %lf\n\033[0m", skewness);
		std::vector<double> result[cmp_num];
		std::vector<std::string> names;
		for (size_t i = 0; i < cmp_num; i++)
		{
			result[i].resize(4, 0);
		}
		for (uint32_t epoch = 0; epoch < epochs; epoch++)
		{
			for (size_t i = 0; i < blocks; i++)
			{
				sketches[0][i] = new WavingSketch<8, 1, DATA_LEN>(memory / (8 * 8 + 1 * 2));
				sketches[1][i] = new SS<DATA_LEN>(memory / 100);
				sketches[2][i] = new USS<DATA_LEN>(memory / 100);
				sketches[3][i] = new Count_Heap<DATA_LEN>(heap_size, (memory - heap_size * (sizeof(Data<DATA_LEN>) + 4)) / 12, 3);
			}
			if (epoch == 0)
			{
				if (skewness == skewnesses[0])
				{
					for (auto& out : outs)
					{
						*out << "MEM(KB)" << ",";
						for (uint32_t i = 0; i < cmp_num; i++)
						{
							*out << sketches[i][0]->name << ",";
						}
						*out << std::endl;
					}
				}
				for (uint32_t i = 0; i < cmp_num; i++)
				{
					names.push_back(sketches[i][0]->name);
				}
				for (auto& out : outs)
				{
					*out << skewness << ",";
				}
			}
			HashMap<DATA_LEN> data2block;
			SplitData<DATA_LEN>(mp, skewness, cnt, blocks, data2block);
			for (int l = 0; l < cnt; ++l) {
				int pos = data2block[items[l]];
				for (int j = 0; j < cmp_num; ++j) {
					sketches[j][pos]->Init(items[l]);
				}
			}
			std::vector<double> result_in_epoch(4);
			for (int i = 0; i < cmp_num; ++i) {
				Abstract<DATA_LEN>::DistributeCheck(mp, topK, data2block, sketches[i], result_in_epoch);
				for (size_t j = 0; j < 4; j++)
				{
					result[i][j] += result_in_epoch[j];
				}
				for (size_t j = 0; j < blocks; j++)
				{
					delete sketches[i][j];
				}
			}
		}
		for (size_t i = 0; i < cmp_num; i++)
		{
			double are = result[i][0] / epochs;
			double cr = result[i][1] / epochs;
			double pr = result[i][2] / epochs;
			double f1 = result[i][3] / epochs;
			printf(
				"%s:\tARE: %f\tCR: %f\tPR: %f\tF1: %f\n",
				names[i].c_str(), are, cr, pr, f1);
			*outs[0] << are << ",";
			*outs[1] << cr << ",";
			*outs[2] << pr << ",";
			*outs[3] << f1 << ",";
		}
		for (auto& out : outs)
		{
			*out << std::endl;
		}
	}
	delete items;
}
template<uint32_t DATA_LEN>
void BenchUniformGlobalTopK(std::string PATH)
{
	printf("\033[0m\033[1;4;33m# Testing function: BenchUniformGlobalTopK\n\033[0m");
	count_type cnt = 0;
	Data<DATA_LEN>* items = NULL;
	std::string file = "caida";

	items = read_standard_data<DATA_LEN>(PATH.c_str(), record_length, &cnt);
	std::cout << "Total " << cnt << " items" << std::endl;
	// Ground truth
	HashMap<DATA_LEN> mp;
	for (int l = 0; l < cnt; ++l) {
		if (mp.find(items[l]) == mp.end())
			mp[items[l]] = 1;
		else
			mp[items[l]] += 1;
	}
	uint32_t topK = Get_TopK(mp, 5000);
	constexpr uint32_t cmp_num = 4;
	uint32_t memory_base = 10000;
	uint32_t mem_inc = 5000;
	constexpr uint32_t mem_var = 9;
	constexpr uint32_t blocks = 10;
	uint32_t heap_size = 1000;
	double skewness = 1.0 / blocks;
	Abstract<DATA_LEN>* sketches[cmp_num][blocks];

	std::vector<std::ostream*> outs(4);
	outs[0] = new std::ofstream(RESULT_FOLDER + "uniformGlobalTopK_" + file + "_ARE.csv");
	outs[1] = new std::ofstream(RESULT_FOLDER + "uniformGlobalTopK_" + file + "_Recall Rate (RR).csv");
	outs[2] = new std::ofstream(RESULT_FOLDER + "uniformGlobalTopK_" + file + "_Precision Rate (PR).csv");
	outs[3] = new std::ofstream(RESULT_FOLDER + "uniformGlobalTopK_" + file + "_F1.csv");

	for (uint32_t round = 0; round < mem_var; round++)
	{
		uint32_t memory = round * mem_inc + memory_base;
		printf("\033[0m\033[1;4;36m> Memory: %d KB\n\033[0m", memory / 1000);
		for (size_t i = 0; i < blocks; i++)
		{
			sketches[0][i] = new WavingSketch<8, 1, DATA_LEN>(memory / (8 * 8 + 1 * 2));
			sketches[1][i] = new SS<DATA_LEN>(memory / 100);
			sketches[2][i] = new USS<DATA_LEN>(memory / 100);
			sketches[3][i] = new Count_Heap<DATA_LEN>(heap_size, (memory - heap_size * (sizeof(Data<DATA_LEN>) + 4)) / 12, 3);
		}
		if (round == 0)
		{
			for (auto& out : outs)
			{
				*out << "MEM(KB)" << ",";
				for (uint32_t i = 0; i < cmp_num; i++)
				{
					*out << sketches[i][0]->name << ",";
				}
				*out << std::endl;
			}
		}
		for (auto& out : outs)
		{
			*out << memory / 1000 << ",";
		}

		HashMap<DATA_LEN> data2block;
		SplitData<DATA_LEN>(mp, skewness, cnt, blocks, data2block);
		for (int l = 0; l < cnt; ++l) {
			int pos = data2block[items[l]];
			for (int j = 0; j < cmp_num; ++j) {
				sketches[j][pos]->Init(items[l]);
			}
		}
		for (int i = 0; i < cmp_num; ++i) {
			Abstract<DATA_LEN>::DistributeCheck(mp, topK, data2block, sketches[i], outs);
			for (size_t j = 0; j < blocks; j++)
			{
				delete sketches[i][j];
			}
		}
		for (auto& out : outs)
		{
			*out << std::endl;
		}
	}
	delete items;
}