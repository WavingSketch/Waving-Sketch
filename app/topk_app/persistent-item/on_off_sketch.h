#ifndef OO_FPI_H
#define OO_FPI_H

/*
 * On-Off sketch on finding persistent items
 */

#include "abstract.h"
#include "bitset.h"

const static int SLOT_NUM = 8;

class OO_FPI : public Abstract {
 public:
  struct Bucket {
    Data items[SLOT_NUM];
    int counters[SLOT_NUM];

    inline int Query(const Data item) {
      for (uint32_t i = 0; i < SLOT_NUM; ++i) {
        if (items[i] == item) return counters[i];
      }
      return 0;
    }
  };

  OO_FPI(uint64_t memory, int _HIT)
      : length((double)memory /
               (sizeof(Bucket) + sizeof(int) + (SLOT_NUM + 1) * BITSIZE)),
        HIT(_HIT) {
    name = "On Off Sketch";
    sep = "\t";
    buckets = new Bucket[length];
    sketch = new int[length];

    memset(buckets, 0, length * sizeof(Bucket));
    memset(sketch, 0, length * sizeof(int));

    bucketBitsets = new BitSet(SLOT_NUM * length);
    sketchBitsets = new BitSet(length);
  }

  ~OO_FPI() {
    delete[] buckets;
    delete[] sketch;
    delete bucketBitsets;
    delete sketchBitsets;
  }

  void Init(const Data& item, uint window) {
    if (window > last_window) {
      last_window = window;
      NewWindow();
    }

    uint32_t pos = hash(item, 17) % length;
    uint32_t bucketBitPos = pos * SLOT_NUM;

    for (uint32_t i = 0; i < SLOT_NUM; ++i) {
      if (buckets[pos].items[i] == item) {
        buckets[pos].counters[i] += (!bucketBitsets->SetNGet(bucketBitPos + i));
        return;
      }
    }

    if (!sketchBitsets->Get(pos)) {
      for (uint32_t i = 0; i < SLOT_NUM; ++i) {
        if (buckets[pos].counters[i] == sketch[pos]) {
          buckets[pos].items[i] = item;
          buckets[pos].counters[i] += 1;
          bucketBitsets->Set(bucketBitPos + i);
          return;
        }
      }

      sketch[pos] += 1;
      sketchBitsets->Set(pos);
    }
  }

  int Query(const Data item) {
    return buckets[hash(item, 17) % length].Query(item);
  }

  void Check(HashMap mp, const std::vector<std::ostream*>& outs) {
    HashMap::iterator it;
    int value = 0, all = 0, hit = 0, size = 0;
    for (it = mp.begin(); it != mp.end(); ++it) {
      value = Query(it->first);
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
    cr = hit / (double)std::max(1, all);
    pr = hit / (double)std::max(1, size);
    *outs[0] << cr << ",";
    *outs[1] << pr << ",";
    *outs[2] <<  2 * pr * cr / ((pr + cr < 1e-9) ? 1.0 : pr + cr) << ",";
    *outs[3] << are << ",";
  }

  void NewWindow() {
    bucketBitsets->Clear();
    sketchBitsets->Clear();
  }

 private:
  const uint32_t length;

  BitSet* bucketBitsets;
  Bucket* buckets;

  BitSet* sketchBitsets;
  int* sketch;
  int HIT;
  int last_window = 0;
};

#endif  // OO_FPI_H