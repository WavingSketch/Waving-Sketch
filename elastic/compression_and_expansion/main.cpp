#include "benchmark.h"
#include <string>
#include <iostream>

using namespace std;

const string folder = "../../dataset/";
string FILE_NAME;
int record_length;
int main(int argc, char* argv[]) {
	if (argc > 2)
	{
		FILE_NAME = argv[1];
		record_length = std::stoi(argv[2]);
	}
	else
	{
		std::cout << "Please Input the dataset path:";
		std::cin >> FILE_NAME;
		std::cout << "Please Input the length of record in dtaset:";
		std::cin >> record_length;
	}
  cout << endl << "**Benchmark**" << endl << endl;
  cout << "1. ";
  BenchCmp((FILE_NAME).c_str());
  cout << "2. ";
  BenchThp((FILE_NAME).c_str());
  cout << "3. ";
  BenchCompress((FILE_NAME).c_str());
  cout << "4. ";
  BenchExpand((FILE_NAME).c_str());
  cout << "5. ";
  BenchCompressMC((FILE_NAME).c_str());
  cout << "6. ";
  BenchExpandMC((FILE_NAME).c_str());
}