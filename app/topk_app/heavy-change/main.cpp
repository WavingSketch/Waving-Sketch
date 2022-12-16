#pragma GCC optimize(2)
#include <iostream>
#include <fstream>
#include <iomanip>
#include <time.h>
#include <unordered_map>
#include <algorithm>
#include <chrono>

#include "fr.h"
#include "fr_cf.h"
#include "abstract.h"
#include "Waving.h"
using namespace std;

#define BLOCK 250000

typedef std::chrono::high_resolution_clock::time_point TP;
inline TP now(){
    return std::chrono::high_resolution_clock::now();
}

const string FOLDER  = "../../../../dataset/";

//=============================================
// const int hit = 1742;
// const string FILE_NAME { "CAIDA2018_8.dat" };
//=============================================
const int hit = 335;
const string FILE_NAME {"demo.dat"};
//=============================================

const int interval = 10000000;
void Test_Hitter(string PATH);
void Test_Speed(string PATH);

int Get_TopK(HashMap mp, int k){
    int size = mp.size();
    int *num = new int [size];
    int pos = 0;
    unordered_map<Data, int, My_Hash>::iterator it;
    for(it = mp.begin();it != mp.end();++it){
        num[pos++] = it->second;
    }
    nth_element(num, num + size - k, num + size);
    int ret = num[size - k];
    delete num;
    return ret;
}

int main()
{
    // ===================== Determine the threshold==================
    // cout << FILE_NAME << endl;
    // HashMap mp;
    // FILE* file = fopen((FOLDER + FILE_NAME).c_str(),"rb");
    // Data packet;
    // int t;
    // int num = 0;
    // while(num < interval)
    // {
    //     fread(packet.str, DATA_LEN, 1, file);
    //     //fread(&t, DATA_LEN, 1, file);
    //     if(mp.find(packet) == mp.end())
    //         mp[packet] = 1;
    //     else
    //         mp[packet] += 1;
    //     num++;
    // }
    // while(num < 2 * interval)
    // {
    //     fread(packet.str, DATA_LEN, 1, file);
    //     //fread(&t, DATA_LEN, 1, file);
    //     if(mp.find(packet) == mp.end())
    //         mp[packet] = -1;
    //     else
    //         mp[packet] -= 1;
    //     num++;
    // }
    // for(HashMap::iterator it = mp.begin();it != mp.end();++it){
    //     it->second = abs(it->second);
    // }
    // printf("hit=%d\n",Get_TopK(mp, 1000));
    // ===============================================================
    printf("\033[0m\033[1;32m====================================================\n\033[0m");
    printf("\033[0m\033[1;32m|          Application: Find Heavy Change          |\n\033[0m");
    printf("\033[0m\033[1;32m====================================================\n\033[0m");
    printf("\033[0m\033[1;32m|                     F1 SCORE                     |\n\033[0m");
    printf("\033[0m\033[1;32m====================================================\n\033[0m");
    Test_Hitter(FOLDER + FILE_NAME);
    printf("\033[0m\033[1;32m====================================================\n\033[0m");
    printf("\033[0m\033[1;32m|                    THROUGHPUT                    |\n\033[0m");
    printf("\033[0m\033[1;32m====================================================\n\033[0m");
    Test_Speed(FOLDER + FILE_NAME);
    printf("\033[0m\033[1;32m====================================================\n\033[0m");
}

void Test_Hitter(string PATH){
    for(int i = 0; i < 5; ++i){
        int memory = 8 * BLOCK + 2 * BLOCK * i;
        printf("\033[0m\033[1;4;36m> Memory size: %dKB\n\033[0m",memory/1000);
        int SKETCH_NUM = 8;
        HashMap mp;
        Abstract* sketch[SKETCH_NUM];
        sketch[0] = new FR(memory, hit);
        sketch[1] = new FR_CF(memory, hit);
        sketch[2] = new WavingSketch<8,1>(memory / 10, hit);
        sketch[3] = new WavingSketch<8,16>(memory / 10, hit);
        sketch[4] = new FR(memory, hit);
        sketch[5] = new FR_CF(memory, hit);
        sketch[6] = new WavingSketch<8,1>(memory / 10, hit);
        sketch[7] = new WavingSketch<8,16>(memory / 10, hit);
        FILE* file = fopen(PATH.c_str(),"rb");
        Data packet;
        int t;
        int num = 0;

        while(num < interval)
        {
            fread(packet.str, DATA_LEN, 1, file);
            if(mp.find(packet) == mp.end())
                mp[packet] = 1;
            else
                mp[packet] += 1;
            num++;

            for(int j = 0;j < SKETCH_NUM / 2;++j){
                sketch[j]->Init(packet);
            }
        }

        while(num < 2 * interval)
        {
            fread(packet.str, DATA_LEN, 1, file);
            if(mp.find(packet) == mp.end())
                mp[packet] = -1;
            else
                mp[packet] -= 1;
            num++;

            for(int j = SKETCH_NUM / 2;j < SKETCH_NUM;++j){
                sketch[j]->Init(packet);
            }
        }
        fclose(file);

        for(int j = 0;j < SKETCH_NUM / 2;++j){
            sketch[j]->Check(mp, sketch[j + (SKETCH_NUM / 2)]);
        }

        ofstream out;
        for(int j = 0;j < SKETCH_NUM / 2;++j){
            printf("\033[0m\033[1;36m|\033[0m\t");
            sketch[j]->print_f1(out, memory);
        }
    }
}

