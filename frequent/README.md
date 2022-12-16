# WavingSketch on finding frequent items

## File description

- `abstract.h`: The base class for class `WavingSketch` and class `Count_Heap`
- `hash.h`: Murmur Hash function, obtained from [here](https://github.com/MapEmbed/MapEmbed/blob/master/CPU/MapEmbed/murmur3.h)
- `StreamSummary.h`: The data structure for SpaceSaving (SS) and Unbiased SpaceSaving (USS)

The contents of the other files are consistent with their names.

## How to run

By default, we use the sample dataset `demo.dat` in `../dataset` (by setting `fid = 0` in `./main.cpp`).
You can also download the dataset `syn.dat` from [here](https://drive.google.com/file/d/1jau6Yc4H4wrYvj-c9Ci1XXnI_1ICdlsm/view?usp=sharing), and put it in `../dataset` and set `fid = 1` in `./main.cpp`. 




To run the tests, first run the following command to generate a Makefile. 
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




