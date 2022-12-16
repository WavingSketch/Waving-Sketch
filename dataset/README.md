# About the datasets

In our paper, we use four datasets to conduct the experiments (CAIDA, Synthetic, Webpage, Network).  CAIDA Anonymized Internet Trace is a data stream of anonymized IP trace collected in 2018. Each packet is identified by its source IP address (4 bytes), source port number (2 bytes), destination IP address (4 bytes), destination port number (2 bytes), and protocol type (1 bytes). Each packet is associated with a 8-byte timestamps of double type. 


Here, we provide two small sample datasets:

- A dataset shard extracted from the real-world CAIDA dataset (`./demo.dat`). For the full CAIDA datasets, please register in [CAIDA](http://www.caida.org/home/) first, and then apply for the traces. 

- A dataset shard extracted from our synthetic dataset `syn.dat`. You can download the full dataset file [here](https://drive.google.com/file/d/1jau6Yc4H4wrYvj-c9Ci1XXnI_1ICdlsm/view?usp=sharing). This is a dataset genreated with the opensource tool [webpolygraph](http://www.web-polygraph.org/). 


**Notification:** The data file in this folder is only used for testing the performance of WavingSketch and the related algorithms in this project. Please do not use these traces for other purpose. 
