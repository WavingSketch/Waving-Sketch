# WavingSketch on Finding Frequent Items

## File Discription

- `abstract.h`: The base class for class `WavingSketch` and class `Count_Heap`
- `hash.h`: Murmur Hash function, obtained from https://github.com/MapEmbed/MapEmbed/blob/master/CPU/MapEmbed/murmur3.h
- `StreamSummary.h`: The data structure for SpaceSaving (SS) and Unbiased SpaceSaving (USS)

The contents of other files are consistent with their names.

## How to Run

To run the code, you can use the dataset `demo.dat` in `../dataset` and set `fid = 0` in `./main.cpp`. Or you can download the dataset `syn.dat` from https://drive.google.com/file/d/1jau6Yc4H4wrYvj-c9Ci1XXnI_1ICdlsm/view?usp=sharing. This is a dataset genreated with the opensource tool webpolygraph(http://www.web-polygraph.org/). You should place it in the directory `../dataset` and set `fid = 1` in `./main.cpp`.

To run the test, first run `cmake .` to generate a Makefile. Then `make`, and a executable file `bench` will be generate. Then `./bench`, the result will be shown on the screen.
