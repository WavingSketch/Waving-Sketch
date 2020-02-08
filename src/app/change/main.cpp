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
#include "interest.h"
#include "Count_Bucket.h"

using namespace std;

#define FILE_NUM 1
#define SKETCH_NUM 2
#define BLOCK 250000

typedef std::chrono::high_resolution_clock::time_point TP;
inline TP now(){
    return std::chrono::high_resolution_clock::now();
}

const string FOLDER  = "../data/";
const string FILE_NAME[1] = { "ip.dat" };


const int hit = 240;
const int interval = 10000000;
//30,1707,693,826
//800000,10000000,10000000,5000000
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

int main(){
    for(int i = 0;i < FILE_NUM;++i){
        //cout << "Dataset,Memory(KB),Speed(Mops)" << endl;
/*
        cout << FILE_NAME[i] << endl;

        HashMap mp;
        FILE* file = fopen((FOLDER + FILE_NAME[i]).c_str(),"rb");
        Data packet;
        int t;
        int num = 0;

        while(num < interval)
        {
            fread(packet.str, DATA_LEN, 1, file);
            //fread(&t, DATA_LEN, 1, file);
            if(mp.find(packet) == mp.end())
                mp[packet] = 1;
            else
                mp[packet] += 1;
            num++;
        }
        cout << mp.size() << endl;

        while(num < 2 * interval)
        {
            fread(packet.str, DATA_LEN, 1, file);
            //fread(&t, DATA_LEN, 1, file);
            if(mp.find(packet) == mp.end())
                mp[packet] = -1;
            else
                mp[packet] -= 1;
            num++;
        }
        cout << mp.size() << endl;


        for(HashMap::iterator it = mp.begin();it != mp.end();++it){
            it->second = abs(it->second);
        }

        cout << Get_TopK(mp, 500) << endl;
*/

        Test_Speed(FOLDER + FILE_NAME[i]);
    }
}

void Test_Hitter(string PATH){
    for(int i = 0; i < 5; ++i){
        int memory = 8 * BLOCK + 2 * BLOCK * i;
        HashMap mp;
        Abstract* sketch[SKETCH_NUM];
        sketch[0] = new Count_Bucket(memory / 10, hit);
        //sketch[1] = new FR(memory, hit);
        //sketch[2] = new FR_CF(memory, hit);
        sketch[1] = new Count_Bucket(memory / 10, hit);
        //sketch[4] = new FR(memory, hit);
        //sketch[5] = new FR_CF(memory, hit);

        FILE* file = fopen(PATH.c_str(),"rb");
        Data packet;
        int t;
        int num = 0;

        while(num < interval)
        {
            fread(packet.str, DATA_LEN, 1, file);
            //fread(&t, DATA_LEN, 1, file);
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
            //fread(&t, DATA_LEN, 1, file);
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
        printf("\nMEM:%dKB\nF1:\n", 2 * memory / 1000);
        for(int j = 0;j < SKETCH_NUM / 2;++j){
            sketch[j]->print_f1(out, memory);
        }

        for(int j = 0;j < SKETCH_NUM;++j){
            delete sketch[j];
        }

    }
}

void Test_Speed(string PATH){
    const int CYCLE = 1;
    for(int i = 0;i < 5;++i){
        int memory = 8 * BLOCK + 2 * BLOCK * i;

        
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
                //fread(&t, DATA_LEN, 1, file);
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
            cout << "FR " << 2 * memory / 1000 << "KB " << (num + 0.0)/ duration << endl;

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
                //fread(&t, DATA_LEN, 1, file);
                num++;

                sketch_first.Init(packet);
            }

            while(num < 2 * interval)
            {
                fread(packet.str, DATA_LEN, 1, file);
                //fread(&t, DATA_LEN, 1, file);
                num++;

                sketch_second.Init(packet);
            }
            TP finish = now();

            double duration = (double)std::chrono::duration_cast<std::chrono::duration<double,std::ratio<1,1000000>>>(finish - start).count();
            cout << "FR_CF " << 2 * memory / 1000 << "KB " << (num + 0.0)/ duration << endl;

            fclose(file);
        }
        

        for(int k = 0;k < CYCLE;++k){
            Count_Bucket sketch_first(memory, hit);
            Count_Bucket sketch_second(memory, hit);
            FILE* file = fopen(PATH.c_str(),"rb");
            Data packet;
            int t;
            int num = 0;

            TP start = now();
            while(num < interval)
            {
                fread(packet.str, DATA_LEN, 1, file);
                //fread(&t, DATA_LEN, 1, file);
                num++;

                sketch_first.Init(packet);
            }

            while(num < 2 * interval)
            {
                fread(packet.str, DATA_LEN, 1, file);
                //fread(&t, DATA_LEN, 1, file);
                num++;

                sketch_second.Init(packet);
            }
            TP finish = now();

            double duration = (double)std::chrono::duration_cast<std::chrono::duration<double,std::ratio<1,1000000>>>(finish - start).count();
            cout << "Counter_Bucket " << 2 * memory / 1000 << "KB " << (num + 0.0)/ duration << endl;

            fclose(file);
        }
    }
}
