#pragma once
class Sketch {
public:
	virtual void Insert(const char* str)=0;
	virtual int Query(const char* str)const=0;
	virtual long double Join(Sketch* other)=0;
	virtual bool CheckHeavy(const char* str){
		return 0;
	}
};