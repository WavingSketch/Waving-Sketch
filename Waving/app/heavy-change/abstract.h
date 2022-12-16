#ifndef ABSTRACT_H
#define ABSTRACT_H

#include "data.h"
#include <iostream>

static const int COUNT[2] = {1, -1};

class Abstract{
public:
    string name;
    double pr;
    double cr;
    string sep;

    virtual void Init(const Data& data) = 0;
    virtual int Query(const Data &data, HashMap &mp) = 0;
    virtual void Check(HashMap mp, Abstract* another) = 0;
    virtual ~Abstract(){};

    void rename(int slot_num,int counter_num)
	{
		char* tp=new char[20];
		std::sprintf(tp,"WavingSketch<%d,%d>",slot_num,counter_num);
		name=tp;
	}

    void print_pr(ofstream& out, int num){
        cout.width(15);
        cout << name << " " << pr << endl;
    }
    void print_cr(ofstream& out, int num){
        cout.width(15);
        cout << name << " " << cr << endl;
    }
    void print_f1(ofstream& out, int num){
        cout << name << sep << 2*pr*cr/(pr+cr) << endl;
    }
};

#endif // ABSTRACT_H
