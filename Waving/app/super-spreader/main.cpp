#pragma GCC optimize(2)
#include <iostream>
#include <fstream>
#include <iomanip>
#include <time.h>
#include <unordered_map>
#include <algorithm>
#include <chrono>

#include "abstract.h"
#include "Waving.h"
#include "opensketch.h"
#include "olf.h"
#include "tlf.h"

using namespace std;

#define BLOCK 100000
const int interval = 10000000;

typedef std::chrono::high_resolution_clock::time_point TP;
inline TP now(){
    return std::chrono::high_resolution_clock::now();
}

const string FOLDER  = "../../dataset/";

//===============================================
// #define HIT 84
// const string FILE_NAME {"CAIDA2018_8.dat"};
//===============================================
#define HIT 20
const string FILE_NAME {"demo.dat"};
//===============================================

void Test_Hitter(string PATH);
void Test_Speed(string PATH);

int Get_TopK(HashMap mp, int k){
    int size = mp.size();
    int *num = new int [size];
    int pos = 0;
    HashMap::iterator it;
    for(it = mp.begin();it != mp.end();++it){
        num[pos++] = it->second;
    }
    nth_element(num, num + size - k, num + size);
    int ret = num[size - k];
    delete num;
    return ret;
}

int main(){
    // ===================== Determine the threshold==================
    // cout << FILE_NAME << endl;

    // HashMap mp;
    // StreamMap sp;

    // FILE* file = fopen((FOLDER + FILE_NAME).c_str(),"rb");
    // Data from;
    // Data to;
    // uint num = 0;
    // while(num<interval)
    // {
    //     fread(from.str, DATA_LEN, 1, file);
    //     fread(&to.str, DATA_LEN, 1, file);

    //     Stream stream(from, to);
    //     if(sp.find(stream) == sp.end()){
    //         sp[stream] = 1;
    //         if(mp.find(from) == mp.end())
    //             mp[from] = 1;
    //         else
    //             mp[from] += 1;
    //     }
    //     num++;
    // }
    // printf("HIT=%d\n", Get_TopK(mp, 250));
    // ===============================================================
    
    printf("\033[0m\033[1;32m====================================================\n\033[0m");
    printf("\033[0m\033[1;32m|         Application: Find Super Spreader         |\n\033[0m");
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
    for(int i = 0;i < 5;++i){
        int memory = 6*BLOCK + BLOCK * i;
        printf("\033[0m\033[1;4;36m> Memory size: %dKB\n\033[0m",memory/1000);
        int SKETCH_NUM = 4;
        Abstract* sketch[SKETCH_NUM];
        sketch[0] = new WavingSketch<8,16>(memory, HIT);
        sketch[1] = new TLF(memory, HIT);
        sketch[2] = new OLF(memory, HIT);
        sketch[3] = new OpenSketch(memory, HIT);

        HashMap mp;
        StreamMap sp;

        FILE* file = fopen(PATH.c_str(),"rb");
        Data from;
        Data to;
        int num = 0;

        while(num<interval)
        {
            fread(from.str, DATA_LEN, 1, file);
            fread(to.str, DATA_LEN, 1, file);
            
            /*
            num++;
            if(num % 1000000 == 0){
                cout << num << endl;
            }
            */
            

            Stream stream(from, to);
            if(sp.find(stream) == sp.end()){
                sp[stream] = 1;
                if(mp.find(from) == mp.end())
                    mp[from] = 1;
                else
                    mp[from] += 1;
            }
            for(int j = 0;j < SKETCH_NUM;++j){
                sketch[j]->Init(from, to);
            }
            num++;
        }
        fclose(file);
        for(int j = 0;j < SKETCH_NUM;++j){
            sketch[j]->Check(mp);
        }


        ofstream out;

        for(int j = 0;j < SKETCH_NUM;++j){
            printf("\033[0m\033[1;36m|\033[0m\t");
            sketch[j]->print_f1(out, memory);
        }

        for(int j = 0;j < SKETCH_NUM;++j){
            delete sketch[j];
        }
    }
}

void Test_Speed(string PATH){
    const int CYCLE = 1;
    Data from;
    Data to;

    for(int i = 0;i < 5;++i){
        int memory = 6*BLOCK + BLOCK * i;
        printf("\033[0m\033[1;4;36m> Memory size: %dKB\n\033[0m",memory/1000);
        for(int k = 0;k < CYCLE;++k){
            WavingSketch<8,16> sketch(memory, HIT);
            FILE* file = fopen(PATH.c_str(),"rb");
            int num = 0;

            TP start = now();
            while(num<interval)
            {
                fread(from.str, DATA_LEN, 1, file);
                fread(to.str, DATA_LEN, 1, file);
                ++num;
                sketch.Init(from, to);
            }
            TP finish = now();

            double duration = (double)std::chrono::duration_cast<std::chrono::duration<double,std::ratio<1,1000000>>>(finish - start).count();
            printf("\033[0m\033[1;36m|\033[0m\t");
            cout << sketch.name << sketch.sep << (num + 0.0)/ duration << endl;

            fclose(file);
        }
        
        for(int k = 0;k < CYCLE;++k){
            TLF sketch(memory, HIT);
            FILE* file = fopen(PATH.c_str(),"rb");
            int num = 0;

            TP start = now();
            while(num<interval)
            {
                fread(from.str, DATA_LEN, 1, file);
                fread(to.str, DATA_LEN, 1, file);
                ++num;
                sketch.Init(from, to);
            }
            TP finish = now();

            double duration = (double)std::chrono::duration_cast<std::chrono::duration<double,std::ratio<1,1000000>>>(finish - start).count();
            printf("\033[0m\033[1;36m|\033[0m\t");
            cout << sketch.name << sketch.sep << (num + 0.0)/ duration << endl;

            fclose(file);
        }
        
        for(int k = 0;k < CYCLE;++k){
            OLF sketch(memory, HIT);
            FILE* file = fopen(PATH.c_str(),"rb");
            int num = 0;

            TP start = now();
            while(num<interval)
            {
                fread(from.str, DATA_LEN, 1, file);
                fread(to.str, DATA_LEN, 1, file);
                ++num;
                sketch.Init(from, to);
            }
            TP finish = now();

            double duration = (double)std::chrono::duration_cast<std::chrono::duration<double,std::ratio<1,1000000>>>(finish - start).count();
            printf("\033[0m\033[1;36m|\033[0m\t");
            cout << sketch.name << sketch.sep << (num + 0.0)/ duration << endl;

            fclose(file);
        }

        for(int k = 0;k < CYCLE;++k){
            OpenSketch sketch(memory, HIT);
            FILE* file = fopen(PATH.c_str(),"rb");
            int num = 0;

            TP start = now();
            while(num<interval)
            {
                fread(from.str, DATA_LEN, 1, file);
                fread(to.str, DATA_LEN, 1, file);
                ++num;
                sketch.Init(from, to);
            }
            TP finish = now();

            double duration = (double)std::chrono::duration_cast<std::chrono::duration<double,std::ratio<1,1000000>>>(finish - start).count();
            printf("\033[0m\033[1;36m|\033[0m\t");
            cout << sketch.name << sketch.sep << (num + 0.0)/ duration << endl;

            fclose(file);
        }
    }
}
