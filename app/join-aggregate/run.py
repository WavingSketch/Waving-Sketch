#!/usr/bin/env python3

import os

filename = './datasets/CAIDA/0.dat'
key_len = '13'
# filename = '/share/zipf_2022/zipf_0.8.dat'
# key_len = '4'

os.system('cd src && make && cd ..')

for i in range(1, 7):    
    for j in range(10, 201, 10):
        version = str(i)
        memory = str(j * 1000)

        command = './src/main -f ' + filename + ' -l ' + \
            key_len + ' -m ' + memory + ' -v ' + version
        os.system(command)
