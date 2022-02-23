#include "benchmark.h"
#include <string>
#include <iostream>

using namespace std;

const string folder = "./dataset/";
const string filenames[] = {"syn.dat"};

int main() {
  cout << endl << "**Benchmark**" << endl << endl;
  cout << "1. ";
  BenchCmp((folder + filenames[0]).c_str());
  cout << "2. ";
  BenchThp((folder + filenames[0]).c_str());
  cout << "3. ";
  BenchCompress((folder + filenames[0]).c_str());
  cout << "4. ";
  BenchExpand((folder + filenames[0]).c_str());
  cout << "5. ";
  BenchCompressMC((folder + filenames[0]).c_str());
  cout << "6. ";
  BenchExpandMC((folder + filenames[0]).c_str());
}