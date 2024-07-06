#pragma once

#include "StreamSummary.h"
template<uint32_t DATA_LEN>
class SS : public Abstract<DATA_LEN> {
public:
	SS(uint32_t _SIZE) {
		this->name = "SS";
		summary = new StreamSummary<DATA_LEN>(_SIZE);
	}
	~SS() { delete summary; }

	void Init(const Data<DATA_LEN>& item) {
		if (!summary->Add_Data(item)) {
			summary->Add_Counter(item, !summary->isFull());
		}
	}

	int Query(const Data<DATA_LEN>& item) { return summary->Query(item); }

private:
	StreamSummary<DATA_LEN>* summary;
};