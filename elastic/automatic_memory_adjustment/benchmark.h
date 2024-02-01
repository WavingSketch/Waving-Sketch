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
void BenchElastic(std::string PATH) {
	printf("\033[0m\033[1;4;33m# Testing function: BenchElastic\n\033[0m");
	count_type cnt = 0;
	data_type* items = NULL;
	std::string file = "caida";

	items = read_standard_data(PATH.c_str(), record_length, &cnt);

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

	WavingSketch<8, 1>* sketches[cmp_num];

	sketches[0] = new WavingSketch<8, 1>(memory_upper_bound / (8 * 8 + 1 * 2));

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
	HashMap mp;
	std::multiset<std::pair<uint32_t, data_type>> topk_items;
	for (int l = 0; l < cnt; ++l) {
		std::pair<uint32_t, data_type> p = std::make_pair(mp[items[l]], items[l]);
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
			bool is_hit = sketches[j]->QueryTopK(items[l]) > 0;
			hits[j].push(is_hit);
			total[j] += is_hit;
			sketches[j]->Insert(items[l]);
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