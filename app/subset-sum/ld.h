#pragma once

#include "LD-Sketch/LDSketch.hpp"
#include "abstract.h"

class LdSketchWrapper : public Abstract {
 public:
  LdSketchWrapper(uint32_t memory_size, uint32_t topK) : Abstract((char *)"LD Sketch") {
    // 4 rows
    // 40 bytes for LD Sktech
    // 104 bytes for each bucket
    int w = (memory_size - 40) / 4 / 104;
    summary = LDSketch_init(w, 4, 1, sizeof(data_type) * 8, topK, 0);
  }
  ~LdSketchWrapper() { delete summary; }

  inline void Insert(const data_type item) {
    LDSketch_update(summary, item, 1);
  }

  inline count_type Query(const data_type item) {
    return LDSketch_up_estimate(summary, item);
  }

  double average_l() {
    int l_sum = 0;
    int count = (summary->h) * (summary->w);
    for (int i = 0; i < count; ++i) {
      l_sum += summary->tbl[i]->max_len;
    }
    return l_sum / (double)count;
  }

 private:
  LDSketch_t *summary;
};