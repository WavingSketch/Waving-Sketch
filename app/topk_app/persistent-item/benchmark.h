#pragma once

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
#include<vector>
#include "Waving.h"
#include "abstract.h"
#include "on_off_sketch.h"
#include "pie.h"
#include "small_space.h"

using namespace std;

#define BLOCK 40000
#define HIT 243
const int MAX_PACKET = 1e7;
extern int record_length;
extern const std::string RESULT_FOLDER;
typedef std::chrono::high_resolution_clock::time_point TP;
inline TP now() { return std::chrono::high_resolution_clock::now(); }
int Get_TopK(HashMap mp, int k) {
	int size = mp.size();
	int* num = new int[size];
	int pos = 0;
	HashMap::iterator it;
	for (it = mp.begin(); it != mp.end(); ++it) {
		num[pos++] = it->second;
	}
	nth_element(num, num + size - k, num + size);
	int ret = num[size - k];
	delete num;
	return ret;
}

Data* read_standard_data(const char* PATH, int interval, int* cnt) {
	struct stat buf;
	// printf("Opening file...\n");
	int fd = open(PATH, O_RDONLY);
	fstat(fd, &buf);

	*cnt = buf.st_size / interval;
	void* raw_addr = mmap(NULL, buf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	close(fd);
	if (raw_addr == MAP_FAILED) {
		printf("[ERROR] MMAP FAILED!\n");
		exit(-1);
	}

	int num = *cnt;
	void* raw_data = malloc(sizeof(Data) * num);
	if (raw_data == NULL) {
		printf("[ERROR] MALLOC FAILED!\n");
		exit(-1);
	}

	Data* data = reinterpret_cast<Data*>(raw_data);
	for (int i = 0; i < num; i++)
		data[i] = *reinterpret_cast<Data*>(raw_addr + interval * i);
	munmap(raw_addr, buf.st_size);
	std::cout << "Toal " << *cnt << " items" << std::endl;
	return data;
}

void Test_Hitter(string PATH) {
	std::vector<std::ostream*> outs(4);
	std::string file = "caida";
	outs[0] = new std::ofstream(RESULT_FOLDER + "persistent_" + file + "_Recall Rate (RR).csv");
	outs[1] = new std::ofstream(RESULT_FOLDER + "persistent_" + file + "_Precision Rate (PR).csv");
	outs[2] = new std::ofstream(RESULT_FOLDER + "persistent_" + file + "_F1.csv");
	outs[3] = new std::ofstream(RESULT_FOLDER + "persistent_" + file + "_ARE.csv");
	int cnt;
	auto records = read_standard_data(PATH.c_str(), record_length, &cnt);
	cnt = std::min(cnt, MAX_PACKET);
	for (int i = 0; i < 5; ++i) {
		constexpr int SKETCH_NUM = 4;
		int memory = BLOCK * (i + 1);
		printf("\033[0m\033[1;4;36m> Memory size: %dKB\n\033[0m", memory / 1000);
		Abstract* sketch[SKETCH_NUM];
		sketch[0] = new WavingSketch<8, 16>(memory, HIT);
		sketch[1] = new Small_Space(memory, HIT);
		sketch[2] = new PIE(memory * 200, HIT, 1600);
		sketch[3] = new OO_FPI(memory, HIT);
		if (i == 0)
		{
			for (auto& out : outs)
			{
				*out << "MEM(KB)" << ",";
				for (uint32_t j = 0; j < SKETCH_NUM; j++)
				{
					*out << sketch[j]->name << ",";
				}
				*out << std::endl;
			}
		}
		for (auto& out : outs)
		{
			*out << memory / 1000 << ",";
		}
		HashMap mp;
		HashMap cycle;
		Data packet;
		uint num = 0;
		uint time = 0;
		// int t;

		while (num < cnt) {
			packet = records[num];
			if (num % 19537 == 0) {
				time += 1;
				cycle.clear();
			}
			num++;

			if (mp.find(packet) == mp.end()) {
				mp[packet] = 1;
				cycle[packet] = 1;
			}
			else if (cycle.find(packet) == cycle.end()) {
				cycle[packet] = 1;
				mp[packet] += 1;
			}

			for (int j = 0; j < SKETCH_NUM; ++j) {
				sketch[j]->Init(packet, time);
			}
		}

		for (int j = 0; j < SKETCH_NUM; ++j) {
			sketch[j]->Check(mp, outs);
		}

		ofstream out;

		for (int j = 0; j < SKETCH_NUM; ++j) {
			printf("\033[0m\033[1;36m|\033[0m\t");
			sketch[j]->print_f1(out, memory);
		}

		for (int j = 0; j < SKETCH_NUM; ++j) {
			delete sketch[j];
		}
		for (auto& out : outs)
		{
			*out << std::endl;
		}
	}
	free(records);
}

void Test_Speed(string PATH) {
	const int CYCLE = 1;
	int cnt;
	auto records = read_standard_data(PATH.c_str(), record_length, &cnt);
	cnt = std::min(cnt, MAX_PACKET);
	std::string file = "caida";
	std::ofstream out(RESULT_FOLDER + "persistent_" + file + "_Throughput(Mops).csv");
	for (int i = 0; i < 5; ++i) {
		int memory = BLOCK * (i + 1);
		printf("\033[0m\033[1;4;36m> Memory size: %dKB\n\033[0m", memory / 1000);
		if (i == 0)
		{
			out << "MEM(KB)" << "," << "WavingSketch<8_16>" << "," << "Small-Space" << ","
				<< "PIE" << "," << "On Off Sketch" << ",";
			out << std::endl;
		}
		out << memory / 1000 << ",";
		for (int k = 0; k < CYCLE; ++k) {
			WavingSketch<8, 16> sketch(memory, HIT);
			Data packet;
			int num = 0;
			uint time = 0;

			TP start = now();
			while (num < cnt) {
				packet = records[num];
				if (num % 19537 == 0) {
					++time;
				}
				++num;

				sketch.Init(packet, time);
			}
			TP finish = now();

			double duration =
				(double)std::chrono::duration_cast<
				std::chrono::duration<double, std::ratio<1, 1000000>>>(finish -
					start)
				.count();
			printf("\033[0m\033[1;36m|\033[0m\t");
			cout << sketch.name << sketch.sep << (num + 0.0) / duration << endl;
			out << (num + 0.0) / duration << ",";
		}

		for (int k = 0; k < CYCLE; ++k) {
			Small_Space sketch(memory, HIT);
			Data packet;
			int num = 0;
			uint time = 0;

			TP start = now();
			while (num < cnt) {
				packet = records[num];
				if (num % 19537 == 0) {
					++time;
				}
				++num;

				sketch.Init(packet, time);
			}
			TP finish = now();

			double duration =
				(double)std::chrono::duration_cast<
				std::chrono::duration<double, std::ratio<1, 1000000>>>(finish -
					start)
				.count();
			printf("\033[0m\033[1;36m|\033[0m\t");
			cout << sketch.name << sketch.sep << (num + 0.0) / duration << endl;
			out << (num + 0.0) / duration << ",";
		}

		for (int k = 0; k < CYCLE; ++k) {
			PIE sketch(memory * 200, HIT, 1600);
			Data packet;
			int num = 0;
			uint time = 0;

			TP start = now();
			while (num < cnt) {
				packet = records[num];
				if (num % 19537 == 0) {
					++time;
				}
				++num;

				sketch.Init(packet, time);
			}
			TP finish = now();

			double duration =
				(double)std::chrono::duration_cast<
				std::chrono::duration<double, std::ratio<1, 1000000>>>(finish -
					start)
				.count();
			printf("\033[0m\033[1;36m|\033[0m\t");
			cout << sketch.name << sketch.sep << (num + 0.0) / duration << endl;
			out << (num + 0.0) / duration << ",";
		}

		for (int k = 0; k < CYCLE; ++k) {
			OO_FPI sketch(memory, HIT);
			Data packet;
			int num = 0;
			uint time = 0;

			TP start = now();
			while (num < cnt) {
				packet = records[num];
				if (num % 19537 == 0) {
					++time;
				}
				++num;

				sketch.Init(packet, time);
			}
			TP finish = now();

			double duration =
				(double)std::chrono::duration_cast<
				std::chrono::duration<double, std::ratio<1, 1000000>>>(finish -
					start)
				.count();
			printf("\033[0m\033[1;36m|\033[0m\t");
			cout << sketch.name << sketch.sep << (num + 0.0) / duration << endl;
			out << (num + 0.0) / duration << ",";
		}
		out << endl;
	}
	free(records);
}