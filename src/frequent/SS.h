#ifndef SS_SS_H
#define SS_SS_H

#include "StreamSummary.h"

class SS : public Abstract{
public:

    SS(uint32_t _SIZE):Abstract((char *)"SS"){
        summary = new StreamSummary(_SIZE);
    }
    ~SS(){
        delete summary;
    }

    void Insert(const data_type item){
        if(!summary->Add_Data(item)){
            summary->Add_Counter(item, !summary->isFull());
        }
    }

    count_type Query(const data_type item){
        return summary->Query(item);
    }
    /**/
    int Count_Nodes(){
        return summary->Count_Nodes();
    }

private:
    StreamSummary* summary;
};

#endif //SS_SS_H
