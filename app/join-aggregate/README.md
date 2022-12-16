# Join-aggregate estimation

## Overview
This folder contains the codes of WavingSketch and the other related algorithms (CM, FAGMS, Skimmed sketch) on performing join-aggregate estimation.


## Requirements

- CMake
- g++
- C++11
- Boost

## File descriptions

We use two datasets to conduct the experiments of join-aggregate estimation.

*  `datasets/`: Two sample datasets extracted from the real-world datasets used in our experiments. More details can be found in the folders. 
*  `src/`: Codes of WavingSketch and the other related algorithms in join-aggregate estimation.

## Datasets

- ` CAIDA.dat` We provide a small shard of the CAIDA dataset, and when reading a single CAIDA dataset file we will treat the first and second halves as two data streams.

- `zipf` We generate a series of synthetic datasets that follow the Zipf distribution. The skewness of the datasets range from 0.0 to 1.0. Each dataset contains approximately 1.0M flows, 32.0M items. The length of each item ID is 4 bytes.

For more details, please refer to our paper. 

## How to run

### 1) Manually compile and run

You can use the following commands to build and run our tests. 

```bash
$ cd src
$ make
$ ./main [-f filename] [-l keylen] [-m memory] [-v version]
```


1. `[-f filename]`: The path of the dataset you want to run. 
2. `[-l keylen]`: An integer, representing the length of each entry in the dataset.
3. `[-m memory]`: An integer, represeting the memory size (in bytes) used by the sketches. 
4. `[-v version]`: An integer, representing the algorithm you want to run. You can refer to `include/Choose_Ske.h` for more details. 

You can modify the `Heavy_Thes` in `main.cpp` to adapt to different datasets. This is the frequency threshold to seprate frequent and infrequent items (used by Skimmed sketch and WavingSketch).

### 2) Automatically compile and run

To make it easier, we also provide a python script to complie-and-run the test in one shot. 
You can directly run the following command to run our tests. 
```
$ python run.py
```



## Output format

Our program will print the statistics about the input dataset and the parameters of the candidate algorithms at the command-line interface. Then our program will generate a file `result.csv` containing the results, including mean error, variance, max min values, insertion throughput, and join time.
