#pragma once

#include "StreamSummary.h"

class SS : public Abstract {
public:
	SS(uint32_t _SIZE) : Abstract((char *)"SS") {
		summary = new StreamSummary(_SIZE);
	}
	~SS() { delete summary; }

	void Insert(const data_type item) {
		if (!summary->Add_Data(item)) {
			summary->Add_Counter(item, !summary->isFull());
		}
	}

	count_type Query(const data_type item) { return summary->Query(item); }

private:
	StreamSummary *summary;
};