void Test_Speed(string PATH){
    const int CYCLE = 1;
    for(int i = 0;i < 5;++i){
        int memory = 8 * BLOCK + 2 * BLOCK * i;
        printf("\033[0m\033[1;4;36m> Memory size: %dKB\n\033[0m",memory/1000);
        
        for(int k = 0;k < CYCLE;++k){
            FR sketch_first(memory, hit);
            FR sketch_second(memory, hit);
            FILE* file = fopen(PATH.c_str(),"rb");
            Data packet;
            int t;
            int num = 0;

            TP start = now();
            while(num < interval)
            {
                fread(packet.str, DATA_LEN, 1, file);
                ++num;

                sketch_first.Init(packet);
            }

            while(num < 2 * interval)
            {
                fread(packet.str, DATA_LEN, 1, file);
                //fread(&t, DATA_LEN, 1, file);
                ++num;

                sketch_second.Init(packet);
            }
            TP finish = now();

            double duration = (double)std::chrono::duration_cast<std::chrono::duration<double,std::ratio<1,1000000>>>(finish - start).count();
            printf("\033[0m\033[1;36m|\033[0m\t");
            cout << "FR " << "\t\t\t" << (num + 0.0)/ duration << endl;

            fclose(file);
        }

        for(int k = 0;k < CYCLE;++k){
            FR_CF sketch_first(memory, hit);
            FR_CF sketch_second(memory, hit);
            FILE* file = fopen(PATH.c_str(),"rb");
            Data packet;
            int t;
            int num = 0;

            TP start = now();
            while(num < interval)
            {
                fread(packet.str, DATA_LEN, 1, file);
                num++;

                sketch_first.Init(packet);
            }

            while(num < 2 * interval)
            {
                fread(packet.str, DATA_LEN, 1, file);
                num++;

                sketch_second.Init(packet);
            }
            TP finish = now();

            double duration = (double)std::chrono::duration_cast<std::chrono::duration<double,std::ratio<1,1000000>>>(finish - start).count();
            printf("\033[0m\033[1;36m|\033[0m\t");
            cout << "FR_CF " << "\t\t\t" << (num + 0.0)/ duration << endl;

            fclose(file);
        }
        
        for(int k = 0;k < CYCLE;++k){
            WavingSketch<8,16> sketch_first(memory/10, hit);
            WavingSketch<8,16> sketch_second(memory/10, hit);
            FILE* file = fopen(PATH.c_str(),"rb");
            Data packet;
            int t;
            int num = 0;

            TP start = now();
            while(num < interval)
            {
                fread(packet.str, DATA_LEN, 1, file);
                num++;

                sketch_first.Init(packet);
            }

            while(num < 2 * interval)
            {
                fread(packet.str, DATA_LEN, 1, file);
                num++;

                sketch_second.Init(packet);
            }
            TP finish = now();

            double duration = (double)std::chrono::duration_cast<std::chrono::duration<double,std::ratio<1,1000000>>>(finish - start).count();
            printf("\033[0m\033[1;36m|\033[0m\t");
            cout << "WavingSketch<8,1> " << "\t" << (num + 0.0)/ duration << endl;

            fclose(file);
        }

        for(int k = 0;k < CYCLE;++k){
            WavingSketch<8,16> sketch_first(memory/10, hit);
            WavingSketch<8,16> sketch_second(memory/10, hit);
            FILE* file = fopen(PATH.c_str(),"rb");
            Data packet;
            int t;
            int num = 0;

            TP start = now();
            while(num < interval)
            {
                fread(packet.str, DATA_LEN, 1, file);
                num++;

                sketch_first.Init(packet);
            }

            while(num < 2 * interval)
            {
                fread(packet.str, DATA_LEN, 1, file);
                num++;

                sketch_second.Init(packet);
            }
            TP finish = now();

            double duration = (double)std::chrono::duration_cast<std::chrono::duration<double,std::ratio<1,1000000>>>(finish - start).count();
            printf("\033[0m\033[1;36m|\033[0m\t");
            cout << "WavingSketch<8,16> " << "\t" << (num + 0.0)/ duration << endl;

            fclose(file);
        }
    }
}
