#include "genzipf.h"
#include "murmur3.h"

#include <iostream>
#include <fstream>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <chrono>

using namespace std;

uint32_t key_len = 4;
uint32_t flow_num = 1E6;
uint32_t packet_num = 32E6;
unordered_map<uint32_t, uint32_t> ground_truth;
map<uint32_t, uint32_t> fsd;

void gen_zipf_dataset(double alpha)
{
    ground_truth.clear();
    fsd.clear();
    uint32_t hash_seed = std::chrono::steady_clock::now().time_since_epoch().count();

    string filename = "zipf_" + to_string(alpha).substr(0, 3) + ".dat";
    ofstream outFile(filename.c_str(), ios::binary);

    for (int i = 0; i < packet_num; ++i)
    {
        uint32_t rand_num = zipf(alpha, flow_num, i == 0);
        uint32_t key = MurmurHash3_x86_32(&rand_num, key_len, hash_seed);
        outFile.write((char *)&key, key_len);
        ground_truth[key]++;
    }
    outFile.close();

    string statname = "zipf_" + to_string(alpha).substr(0, 3) + ".stat";
    ofstream outStat(statname.c_str());

    cout << ground_truth.size() << " flows, " << packet_num << " packets" << endl;
    outStat << ground_truth.size() << " flows, " << packet_num << " packets" << endl;
    for (auto pr : ground_truth)
        fsd[pr.second]++;
    for (auto pr : fsd)
        outStat << pr.first << "\t\t" << pr.second << endl;
}

int main()
{
    for (double alpha = 0.8; alpha < 0.81; alpha += 0.1)
    {
        cout << "generating zipf " << alpha << endl;
        gen_zipf_dataset(alpha);
    }
    return 0;
}
