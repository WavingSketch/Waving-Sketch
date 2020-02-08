# WavingSketch

## Introduction

In high-speed data streams, frequent items refer to items whose number of appearances exceeds a predefined threshold. Finding frequent items is a classic and critical problem in many applications, and unbiased estimation is an important aspect of this problem. However, existing algorithms which can achieve unbiased estimation suffer from poor accuracy. In this paper, we propose a new algorithm, WavingSketch, which is more accurate than existing unbiased algorithms while achieving unbiasedness. WavingSketch is also generic, and we show how it can be applied to three other different applications: finding heavy changes, finding persistent items, and finding Super-Spreaders.	We prove its unbiasedness and a much lower error compared with the state-of-the-art technique, Unbiased SpaceSaving. Our experimental results show that, compared with Unbiased SpaceSaving, WavingSketch increases the insertion throughput by 4.50 times in average and decreases the error to up to 9 * 10^6 times (66 times in average) in finding frequent items. For other applications, WavingSketch can also decrease the error to up to 56 times.

## Descriptions

We provide the source codes of WavingSketch and the algorithms we compared with in the task of finding frequent items in `./src`. We also provide the source codes for the specific version of WavingSketch and the algorithms we compared with in the three applications in `./src`, respectively.
