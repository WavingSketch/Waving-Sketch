#include <iostream>
#include <fstream>
#include <iomanip>
#include <time.h>
#include <unordered_map>
#include <algorithm>
#include <chrono>

#include "abstract.h"
#include "interest.h"
#include "opensketch.h"
#include "olf.h"
#include "tlf.h"
#include "Count_Bucket.h"

using namespace std;

#define FILE_NUM 1
#define SKETCH_NUM 1
#define HIT 70
#define BLOCK 100000

typedef std::chrono::high_resolution_clock::time_point TP;
inline TP now(){
    return std::chrono::high_resolution_clock::now();
}

const string FOLDER  = "../data/";
const string FILE_NAME[1] = {"ip.dat"};
//67,68,66,70
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
        StreamMap sp;

        FILE* file = fopen((FOLDER + FILE_NAME[i]).c_str(),"rb");
        Data from;
        Data to;

        while(fread(from.str, DATA_LEN, 1, file) > 0)
        {
            fread(&to.str, DATA_LEN, 1, file);

            Stream stream(from, to);
            if(sp.find(stream) == sp.end()){
                sp[stream] = 1;
                if(mp.find(from) == mp.end())
                    mp[from] = 1;
                else
                    mp[from] += 1;
            }
        }
        cout << Get_TopK(mp, 250) << endl;
        cout << mp.size() << endl;
        cout << sp.size() << endl;*/

        Test_Speed(FOLDER + FILE_NAME[i]);
    }
}

void Test_Hitter(string PATH){
    for(int i = 0;i < 5;++i){
        int memory = 6*BLOCK + BLOCK * i;
        Count_Bucket sketch(memory, HIT);

        HashMap mp;
        StreamMap sp;

        FILE* file = fopen(PATH.c_str(),"rb");
        Data from;
        Data to;
        int num = 0;

        while(fread(from.str, DATA_LEN, 1, file) > 0)
        {
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
            sketch.Init(from, to);
        }
        fclose(file);
        sketch.Check(mp);


        ofstream out;

        sketch.print_are(out, memory);

        sketch.print_cr(out, memory);

        sketch.print_pr(out, memory);
    }
}

void Test_Speed(string PATH){
    const int CYCLE = 1;
    Data from;
    Data to;

    for(int i = 0;i < 5;++i){
        int memory = 6*BLOCK + BLOCK * i;
        for(int k = 0;k < CYCLE;++k){
            Count_Bucket sketch(memory, HIT);
            FILE* file = fopen(PATH.c_str(),"rb");
            int num = 0;

            TP start = now();
            while(fread(from.str, DATA_LEN, 1, file) > 0)
            {
                fread(to.str, DATA_LEN, 1, file);
                ++num;
                sketch.Init(from, to);
            }
            TP finish = now();

            double duration = (double)std::chrono::duration_cast<std::chrono::duration<double,std::ratio<1,1000000>>>(finish - start).count();
            cout << sketch.name << " " << memory / 1000 << "KB " << (num + 0.0)/ duration << endl;

            fclose(file);
        }
        
        for(int k = 0;k < CYCLE;++k){
            TLF sketch(memory, HIT);
            FILE* file = fopen(PATH.c_str(),"rb");
            int num = 0;

            TP start = now();
            while(fread(from.str, DATA_LEN, 1, file) > 0)
            {
                fread(to.str, DATA_LEN, 1, file);
                ++num;
                sketch.Init(from, to);
            }
            TP finish = now();

            double duration = (double)std::chrono::duration_cast<std::chrono::duration<double,std::ratio<1,1000000>>>(finish - start).count();
            cout << sketch.name << " " << memory / 1000 << "KB " << (num + 0.0)/ duration << endl;

            fclose(file);
        }
        
        for(int k = 0;k < CYCLE;++k){
            OLF sketch(memory, HIT);
            FILE* file = fopen(PATH.c_str(),"rb");
            int num = 0;

            TP start = now();
            while(fread(from.str, DATA_LEN, 1, file) > 0)
            {
                fread(to.str, DATA_LEN, 1, file);
                ++num;
                sketch.Init(from, to);
            }
            TP finish = now();

            double duration = (double)std::chrono::duration_cast<std::chrono::duration<double,std::ratio<1,1000000>>>(finish - start).count();
            cout << sketch.name << " " << memory / 1000 << "KB " << (num + 0.0)/ duration << endl;

            fclose(file);
        }

        for(int k = 0;k < CYCLE;++k){
            OpenSketch sketch(memory, HIT);
            FILE* file = fopen(PATH.c_str(),"rb");
            int num = 0;

            TP start = now();
            while(fread(from.str, DATA_LEN, 1, file) > 0)
            {
                fread(to.str, DATA_LEN, 1, file);
                ++num;
                sketch.Init(from, to);
            }
            TP finish = now();

            double duration = (double)std::chrono::duration_cast<std::chrono::duration<double,std::ratio<1,1000000>>>(finish - start).count();
            cout << sketch.name << " " << memory / 1000 << "KB " << (num + 0.0)/ duration << endl;

            fclose(file);
        }
    }
}
