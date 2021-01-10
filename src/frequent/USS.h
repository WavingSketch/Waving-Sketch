#pragma once

#include "StreamSummary.h"

class USS : public Abstract {
public:
	USS(uint32_t _SIZE) : Abstract((char *)"USS") {
		summary = new StreamSummary(_SIZE);
	}
	~USS() { delete summary; }

	void Insert(const data_type item) {
		if (!summary->Add_Data(item)) {
			if (summary->isFull()) {
				if (rd() % (summary->Min_Num() + 1) == 0) {
					summary->Add_Counter(item, false);
				}
				else {
					summary->Add_Min();
				}
			}
			else {
				summary->Add_Counter(item, true);
			}
		}
	}

	count_type Query(const data_type item) { return summary->Query(item); }

private:
	StreamSummary *summary;
};