#pragma once
#include <climits>
#include <vector>
#include <string.h>
#include<queue>
#include <set>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <unordered_map>
#include "data.h"
#include "utils.h"
const int MAX_PACKET = 1e7;
extern int record_length;
extern const std::string RESULT_FOLDER;
typedef std::chrono::high_resolution_clock::time_point TP;
inline TP now() { return std::chrono::high_resolution_clock::now(); }
template<uint32_t DATA_LEN>
int Get_TopK(HashMap<DATA_LEN> mp, int k) {
	int size = mp.size();
	int* num = new int[size];
	int pos = 0;
	typename HashMap<DATA_LEN>::iterator it;
	for (it = mp.begin(); it != mp.end(); ++it) {
		num[pos++] = it->second;
	}
	nth_element(num, num + size - k, num + size);
	int ret = num[size - k];
	delete num;
	return ret;
}
template<uint32_t DATA_LEN>
Data<DATA_LEN>* read_standard_data(const char* PATH, int interval, int* cnt) {
	struct stat buf;
	// printf("Opening file...\n");
	int fd = open(PATH, O_RDONLY);
	fstat(fd, &buf);

	*cnt = buf.st_size / interval;
	uint8_t* raw_addr = (uint8_t*)mmap(NULL, buf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	close(fd);
	if (raw_addr == MAP_FAILED) {
		printf("[ERROR] MMAP FAILED!\n");
		exit(-1);
	}

	int num = *cnt;
	void* raw_data = malloc(DATA_LEN * num);
	if (raw_data == NULL) {
		printf("[ERROR] MALLOC FAILED!\n");
		exit(-1);
	}

	Data<DATA_LEN>* data = reinterpret_cast<Data<DATA_LEN>*>(raw_data);
	for (int i = 0; i < num; i++)
		data[i] = *reinterpret_cast<Data<DATA_LEN>*>(raw_addr + interval * i);
	munmap(raw_addr, buf.st_size);
	return data;
}
template<uint32_t DATA_LEN>
Data<DATA_LEN>* read_standard_pair_data(const char* PATH, int interval, int* cnt) {
	struct stat buf;
	// printf("Opening file...\n");
	int fd = open(PATH, O_RDONLY);
	fstat(fd, &buf);

	*cnt = buf.st_size / interval;
	uint8_t* raw_addr = (uint8_t*)mmap(NULL, buf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	close(fd);
	if (raw_addr == MAP_FAILED) {
		printf("[ERROR] MMAP FAILED!\n");
		exit(-1);
	}

	int num = *cnt;
	void* raw_data = malloc(DATA_LEN * num * 2);
	if (raw_data == NULL) {
		printf("[ERROR] MALLOC FAILED!\n");
		exit(-1);
	}

	Data<DATA_LEN>* data = reinterpret_cast<Data<DATA_LEN>*>(raw_data);
	for (int i = 0; i < num; i++) {
		data[2 * i] = *reinterpret_cast<Data<DATA_LEN>*>(raw_addr + interval * i);
		data[2 * i + 1] =
			*reinterpret_cast<Data<DATA_LEN>*>(raw_addr + interval * i + DATA_LEN);
	}
	munmap(raw_addr, buf.st_size);
	return data;
}
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
template<uint32_t DATA_LEN>
Data<DATA_LEN>* read_large_data(const char* folder, int interval, int* cnt) {
	struct stat buf;
	// printf("Opening file...\n");
	std::vector<std::string> filenames;
	getFileNames(folder, filenames);
	std::cout << "Toal " << filenames.size() << " file" << std::endl;
	sort(filenames.begin(), filenames.end());
	std::vector<std::pair<Data<DATA_LEN>*, uint32_t>> collections;
	uint32_t total = 0;
	for (auto& PATH : filenames)
	{
		if (PATH.find(".dat") == PATH.npos) continue;
		int num;
		Data<DATA_LEN>* data = read_standard_data<DATA_LEN>(PATH.c_str(), interval, &num);
		collections.push_back(std::make_pair(data, num));
		total += num;
		std::cout << PATH << ":" << num << " items" << std::endl;
	}
	*cnt = total;
	Data<DATA_LEN>* ret = (Data<DATA_LEN>*)malloc(sizeof(Data<DATA_LEN>) * total);
	Data<DATA_LEN>* p1 = ret;
	for (auto& p : collections)
	{
		Data<DATA_LEN>* data = p.first;
		int num = p.second;
		std::copy(data, data + num, p1);
		p1 += num;
		delete data;
	}
	std::cout << "Toal " << *cnt << " items" << std::endl;
	return ret;
}
/*void read_data(uint32_t fid, std::string& file, data_type*& items, int32_t& cnt)
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
		//items = read_standard_data("/share/zipf_2022/zipf_1.0.dat", 4, &cnt);
		//items = read_standard_data("../../../../datasets/zipf/syn.dat", 4, &cnt);
	}
	else if (fid == 2)
	{
		file = "caida";
		items = read_standard_data("../dataset/CAIDA2018.dat", 21, &cnt);
		//items = read_standard_data("../../../../datasets/caida/trace3.dat", 21, &cnt);
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
}*/