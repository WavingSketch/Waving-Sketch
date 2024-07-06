#pragma once



#include"../../../../include/common.h"
#include "../../../../include/Waving.h"
#include "../../../../include/abstract.h"
#include "../../../../include/olf.h"
#include "../../../../include/opensketch.h"
#include "../../../../include/SpreadSketch.h"
#include "../../../../include/tlf.h"

#define SuperSpread_BLOCK 100000
#define SuperSpread_HIT 84

template<uint32_t DATA_LEN>
void SuperSpread_Test_Hitter(string PATH) {
	std::vector<std::ostream*> outs(4);
	std::string file = "caida";
	outs[0] = new std::ofstream(RESULT_FOLDER + "superSpread_" + file + "_ARE.csv");
	outs[1] = new std::ofstream(RESULT_FOLDER + "superSpread_" + file + "_Recall Rate (RR).csv");
	outs[2] = new std::ofstream(RESULT_FOLDER + "superSpread_" + file + "_Precision Rate (PR).csv");
	outs[3] = new std::ofstream(RESULT_FOLDER + "superSpread_" + file + "_F1.csv");
	for (int i = 0; i < 5; ++i) {
		int memory = 6 * SuperSpread_BLOCK + SuperSpread_BLOCK * i;
		printf("\033[0m\033[1;4;36m> Memory size: %dKB\n\033[0m", memory / 1000);
		int SKETCH_NUM = 5;
		Abstract<DATA_LEN>* sketch[SKETCH_NUM];
		sketch[0] = new WavingSketch<8, 16, DATA_LEN>(memory, SuperSpread_HIT);
		sketch[1] = new TLF<DATA_LEN>(memory, SuperSpread_HIT);
		sketch[2] = new OLF<DATA_LEN>(memory, SuperSpread_HIT);
		sketch[3] = new OpenSketch<DATA_LEN>(memory, SuperSpread_HIT);
		sketch[4] = new SpreadSketchWrapper<DATA_LEN>(memory, SuperSpread_HIT);

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
		StreamMap<DATA_LEN> sp;

		int cnt;
		Data<DATA_LEN>* records = read_standard_pair_data<DATA_LEN>(PATH.c_str(), record_length, &cnt);
		Data<DATA_LEN> from;
		Data<DATA_LEN> to;
		int num = 0;
		cnt = std::min(cnt, MAX_PACKET);
		while (num < cnt) {
			from = records[num * 2];
			to = records[num * 2 + 1];

			/*
			num++;
			if(num % 1000000 == 0){
				cout << num << endl;
			}
			*/

			Stream<DATA_LEN> stream(from, to);
			if (sp.find(stream) == sp.end()) {
				sp[stream] = 1;
				if (mp.find(from) == mp.end())
					mp[from] = 1;
				else
					mp[from] += 1;
			}
			for (int j = 0; j < SKETCH_NUM; ++j) {
				sketch[j]->Init(from, to);
			}
			num++;
		}
		free(records);
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
}
template<uint32_t DATA_LEN>
void SuperSpread_Test_Speed(string PATH) {
	const int CYCLE = 1;
	Data<DATA_LEN> from;
	Data<DATA_LEN> to;
	std::string file = "caida";
	std::ofstream out(RESULT_FOLDER + "superSpread_" + file + "_Throughput(Mops).csv");
	for (int i = 0; i < 5; ++i) {
		int memory = 6 * SuperSpread_BLOCK + SuperSpread_BLOCK * i;
		printf("\033[0m\033[1;4;36m> Memory size: %dKB\n\033[0m", memory / 1000);

		if (i == 0)
		{
			out << "MEM(KB)" << "," << "WavingSketch<8,1>" << "," << "TLF" << ","
				<< "OLF" << "," << "opensketch" << "," << "SpreadSketch";
			out << std::endl;
		}
		out << memory / 1000 << ",";

		for (int k = 0; k < CYCLE; ++k) {
			WavingSketch<8, 16, DATA_LEN> sketch(memory, SuperSpread_HIT);
			int cnt;
			Data<DATA_LEN>* records = read_standard_pair_data<DATA_LEN>(PATH.c_str(), record_length, &cnt);
			int num = 0;

			TP start = now();
			cnt = std::min(cnt, MAX_PACKET);
			while (num < cnt) {
				from = records[num * 2];
				to = records[num * 2 + 1];
				++num;
				sketch.Init(from, to);
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
			free(records);
		}

		for (int k = 0; k < CYCLE; ++k) {
			TLF<DATA_LEN> sketch(memory, SuperSpread_HIT);
			int cnt;
			Data<DATA_LEN>* records = read_standard_pair_data<DATA_LEN>(PATH.c_str(), record_length, &cnt);
			int num = 0;

			TP start = now();
			cnt = std::min(cnt, MAX_PACKET);
			while (num < cnt) {
				from = records[num * 2];
				to = records[num * 2 + 1];
				++num;
				sketch.Init(from, to);
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
			free(records);
		}

		for (int k = 0; k < CYCLE; ++k) {
			OLF<DATA_LEN> sketch(memory, SuperSpread_HIT);
			int cnt;
			Data<DATA_LEN>* records = read_standard_pair_data<DATA_LEN>(PATH.c_str(), record_length, &cnt);
			int num = 0;

			TP start = now();
			cnt = std::min(cnt, MAX_PACKET);
			while (num < cnt) {
				from = records[num * 2];
				to = records[num * 2 + 1];
				++num;
				sketch.Init(from, to);
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
			free(records);
		}

		for (int k = 0; k < CYCLE; ++k) {
			OpenSketch<DATA_LEN> sketch(memory, SuperSpread_HIT);
			int cnt;
			Data<DATA_LEN>* records = read_standard_pair_data<DATA_LEN>(PATH.c_str(), record_length, &cnt);
			int num = 0;

			TP start = now();
			cnt = std::min(cnt, MAX_PACKET);
			while (num < cnt) {
				from = records[num * 2];
				to = records[num * 2 + 1];
				++num;
				sketch.Init(from, to);
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
			free(records);
		}

		for (int k = 0; k < CYCLE; ++k) {
			SpreadSketchWrapper<DATA_LEN> sketch(memory, SuperSpread_HIT);
			int cnt;
			Data<DATA_LEN>* records = read_standard_pair_data<DATA_LEN>(PATH.c_str(), record_length, &cnt);
			int num = 0;

			TP start = now();
			cnt = std::min(cnt, MAX_PACKET);
			while (num < cnt) {
				from = records[num * 2];
				to = records[num * 2 + 1];
				++num;
				sketch.Init(from, to);
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
			free(records);
		}
		out << endl;
	}
}
