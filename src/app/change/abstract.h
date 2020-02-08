#ifndef ABSTRACT_H
#define ABSTRACT_H

#include "data.h"
#include <iostream>

class Abstract{
public:
    string name;
    double pr;
    double cr;

    virtual void Init(const Data& data) = 0;
    virtual int Query(const Data &data, HashMap &mp) = 0;
    virtual void Check(HashMap mp, Abstract* another) = 0;
    virtual ~Abstract(){};

    void print_pr(ofstream& out, int num){
        //out << name << "," << num << "," << pr << endl;
        cout.width(15);
        cout << name << " " << pr << endl;
    }
    void print_cr(ofstream& out, int num){
        //out << name << "," << num << "," << cr << endl;
        cout.width(15);
        cout << name << " " << cr << endl;
    }
    void print_f1(ofstream& out, int num){
        //out << name << "," << num << "," << cr << endl;
        cout.width(15);
        cout << name << " " << 2*pr*cr/(pr+cr) << endl;
    }
};

#endif // ABSTRACT_H
