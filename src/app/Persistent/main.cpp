#include <iostream>
#include <fstream>
#include <iomanip>
#include <time.h>
#include <unordered_map>
#include <algorithm>
#include <chrono>

#include "abstract.h"
#include "pie.h"
#include "interest.h"
#include "small_space.h"
#include "Count_Bucket.h"

using namespace std;

#define FILE_NUM 1
#define SKETCH_NUM 1
#define HIT 564
#define BLOCK 200000

typedef std::chrono::high_resolution_clock::time_point TP;
inline TP now(){
    return std::chrono::high_resolution_clock::now();
}

const string FOLDER  = "../data/";
const string FILE_NAME[1] = {"ip.dat"};
//255,303,564,1094
//6250,10000,19537,20000(1600)
//stack,015,ip,web
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
    for(int i = 0;i < FILE_NUM;++i){
        /*cout << FILE_NAME[i] << endl;

        HashMap mp;
        HashMap cycle;
        FILE* file = fopen((FOLDER + FILE_NAME[i]).c_str(),"rb");
        Data packet;
        uint num = 0;
        uint time = 0;
        uint t = 0;

        while(fread(packet.str, DATA_LEN, 1, file) > 0)
        {
            fread(&t, DATA_LEN, 1, file);
            if(num % 6250 == 0){
                time += 1;
                cycle.clear();
            }
            num++;
            if(mp.find(packet) == mp.end()){
                mp[packet] = 1;
                cycle[packet] = 1;
            }
            else if(cycle.find(packet) == cycle.end()){
                cycle[packet] = 1;
                mp[packet] += 1;
            }

        }
        cout << Get_TopK(mp, 2000) << endl;
        cout << time << endl;*/

        Test_Speed(FOLDER + FILE_NAME[i]);
    }
}

void Test_Hitter(string PATH){
    for(int i = 0;i < 5;++i){
        int memory = BLOCK * (i + 1);
        Abstract* sketch[SKETCH_NUM];
        sketch[0] = new Count_Bucket(memory, HIT);
        //sketch[1] = new Small_Space(memory, HIT);
        //sketch[2] = new PIE(memory * 200, HIT, 1600);

        HashMap mp;
        HashMap cycle;
        FILE* file = fopen(PATH.c_str(),"rb");
        Data packet;
        uint num = 0;
        uint time = 0;
        //int t;

        while(fread(packet.str, DATA_LEN, 1, file) > 0)
        {
            //fread(&t, DATA_LEN, 1, file);
            if(num % 19537 == 0){
                time += 1;
                cycle.clear();
            }
            num++;

            if(mp.find(packet) == mp.end()){
                mp[packet] = 1;
                cycle[packet] = 1;
            }
            else if(cycle.find(packet) == cycle.end()){
                cycle[packet] = 1;
                mp[packet] += 1;
            }

            for(int j = 0;j < SKETCH_NUM;++j){
                sketch[j]->Init(packet, time);
            }
        }

        fclose(file);

        for(int j = 0;j < SKETCH_NUM;++j){
            sketch[j]->Check(mp);
        }


        ofstream out;

        for(int j = 0;j < SKETCH_NUM;++j){
            sketch[j]->print_are(out, memory);
        }

        for(int j = 0;j < SKETCH_NUM;++j){
            sketch[j]->print_cr(out, memory);
        }

        for(int j = 0;j < SKETCH_NUM;++j){
            sketch[j]->print_pr(out, memory);
        }

        for(int j = 0;j < SKETCH_NUM;++j){
            delete sketch[j];
        }

    }
}


void Test_Speed(string PATH){
    const int CYCLE = 1;
    for(int i = 0;i < 5;++i){
        int memory = BLOCK * (i + 1);

        for(int k = 0;k < CYCLE;++k){
            Count_Bucket sketch(memory, HIT);
            FILE* file = fopen(PATH.c_str(),"rb");
            Data packet;
            int num = 0;
            uint time = 0;

            TP start = now();
            while(fread(packet.str, DATA_LEN, 1, file) > 0)
            {
                if(num % 19537 == 0){
                    ++time;
                }
                ++num;

                sketch.Init(packet, time);
            }
            TP finish = now();

            double duration = (double)std::chrono::duration_cast<std::chrono::duration<double,std::ratio<1,1000000>>>(finish - start).count();
            cout << sketch.name << " " << memory / 1000 << "KB " << (num + 0.0)/ duration << endl;

            fclose(file);
        }

        for(int k = 0;k < CYCLE;++k){
            Small_Space sketch(memory, HIT);
            FILE* file = fopen(PATH.c_str(),"rb");
            Data packet;
            int num = 0;
            uint time = 0;

            TP start = now();
            while(fread(packet.str, DATA_LEN, 1, file) > 0)
            {
                if(num % 19537 == 0){
                    ++time;
                }
                ++num;

                sketch.Init(packet, time);
            }
            TP finish = now();

            double duration = (double)std::chrono::duration_cast<std::chrono::duration<double,std::ratio<1,1000000>>>(finish - start).count();
            cout << sketch.name << " " << memory / 1000 << "KB " << (num + 0.0)/ duration << endl;

            fclose(file);
        }

        for(int k = 0;k < CYCLE;++k){
            PIE sketch(memory * 200, HIT, 1600);
            FILE* file = fopen(PATH.c_str(),"rb");
            Data packet;
            int num = 0;
            uint time = 0;

            TP start = now();
            while(fread(packet.str, DATA_LEN, 1, file) > 0)
            {
                if(num % 19537 == 0){
                    ++time;
                }
                ++num;

                sketch.Init(packet, time);
            }
            TP finish = now();

            double duration = (double)std::chrono::duration_cast<std::chrono::duration<double,std::ratio<1,1000000>>>(finish - start).count();
            cout << sketch.name << " " << memory / 1000 << "KB " << (num + 0.0)/ duration << endl;

            fclose(file);
        }
        
    }
}

