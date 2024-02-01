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
// Basic test function, to test ARE, CR, PR
void BenchCmp(std::string PATH) {
	printf("\033[0m\033[1;4;33m# Testing function: BenchCmp\n\033[0m");
	count_type cnt = 0;
	data_type* items = NULL;
	std::string file = "caida";

	items = read_standard_data(PATH.c_str(), record_length, &cnt);
	std::cout << "Total " << cnt << " items" << std::endl;
	int32_t mem_base = 0;
	int32_t mem_inc = 500000;
	constexpr int32_t mem_var = 6;
	constexpr int32_t cmp_num = 6;
	int heap_size = 2500, K = 2000;
	// constexpr int32_t num_inc = 10;
	int nshrink = 0;
	// Ground truth
	HashMap mp;
	for (int l = 0; l < cnt; ++l) {
		if (mp.find(items[l]) == mp.end())
			mp[items[l]] = 1;
		else
			mp[items[l]] += 1;
	}
	uint32_t topK = Get_TopK(mp, K);

	Abstract* sketches[mem_var][cmp_num];

	for (int i = 0; i < mem_var; ++i) {
		int memory = (mem_base + mem_inc * (i + 1));
		sketches[i][0] = new WavingSketch<8, 1>(memory / (8 * 8 + 1 * 2));
		sketches[i][1] = new WavingSketch<8, 16>(memory / (8 * 8 + 16 * 2));
		sketches[i][2] = new SS(memory / 100);
		sketches[i][3] = new USS(memory / 100);
		sketches[i][4] = new Count_Heap(heap_size, (memory - heap_size * sizeof(Heap::Counter)) / 12, 3);
		sketches[i][5] = new LdSketchWrapper(memory, topK);
	}

	std::vector<std::ostream*> outs(4);
	outs[0] = new std::ofstream(RESULT_FOLDER+"frequent_" + file + "_ARE.csv");
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
			((LdSketchWrapper*)sketches[i][5])->average_l());
		for (int l = 0; l < cnt; ++l) {
			for (int j = 0; j < cmp_num; ++j) {
				sketches[i][j]->Insert(items[l]);
			}
		}
		printf("After average l: %lf\n",
			((LdSketchWrapper*)sketches[i][5])->average_l());

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
void BenchThp(std::string PATH) {
	printf("\033[0m\033[1;4;33m# Testing function: BenchThp\n\033[0m");
	count_type cnt = 0;
	data_type* items = NULL;
	std::string file = "caida";

	items = read_standard_data(PATH.c_str(), record_length, &cnt);
	std::cout << "Total " << cnt << " items" << std::endl;
	int32_t mem_base = 0;
	int32_t mem_inc = 50000;
	constexpr int32_t mem_var = 6;
	constexpr int32_t cmp_num = 6;
	constexpr int32_t round = 1;
	int heap_size = 2500, K = 2000;
	std::ofstream out(RESULT_FOLDER + "frequent_" + file + "_Throughput(Mops).csv");
	HashMap mp;
	for (int l = 0; l < cnt; ++l) {
		if (mp.find(items[l]) == mp.end())
			mp[items[l]] = 1;
		else
			mp[items[l]] += 1;
	}
	uint32_t topK = Get_TopK(mp, K);

	Abstract* sketches[mem_var][cmp_num];

	int progress = 0;

	for (int i = 0; i < mem_var; ++i) {
		printf("\033[0m\033[1;4;36m> Memory size: %dKB\n\033[0m",
			(mem_base + mem_inc * (i + 1)) / 1000);
		double thp[round][cmp_num] = {};
		double avg_thp[cmp_num] = {};
		for (int j = 0; j < round; ++j) {
			int memory = (mem_base + mem_inc * (i + 1));
			sketches[i][0] = new WavingSketch<8, 1>(memory / (8 * 8 + 1 * 2));
			sketches[i][1] = new WavingSketch<8, 16>(memory / (8 * 8 + 16 * 2));
			sketches[i][2] = new SS(memory / 100);
			sketches[i][3] = new USS(memory / 100);
			sketches[i][4] = new Count_Heap(heap_size, (memory - heap_size * sizeof(Heap::Counter)) / 12, 3);
			sketches[i][5] = new LdSketchWrapper(memory, topK);

			for (int l = 0; l < cmp_num; ++l) {
				TP start, finish;

				start = now();
				for (int m = 0; m < cnt; ++m) {
					sketches[i][l]->Insert(items[m]);
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
				sketches[i][l]->name, avg_thp[l] / round);
			out << avg_thp[l] / round << ",";
			delete sketches[i][l];
		}
		out << std::endl;
	}

	delete items;
	out.close();
}