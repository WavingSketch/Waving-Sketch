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
    int HIT;

    virtual void Init(const Data& from, const Data& to) = 0;
    virtual void Check(HashMap mp) = 0;
    virtual ~Abstract(){}

    void print_are(ofstream& out, int num){
        out << name << "," << num << "," << are << endl;
        cout << name << "," << num << "," << are << endl;
    }
    void print_aae(ofstream& out, int num){
        out << name << "," << num << "," << aae << endl;
        cout << name << "," << num << "," << aae << endl;
    }
    void print_pr(ofstream& out, int num){
        out << name << "," << num << "," << pr << endl;
        cout << name << "," << num << "," << pr << endl;
    }
    void print_cr(ofstream& out, int num){
        out << name << "," << num << "," << cr << endl;
        cout << name << "," << num << "," << cr << endl;
    }
};

#endif // ABSTRACT_H
