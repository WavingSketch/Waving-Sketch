# WavingSketch using SIMD optimization 

## File Discriptions

- `abstract.h`: Base class for class `WavingSketch` and class `Count_Heap`
- `hash.h`: BOB Hash functions, obtained from http://burtleburtle.net/bob/hash/evahash.html
- `murmur3.h`: Murmur Hash functions, which is faster than BOB Hash, obtained from https://github.com/MapEmbed/MapEmbed/blob/master/CPU/MapEmbed/murmur3.h
- `WavingSkecth.h`: Implementation of WavingSketch, including both non-SIMD implementation and SIMD implementation.
- `benchmark.h`: Implementation of the benchmarks, including the benchmarks for our compression and expansion experiments. (see details in Result Explanation)
- `StreamSummary.h`: Implementation of SpaceSaving (SS) and Unbiased SpaceSaving (USS).
- `ResultExample.txt`: An example of the benchmark result.

The contents of other sources are consistent with their names.

## How to Run

We provide a small sample dataset in `./dataset/syn.dat`. You can download the full synthetic dataset from https://drive.google.com/file/d/1jau6Yc4H4wrYvj-c9Ci1XXnI_1ICdlsm/view?usp=sharing, and replace the sample dataset with it. This is a dataset genreated with the opensource tool webpolygraph(http://www.web-polygraph.org/). 

Since the SIMD code uses AVX-512 instructions, you must have your CPU AVX512-supported. 

To run the benchmarks, first run `cmake .` to generate a Makefile. Then `make`, and a
executable file `bench` will be generate. Then `./bench`, the result will be
shown on the screen.

## Result Explanation

Here is an example of the benchmark result.

```

**Benchmark**

1. Comparison with SS, USS, Count+Heap on ARE, CR and PR

Memory size: 50KB

WavingSketch4:	ARE: 0.090973	CR: 0.810215	PR: 0.449819
WavingSketch8:	ARE: 0.051634	CR: 0.819229	PR: 0.380642
WavingSketch16:	ARE: 0.029987	CR: 0.823235	PR: 0.334418
WavingSketch32:	ARE: 0.016688	CR: 0.826239	PR: 0.314525
          SS:	ARE: 6.977248	CR: 0.123185	PR: 0.492000
         USS:	ARE: 7.344960	CR: 0.125188	PR: 0.500000
  Count_Heap:	ARE: 0.000000	CR: 1.000000	PR: 1.000000
     WSSIMD4:	ARE: 0.128757	CR: 0.759639	PR: 0.483892
     WSSIMD8:	ARE: 0.065272	CR: 0.802704	PR: 0.385893
    WSSIMD16:	ARE: 0.035548	CR: 0.808212	PR: 0.345241
    WSSIMD32:	ARE: 0.020682	CR: 0.817727	PR: 0.310220
WavingSketchMC<4, 16>:	ARE: 0.018329	CR: 0.810215	PR: 0.832305
WavingSketchMC<8, 16>:	ARE: 0.004724	CR: 0.910366	PR: 0.826739
WavingSketchMC<16, 16>:	ARE: 0.003161	CR: 0.928893	PR: 0.838608
WavingSketchMC<32, 16>:	ARE: 0.002120	CR: 0.910366	PR: 0.781599
WSSIMDMC<4, 16>:	ARE: 0.018329	CR: 0.810215	PR: 0.832305
WSSIMDMC<8, 16>:	ARE: 0.004724	CR: 0.910366	PR: 0.826739
WSSIMDMC<16, 16>:	ARE: 0.003161	CR: 0.928893	PR: 0.838608
WSSIMDMC<32, 16>:	ARE: 0.002120	CR: 0.910366	PR: 0.781599

...
```

* `WavingSketch<int>` stands for the WavingSketch without SIMD optimization using `<int>` cells per buckets. For Example, `WavingSketch4` stands for the non-SIMD implementation of the WavingSketch with 4 cells per buckets.

* `WSSIMD<int>` stands for the SIMD implementation of the WavingSketch
with `<int> - 1` cells per buckets (Because of the alignment requirements of the SIMD instructions, we reserve a cell and treat it as the Waving Counter in our implementation).
For Example, `WavingSketch4` stands for the SIMD implementation of the
WavingSketch with 3 cells per buckets.

* `WavingSketchMC<<int1>, <int2>>` stands for the non-SIMD implementation of the WavingSketch, with the optimization using multi-counter bucket. Each bucket has `<int1>` counters and `<int2>` cells. For Example, `WavingSketchMC<4, 16>` stands for the non-SIMD implementation of the WavingSketch, with 4 counters and 16 cells per bucket.

* `WSSIMDMC<<int1>, <int2>>` stands for the SIMD implementation of the
WavingSketch, with the optimization using multi-counter bucket. Each bucket has `<int1>` counters and `<int2>` cells. For Example, `WSSIMDMC<4, 16>` stands for the SIMD implementation of the WavingSketch, with 4 counters and 16 cells per bucket.

The benchmark can be divided into six parts.

The first two parts is the comparison with SS, USS, Count+Heap on ARE, CR, PR and Throughput.
By default, the benchmark tests the performance of each algorithm with memory size from 50KB to 300KB.
Both SIMD implementation and non-SIMD implementation are taken into consideration.

The third part compares the performance of the WavingSketch algorithm (without SIMD acceleration) between with and without elastic compression.
The fourth part compares the performance of the WavingSketch algorithm (without SIMD acceleration) between with and without elastic expansion.
The rest two parts are the SIMD accelerated version of the third and fourth part.
