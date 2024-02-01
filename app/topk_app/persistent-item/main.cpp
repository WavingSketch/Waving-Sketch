#pragma GCC optimize(2)
#include "benchmark.h"

const string FOLDER = "../../../dataset/";
const string RESULT_FOLDER = "./";
int record_length;
string FILE_NAME;


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
		"\033[0m\033[1;32m===================================================="
		"\n\033[0m");
	printf(
		"\033[0m\033[1;32m|        Application: Find Persistent items        "
		"|\n\033[0m");
	printf(
		"\033[0m\033[1;32m===================================================="
		"\n\033[0m");
	printf(
		"\033[0m\033[1;32m|                     F1 SCORE                     "
		"|\n\033[0m");
	printf(
		"\033[0m\033[1;32m===================================================="
		"\n\033[0m");
	Test_Hitter(FILE_NAME);
	printf(
		"\033[0m\033[1;32m===================================================="
		"\n\033[0m");
	printf(
		"\033[0m\033[1;32m|                    THROUGHPUT                    "
		"|\n\033[0m");
	printf(
		"\033[0m\033[1;32m===================================================="
		"\n\033[0m");
	Test_Speed(FILE_NAME);
	printf(
		"\033[0m\033[1;32m===================================================="
		"\n\033[0m");
}
