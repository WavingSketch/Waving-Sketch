#ifndef ABSTRACT_H
#define ABSTRACT_H

#include "data.h"
#include <iostream>

class Abstract{
public:
    string name;
    double aae;
    double are;
    double pr;
    double cr;
    string sep;
    int HIT;

    virtual void Init(const Data& data, uint time) = 0;
    virtual void Check(HashMap mp) = 0;
    virtual ~Abstract(){}

    void rename(int slot_num,int counter_num)
	{
		char* tp=new char[20];
		std::sprintf(tp,"WavingSketch<%d,%d>",slot_num,counter_num);
		name=tp;
	}

    void print_are(ofstream& out, int num){
        out << name << "," << num << ", are: " << are << endl;
        cout << name << "," << num << ", are: " << are << endl;
    }
    void print_aae(ofstream& out, int num){
        out << name << "," << num << ", aae: " << aae << endl;
        cout << name << "," << num << ", aae: " << aae << endl;
    }
    void print_pr(ofstream& out, int num){
        out << name << "," << num << ", pr: " << pr << endl;
        cout << name << "," << num << ", pr: " << pr << endl;
    }
    void print_cr(ofstream& out, int num){
        out << name << "," << num << ", cr: " << cr << endl;
        cout << name << "," << num << ", cr: " << cr << endl;
    }
    void print_f1(ofstream& out, int num){
        cout << name << sep << 2*pr*cr/(pr+cr) << endl;
    }
};

#endif // ABSTRACT_H
