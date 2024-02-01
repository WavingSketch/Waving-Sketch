# The source codes of JoinSketch and the algorithms we compared with.

## Requirements

- CMake
- g++
- C++11
- Boost

## Run tests

You can use the following commands to build and run our tests. 

```bash
$ make
$ ./main [-f filename] [-l keylen] [-m memory] [-v version]
```

1. `[-f filename]`: The path of the dataset you want to run. 
2. `[-l keylen]`: An integer, representing the length of each entry in the dataset.
3. `[-m memory]`: An integer, represeting the memory size (in bytes) used by the JoinSketch. 
4. `[-v version]`: An integer, representing the algorithm you want to run. You can refer to `include/Choose_Ske.h`

You can modify the `Heavy_Thes` in `main.cpp` to adapt to different datasets.

### Output Format

Our program will print the statistics about the input dataset and the parameters of the candidate algorithms at the command-line interface. Then our program will generate a file `result.csv` containing information such as mean error, variance, max min and throughput.
