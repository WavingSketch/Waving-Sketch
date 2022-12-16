# WavingSketch on Apache Flink

We implement WavingSketch on top of Apache Flink to show that our solution can be easily integrated into modern stream
processing framework and work in distributed environment. The results show that WavingSketch can smoothly work on top of
Flink framework.

## File Structure

`wavingsketch/WavingSketch.java`: The implementation of the basic WavingSketch.

`wavingsketch/WavingSketchFunction.java`: A wrapper class which wraps the original WavingSketch.

`wavingsketch/WavingSketchJob.java`: The implementation of the main method, which gets its input stream by reading the (HDFS) file, and insert them into the WavingSketch.

`wavingsketch/Murmur.java`: A hash function, the algorithm of which is almost the same as the C++ implementation from https://github.com/MapEmbed/MapEmbed/blob/master/CPU/MapEmbed/murmur3.h

`wavingsketch/WavingSketchTest.java`: A benchmark for testing, which can output similar result to the C++ implementation.

`flink.dat`: The sample input file, which is generated based on CAIDA datasets.

## Input File

We generate the input file based on CAIDA datasets. We present a sample dataset `flink.dat` here, which consists of
1,000,000 items, among which 39,954 items are distinct. For the full CAIDA datasets, please register
in [CAIDA](http://www.caida.org/home/) first and then apply for the traces.

The format of the input file is as follows. Each row of the input file is a 2-tuple consisting of an ID (Integer) and a
timestamp (Long), seperated by a comma.

```
id1,timestamp1
id2,timestamp2
id3,timestamp3
......
```

## Requirements

- `Flink 1.13.1`

- `Hadoop 2.8.3`

To address the problem of dependency when using Hadoop Distributed File System in
Flink, [flink-shaded-hadoop-2-uber-2.8.3-9.0.jar]([Central Repository: org/apache/flink/flink-shaded-hadoop-2-uber/2.8.3-9.0](https://repo.maven.apache.org/maven2/org/apache/flink/flink-shaded-hadoop-2-uber/2.8.3-9.0/))
is needed. Please add it to `{FLINK_HOME}/lib/`.

## How to run

1. Build the package. The jar package can be built by `Maven 3.2.5`.

2. Set up Flink configuration and Hadoop Distributed File System. Copy the input file to HDFS.

3. Run. Suppose the path of the input file is `hdfs://test.txt`, and the path of the jar package
   is `/root/WavingSketch.jar`, you can use the following command to run the codes.

   ```shell
   {FLINK_HOME}/bin/flink run /root/WavingSketch.jar -p k --input hdfs:///test.txt
   ```

   where `k` is the parallism (# parallel instances) in Flink.

   

