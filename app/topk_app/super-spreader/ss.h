#ifndef SS_H
#define SS_H

#include "SpreadSketch/spreadsketch.hpp"
#include "abstract.h"

class SpreadSketchWrapper : public Abstract {
 public:
  SpreadSketchWrapper(int memory, int _HIT) : HIT(_HIT) {
    // total_mem = cmdepth*cmwidth*(memory+lgn+8)/8
    int width = memory * 8 / 4 / (438 + 32 + 8);
    sketch = new DetectorSS(4, width, 32, 79, 3, 438);
    name = "SpreadSketch";
    sep = "\t";
  }
  ~SpreadSketchWrapper() { delete sketch; }

  void Init(const Data& from, const Data& to) {
    key_tp src_ip = *(key_tp*)from.str;
    key_tp dst_ip = *(key_tp*)to.str;
    sketch->Update(src_ip, dst_ip, 1);
  };
  void Check(HashMap mp, const std::vector<std::ostream*>& outs) {
    HashMap::iterator it;
    vector<pair<key_tp, val_tp>> result;
    sketch->Query(HIT, result);
    HashMap our_result;
    for (auto const& p : result) {
      Data key;
      memcpy(key.str, &(p.first), sizeof(Data));
      our_result[key] = p.second;
    }

    int value = 0, all = 0, hit = 0, size = 0;
    for (it = mp.begin(); it != mp.end(); ++it) {
      value = our_result[it->first];
      if (it->second > HIT) {
        all++;
        if (value > HIT) {
          hit += 1;
          aae += abs(it->second - value);
          are += abs(it->second - value) / (double)it->second;
        }
      }
      if (value > HIT) size += 1;
    }

    aae /= hit;
    are /= hit;
    cr = hit / (double)all;
    pr = hit / (double)size;
    *outs[0] << are << ",";
    *outs[1] << cr << ",";
    *outs[2] << pr << ",";
    *outs[3] << 2 * pr * cr / ((cr + pr < 1e-6) ? 1 : cr + pr) << ",";
  }

 private:
  DetectorSS* sketch;
  int HIT;
};

#endif  // SS_H