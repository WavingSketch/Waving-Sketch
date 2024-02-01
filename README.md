# WavingSketch: An Unbiased and Generic Sketch for Finding Top-k Items in Data Streams

This repository contains all related code of our paper "WavingSketch: An Unbiased and Generic Sketch for Finding Top-k Items in Data Streams". 

## Introduction

Finding top-k items in data streams is a fundamental problem in data mining. Unbiased estimation is well acknowledged as an elegant and important property for top-k algorithms. In this paper, we propose a novel sketch algorithm, called WavingSketch, which is more accurate than existing unbiased algorithms. We theoretically prove that WavingSketch can provide unbiased estimation, and derive its error bound. WavingSketch is generic to measurement tasks, and we apply it to five applications: finding top-k frequent items, finding top-k heavy changes, finding top-k persistent items, finding top-k Super-Spreaders, and join-aggregate estimation. Our experimental results show that, compared with the state-of-the-art Unbiased Space-Saving, WavingSketch achieves 9.3× faster speed and 10<sup>3</sup>× smaller error on finding frequent items. For other applications, WavingSketch also achieves higher accuracy and faster speed.


## About this repo

- `frequent` contains codes of WavingSketch (including Multi-Counter WavingSketch) and the algorithms we compared with in the application of finding frequent items. 

- `app` contains codes of WavingSketch and the related algorithms in the other six applications:  finding top-k heavy changes, 
finding top-k persistent items, finding top-k Super-Spreaders, finding global top-k, subset query, and join-aggregate estimation.

- `elastic` contains codes to evaluate the performance of the elastic operations and automatic memory adjustment of WavingSketch. In this folder, we implement WavingSketch (including Multi-Counter WavingSketch) and the related algortihms using SIMD instructions. 

- `dataset` contains two small dataset shards to run our tests. We also upload our synthetic datasets to google drive and leave a link here. 

- `flink` contains codes of WavingSketch implemented on top of Apache Flink and a sample dataset used in Flink experiments. 

- More details can be found in the folders. 