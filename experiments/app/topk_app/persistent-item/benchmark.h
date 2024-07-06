#pragma once
#include"../../../../include/common.h"
#include "../../../../include/Waving.h"
#include "../../../../include/abstract.h"
#include "../../../../include/on_off_sketch.h"
#include "../../../../include/pie.h"
#include "../../../../include/small_space.h"

using namespace std;

#define Persistent_BLOCK 40000
#define Persistent_HIT 243
template<uint32_t DATA_LEN>
void Persistent_Test_Hitter(string PATH) {
	std::vector<std::ostream*> outs(4);
	std::string file = "caida";
	outs[0] = new std::ofstream(RESULT_FOLDER + "persistent_" + file + "_Recall Rate (RR).csv");
	outs[1] = new std::ofstream(RESULT_FOLDER + "persistent_" + file + "_Precision Rate (PR).csv");
	outs[2] = new std::ofstream(RESULT_FOLDER + "persistent_" + file + "_F1.csv");
	outs[3] = new std::ofstream(RESULT_FOLDER + "persistent_" + file + "_ARE.csv");
	int cnt;
	auto records = read_standard_data<DATA_LEN>(PATH.c_str(), record_length, &cnt);
	cnt = std::min(cnt, MAX_PACKET);
	for (int i = 0; i < 5; ++i) {
		constexpr int SKETCH_NUM = 4;
		int memory = Persistent_BLOCK * (i + 1);
		printf("\033[0m\033[1;4;36m> Memory size: %dKB\n\033[0m", memory / 1000);
		Abstract<DATA_LEN>* sketch[SKETCH_NUM];
		sketch[0] = new WavingSketch<8, 16, DATA_LEN>(memory, Persistent_HIT);
		sketch[1] = new Small_Space<DATA_LEN>(memory, Persistent_HIT);
		sketch[2] = new PIE<DATA_LEN>(memory * 200, Persistent_HIT, 1600);
		sketch[3] = new OO_FPI<DATA_LEN>(memory, Persistent_HIT);
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
		HashMap<DATA_LEN> mp;
		HashMap<DATA_LEN> cycle;
		Data<DATA_LEN> packet;
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
template<uint32_t DATA_LEN>
void Persistent_Test_Speed(string PATH) {
	const int CYCLE = 1;
	int cnt;
	auto records = read_standard_data<DATA_LEN>(PATH.c_str(), record_length, &cnt);
	cnt = std::min(cnt, MAX_PACKET);
	std::string file = "caida";
	std::ofstream out(RESULT_FOLDER + "persistent_" + file + "_Throughput(Mops).csv");
	for (int i = 0; i < 5; ++i) {
		int memory = Persistent_BLOCK * (i + 1);
		printf("\033[0m\033[1;4;36m> Memory size: %dKB\n\033[0m", memory / 1000);
		if (i == 0)
		{
			out << "MEM(KB)" << "," << "WavingSketch<8_16>" << "," << "Small-Space" << ","
				<< "PIE" << "," << "On Off Sketch" << ",";
			out << std::endl;
		}
		out << memory / 1000 << ",";
		for (int k = 0; k < CYCLE; ++k) {
			WavingSketch<8, 16, DATA_LEN> sketch(memory, Persistent_HIT);
			Data<DATA_LEN> packet;
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
			Small_Space<DATA_LEN> sketch(memory, Persistent_HIT);
			Data<DATA_LEN> packet;
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
			PIE<DATA_LEN> sketch(memory * 200, Persistent_HIT, 1600);
			Data<DATA_LEN> packet;
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
			OO_FPI<DATA_LEN> sketch(memory, Persistent_HIT);
			Data<DATA_LEN> packet;
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