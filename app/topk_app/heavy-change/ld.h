#pragma once

#include "LD-Sketch/LDSketch.hpp"
#include "abstract.h"

class LdSketchWrapper : public Abstract {
 public:
  LdSketchWrapper(uint32_t memory_size, int _HIT) : HIT(_HIT) {
    // 4 rows
    // 40 bytes for LD Sketch
    // 104 bytes for each bucket
    int w = (memory_size - 40) / 4 / 104;
    summary = LDSketch_init(w, 4, 1, sizeof(Data) * 8, HIT, 0);
    name = "LD Sketch";
    sep = "\t";
  }
  ~LdSketchWrapper() { delete summary; }

  void Init(const Data &data) {
    unsigned char *key = (unsigned char *)(&(data.str[0]));
    LDSketch_update(summary, key, 1);
  }

  int Query(const Data &data, HashMap &mp) {
    // estimate the frequency of data and store the estimations in mp
    unsigned char *key = (unsigned char *)(&(data.str[0]));
    mp[up_key] = LDSketch_up_estimate(summary, key);
    mp[low_key] = LDSketch_low_estimate(summary, key);
    return 0;
  }

  void Check(HashMap mp, Abstract *another, const std::vector<std::ostream*>& outs) {
    HashMap est1, est2;

    HashMap::iterator it;
    int value = 0, all = 0, hit = 0, size = 0;
    for (it = mp.begin(); it != mp.end(); ++it) {
      Query(it->first, est1);
      another->Query(it->first, est2);

      value = max(abs(est1[up_key] - est2[low_key]),
                  abs(est2[up_key] - est1[low_key]));
      if (abs(it->second) > HIT) {
        all++;
        if (value > HIT) {
          hit += 1;
        }
      }
      if (value > HIT) size += 1;
    }

    cr = hit / (double)all;
    pr = hit / (double)size;
    *outs[0] << cr << ",";
    *outs[1] << pr << ",";
    *outs[2] << 2 * pr * cr / ((pr + cr < 1e-9) ? 1.0 : pr + cr) << ",";
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
  int HIT;
  const Data up_key = {"up_key"}, low_key = {"low_key"};
};