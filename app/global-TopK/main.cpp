#include <iostream>
#include <string>

#include "benchmark.h"

using namespace std;
int record_length;
std::string FILE_NAME;
const std::string FOLDER = "../../dataset/";
const std::string RESULT_FOLDER = "./";
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
		printf(
			"\033[0m\033[1;32m|                                  1. Test on "
			"UniformGlobalTopK                                 |\n\033[0m");
		printf(
			"\033[0m\033[1;32m======================================================="
			"====================================\n\033[0m");
		BenchUniformGlobalTopK(FILE_NAME);
		printf(
			"\033[0m\033[1;32m======================================================="
			"====================================\n\033[0m");
		printf(
			"\033[0m\033[1;32m|                                  2. Test on "
			"SkewGlobalTopK                                   |\n\033[0m");
		printf(
			"\033[0m\033[1;32m======================================================="
			"====================================\n\033[0m");
		BenchSkewGlobalTopK(FILE_NAME);
		printf(
			"\033[0m\033[1;32m======================================================="
			"====================================\n\033[0m");
}
