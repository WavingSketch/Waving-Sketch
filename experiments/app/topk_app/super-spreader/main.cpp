#pragma GCC optimize(2)
#include"benchmark.h"

using namespace std;



const string FOLDER = "../../../dataset/";
const string RESULT_FOLDER = "./";
int record_length;
string FILE_NAME;

int main(int argc, char* argv[]) {
  // ===================== Determine the threshold==================
  // cout << FILE_NAME << endl;

  // HashMap mp;
  // StreamMap sp;

  // FILE* file = fopen((FILE_NAME).c_str(),"rb");
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
      "\033[0m\033[1;32m|         Application: Find Super Spreader         "
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
  SuperSpread_Test_Hitter<4>(FILE_NAME);
  printf(
      "\033[0m\033[1;32m===================================================="
      "\n\033[0m");
  printf(
      "\033[0m\033[1;32m|                    THROUGHPUT                    "
      "|\n\033[0m");
  printf(
      "\033[0m\033[1;32m===================================================="
      "\n\033[0m");
  SuperSpread_Test_Speed<4>(FILE_NAME);
  printf(
      "\033[0m\033[1;32m===================================================="
      "\n\033[0m");
}

