GCC = g++
CFLAGS = -O2 -w -std=c++14 
SSEFLAGS =   -march=native
FILES = main.out 

all: $(FILES) 

main.out: main.cpp
	$(GCC) $(CFLAGS) $(SSEFLAGS) -o main.out main.cpp

.PHONY: clean
clean:
	-rm -f *.out $(objs)
