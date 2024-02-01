# WavingSketch on Finding Frequent Items

## Overview

This folder contains the codes of WavingSketch and the related algorithms on finding frequent items.


## File description

- `abstract.h`: Base class for class `WavingSketch`, class `SpaceSaving`, class `UnbiassedSpaceSaving`, class `Count_heap` and `LdSketch`.
- `hash.h`: BOB Hash functions, obtained from [here](http://burtleburtle.net/bob/hash/evahash.html)
- `WavingSkecth.h`: Implementation of WavingSketch.
- `SS.h`:  Implementation of SpaceSaving. 
- `USS.h`:  Implementation of Unbiassed SpaceSaving. 
- `Count.h`:  Implementation of CountSketch. 
- `Count_Heap.h`:  Implementation of CountSketch with a heap which is uesd for accommodating frequent items.
- `ld.h`,`LD-Sketch\`: Implementation of LD-Sketch. 
- `StreamSummary.h`: Implementation of Space-Saving (SS) and Unbiased Space-Saving (USS).
- `benchmark.h`: Implementation of the benchmarks, including the benchmarks for 
our global top-k experiments conducted on data streams of equal size and that on data streams of skewed size. 

## Requirements

- CMake
- g++
- C++11

## How to run

You can enter the folder to run the codes. 
After entering the folder, first run the following command to generate a Makefile. 
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


The results will be shown on the screen. After the program completes the calculation, we will output statistical tables of several different indicators (ARE, PR, RR, F1, etc.) for you to analyze.
.
