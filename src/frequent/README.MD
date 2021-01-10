# Source Codes for Finding Frequent Items

## Discriptions

- `abstract.h`: The base class for class `WavingSketch` and class `Count_Heap`
- `hash.h`: The hash function, from http://burtleburtle.net/bob/hash/evahash.html
- `StreamSummary.h`: The data structure for `SS` and `USS`

The contents of other sources are consistent with their names.

## Run

To run the code, you first need to download the dataset from https://drive.google.com/file/d/1jau6Yc4H4wrYvj-c9Ci1XXnI_1ICdlsm/view?usp=sharing. This is a dataset genreated with the opensource tool webpolygraph(http://www.web-polygraph.org/). You should create a directory `dataset` that is in the same directory with directory `frequent`, and put the downloaded dataset there.

To run, first run `cmake .` to generate a Makefile. Then `make`, and a executable file `bench` will be generate. Then `./bench`, the result will be shown on the screen.
