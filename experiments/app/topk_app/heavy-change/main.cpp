#pragma GCC optimize(2)
#include "benchmark.h"
using namespace std;

const std::string FOLDER = "../../../dataset/";
const std::string RESULT_FOLDER = "./";

std::string FILE_NAME;
int record_length = 21;


int main(int argc,char* argv[]) {
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
      "\033[0m\033[1;32m|          Application: Find Heavy Change          "
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
  HeavyChange_Test_Hitter<8>(FILE_NAME);
  printf(
      "\033[0m\033[1;32m===================================================="
      "\n\033[0m");
  printf(
      "\033[0m\033[1;32m|                    THROUGHPUT                    "
      "|\n\033[0m");
  printf(
      "\033[0m\033[1;32m===================================================="
      "\n\033[0m");
  HeavyChange_Test_Speed<8>(FILE_NAME);
  printf(
      "\033[0m\033[1;32m===================================================="
      "\n\033[0m");
}