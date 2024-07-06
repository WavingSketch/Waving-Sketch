#pragma once

#include <time.h>
#include <unistd.h>

#include <algorithm>
#include <climits>

#include "Sketch.h"

typedef uint32_t data_type;  // note: according to your dataset
typedef int32_t count_type;  // note: according to your dataset

inline uint32_t hash32(data_type item, uint32_t seed = 0) {
  uint32_t temp;
  MurmurHash3_x86_32(&item, sizeof(data_type), seed, &temp);
  return temp;
}
static const count_type COUNT[2] = {1, -1};

#define factor 1

template <uint32_t slot_num, uint32_t counter_num>
class WavingSketch : public Sketch {
 public:
  struct Bucket {
    data_type items[slot_num];
    count_type counters[slot_num];
    int16_t incast[counter_num];

    void Insert(const data_type item, uint32_t seed_s, uint32_t seed_incast) {
      uint32_t choice = hash32(item, seed_s) & 1;
      uint32_t whichcast = hash32(item, seed_incast) % counter_num;
      count_type min_num = INT_MAX;
      uint32_t min_pos = -1;

      for (uint32_t i = 0; i < slot_num; ++i) {
        if (counters[i] == 0) {
          // The error free item's counter is negative, which is a trick to
          // be differentiated from items which are not error free.
          items[i] = item;
          counters[i] = -1;
          return;
        } else if (items[i] == item) {
          if (counters[i] < 0)
            counters[i]--;
          else {
            counters[i]++;
            incast[whichcast] += COUNT[choice];
          }
          return;
        }

        count_type counter_val = std::abs(counters[i]);
        if (counter_val < min_num) {
          min_num = counter_val;
          min_pos = i;
        }
      }

      if (incast[whichcast] * COUNT[choice] >= int(min_num * factor)) {
        if (counters[min_pos] < 0) {
          uint32_t min_choice = hash32(items[min_pos], seed_s) & 1;
          incast[hash32(items[min_pos], seed_incast) % counter_num] -=
              COUNT[min_choice] * counters[min_pos];
        }
        items[min_pos] = item;
        counters[min_pos] = min_num + 1;
      }
      incast[whichcast] += COUNT[choice];
    }

    count_type Query(const data_type item, uint32_t seed_s,
                     uint32_t seed_incast) const {
      for (uint32_t i = 0; i < slot_num; ++i) {
        if (items[i] == item) {
          return std::abs(counters[i]);
        }
      }
      return 0;
    }

    bool hold(const data_type item) const {
      for (int i = 0; i < slot_num; ++i) {
        if (items[i] == item) return true;
      }
      return false;
    }

    count_type smart_query(const data_type item, uint32_t seed_s, uint32_t seed_incast) const {
      for (int i = 0; i < slot_num; ++i) {
        if (items[i] == item) {
            return std::abs(counters[i]);
        }
      }
      uint32_t whichcast = hash32(item, seed_incast) % counter_num;
      uint32_t choice = hash32(item, seed_s) & 1;
      auto ret = incast[whichcast] * COUNT[choice];
      return ret;
    }

    long double Join1(const Bucket &other, uint32_t seed_s,
                      uint32_t seed_incast) const {
      long double re = 0;
      // Process items held by this bucket exclusively
      for (int i = 0; i < slot_num; ++i) {
        auto item = items[i];
        if (!other.hold(item)) {
          re += smart_query(item, seed_s, seed_incast) * other.smart_query(item, seed_s, seed_incast);
        }
      }
      // Process items held by the other bucket exclusively
      for (int i = 0; i < slot_num; ++i) {
        auto item = other.items[i];
        if (!hold(item)) {
          re += smart_query(item, seed_s, seed_incast) * other.smart_query(item, seed_s, seed_incast);
        }
      }
      // Process item held by both this bucket and the other
      // for (int i = 0; i < slot_num; ++i) {
      //   auto item = items[i];
      //   if (other.hold(item)) {
      //     re += std::abs(counters[i]) * other.Query(item, seed_s, seed_incast);
      //   }
      // }
      for (int i = 0; i < slot_num; ++i) {
        auto item = items[i];
        if (other.hold(item)) {
          re += smart_query(item, seed_s, seed_incast) * other.smart_query(item, seed_s, seed_incast);
        }
      }
      return re;
    }
  };

  WavingSketch(uint32_t w, uint32_t d, uint32_t hash_seed) {
    BUCKET_NUM = w / sizeof(Bucket);
    buckets = new Bucket[BUCKET_NUM];
    memset(buckets, 0, BUCKET_NUM * sizeof(Bucket));
    // seed_choice = std::clock();
    // sleep(1);  // need to sleep for a while, or seed_choice and seed_incast
    //            // might get the same seed!
    // seed_incast = std::clock();
    // sleep(1);  // need to sleep for a while, or seed_incast and seed_s might
    // get
    //            // the same seed!
    // seed_s = std::clock();
    seed_choice = hash_seed;
    seed_incast = hash32(seed_choice, hash_seed);
    seed_s = hash32(seed_incast, hash_seed);
  }
  ~WavingSketch() { delete[] buckets; }

  void Insert(const char *str) override {
    const data_type &item = *(const data_type *)str;
    uint32_t bucket_pos = hash32(item, seed_choice) % BUCKET_NUM;
    buckets[bucket_pos].Insert(item, seed_s, seed_incast);
  }

  int Query(const char *str) const override {
    const data_type &item = *(const data_type *)str;
    uint32_t bucket_pos = hash32(item, seed_choice) % BUCKET_NUM;
    return buckets[bucket_pos].Query(item, seed_s, seed_incast);
  }

  long double Join(Sketch *_other) override {
    auto other = (const WavingSketch<slot_num, counter_num> *)_other;
    long double re = 0;
    for (int i = 0; i < BUCKET_NUM; ++i) {
      const auto &bucket = buckets[i];
      const auto &other_bucket = other->buckets[i];
      re += bucket.Join1(other_bucket, seed_s, seed_incast);
    }
    return re;
  }

 private:
  Bucket *buckets;
  uint32_t BUCKET_NUM;
  uint32_t seed_choice;
  uint32_t seed_s;
  uint32_t seed_incast;
};