#pragma once

#include "StreamSummary.h"
template<uint32_t DATA_LEN>
class USS : public Abstract<DATA_LEN> {
public:
	USS(uint32_t _SIZE) {
		this->name = "USS";
		summary = new StreamSummary<DATA_LEN>(_SIZE);
	}
	~USS() { delete summary; }

	void Init(const Data<DATA_LEN>& item) {
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

	int Query(const Data<DATA_LEN>& item) { return summary->Query(item); }

private:
	StreamSummary<DATA_LEN>* summary;
};