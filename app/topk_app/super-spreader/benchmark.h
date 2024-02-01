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
#include "olf.h"
#include "opensketch.h"
#include "ss.h"
#include "tlf.h"

#define BLOCK 100000
const int MAX_PACKET = 1e7;
typedef std::chrono::high_resolution_clock::time_point TP;
inline TP now() { return std::chrono::high_resolution_clock::now(); }

extern int record_length;
extern const std::string RESULT_FOLDER;
#define HIT 84

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
    void* raw_data = malloc(sizeof(Data) * num * 2);
    if (raw_data == NULL) {
        printf("[ERROR] MALLOC FAILED!\n");
        exit(-1);
    }

    Data* data = reinterpret_cast<Data*>(raw_data);
    for (int i = 0; i < num; i++) {
        data[2 * i] = *reinterpret_cast<Data*>(raw_addr + interval * i);
        data[2 * i + 1] =
            *reinterpret_cast<Data*>(raw_addr + interval * i + sizeof(Data));
    }
    munmap(raw_addr, buf.st_size);
    return data;
}

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
void Test_Hitter(string PATH) {
    std::vector<std::ostream*> outs(4);
    std::string file = "caida";
    outs[0] = new std::ofstream(RESULT_FOLDER + "superSpread_" + file + "_ARE.csv");
    outs[1] = new std::ofstream(RESULT_FOLDER + "superSpread_" + file + "_Recall Rate (RR).csv");
    outs[2] = new std::ofstream(RESULT_FOLDER + "superSpread_" + file + "_Precision Rate (PR).csv");
    outs[3] = new std::ofstream(RESULT_FOLDER + "superSpread_" + file + "_F1.csv");
    for (int i = 0; i < 5; ++i) {
        int memory = 6 * BLOCK + BLOCK * i;
        printf("\033[0m\033[1;4;36m> Memory size: %dKB\n\033[0m", memory / 1000);
        int SKETCH_NUM = 5;
        Abstract* sketch[SKETCH_NUM];
        sketch[0] = new WavingSketch<8, 16>(memory, HIT);
        sketch[1] = new TLF(memory, HIT);
        sketch[2] = new OLF(memory, HIT);
        sketch[3] = new OpenSketch(memory, HIT);
        sketch[4] = new SpreadSketchWrapper(memory, HIT);

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
        StreamMap sp;

        int cnt;
        Data* records = read_standard_data(PATH.c_str(), record_length, &cnt);
        Data from;
        Data to;
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

            Stream stream(from, to);
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

void Test_Speed(string PATH) {
    const int CYCLE = 1;
    Data from;
    Data to;
    std::string file = "caida";
    std::ofstream out(RESULT_FOLDER + "superSpread_" + file + "_Throughput(Mops).csv");
    for (int i = 0; i < 5; ++i) {
        int memory = 6 * BLOCK + BLOCK * i;
        printf("\033[0m\033[1;4;36m> Memory size: %dKB\n\033[0m", memory / 1000);

        if (i == 0)
        {
            out << "MEM(KB)" << "," << "WavingSketch<8,1>" << "," << "TLF" << ","
                << "OLF" << "," << "opensketch" << "," << "SpreadSketch";
            out << std::endl;
        }
        out << memory / 1000 << ",";

        for (int k = 0; k < CYCLE; ++k) {
            WavingSketch<8, 16> sketch(memory, HIT);
            int cnt;
            Data* records = read_standard_data(PATH.c_str(), record_length, &cnt);
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
            TLF sketch(memory, HIT);
            int cnt;
            Data* records = read_standard_data(PATH.c_str(), record_length, &cnt);
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
            OLF sketch(memory, HIT);
            int cnt;
            Data* records = read_standard_data(PATH.c_str(), record_length, &cnt);
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
            OpenSketch sketch(memory, HIT);
            int cnt;
            Data* records = read_standard_data(PATH.c_str(), record_length, &cnt);
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
            SpreadSketchWrapper sketch(memory, HIT);
            int cnt;
            Data* records = read_standard_data(PATH.c_str(), record_length, &cnt);
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
