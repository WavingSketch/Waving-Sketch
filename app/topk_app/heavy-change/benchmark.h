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

#include "Waving.h"
#include "abstract.h"
#include "fr.h"
#include "fr_cf.h"
#include "ld.h"
#define BLOCK 250000
const int hit = 1742;
const int MAX_PACKET=1e7;
extern int record_length;
extern const std::string RESULT_FOLDER;
typedef std::chrono::high_resolution_clock::time_point TP;
inline TP now() { return std::chrono::high_resolution_clock::now(); }

int Get_TopK(HashMap mp, int k) {
    int size = mp.size();
    int* num = new int[size];
    int pos = 0;
    unordered_map<Data, int, My_Hash>::iterator it;
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
    return data;
}

void Test_Hitter(string PATH) {
    std::vector<std::ostream*> outs(3);
    std::string file = "caida";
    outs[0] = new std::ofstream(RESULT_FOLDER + "heavychange_" + file + "_Recall Rate (RR).csv");
    outs[1] = new std::ofstream(RESULT_FOLDER + "heavychange_" + file + "_Precision Rate (PR).csv");
    outs[2] = new std::ofstream(RESULT_FOLDER + "heavychange_" + file + "_F1.csv");
    for (int i = 0; i < 5; ++i) {
        int memory = 8 * BLOCK + 2 * BLOCK * i;
        printf("\033[0m\033[1;4;36m> Memory size: %dKB\n\033[0m", memory / 1000);
        int SKETCH_NUM = 10;
        auto mp = HashMap();
        Abstract* sketch[SKETCH_NUM];
        sketch[0] = new FR(memory, hit);
        sketch[1] = new FR_CF(memory, hit);
        sketch[2] = new WavingSketch<8, 1>(memory / 10, hit);
        sketch[3] = new WavingSketch<8, 16>(memory / 10, hit);
        sketch[4] = new LdSketchWrapper(memory / 10, hit);
        sketch[5] = new FR(memory, hit);
        sketch[6] = new FR_CF(memory, hit);
        sketch[7] = new WavingSketch<8, 1>(memory / 10, hit);
        sketch[8] = new WavingSketch<8, 16>(memory / 10, hit);
        sketch[9] = new LdSketchWrapper(memory / 10, hit);


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
            read_standard_data(PATH.c_str(), record_length, &record_count);
        Data packet;
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
            ((LdSketchWrapper*)sketch[4])->average_l());
        printf("After average l second half: %lf\n",
            ((LdSketchWrapper*)sketch[4])->average_l());
        for (auto& out : outs)
        {
            *out << std::endl;
        }
    }
}

void Test_Speed(string PATH) {
    int record_count;
    auto records = read_standard_data(PATH.c_str(), record_length, &record_count);
    const int CYCLE = 1;
    std::string file = "caida";
    std::ofstream out(RESULT_FOLDER + "heavychange_" + file + "_Throughput(Mops).csv");
    int slot_packets = std::min(record_count / 2, MAX_PACKET);
    for (int i = 0; i < 5; ++i) {
        int memory = 8 * BLOCK + 2 * BLOCK * i;
        printf("\033[0m\033[1;4;36m> Memory size: %dKB\n\033[0m", memory / 1000);
        if (i == 0)
        {
            out << "MEM(KB)" << "," << "FR" << "," << "FR+CF" << ","
                << "WavingSketch<8,1>" << "," << "WavingSketch<8_16>" << "," << "LD Sketch";
            out << std::endl;
        }
        out << memory / 1000 << ",";
        for (int k = 0; k < CYCLE; ++k) {
            FR sketch_first(memory, hit);
            FR sketch_second(memory, hit);
            Data packet;
            int t;
            int num = 0;

            TP start = now();
            while (num < slot_packets) {
                packet = records[num];
                ++num;

                sketch_first.Init(packet);
            }

            while (num < 2* slot_packets) {
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
            FR_CF sketch_first(memory, hit);
            FR_CF sketch_second(memory, hit);
            Data packet;
            int t;
            int num = 0;

            TP start = now();
            while (num < slot_packets) {
                packet = records[num];
                ++num;

                sketch_first.Init(packet);
            }

            while (num < 2* slot_packets) {
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
            WavingSketch<8, 16> sketch_first(memory / 10, hit);
            WavingSketch<8, 16> sketch_second(memory / 10, hit);
            Data packet;
            int t;
            int num = 0;

            TP start = now();
            while (num < slot_packets) {
                packet = records[num];
                ++num;

                sketch_first.Init(packet);
            }

            while (num < 2* slot_packets) {
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
            WavingSketch<8, 16> sketch_first(memory / 10, hit);
            WavingSketch<8, 16> sketch_second(memory / 10, hit);
            Data packet;
            int t;
            int num = 0;

            TP start = now();
            while (num < slot_packets) {
                packet = records[num];
                ++num;

                sketch_first.Init(packet);
            }

            while (num < 2* slot_packets) {
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
            LdSketchWrapper sketch_first(memory, hit);
            LdSketchWrapper sketch_second(memory, hit);
            Data packet;
            int t;
            int num = 0;

            TP start = now();
            while (num < slot_packets) {
                packet = records[num];
                ++num;

                sketch_first.Init(packet);
            }

            while (num < 2* slot_packets) {
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

