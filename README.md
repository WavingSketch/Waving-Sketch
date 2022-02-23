# WavingSketch

## Introduction

Finding top-k items in data streams is a fundamental problem in data mining. Existing algorithms that can achieve unbiased estimation suffer from poor accuracy. In this paper, we propose a new sketch, WavingSketch, which is much more accurate than existing unbiased algorithms. WavingSketch is generic, and we show how it can be applied to four applications: finding top-k frequent items, finding top-k heavy changes, finding top-k persistent items, and finding top-k Super-Spreaders. We theoretically prove that WavingSketch can provide unbiased estimation, and then give an error bound of our algorithm. Our experimental results show that, compared with the state-of-the-art, WavingSketch has 5 times higher insertion speed and at least 10<sup>3</sup> times lower error rate in finding frequent items under small memory overhead. For other applications, WavingSketch also achieves higher accuracy and faster insertion speed. All related codes are open-sourced at Github

## About this repo

- `Waving` contains codes of WavingSketch (including Multi-Counter WavingSketch) and the algorithms we compared with in the four tasks. 

- `SIMD` contains codes of WavingSketch (including Multi-Counter WavingSketch) and the related algortihms implemented using SIMD optimization in the task of finding frequent items. It also contains the codes of our compression and expansion experiments. 

- `Flink` contains codes of WavingSketch implemented on top of Apache Flink and a sample dataset used in Flink experiments. 

- More details can be found in the folders. 