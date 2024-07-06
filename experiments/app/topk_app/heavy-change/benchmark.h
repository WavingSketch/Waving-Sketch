#pragma once



#include"../../../../include/common.h"
#include "../../../../include/Waving.h"
#include "../../../../include/abstract.h"
#include "../../../../include/fr.h"
#include "../../../../include/fr_cf.h"
#include "../../../../include/ld.h"
#define HeavyChange_BLOCK 250000
#define HeavyChange_HIT  1742

template<uint32_t DATA_LEN>
void HeavyChange_Test_Hitter(string PATH) {
	std::vector<std::ostream*> outs(3);
	std::string file = "caida";
	outs[0] = new std::ofstream(RESULT_FOLDER + "heavychange_" + file + "_Recall Rate (RR).csv");
	outs[1] = new std::ofstream(RESULT_FOLDER + "heavychange_" + file + "_Precision Rate (PR).csv");
	outs[2] = new std::ofstream(RESULT_FOLDER + "heavychange_" + file + "_F1.csv");
	for (int i = 0; i < 5; ++i) {
		int memory = 8 * HeavyChange_BLOCK + 2 * HeavyChange_BLOCK * i;
		printf("\033[0m\033[1;4;36m> Memory size: %dKB\n\033[0m", memory / 1000);
		int SKETCH_NUM = 10;
		auto mp = HashMap<DATA_LEN>();
		Abstract<DATA_LEN>* sketch[SKETCH_NUM];
		sketch[0] = new FR<DATA_LEN>(memory, HeavyChange_HIT);
		sketch[1] = new FR_CF<DATA_LEN>(memory, HeavyChange_HIT);
		sketch[2] = new WavingSketch<8, 1, DATA_LEN>(memory / 10, HeavyChange_HIT);
		sketch[3] = new WavingSketch<8, 16, DATA_LEN>(memory / 10, HeavyChange_HIT);
		sketch[4] = new LdSketchWrapper<DATA_LEN>(memory / 10, HeavyChange_HIT);
		sketch[5] = new FR<DATA_LEN>(memory, HeavyChange_HIT);
		sketch[6] = new FR_CF<DATA_LEN>(memory, HeavyChange_HIT);
		sketch[7] = new WavingSketch<8, 1, DATA_LEN>(memory / 10, HeavyChange_HIT);
		sketch[8] = new WavingSketch<8, 16, DATA_LEN>(memory / 10, HeavyChange_HIT);
		sketch[9] = new LdSketchWrapper<DATA_LEN>(memory / 10, HeavyChange_HIT);

		if (i == 0)
		{
			for (auto& out : outs)
			{
				*out << "MEM(KB)" << ",";
				for (uint32_t j = 0; j < SKETCH_NUM / 2; j++)
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

		int record_count;
		auto records =
			read_standard_data<DATA_LEN>(PATH.c_str(), record_length, &record_count);
		Data<DATA_LEN> packet;
		int t;
		int num = 0;
		int slot_packets = std::min(record_count / 2, MAX_PACKET);
		while (num < slot_packets) {
			packet = records[num];
			if (mp.find(packet) == mp.end())
				mp[packet] = 1;
			else
				mp[packet] += 1;
			num++;

			for (int j = 0; j < SKETCH_NUM / 2; ++j) {
				sketch[j]->Init(packet);
			}
		}

		while (num < 2 * slot_packets) {
			packet = records[num];
			if (mp.find(packet) == mp.end())
				mp[packet] = -1;
			else
				mp[packet] -= 1;
			num++;

			for (int j = SKETCH_NUM / 2; j < SKETCH_NUM; ++j) {
				sketch[j]->Init(packet);
			}
		}

		free(records);

		for (int j = 0; j < SKETCH_NUM / 2; ++j) {
			sketch[j]->Check(mp, sketch[j + (SKETCH_NUM / 2)], outs);
		}

		ofstream out;
		for (int j = 0; j < SKETCH_NUM / 2; ++j) {
			printf("\033[0m\033[1;36m|\033[0m\t");
			sketch[j]->print_f1(out, memory);
		}

		printf("After average l first half: %lf\n",
			((LdSketchWrapper<DATA_LEN>*)sketch[4])->average_l());
		printf("After average l second half: %lf\n",
			((LdSketchWrapper<DATA_LEN>*)sketch[4])->average_l());
		for (auto& out : outs)
		{
			*out << std::endl;
		}
	}
}
template<uint32_t DATA_LEN>
void HeavyChange_Test_Speed(string PATH) {
	int record_count;
	auto records = read_standard_data<DATA_LEN>(PATH.c_str(), record_length, &record_count);
	const int CYCLE = 1;
	std::string file = "caida";
	std::ofstream out(RESULT_FOLDER + "heavychange_" + file + "_Throughput(Mops).csv");
	int slot_packets = std::min(record_count / 2, MAX_PACKET);
	for (int i = 0; i < 5; ++i) {
		int memory = 8 * HeavyChange_BLOCK + 2 * HeavyChange_BLOCK * i;
		printf("\033[0m\033[1;4;36m> Memory size: %dKB\n\033[0m", memory / 1000);
		if (i == 0)
		{
			out << "MEM(KB)" << "," << "FR" << "," << "FR+CF" << ","
				<< "WavingSketch<8,1>" << "," << "WavingSketch<8_16>" << "," << "LD Sketch";
			out << std::endl;
		}
		out << memory / 1000 << ",";
		for (int k = 0; k < CYCLE; ++k) {
			FR<DATA_LEN> sketch_first(memory, HeavyChange_HIT);
			FR<DATA_LEN> sketch_second(memory, HeavyChange_HIT);
			Data<DATA_LEN> packet;
			int t;
			int num = 0;

			TP start = now();
			while (num < slot_packets) {
				packet = records[num];
				++num;

				sketch_first.Init(packet);
			}

			while (num < 2 * slot_packets) {
				packet = records[num];
				// fread(&t, DATA_LEN, 1, file);
				++num;

				sketch_second.Init(packet);
			}
			TP finish = now();

			double duration =
				(double)std::chrono::duration_cast<
				std::chrono::duration<double, std::ratio<1, 1000000>>>(finish -
					start)
				.count();
			printf("\033[0m\033[1;36m|\033[0m\t");
			cout << "FR "
				<< "\t\t\t" << (num + 0.0) / duration << endl;
			out << (num + 0.0) / duration << ",";
		}

		for (int k = 0; k < CYCLE; ++k) {
			FR_CF<DATA_LEN> sketch_first(memory, HeavyChange_HIT);
			FR_CF<DATA_LEN> sketch_second(memory, HeavyChange_HIT);
			Data<DATA_LEN> packet;
			int t;
			int num = 0;

			TP start = now();
			while (num < slot_packets) {
				packet = records[num];
				++num;

				sketch_first.Init(packet);
			}

			while (num < 2 * slot_packets) {
				packet = records[num];
				num++;

				sketch_second.Init(packet);
			}
			TP finish = now();

			double duration =
				(double)std::chrono::duration_cast<
				std::chrono::duration<double, std::ratio<1, 1000000>>>(finish -
					start)
				.count();
			printf("\033[0m\033[1;36m|\033[0m\t");
			cout << "FR_CF "
				<< "\t\t\t" << (num + 0.0) / duration << endl;
			out << (num + 0.0) / duration << ",";
		}

		for (int k = 0; k < CYCLE; ++k) {
			WavingSketch<8, 16, DATA_LEN> sketch_first(memory / 10, HeavyChange_HIT);
			WavingSketch<8, 16, DATA_LEN> sketch_second(memory / 10, HeavyChange_HIT);
			Data<DATA_LEN> packet;
			int t;
			int num = 0;

			TP start = now();
			while (num < slot_packets) {
				packet = records[num];
				++num;

				sketch_first.Init(packet);
			}

			while (num < 2 * slot_packets) {
				packet = records[num];
				num++;

				sketch_second.Init(packet);
			}
			TP finish = now();

			double duration =
				(double)std::chrono::duration_cast<
				std::chrono::duration<double, std::ratio<1, 1000000>>>(finish -
					start)
				.count();
			printf("\033[0m\033[1;36m|\033[0m\t");
			cout << "WavingSketch<8,1> "
				<< "\t" << (num + 0.0) / duration << endl;
			out << (num + 0.0) / duration << ",";
		}

		for (int k = 0; k < CYCLE; ++k) {
			WavingSketch<8, 16, DATA_LEN> sketch_first(memory / 10, HeavyChange_HIT);
			WavingSketch<8, 16, DATA_LEN> sketch_second(memory / 10, HeavyChange_HIT);
			Data<DATA_LEN> packet;
			int t;
			int num = 0;

			TP start = now();
			while (num < slot_packets) {
				packet = records[num];
				++num;

				sketch_first.Init(packet);
			}

			while (num < 2 * slot_packets) {
				packet = records[num];
				num++;

				sketch_second.Init(packet);
			}
			TP finish = now();

			double duration =
				(double)std::chrono::duration_cast<
				std::chrono::duration<double, std::ratio<1, 1000000>>>(finish -
					start)
				.count();
			printf("\033[0m\033[1;36m|\033[0m\t");
			cout << "WavingSketch<8,16> "
				<< "\t" << (num + 0.0) / duration << endl;
			out << (num + 0.0) / duration << ",";
		}

		for (int k = 0; k < CYCLE; ++k) {
			LdSketchWrapper<DATA_LEN> sketch_first(memory, HeavyChange_HIT);
			LdSketchWrapper<DATA_LEN> sketch_second(memory, HeavyChange_HIT);
			Data<DATA_LEN> packet;
			int t;
			int num = 0;

			TP start = now();
			while (num < slot_packets) {
				packet = records[num];
				++num;

				sketch_first.Init(packet);
			}

			while (num < 2 * slot_packets) {
				packet = records[num];
				num++;

				sketch_second.Init(packet);
			}
			TP finish = now();

			double duration =
				(double)std::chrono::duration_cast<
				std::chrono::duration<double, std::ratio<1, 1000000>>>(finish -
					start)
				.count();
			printf("\033[0m\033[1;36m|\033[0m\t");
			cout << "LD Sketch "
				<< "\t" << (num + 0.0) / duration << endl;
			out << (num + 0.0) / duration << ",";
		}
		out << endl;
	}
	free(records);
}
