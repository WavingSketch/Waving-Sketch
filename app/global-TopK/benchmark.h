#pragma once

#include <fcntl.h>
#include <chrono>
#include <climits>
#include <iostream>
#include <vector>
#include <fstream>
#include <string.h>
#include<queue>
#include <set>
#include "Count_Heap.h"
#include "SS.h"
#include "USS.h"
#include "WavingSketch.h"
#include "ld.h"
#include "CM.h"
#include "Count.h"
typedef std::chrono::high_resolution_clock::time_point TP;
#define max(a,b) (((a)>(b))?(a):(b))
#define min(a,b) (((a)<(b))?(a):(b))
inline TP now() { return std::chrono::high_resolution_clock::now(); }

struct TUPLES {
	uint8_t array[21];
};
extern int record_length;
extern const std::string RESULT_FOLDER;
#ifndef WIN32
#include <sys/types.h>
#include <dirent.h>
void getFileNames(std::string path, std::vector<std::string>& files)
{
	DIR* pDir;
	struct dirent* ptr;
	if (!(pDir = opendir(path.c_str()))) {
		std::cout << "Folder doesn't Exist!" << std::endl;
		exit(-1);
	}
	while ((ptr = readdir(pDir)) != 0) {
		if (strcmp(ptr->d_name, ".") != 0 && strcmp(ptr->d_name, "..") != 0) {
			std::string filename(ptr->d_name);
			files.push_back(path + "/" + filename);
		}
	}
}
#else
#include<io.h>
void getFileNames(std::string path, std::vector<std::string>& files)
{
	intptr_t hFile = 0;
	struct _finddata_t fileinfo;
	std::string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			//如果是目录,递归查找
			//如果不是,把文件绝对路径存入vector中
			if ((fileinfo.attrib & _A_SUBDIR))
			{
				//if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
				//	getFileNames(p.assign(path).append("\\").append(fileinfo.name), files);
			}
			else
			{
				files.push_back(p.assign(path).append("\\").append(fileinfo.name));
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}
#endif // WIN32
data_type* read_standard_data(const char* PATH, int interval, int* cnt) {
	std::vector<data_type> items;
	uint8_t* it = new uint8_t[interval];

	FILE* data = fopen(PATH, "rb");
	if (data == NULL)
	{
		std::cout << "File don't exist\n";
		exit(0);
	}
	*cnt = 0;
	int t;
	while (fread(it, interval, 1, data) > 0)
	{
		(*cnt)++;
		data_type a = *(data_type*)it;
		items.push_back(a);
	}
	data_type* ret = new data_type[*cnt];
	for (size_t i = 0; i < *cnt; i++)
	{
		ret[i] = items[i];
	}
	fclose(data);
	return ret;
}
data_type* read_large_data(const char* folder, int interval, int* cnt) {
	struct stat buf;
	// printf("Opening file...\n");
	std::vector<std::string> filenames;
	getFileNames(folder, filenames);
	std::cout << "Toal " << filenames.size() << " file" << std::endl;
	sort(filenames.begin(), filenames.end());
	std::vector<std::pair<data_type*, uint32_t>> collections;
	uint32_t total = 0;
	for (auto& PATH : filenames)
	{
		if (PATH.find(".dat") == PATH.npos) continue;
		int num;
		data_type* data = read_standard_data(PATH.c_str(), interval, &num);
		collections.push_back(std::make_pair(data, num));
		total += num;
		std::cout << PATH << ":" << num << " items" << std::endl;
	}
	*cnt = total;
	data_type* ret = (data_type*)malloc(sizeof(data_type) * total);
	data_type* p1 = ret;
	for (auto& p : collections)
	{
		data_type* data = p.first;
		int num = p.second;
		std::copy(data, data + num, p1);
		p1 += num;
		delete data;
	}
	std::cout << "Toal " << *cnt << " items" << std::endl;
	return ret;
}
void read_data(uint32_t fid, std::string& file, data_type*& items, int32_t& cnt)
{
	if (fid == 0)
	{
		file = "demo";
		items = read_standard_data("../dataset/demo.dat", 21, &cnt);
	}
	else if (fid == 1)
	{
		file = "syn";
		items = read_standard_data("../dataset/syn.dat", 4, &cnt);
		//items = read_standard_data("../../../../datasets/zipf/syn.dat", 4, &cnt);
	}
	else if (fid == 2)
	{
		file = "caida";
		//items = read_standard_data("../dataset/CAIDA2018.dat", 21, &cnt);
		items = read_standard_data("../../../../datasets/caida/trace3.dat", 21, &cnt);
	}
	else if (fid == 3)
	{
		file = "web";
		items = read_standard_data("../dataset/webpage.dat", 8, &cnt);
	}
	else if (fid == 4)
	{
		file = "campus";
		items = read_standard_data("../dataset/campus.dat", 13, &cnt);
	}
	else if (fid == 5)
	{
		file = "large";
		items = read_large_data("/share/data2block/CAIDA2018/dataset", 21, &cnt);
	}
	else if (fid == 6)
	{
		file = "extraexp3";
		items = read_standard_data("../dataset/syn.dat", 4, &cnt);
		//items = read_standard_data("../../../../datasets/caida/trace3.dat", 21, &cnt);
	}
	else if (fid == 7)
	{
		file = "syn";
		//items = read_standard_data("../dataset/syn_waving.dat", 4, &cnt);
		items = read_standard_data("../dataset/syn_waving.dat", 4, &cnt);
	}
}
uint32_t Get_TopK(HashMap mp, uint32_t k) {
	uint32_t size = mp.size();
	uint32_t* num = new uint32_t[size];

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
void SplitData(const HashMap& mp, double skewness, uint32_t total, uint32_t blocks, HashMap& data2block)
{
	std::vector<std::pair<count_type, data_type>> lst;
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
void BenchSkewGlobalTopK(std::string PATH)
{
	printf("\033[0m\033[1;4;33m# Testing function: BenchSkewGlobalTopK\n\033[0m");
	count_type cnt = 0;
	data_type* items = NULL;
	std::string file = "caida";

	items = read_standard_data(PATH.c_str(), record_length, &cnt);
	std::cout << "Total " << cnt << " items" << std::endl;
	// Ground truth
	HashMap mp;
	for (int l = 0; l < cnt; ++l) {
		if (mp.find(items[l]) == mp.end())
			mp[items[l]] = 1;
		else
			mp[items[l]] += 1;
	}
	uint32_t topK = Get_TopK(mp, 2000);
	constexpr uint32_t cmp_num = 4;
	constexpr uint32_t memory = 10000;
	constexpr uint32_t blocks = 100, epochs = 2;
	uint32_t heap_size = 1000;
	std::vector<double> skewnesses{ 0.01,0.02,0.03,0.05,0.1,0.2,0.3,0.4,0.5 };
	uint32_t skew_var = skewnesses.size();
	Abstract* sketches[cmp_num][blocks];

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
				sketches[0][i] = new WavingSketch<8, 1>(memory / (8 * 8 + 1 * 2));
				sketches[1][i] = new SS(memory / 100);
				sketches[2][i] = new USS(memory / 100);
				sketches[3][i] = new Count_Heap(heap_size, (memory - heap_size * sizeof(Heap::Counter)) / 12, 3);
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
			HashMap data2block;
			SplitData(mp, skewness, cnt, blocks, data2block);
			for (int l = 0; l < cnt; ++l) {
				int pos = data2block[items[l]];
				for (int j = 0; j < cmp_num; ++j) {
					sketches[j][pos]->Insert(items[l]);
				}
			}
			std::vector<double> result_in_epoch(4);
			for (int i = 0; i < cmp_num; ++i) {
				Abstract::DistributeCheck(mp, topK, data2block, sketches[i], result_in_epoch);
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
				names[i], are, cr, pr, f1);
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
void BenchUniformGlobalTopK(std::string PATH)
{
	printf("\033[0m\033[1;4;33m# Testing function: BenchUniformGlobalTopK\n\033[0m");
	count_type cnt = 0;
	data_type* items = NULL;
	std::string file = "caida";

	items = read_standard_data(PATH.c_str(), record_length, &cnt);
	std::cout << "Total " << cnt << " items" << std::endl;
	// Ground truth
	HashMap mp;
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
	Abstract* sketches[cmp_num][blocks];

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
			sketches[0][i] = new WavingSketch<8, 1>(memory / (8 * 8 + 1 * 2));
			sketches[1][i] = new SS(memory / 100);
			sketches[2][i] = new USS(memory / 100);
			sketches[3][i] = new Count_Heap(heap_size, (memory - heap_size * sizeof(Heap::Counter)) / 12, 3);
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

		HashMap data2block;
		SplitData(mp, skewness, cnt, blocks, data2block);
		for (int l = 0; l < cnt; ++l) {
			int pos = data2block[items[l]];
			for (int j = 0; j < cmp_num; ++j) {
				sketches[j][pos]->Insert(items[l]);
			}
		}
		for (int i = 0; i < cmp_num; ++i) {
			Abstract::DistributeCheck(mp, topK, data2block, sketches[i], outs);
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