# WavingSketch: An Unbiased and Generic Sketch for Finding Top-k Items in Data Streams

This repository contains all related code of our paper "WavingSketch: An Unbiased and Generic Sketch for Finding Top-k Items in Data Streams". 

## Introduction

Finding top-k items in data streams is a fundamental problem in data mining. Unbiased estimation is well acknowledged as an elegant and important property for top-k algorithms. In this paper, we propose a novel sketch algorithm, called WavingSketch, which is more accurate than existing unbiased algorithms. We theoretically prove that WavingSketch can provide unbiased estimation, and derive its error bound. WavingSketch is generic to measurement tasks, and we apply it to five applications: finding top-k frequent items, finding top-k heavy changes, finding top-k persistent items, finding top-k Super-Spreaders, and join-aggregate estimation. Our experimental results show that, compared with the state-of-the-art Unbiased Space-Saving, WavingSketch achieves 9.3× faster speed and 10<sup>3</sup>× smaller error on finding frequent items. For other applications, WavingSketch also achieves higher accuracy and faster speed.


## About this repo

- `include` contains codes of WavingSketch (including Multi-Counter WavingSketch) and the algorithms. 

- `experiments` contains codes of WavingSketch and the related algorithms in finding frequent items, the performance of the elastic operations and automatic memory adjustment of WavingSketch and the other six applications
- `dataset` contains two small dataset shards to run our tests. We also upload our synthetic datasets to google drive and leave a link here.

- More details can be found in the folders.

## Requirements

- CMake
- g++
- C++11

## How to run
### Normal Experiment
first run the following command to generate a Makefile. 
```
$ mkdir build
$ cd build
$ cmake ..
```

Then run the following command to generate a executable file `bench`.  
```
$ make
```

Finally, run the following command to generate the results. 
```
$ ./bench [filename] [keylen]
```
1. `[filename]`: The path of the dataset you want to run. 
2. `[keylen]`: An integer, representing the length of each entry in the dataset.

### Flink
Needs to be compiled separately to generate an executable program.the More details can be found in the folders.
### Join-aggregate
Needs to be compiled separately to generate an executable program.More details can be found in the folders.
### compression_and_expansion
Needs to be compiled separately to generate an executable program.More details can be found in the folders.


The results will be shown on the screen. After the program completes the calculation, we will output statistical tables of several different indicators (ARE, PR, RR, F1, etc.) for you to analyze.