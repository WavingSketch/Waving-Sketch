# WavingSketch on three top-k applications

## Overview

This folder contains the codes of WavingSketch and the related algorithms on three top-k applications: finding top-k heavy changes, finding top-k persistent items, and finding top-k Super-Spreaders. 


## File description

- `./heavy-change`: implementation of WavingSketch in finding heavy changes. 

- `./persistent-item`: implementation of WavingSketch in finding persistent items. 

- `./super-spreader`: implementation of WavingSketch in finding super-spreaders. 


## How to run

You can enter the three folders to run the codes. 
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
$ ./bench
```

The results will be shown on the screen. 


By default, we use the small sample dataset `demo.dat` in `../../dataset`. You can modify the constant string `FOLDER` in `main.cpp` to specify other datasets. 
