#pragma once

#include "abstract.h"
#include <immintrin.h>
#include <unistd.h>

#define factor 1
#define NO_ERROR_FIRST
// you should choose ONE AND ONLY ONE from the following four definition
// #define NO_INCAST
#define COPY_INCAST
// #define SPLIT_INCAST
// #define THRESHOLD_INCAST

template<uint32_t slot_num>
class WavingSketch;
template<uint32_t slot_num, uint32_t counter_num>
class WavingSketchMC;
template<uint32_t slot_num>
int32_t compress(WavingSketch<slot_num> *src, WavingSketch<slot_num> *dst);
template<uint32_t slot_num>
int32_t expand(WavingSketch<slot_num> *src, WavingSketch<slot_num> *dst);
template<uint32_t slot_num, uint32_t counter_num>
int32_t compressMC(WavingSketchMC<slot_num, counter_num> *src,
                   WavingSketchMC<slot_num, counter_num> *dst);
template<uint32_t slot_num, uint32_t counter_num>
int32_t expandMC(WavingSketchMC<slot_num, counter_num> *src,
                 WavingSketchMC<slot_num, counter_num> *dst);

template<uint32_t slot_num>
class WavingSketch : public Abstract {
 public:
  struct Bucket {
    data_type items[slot_num];
    count_type counters[slot_num];
    count_type incast;

    void Insert(const data_type item) {
      uint32_t choice = hash(item, 17) & 1;
      count_type min_num = INT_MAX;
      uint32_t min_pos = -1;

      for (uint32_t i = 0; i < slot_num; ++i) {
        if (counters[i] == 0) {
          items[i] = item;
          counters[i] = -1;
          return;
        } else if (items[i] == item) {
          if (counters[i] < 0)
            // negative means no-error
            counters[i]--;
          else {
            counters[i]++;
            incast += COUNT[choice];
          }
          return;
        }

        count_type counter_val = std::abs(counters[i]);
        if (counter_val < min_num) {
          min_num = counter_val;
          min_pos = i;
        }
      }

      if (incast * COUNT[choice] >= int(min_num * factor)) {
        if (counters[min_pos] < 0) {
          uint32_t min_choice = hash(items[min_pos], 17) & 1;
          incast -= COUNT[min_choice] * counters[min_pos];
        }
        items[min_pos] = item;
        counters[min_pos] = min_num + 1;
      }
      // increase incast after exchange?
      incast += COUNT[choice];
    }

    count_type Query(const data_type item) {
      uint32_t choice = hash(item, 17) & 1;

      for (uint32_t i = 0; i < slot_num; ++i) {
        if (items[i] == item) {
          return std::abs(counters[i]);
        }
      }

      return 0; // incast * COUNT[choice];
    }
  };

  WavingSketch(uint32_t _BUCKET_NUM)
      : Abstract((char *) "WavingSketch"), BUCKET_NUM(_BUCKET_NUM) {
    rename("WavingSketch" + std::to_string(slot_num));
    buckets = new Bucket[BUCKET_NUM];
    memset(buckets, 0, BUCKET_NUM * sizeof(Bucket));
  }
  ~WavingSketch() { delete[] buckets; }

  void Insert(const data_type item) {
    uint32_t bucket_pos = hash(item) % BUCKET_NUM;
    buckets[bucket_pos].Insert(item);
  }

  count_type Query(const data_type item) {
    uint32_t bucket_pos = hash(item) % BUCKET_NUM;
    return buckets[bucket_pos].Query(item);
  }

 private:
  Bucket *buckets;
  const uint32_t BUCKET_NUM;
  friend int32_t compress<slot_num>(WavingSketch<slot_num> *src,
                                    WavingSketch<slot_num> *dst);
  friend int32_t expand<slot_num>(WavingSketch<slot_num> *src,
                                  WavingSketch<slot_num> *dst);
};

// compress WavingSketch from src to dst
// return 1 on success
// return 0 on failure
template<uint32_t slot_num>
int32_t compress(WavingSketch<slot_num> *src, WavingSketch<slot_num> *dst) {
  // src->BUCKET_NUM must be multiples of dst->BUCKET_NUM
  if (src->BUCKET_NUM % dst->BUCKET_NUM) return 0;
  uint32_t ratio = src->BUCKET_NUM / dst->BUCKET_NUM;

  for (uint32_t i = 0; i < dst->BUCKET_NUM; ++i) {
    std::vector<std::pair<data_type, count_type>> items;
    count_type incast = 0;
    for (uint32_t j = 0; j < ratio; ++j) {
      uint32_t index = j * dst->BUCKET_NUM + i;
      for (uint32_t k = 0; k < slot_num; ++k) {
        items.emplace_back(src->buckets[index].items[k],
                           src->buckets[index].counters[k]);
      }
      incast += src->buckets[index].incast;
    }

    // get slot_num-largest flows from items
    auto compare = [](const std::pair<data_type, count_type> &lhs,
                      const std::pair<data_type, count_type> &rhs) -> bool {
#ifdef NO_ERROR_FIRST
      // no-error first
      if (lhs.second < 0 && rhs.second > 0) return true;
      if (lhs.second > 0 && rhs.second < 0) return false;
#endif
      return std::abs(lhs.second) > std::abs(rhs.second);
    };
    std::sort(items.begin(), items.end(), compare);

    // for slot_num-largest flows, store them in dst
    for (uint32_t j = 0; j < slot_num; ++j) {
      dst->buckets[i].items[j] = items[j].first;
      dst->buckets[i].counters[j] = items[j].second;
    }

    // for the rest items, throw them to incast
    for (uint32_t j = slot_num; j < items.size(); ++j) {
      if (items[j].second < 0) {
        uint32_t min_choice = hash(items[j].first, 17) & 1;
        incast -= COUNT[min_choice] * items[j].second;
      }
    }
    dst->buckets[i].incast = incast;
  }
  return 1;
}

// expand WavingSketch from src to dst
// return 1 on success
// return 0 on failure
template<uint32_t slot_num>
int32_t expand(WavingSketch<slot_num> *src, WavingSketch<slot_num> *dst) {
  // dst->BUCKET_NUM must be multiples of src->BUCKET_NUM
  if (dst->BUCKET_NUM % src->BUCKET_NUM) return 0;
  uint32_t ratio = dst->BUCKET_NUM / src->BUCKET_NUM;

  for (uint32_t i = 0; i < src->BUCKET_NUM; ++i) {
    count_type incast = 0;
    for (uint32_t j = 0; j < slot_num; ++j) {
      if (src->buckets[i].counters[j] != 0) {
        uint32_t bucket_pos =
            hash(src->buckets[i].items[j]) % dst->BUCKET_NUM;
        for (uint32_t k = 0; k < slot_num; ++k) {
          if (dst->buckets[bucket_pos].counters[k] == 0) {
            dst->buckets[bucket_pos].items[k] =
                src->buckets[i].items[j];
            dst->buckets[bucket_pos].counters[k] =
                src->buckets[i].counters[j];
            break;
          }
        }
      } else break;
    }

    // do not keep any incast by default
#ifdef COPY_INCAST
    incast = src->buckets[i].incast;
#endif
#ifdef SPLIT_INCAST
    incast = src->buckets[i].incast / static_cast<int32_t>(ratio);
#endif
#ifdef THRESHOLD_INCAST
    uint32_t threshold = UINT32_MAX;
    for (uint32_t j = 0; j < slot_num; ++j) {
      if (src->buckets[i].counters[j] == 0) break;
      if (std::abs(src->buckets[i].counters[j]) < threshold)
        threshold = std::abs(src->buckets[i].counters[j]);
    }
    if (std::abs(src->buckets[i].incast) >= threshold)
      incast = src->buckets[i].incast;
#endif
#ifndef NO_INCAST
    for (uint32_t j = 0; j < ratio; ++j) {
      uint32_t index = j * src->BUCKET_NUM + i;
      dst->buckets[index].incast = incast;
    }
#endif
  }
  return 1;
}

// for now only support 32-bits data_type and 32-bits count_type
class WavingSketchSIMD128 : public Abstract {
 public:
  static constexpr uint32_t
      slot_num = 16 / std::max(sizeof(data_type), sizeof(count_type));
  struct Bucket {
    data_type items[slot_num];
    count_type counters[slot_num];
    // use the last counter as incast

    void Insert(const data_type item) {
      uint32_t choice = hash(item, 17) & 1;
      uint32_t min_pos = -1;

      __m128i reference_item;
      int matched;

      // try to find same item
      reference_item = _mm_set1_epi32(item);
      matched = _mm_cmpeq_epi32_mask(reference_item, *(__m128i *) items);

      if (matched != 0) {
        int matched_index = _tzcnt_u32((uint32_t) matched);
        if (counters[matched_index] < 0)
          // negative means no-error
          counters[matched_index]--;
        else {
          counters[matched_index]++;
          counters[slot_num - 1] += COUNT[choice];
        }
        return;
      }

      // try to find free buckets
      // no need to construct each time
      static const __m128i reference_item_0 = _mm_set1_epi32(0);
      matched = _mm_cmpeq_epi32_mask(reference_item_0, *(__m128i *) counters);

      if (matched != 0 && matched != 8) {
        int matched_index = _tzcnt_u32(static_cast<uint32_t>(matched));
        items[matched_index] = item;
        counters[matched_index] = -1;
        return;
      }

      // get smallest counter's index

      __m128i counter_data = _mm_abs_epi32(*(__m128i *) counters);
      const static __m128i mask = _mm_set_epi32(0x7FFFFFFF, 0, 0, 0);
      counter_data = _mm_or_si128(counter_data, mask);

      __m128i min1 = _mm_shuffle_epi32(counter_data, _MM_SHUFFLE(0, 0, 3, 2));
      __m128i min2 = _mm_min_epi32(counter_data, min1);
      __m128i min3 = _mm_shuffle_epi32(min2, _MM_SHUFFLE(0, 0, 0, 1));
      __m128i min4 = _mm_min_epi32(min2, min3);
      int min_num = _mm_cvtsi128_si32(min4);

      reference_item = _mm_set1_epi32(min_num);
      matched = _mm_cmpeq_epi32_mask(reference_item, counter_data);
      min_pos = _tzcnt_u32(static_cast<uint32_t>(matched));

      /*
      for (uint32_t i = 0; i < slot_num - 1; ++i) {
          if (counters[i] == 0) {
              items[i] = item;
              counters[i] = -1;
              return;
          } else if (items[i] == item) {
              if (counters[i] < 0)
                  // negative means no-error
                  counters[i]--;
              else {
                  counters[i]++;
                  counters[slot_num - 1] += COUNT[choice];
              }
              return;
          }

          count_type counter_val = std::abs(counters[i]);
          if (counter_val < min_num) {
              min_num = counter_val;
              min_pos = i;
          }
      }
      */
      if (counters[slot_num - 1] * COUNT[choice] >= int(min_num * factor)) {
        if (counters[min_pos] < 0) {
          uint32_t min_choice = hash(items[min_pos], 17) & 1;
          counters[slot_num - 1] -= COUNT[min_choice] * counters[min_pos];
        }
        items[min_pos] = item;
        counters[min_pos] = min_num + 1;
      }
      // increase incast after exchange
      counters[slot_num - 1] += COUNT[choice];
    }

    count_type Query(const data_type item) {
      // uint32_t choice = hash(item, 17) & 1;
      __m128i reference_item = _mm_set1_epi32(item);
      int matched = _mm_cmpeq_epi32_mask(reference_item, *(__m128i *) items);

      if (matched != 0) {
        int matched_index = _tzcnt_u32((uint32_t) matched);
        return std::abs(counters[matched_index]);
      }
      /*
      for (uint32_t i = 0; i < slot_num - 1; ++i) {
          if (items[i] == item) {
              return std::abs(counters[i]);
          }
      }
      */
      return 0; // incast * COUNT[choice];
    }
  };

  WavingSketchSIMD128(uint32_t _BUCKET_NUM)
      : Abstract((char *) "WSSIMD4"), BUCKET_NUM(_BUCKET_NUM) {
    buckets = new(std::align_val_t{32}) Bucket[BUCKET_NUM];
    memset(buckets, 0, BUCKET_NUM * sizeof(Bucket));
  }
  ~WavingSketchSIMD128() { delete[] buckets; }

  void Insert(const data_type item) {
    uint32_t bucket_pos = hash(item) % BUCKET_NUM;
    buckets[bucket_pos].Insert(item);
  }

  count_type Query(const data_type item) {
    uint32_t bucket_pos = hash(item) % BUCKET_NUM;
    return buckets[bucket_pos].Query(item);
  }

 private:
  Bucket *buckets;
  const uint32_t BUCKET_NUM;
};

// for now only support 32-bits data_type and 32-bits count_type
class WavingSketchSIMD256 : public Abstract {
 public:
  static constexpr uint32_t
      slot_num = 32 / std::max(sizeof(data_type), sizeof(count_type));
  struct Bucket {
    data_type items[slot_num];
    count_type counters[slot_num];
    // use the last counter as incast

    void Insert(const data_type item) {
      uint32_t choice = hash(item, 17) & 1;
      uint32_t min_pos = -1;

      __m256i reference_item;
      int matched;

      // try to find same item
      reference_item = _mm256_set1_epi32(item);
      // a_comp = _mm256_cmpeq_epi32(reference_item, *(__m256i *)items);
      // matched = _mm256_movemask_ps(_mm256_castsi256_ps(a_comp));
      matched = _mm256_cmpeq_epi32_mask(reference_item, *(__m256i *) items);

      if (matched != 0) {
        int matched_index = _tzcnt_u32((uint32_t) matched);
        if (counters[matched_index] < 0)
          // negative means no-error
          counters[matched_index]--;
        else {
          counters[matched_index]++;
          counters[slot_num - 1] += COUNT[choice];
        }
        return;
      }

      // try to find free buckets
      // no need to construct each time
      static const __m256i reference_item_0 = _mm256_set1_epi32(0);
      // a_comp = _mm256_cmpeq_epi32(reference_item_0, *(__m256i *)counters);
      // matched = _mm256_movemask_ps(_mm256_castsi256_ps(a_comp));
      matched =
          _mm256_cmpeq_epi32_mask(reference_item_0, *(__m256i *) counters);

      if (matched != 0 && matched != 128) {
        int matched_index = _tzcnt_u32(static_cast<uint32_t>(matched));
        items[matched_index] = item;
        counters[matched_index] = -1;
        return;
      }

      // get smallest counter's index

      __m256i counter_data = _mm256_abs_epi32(*(__m256i *) counters);
      const static __m256i
          mask = _mm256_set_epi32(0x7FFFFFFF, 0, 0, 0, 0, 0, 0, 0);
      counter_data = _mm256_or_si256(counter_data, mask);

      __m128i low_part = _mm256_extracti128_si256(counter_data, 0);
      __m128i high_part = _mm256_extracti128_si256(counter_data, 1);

      __m128i min0 = _mm_min_epi32(low_part, high_part);
      __m128i min1 = _mm_shuffle_epi32(min0, _MM_SHUFFLE(0, 0, 3, 2));
      __m128i min2 = _mm_min_epi32(min0, min1);
      __m128i min3 = _mm_shuffle_epi32(min2, _MM_SHUFFLE(0, 0, 0, 1));
      __m128i min4 = _mm_min_epi32(min2, min3);
      int min_num = _mm_cvtsi128_si32(min4);

      reference_item = _mm256_set1_epi32(min_num);
      // a_comp = _mm256_cmpeq_epi32(reference_item, counter_data);
      // matched = _mm256_movemask_ps(_mm256_castsi256_ps(a_comp));
      matched = _mm256_cmpeq_epi32_mask(reference_item, counter_data);
      min_pos = _tzcnt_u32(static_cast<uint32_t>(matched));

      /*
      for (uint32_t i = 0; i < slot_num - 1; ++i) {
          if (counters[i] == 0) {
              items[i] = item;
              counters[i] = -1;
              return;
          } else if (items[i] == item) {
              if (counters[i] < 0)
                  // negative means no-error
                  counters[i]--;
              else {
                  counters[i]++;
                  counters[slot_num - 1] += COUNT[choice];
              }
              return;
          }

          count_type counter_val = std::abs(counters[i]);
          if (counter_val < min_num) {
              min_num = counter_val;
              min_pos = i;
          }
      }
      */
      if (counters[slot_num - 1] * COUNT[choice] >= int(min_num * factor)) {
        if (counters[min_pos] < 0) {
          uint32_t min_choice = hash(items[min_pos], 17) & 1;
          counters[slot_num - 1] -= COUNT[min_choice] * counters[min_pos];
        }
        items[min_pos] = item;
        counters[min_pos] = min_num + 1;
      }
      // increase incast after exchange
      counters[slot_num - 1] += COUNT[choice];
    }

    count_type Query(const data_type item) {
      // uint32_t choice = hash(item, 17) & 1;
      __m256i reference_item = _mm256_set1_epi32(item);
      // __m256i a_comp = _mm256_cmpeq_epi32(reference_item, *(__m256i *)items);
      // int matched = _mm256_movemask_ps(_mm256_castsi256_ps(a_comp));
      int matched = _mm256_cmpeq_epi32_mask(reference_item, *(__m256i *) items);

      if (matched != 0) {
        int matched_index = _tzcnt_u32((uint32_t) matched);
        return std::abs(counters[matched_index]);
      }
      /*
      for (uint32_t i = 0; i < slot_num - 1; ++i) {
          if (items[i] == item) {
              return std::abs(counters[i]);
          }
      }
      */
      return 0; // incast * COUNT[choice];
    }
  };

  WavingSketchSIMD256(uint32_t _BUCKET_NUM)
      : Abstract((char *) "WSSIMD8"), BUCKET_NUM(_BUCKET_NUM) {
    buckets = new(std::align_val_t{64}) Bucket[BUCKET_NUM];
    memset(buckets, 0, BUCKET_NUM * sizeof(Bucket));
  }
  ~WavingSketchSIMD256() { delete[] buckets; }

  void Insert(const data_type item) {
    uint32_t bucket_pos = hash(item) % BUCKET_NUM;
    buckets[bucket_pos].Insert(item);
  }

  count_type Query(const data_type item) {
    uint32_t bucket_pos = hash(item) % BUCKET_NUM;
    return buckets[bucket_pos].Query(item);
  }

 private:
  Bucket *buckets;
  const uint32_t BUCKET_NUM;
};

// for now only support 32-bits data_type and 32-bits count_type
class WavingSketchSIMD512 : public Abstract {
 public:
  static constexpr uint32_t
      slot_num = 64 / std::max(sizeof(data_type), sizeof(count_type));
  struct Bucket {
    data_type items[slot_num];
    count_type counters[slot_num];
    // use the last counter as incast

    void Insert(const data_type item) {
      uint32_t choice = hash(item, 17) & 1;
      uint32_t min_pos = -1;

      __m256i reference_item;
      int matched;

      // try to find same item
      reference_item = _mm256_set1_epi32(item);
      // a_comp = _mm256_cmpeq_epi32(reference_item, *(__m256i *)items);
      // matched = _mm256_movemask_ps(_mm256_castsi256_ps(a_comp));
      matched = _mm256_cmpeq_epi32_mask(reference_item, *(__m256i *) items);

      if (matched != 0) {
        int matched_index = _tzcnt_u32((uint32_t) matched);
        if (counters[matched_index] < 0)
          // negative means no-error
          counters[matched_index]--;
        else {
          counters[matched_index]++;
          counters[slot_num - 1] += COUNT[choice];
        }
        return;
      }

      // a_comp = _mm256_cmpeq_epi32(reference_item, *(__m256i *)(items + 8));
      // matched = _mm256_movemask_ps(_mm256_castsi256_ps(a_comp));
      matched =
          _mm256_cmpeq_epi32_mask(reference_item, *(__m256i *) (items + 8));

      if (matched != 0) {
        int matched_index = _tzcnt_u32((uint32_t) matched) + 8;
        if (counters[matched_index] < 0)
          // negative means no-error
          counters[matched_index]--;
        else {
          counters[matched_index]++;
          counters[slot_num - 1] += COUNT[choice];
        }
        return;
      }

      // try to find free buckets
      // no need to construct each time
      static const __m256i reference_item_0 = _mm256_set1_epi32(0);
      // a_comp = _mm256_cmpeq_epi32(reference_item_0, *(__m256i *)counters);
      // matched = _mm256_movemask_ps(_mm256_castsi256_ps(a_comp));
      matched =
          _mm256_cmpeq_epi32_mask(reference_item_0, *(__m256i *) counters);

      if (matched != 0) {
        int matched_index = _tzcnt_u32(static_cast<uint32_t>(matched));
        items[matched_index] = item;
        counters[matched_index] = -1;
        return;
      }

      // a_comp = _mm256_cmpeq_epi32(reference_item_0, *(__m256i *)(counters + 8));
      // matched = _mm256_movemask_ps(_mm256_castsi256_ps(a_comp));
      matched = _mm256_cmpeq_epi32_mask(reference_item_0,
                                        *(__m256i *) (counters + 8));

      if (matched != 0 && matched != 128) {
        int matched_index = _tzcnt_u32(static_cast<uint32_t>(matched)) + 8;
        items[matched_index] = item;
        counters[matched_index] = -1;
        return;
      }

      // get smallest counter's index

      __m256i low_256 = _mm256_abs_epi32(*(__m256i *) counters);
      __m256i high_256 = _mm256_abs_epi32(*(__m256i *) (counters + 8));
      const static __m256i
          mask = _mm256_set_epi32(0x7FFFFFFF, 0, 0, 0, 0, 0, 0, 0);
      high_256 = _mm256_or_si256(high_256, mask);

      __m256i min_256 = _mm256_min_epi32(low_256, high_256);

      __m128i low_part = _mm256_extracti128_si256(min_256, 0);
      __m128i high_part = _mm256_extracti128_si256(min_256, 1);

      __m128i min0 = _mm_min_epi32(low_part, high_part);
      __m128i min1 = _mm_shuffle_epi32(min0, _MM_SHUFFLE(0, 0, 3, 2));
      __m128i min2 = _mm_min_epi32(min0, min1);
      __m128i min3 = _mm_shuffle_epi32(min2, _MM_SHUFFLE(0, 0, 0, 1));
      __m128i min4 = _mm_min_epi32(min2, min3);
      int min_num = _mm_cvtsi128_si32(min4);

      reference_item = _mm256_set1_epi32(min_num);
      // a_comp = _mm256_cmpeq_epi32(reference_item, low_256);
      // matched = _mm256_movemask_ps(_mm256_castsi256_ps(a_comp));
      matched = _mm256_cmpeq_epi32_mask(reference_item, low_256);
      if (matched != 0) {
        min_pos = _tzcnt_u32(static_cast<uint32_t>(matched));
      } else {
        // a_comp = _mm256_cmpeq_epi32(reference_item, high_256);
        // matched = _mm256_movemask_ps(_mm256_castsi256_ps(a_comp));
        matched = _mm256_cmpeq_epi32_mask(reference_item, high_256);
        min_pos = _tzcnt_u32(static_cast<uint32_t>(matched)) + 8;
      }
/*
        for (uint32_t i = 0; i < slot_num - 1; ++i) {
            if (counters[i] == 0) {
                items[i] = item;
                counters[i] = -1;
                return;
            } else if (items[i] == item) {
                if (counters[i] < 0)
                    // negative means no-error
                    counters[i]--;
                else {
                    counters[i]++;
                    counters[slot_num - 1] += COUNT[choice];
                }
                return;
            }

            count_type counter_val = std::abs(counters[i]);
            if (counter_val < min_num) {
                min_num = counter_val;
                min_pos = i;
            }
        }
*/
      if (counters[slot_num - 1] * COUNT[choice] >= int(min_num * factor)) {
        if (counters[min_pos] < 0) {
          uint32_t min_choice = hash(items[min_pos], 17) & 1;
          counters[slot_num - 1] -= COUNT[min_choice] * counters[min_pos];
        }
        items[min_pos] = item;
        counters[min_pos] = min_num + 1;
      }
      // increase incast after exchange
      counters[slot_num - 1] += COUNT[choice];
    }

    count_type Query(const data_type item) {
      // uint32_t choice = hash(item, 17) & 1;

      __m256i reference_item;
      int matched;

      reference_item = _mm256_set1_epi32(item);
      // a_comp = _mm256_cmpeq_epi32(reference_item, *(__m256i *)items);
      // matched = _mm256_movemask_ps(_mm256_castsi256_ps(a_comp));
      matched = _mm256_cmpeq_epi32_mask(reference_item, *(__m256i *) items);

      if (matched != 0) {
        int matched_index = _tzcnt_u32((uint32_t) matched);
        return std::abs(counters[matched_index]);
      }

      // a_comp = _mm256_cmpeq_epi32(reference_item, *(__m256i *)(items + 8));
      // matched = _mm256_movemask_ps(_mm256_castsi256_ps(a_comp));
      matched =
          _mm256_cmpeq_epi32_mask(reference_item, *(__m256i *) (items + 8));

      if (matched != 0) {
        int matched_index = _tzcnt_u32((uint32_t) matched) + 8;
        return std::abs(counters[matched_index]);
      }
      /*
      for (uint32_t i = 0; i < slot_num - 1; ++i) {
          if (items[i] == item) {
              return std::abs(counters[i]);
          }
      }
        */
      return 0; // incast * COUNT[choice];
    }
  };

  WavingSketchSIMD512(uint32_t _BUCKET_NUM)
      : Abstract((char *) "WSSIMD16"), BUCKET_NUM(_BUCKET_NUM) {
    buckets = new(std::align_val_t{128}) Bucket[BUCKET_NUM];
    memset(buckets, 0, BUCKET_NUM * sizeof(Bucket));
  }
  ~WavingSketchSIMD512() { delete[] buckets; }

  void Insert(const data_type item) {
    uint32_t bucket_pos = hash(item) % BUCKET_NUM;
    buckets[bucket_pos].Insert(item);
  }

  count_type Query(const data_type item) {
    uint32_t bucket_pos = hash(item) % BUCKET_NUM;
    return buckets[bucket_pos].Query(item);
  }

 private:
  Bucket *buckets;
  const uint32_t BUCKET_NUM;
};

// for now only support 32-bits data_type and 32-bits count_type
class WavingSketchSIMD1024 : public Abstract {
 public:
  static constexpr uint32_t
      slot_num = 128 / std::max(sizeof(data_type), sizeof(count_type));
  struct Bucket {
    data_type items[slot_num];
    count_type counters[slot_num];
    // use the last counter as incast

    void Insert(const data_type item) {
      uint32_t choice = hash(item, 17) & 1;
      uint32_t min_pos = -1;

      __m256i reference_item;
      int matched;

      // try to find same item
      reference_item = _mm256_set1_epi32(item);
      matched = _mm256_cmpeq_epi32_mask(reference_item, *(__m256i *) items);

      if (matched != 0) {
        int matched_index = _tzcnt_u32((uint32_t) matched);
        if (counters[matched_index] < 0)
          // negative means no-error
          counters[matched_index]--;
        else {
          counters[matched_index]++;
          counters[slot_num - 1] += COUNT[choice];
        }
        return;
      }

      matched =
          _mm256_cmpeq_epi32_mask(reference_item, *(__m256i *) (items + 8));

      if (matched != 0) {
        int matched_index = _tzcnt_u32((uint32_t) matched) + 8;
        if (counters[matched_index] < 0)
          // negative means no-error
          counters[matched_index]--;
        else {
          counters[matched_index]++;
          counters[slot_num - 1] += COUNT[choice];
        }
        return;
      }

      matched =
          _mm256_cmpeq_epi32_mask(reference_item, *(__m256i *) (items + 16));

      if (matched != 0) {
        int matched_index = _tzcnt_u32((uint32_t) matched) + 16;
        if (counters[matched_index] < 0)
          // negative means no-error
          counters[matched_index]--;
        else {
          counters[matched_index]++;
          counters[slot_num - 1] += COUNT[choice];
        }
        return;
      }

      matched =
          _mm256_cmpeq_epi32_mask(reference_item, *(__m256i *) (items + 24));

      if (matched != 0) {
        int matched_index = _tzcnt_u32((uint32_t) matched) + 24;
        if (counters[matched_index] < 0)
          // negative means no-error
          counters[matched_index]--;
        else {
          counters[matched_index]++;
          counters[slot_num - 1] += COUNT[choice];
        }
        return;
      }

      // try to find free buckets
      // no need to construct each time
      static const __m256i reference_item_0 = _mm256_set1_epi32(0);
      matched =
          _mm256_cmpeq_epi32_mask(reference_item_0, *(__m256i *) counters);

      if (matched != 0) {
        int matched_index = _tzcnt_u32(static_cast<uint32_t>(matched));
        items[matched_index] = item;
        counters[matched_index] = -1;
        return;
      }

      matched = _mm256_cmpeq_epi32_mask(reference_item_0,
                                        *(__m256i *) (counters + 8));

      if (matched != 0) {
        int matched_index = _tzcnt_u32(static_cast<uint32_t>(matched)) + 8;
        items[matched_index] = item;
        counters[matched_index] = -1;
        return;
      }

      matched = _mm256_cmpeq_epi32_mask(reference_item_0,
                                        *(__m256i *) (counters + 16));

      if (matched != 0) {
        int matched_index = _tzcnt_u32(static_cast<uint32_t>(matched)) + 16;
        items[matched_index] = item;
        counters[matched_index] = -1;
        return;
      }

      matched = _mm256_cmpeq_epi32_mask(reference_item_0,
                                        *(__m256i *) (counters + 24));

      if (matched != 0 && matched != 128) {
        int matched_index = _tzcnt_u32(static_cast<uint32_t>(matched)) + 24;
        items[matched_index] = item;
        counters[matched_index] = -1;
        return;
      }

      // get smallest counter's index

      __m256i a_256 = _mm256_abs_epi32(*(__m256i *) counters);
      __m256i b_256 = _mm256_abs_epi32(*(__m256i *) (counters + 8));
      __m256i c_256 = _mm256_abs_epi32(*(__m256i *) (counters + 16));
      __m256i d_256 = _mm256_abs_epi32(*(__m256i *) (counters + 24));
      const static __m256i
          mask = _mm256_set_epi32(0x7FFFFFFF, 0, 0, 0, 0, 0, 0, 0);
      d_256 = _mm256_or_si256(d_256, mask);

      __m256i min_ab = _mm256_min_epi32(a_256, b_256);
      __m256i min_cd = _mm256_min_epi32(c_256, d_256);

      __m256i min_256 = _mm256_min_epi32(min_ab, min_cd);

      __m128i low_part = _mm256_extracti128_si256(min_256, 0);
      __m128i high_part = _mm256_extracti128_si256(min_256, 1);

      __m128i min0 = _mm_min_epi32(low_part, high_part);
      __m128i min1 = _mm_shuffle_epi32(min0, _MM_SHUFFLE(0, 0, 3, 2));
      __m128i min2 = _mm_min_epi32(min0, min1);
      __m128i min3 = _mm_shuffle_epi32(min2, _MM_SHUFFLE(0, 0, 0, 1));
      __m128i min4 = _mm_min_epi32(min2, min3);
      int min_num = _mm_cvtsi128_si32(min4);

      reference_item = _mm256_set1_epi32(min_num);
      matched = _mm256_cmpeq_epi32_mask(reference_item, a_256);
      if (matched != 0) {
        min_pos = _tzcnt_u32(static_cast<uint32_t>(matched));
      } else {
        matched = _mm256_cmpeq_epi32_mask(reference_item, b_256);
        if (matched != 0) {
          min_pos = _tzcnt_u32(static_cast<uint32_t>(matched)) + 8;
        } else {
          matched = _mm256_cmpeq_epi32_mask(reference_item, c_256);
          if (matched != 0) {
            min_pos = _tzcnt_u32(static_cast<uint32_t>(matched)) + 16;
          } else {
            matched = _mm256_cmpeq_epi32_mask(reference_item, d_256);
            min_pos = _tzcnt_u32(static_cast<uint32_t>(matched)) + 24;
          }
        }
      }
/*
        for (uint32_t i = 0; i < slot_num - 1; ++i) {
            if (counters[i] == 0) {
                items[i] = item;
                counters[i] = -1;
                return;
            } else if (items[i] == item) {
                if (counters[i] < 0)
                    // negative means no-error
                    counters[i]--;
                else {
                    counters[i]++;
                    counters[slot_num - 1] += COUNT[choice];
                }
                return;
            }

            count_type counter_val = std::abs(counters[i]);
            if (counter_val < min_num) {
                min_num = counter_val;
                min_pos = i;
            }
        }
*/
      if (counters[slot_num - 1] * COUNT[choice] >= int(min_num * factor)) {
        if (counters[min_pos] < 0) {
          uint32_t min_choice = hash(items[min_pos], 17) & 1;
          counters[slot_num - 1] -= COUNT[min_choice] * counters[min_pos];
        }
        items[min_pos] = item;
        counters[min_pos] = min_num + 1;
      }
      // increase incast after exchange
      counters[slot_num - 1] += COUNT[choice];
    }

    count_type Query(const data_type item) {
      // uint32_t choice = hash(item, 17) & 1;

      __m256i reference_item;
      int matched;

      reference_item = _mm256_set1_epi32(item);
      matched = _mm256_cmpeq_epi32_mask(reference_item, *(__m256i *) items);

      if (matched != 0) {
        int matched_index = _tzcnt_u32((uint32_t) matched);
        return std::abs(counters[matched_index]);
      }

      matched =
          _mm256_cmpeq_epi32_mask(reference_item, *(__m256i *) (items + 8));

      if (matched != 0) {
        int matched_index = _tzcnt_u32((uint32_t) matched) + 8;
        return std::abs(counters[matched_index]);
      }

      matched =
          _mm256_cmpeq_epi32_mask(reference_item, *(__m256i *) (items + 16));

      if (matched != 0) {
        int matched_index = _tzcnt_u32((uint32_t) matched) + 16;
        return std::abs(counters[matched_index]);
      }

      matched =
          _mm256_cmpeq_epi32_mask(reference_item, *(__m256i *) (items + 24));

      if (matched != 0) {
        int matched_index = _tzcnt_u32((uint32_t) matched) + 24;
        return std::abs(counters[matched_index]);
      }
      /*
      for (uint32_t i = 0; i < slot_num - 1; ++i) {
          if (items[i] == item) {
              return std::abs(counters[i]);
          }
      }
        */
      return 0; // incast * COUNT[choice];
    }
  };

  WavingSketchSIMD1024(uint32_t _BUCKET_NUM)
      : Abstract((char *) "WSSIMD32"), BUCKET_NUM(_BUCKET_NUM) {
    buckets = new(std::align_val_t{256}) Bucket[BUCKET_NUM];
    memset(buckets, 0, BUCKET_NUM * sizeof(Bucket));
  }
  ~WavingSketchSIMD1024() { delete[] buckets; }

  void Insert(const data_type item) {
    uint32_t bucket_pos = hash(item) % BUCKET_NUM;
    buckets[bucket_pos].Insert(item);
  }

  count_type Query(const data_type item) {
    uint32_t bucket_pos = hash(item) % BUCKET_NUM;
    return buckets[bucket_pos].Query(item);
  }

 private:
  Bucket *buckets;
  const uint32_t BUCKET_NUM;
};

template<uint32_t slot_num, uint32_t counter_num>
class WavingSketchMC : public Abstract {
 public:
  struct node {
    count_type counter;
    data_type item;
    node() : counter(0), item(0) {};
    bool operator<(const node &tp) const {
      if (counter < 0) // true
      {
        if (tp.counter < 0)
          return std::abs(counter) > std::abs(tp.counter);
        else
          return 1;
      } else {
        if (tp.counter < 0)
          return 0;
        else
          return std::abs(counter) > std::abs(tp.counter);
      }
    }
  };

  struct Bucket {
    data_type items[slot_num];
    count_type counters[slot_num];
    int16_t incast[counter_num];

    void Insert(const data_type item, uint32_t seed_s, uint32_t seed_incast) {
      uint32_t choice = hash(item, seed_s) & 1;
      uint32_t whichcast = hash(item, seed_incast) % counter_num;
      count_type min_num = INT_MAX;
      uint32_t min_pos = -1;

      for (uint32_t i = 0; i < slot_num; ++i) {
        if (counters[i] == 0) {
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
          uint32_t min_choice = hash(items[min_pos], seed_s) & 1;
          incast[hash(items[min_pos], seed_incast) % counter_num] -=
              COUNT[min_choice] * counters[min_pos];
        }
        items[min_pos] = item;
        counters[min_pos] = min_num + 1;
      }
      incast[whichcast] += COUNT[choice];
    }

    count_type Query(const data_type item,
                     uint32_t seed_s,
                     uint32_t seed_incast) {
      for (uint32_t i = 0; i < slot_num; ++i) {
        if (items[i] == item) {
          return std::abs(counters[i]);
        }
      }
      return 0;
    }
  };

  WavingSketchMC(uint32_t _BUCKET_NUM)
      : Abstract((char *) "WavingSketch"), BUCKET_NUM(_BUCKET_NUM) {
    buckets = new Bucket[BUCKET_NUM];
    memset(buckets, 0, BUCKET_NUM * sizeof(Bucket));
    rename("WavingSketchMC<" + std::to_string(slot_num) + ", "
               + std::to_string(counter_num) + ">");
    // seed_choice = std::clock();
    // sleep(1);
    // seed_incast = std::clock();
    // sleep(1);
    // seed_s = std::clock();
    // fix seed for predictable result
    seed_choice = 123;
    seed_incast = 456;
    seed_s = 789;
  }
  ~WavingSketchMC() { delete[] buckets; }

  void Insert(const data_type item) {
    uint32_t bucket_pos = hash(item, seed_choice) % BUCKET_NUM;
    buckets[bucket_pos].Insert(item, seed_s, seed_incast);
  }

  void copybuck(Bucket *src, Bucket *dest) {
    for (int i = 0; i < counter_num; i++)
      dest->incast[i] = src->incast[i];
    for (int i = 0; i < slot_num; i++) {
      dest->items[i] = src->items[i];
      dest->counters[i] = src->counters[i];
    }
  }

  void mergebuck(Bucket *src1, Bucket *src2, Bucket *dest, bool debug) {
    for (int i = 0; i < counter_num; i++)
      dest->incast[i] = src1->incast[i] + src2->incast[i];
    node *tp1 = new node[slot_num << 1];
    node *p = tp1;
    for (uint32_t i = 0; i < slot_num; i++) {
      p->item = src1->items[i];
      p->counter = src1->counters[i];
      p++;
    }
    for (uint32_t i = 0; i < slot_num; i++) {
      p->item = src2->items[i];
      p->counter = src2->counters[i];
      p++;
    }
    std::sort(tp1, p);

    bool output;
    if (debug) {
      // std::cout << "output? ";
      // std::cin >> output;
      // if (output)
      // {
      // 	for (uint32_t i = 0; i < 2 * slot_num;i++)
      // 	{
      // 		std::cout << "flag: " << int(tp1[i].flag) << ", counter: " << tp1[i].counter << ", item:" << tp1[i].item << std::endl;
      // 	}
      // }
    }

    uint32_t i = 0;
    for (i = 0; i < slot_num; i++) {
      dest->items[i] = tp1[i].item;
      dest->counters[i] = tp1[i].counter;
    }
    uint32_t end = slot_num << 1;
    while (i < end && tp1[i].counter < 0) // flag=true
    {
      uint32_t min_choice = hash(tp1[i].item, seed_s) & 1;
      uint32_t whichcast = hash(tp1[i].item, seed_incast) % counter_num;
      dest->incast[whichcast] -= COUNT[min_choice] * tp1[i].counter;
      i++;
    }
    delete[] tp1;
    if (debug) {
      std::cout << "output? ";
      std::cin >> output;
      if (output) {
        for (uint32_t i = 0; i < slot_num; i++) {
          std::cout << "counter: " << dest->counters[i] << ", item: "
                    << dest->items[i] << std::endl;
        }
      }
    }
  }

  void shrink(bool debug) {
    // shrink to half of total buckets
    if (BUCKET_NUM % 2) {
      printf("Bucket num %d is odd, can NOT shrink!\n", BUCKET_NUM);
      printf("FATAL ERROR!\n");
      exit(-1);
    }
    uint32_t num = (BUCKET_NUM + 1) >> 1;
    Bucket *tpbuck = new Bucket[num];
    for (uint32_t i = 0; i < num; i++) {
      if (i + num < BUCKET_NUM)
        mergebuck(&buckets[i], &buckets[i + num], &tpbuck[i], debug);
      else
        copybuck(&buckets[i], &tpbuck[i]);
    }
    BUCKET_NUM = num;
    delete[] buckets;
    buckets = tpbuck;
  }

  count_type Query(const data_type item) {
    uint32_t bucket_pos = hash(item, seed_choice) % BUCKET_NUM;
    return buckets[bucket_pos].Query(item, seed_s, seed_incast);
  }

  void expand() {
    uint32_t num = BUCKET_NUM << 1;
    Bucket *tpbuck = new Bucket[num];
    for (uint32_t i = 0; i < BUCKET_NUM; i++) {
      copybuck(&buckets[i], &tpbuck[i]);
      copybuck(&buckets[i], &tpbuck[BUCKET_NUM + i]);
    }
    for (uint32_t i = 0; i < num; i++) {
      for (uint32_t j = 0; j < slot_num; j++) {
        if (hash(tpbuck[i].items[j], seed_choice) % num != i) {
          if (tpbuck[i].counters[j] > 0) // flag=False
          {
            tpbuck[i].incast[hash(tpbuck[i].items[j], seed_incast)
                % counter_num] -= COUNT[hash(tpbuck[i].items[j], seed_s) & 1]
                * tpbuck[i].counters[j];
          }
          tpbuck[i].counters[j] = 0;
          tpbuck[i].items[j] = 0;
        }
      }
      for (uint32_t j = 0; j < slot_num; j++) {
        if (tpbuck[i].counters[j] == 0) {
          uint32_t k = j + 1;
          for (; k < slot_num && tpbuck[i].counters[k] == 0; k++);
          if (k == slot_num)
            break;
          tpbuck[i].counters[j] = tpbuck[i].counters[k];
          tpbuck[i].items[j] = tpbuck[i].items[k];
          tpbuck[i].counters[k] = 0;
        }
      }
      for (int j = 0; j < counter_num; j++)
        tpbuck[i].incast[j] = tpbuck[i].incast[j] >> 1;
    }
    BUCKET_NUM = num;
    delete[] buckets;
    buckets = tpbuck;
  }

  void printcounters(int HIT) {
    uint32_t cnt = 0;
    uint32_t heavynum = 0;
    for (uint32_t i = 0; i < BUCKET_NUM; i++) {
      for (uint32_t j = 0; j < slot_num; j++) {
        if (buckets[i].counters[j] < 0) {
          cnt++;
        }
        if (std::abs(buckets[i].counters[j]) > HIT) {
          heavynum++;
        }
      }
    }
    std::cout << "# total buckets: " << BUCKET_NUM << std::endl;
    std::cout << "# top-2000 elements: " << heavynum << std::endl;
    std::cout << "# elements with flag ture: " << cnt << std::endl;
    std::cout << "# total elements: " << BUCKET_NUM * slot_num << std::endl;

  }

 private:
  Bucket *buckets;
  uint32_t BUCKET_NUM;
  uint32_t seed_choice;
  uint32_t seed_s;
  uint32_t seed_incast;
  friend int32_t compressMC<slot_num, counter_num>(WavingSketchMC<slot_num,
                                                                  counter_num> *src,
                                                   WavingSketchMC<slot_num,
                                                                  counter_num> *dst);
  friend int32_t expandMC<slot_num, counter_num>(WavingSketchMC<slot_num,
                                                                counter_num> *src,
                                                 WavingSketchMC<slot_num,
                                                                counter_num> *dst);
};

// compress WavingSketchMC from src to dst
// return 1 on success
// return 0 on failure
template<uint32_t slot_num, uint32_t counter_num>
int32_t compressMC(WavingSketchMC<slot_num, counter_num> *src,
                   WavingSketchMC<slot_num, counter_num> *dst) {
  // src->BUCKET_NUM must be multiples of dst->BUCKET_NUM
  if (src->BUCKET_NUM % dst->BUCKET_NUM) return 0;
  uint32_t ratio = src->BUCKET_NUM / dst->BUCKET_NUM;

  // copy hash args
  dst->seed_choice = src->seed_choice;
  dst->seed_s = src->seed_s;
  dst->seed_incast = src->seed_incast;

  for (uint32_t i = 0; i < dst->BUCKET_NUM; ++i) {
    std::vector<std::pair<data_type, count_type>> items;
    count_type incast[counter_num] = {};
    for (uint32_t j = 0; j < ratio; ++j) {
      uint32_t index = j * dst->BUCKET_NUM + i;
      for (uint32_t k = 0; k < slot_num; ++k) {
        items.emplace_back(src->buckets[index].items[k],
                           src->buckets[index].counters[k]);
      }
      for (int k = 0; k < counter_num; ++k) {
        incast[k] += src->buckets[index].incast[k];
      }
    }

    // get slot_num-largest flows from items
    auto compare = [](const std::pair<data_type, count_type> &lhs,
                      const std::pair<data_type, count_type> &rhs) -> bool {
      // no-error first
      if (lhs.second < 0 && rhs.second > 0) return true;
      if (lhs.second > 0 && rhs.second < 0) return false;
      return std::abs(lhs.second) > std::abs(rhs.second);
    };
    std::sort(items.begin(), items.end(), compare);

    // for slot_num-largest flows, store them in dst
    for (uint32_t j = 0; j < slot_num; ++j) {
      dst->buckets[i].items[j] = items[j].first;
      dst->buckets[i].counters[j] = items[j].second;
    }

    // for the rest items, throw them to incast
    for (uint32_t j = slot_num; j < items.size(); ++j) {
      if (items[j].second < 0) {

        uint32_t min_choice = hash(items[j].first, src->seed_s) & 1;
        uint32_t
            whichcast = hash(items[j].first, src->seed_incast) % counter_num;
        incast[whichcast] -= COUNT[min_choice] * items[j].second;
      }
    }
    for (int j = 0; j < counter_num; ++j) {
      dst->buckets[i].incast[j] = incast[j];
    }
  }
  return 1;
}

// expand WavingSketchMC from src to dst
// return 1 on success
// return 0 on failure
template<uint32_t slot_num, uint32_t counter_num>
int32_t expandMC(WavingSketchMC<slot_num, counter_num> *src,
                 WavingSketchMC<slot_num, counter_num> *dst) {
  // dst->BUCKET_NUM must be multiples of src->BUCKET_NUM
  if (dst->BUCKET_NUM % src->BUCKET_NUM) return 0;
  uint32_t ratio = dst->BUCKET_NUM / src->BUCKET_NUM;

  // copy hash args
  dst->seed_choice = src->seed_choice;
  dst->seed_s = src->seed_s;
  dst->seed_incast = src->seed_incast;

  // copy heavy part
  for (uint32_t i = 0; i < src->BUCKET_NUM; ++i) {
    for (uint32_t j = 0; j < slot_num; ++j) {
      if (src->buckets[i].counters[j] != 0) {
        uint32_t bucket_pos =
            hash(src->buckets[i].items[j], src->seed_choice) % dst->BUCKET_NUM;
        for (uint32_t k = 0; k < slot_num; ++k) {
          if (dst->buckets[bucket_pos].counters[k] == 0) {
            dst->buckets[bucket_pos].items[k] =
                src->buckets[i].items[j];
            dst->buckets[bucket_pos].counters[k] =
                src->buckets[i].counters[j];
            break;
          }
        }
      } else break;
    }

    // copy incast
    for (uint32_t j = 0; j < ratio; ++j) {
      uint32_t index = j * src->BUCKET_NUM + i;
      for (int k = 0; k < counter_num; ++k) {
        dst->buckets[index].incast[k] = src->buckets[i].incast[k];
      }
    }
  }

  return 1;
}

class WSSIMDMC128 : public Abstract {
 public:
  static constexpr uint32_t slot_num = 4;
  static constexpr uint32_t counter_num = 16;

  struct Bucket {
    data_type items[slot_num];
    count_type counters[slot_num];
    int16_t incast[counter_num];

    void Insert(const data_type item, uint32_t seed_s, uint32_t seed_incast) {
      uint32_t choice = hash(item, seed_s) & 1;
      uint32_t whichcast = hash(item, seed_incast) % counter_num;
      uint32_t min_pos = -1;

      __m128i reference_item;
      int matched;

      // try to find same item
      reference_item = _mm_set1_epi32(item);
      matched = _mm_cmpeq_epi32_mask(reference_item, *(__m128i *) items);

      if (matched != 0) {
        int matched_index = _tzcnt_u32((uint32_t) matched);
        if (counters[matched_index] < 0)
          // negative means no-error
          counters[matched_index]--;
        else {
          counters[matched_index]++;
          incast[whichcast] += COUNT[choice];
        }
        return;
      }

      // try to find free buckets
      // no need to construct each time
      static const __m128i reference_item_0 = _mm_set1_epi32(0);
      matched = _mm_cmpeq_epi32_mask(reference_item_0, *(__m128i *) counters);

      if (matched != 0) {
        int matched_index = _tzcnt_u32(static_cast<uint32_t>(matched));
        items[matched_index] = item;
        counters[matched_index] = -1;
        return;
      }

      // get smallest counter's index

      __m128i counter_data = _mm_abs_epi32(*(__m128i *) counters);

      __m128i min1 = _mm_shuffle_epi32(counter_data, _MM_SHUFFLE(0, 0, 3, 2));
      __m128i min2 = _mm_min_epi32(counter_data, min1);
      __m128i min3 = _mm_shuffle_epi32(min2, _MM_SHUFFLE(0, 0, 0, 1));
      __m128i min4 = _mm_min_epi32(min2, min3);
      int min_num = _mm_cvtsi128_si32(min4);

      reference_item = _mm_set1_epi32(min_num);
      matched = _mm_cmpeq_epi32_mask(reference_item, counter_data);
      min_pos = _tzcnt_u32(static_cast<uint32_t>(matched));

      if (incast[whichcast] * COUNT[choice] >= int(min_num * factor)) {
        if (counters[min_pos] < 0) {
          uint32_t min_choice = hash(items[min_pos], seed_s) & 1;
          incast[hash(items[min_pos], seed_incast) % counter_num] -=
              COUNT[min_choice] * counters[min_pos];
        }
        items[min_pos] = item;
        counters[min_pos] = min_num + 1;
      }
      incast[whichcast] += COUNT[choice];
    }

    count_type Query(const data_type item,
                     uint32_t seed_s,
                     uint32_t seed_incast) {
      __m128i reference_item = _mm_set1_epi32(item);
      int matched = _mm_cmpeq_epi32_mask(reference_item, *(__m128i *) items);

      if (matched != 0) {
        int matched_index = _tzcnt_u32((uint32_t) matched);
        return std::abs(counters[matched_index]);
      }
      return 0;
    }
  };

  WSSIMDMC128(uint32_t _BUCKET_NUM)
      : Abstract((char *) "WavingSketch"), BUCKET_NUM(_BUCKET_NUM) {
    buckets = new(std::align_val_t{32}) Bucket[BUCKET_NUM];
    memset(buckets, 0, BUCKET_NUM * sizeof(Bucket));
    rename("WSSIMDMC<4, 16>");
    // seed_choice = std::clock();
    // sleep(1);
    // seed_incast = std::clock();
    // sleep(1);
    // seed_s = std::clock();
    // fix seed for predictable result
    seed_choice = 123;
    seed_incast = 456;
    seed_s = 789;
  }
  ~WSSIMDMC128() { delete[] buckets; }

  void Insert(const data_type item) {
    uint32_t bucket_pos = hash(item, seed_choice) % BUCKET_NUM;
    buckets[bucket_pos].Insert(item, seed_s, seed_incast);
  }

  count_type Query(const data_type item) {
    uint32_t bucket_pos = hash(item, seed_choice) % BUCKET_NUM;
    return buckets[bucket_pos].Query(item, seed_s, seed_incast);
  }

 private:
  Bucket *buckets;
  uint32_t BUCKET_NUM;
  uint32_t seed_choice;
  uint32_t seed_s;
  uint32_t seed_incast;
};

class WSSIMDMC256 : public Abstract {
 public:
  static constexpr uint32_t slot_num = 8;
  static constexpr uint32_t counter_num = 16;

  struct Bucket {
    data_type items[slot_num];
    count_type counters[slot_num];
    int16_t incast[counter_num];

    void Insert(const data_type item, uint32_t seed_s, uint32_t seed_incast) {
      uint32_t choice = hash(item, seed_s) & 1;
      uint32_t whichcast = hash(item, seed_incast) % counter_num;
      uint32_t min_pos = -1;

      __m256i reference_item;
      int matched;

      // try to find same item
      reference_item = _mm256_set1_epi32(item);
      // a_comp = _mm256_cmpeq_epi32(reference_item, *(__m256i *)items);
      // matched = _mm256_movemask_ps(_mm256_castsi256_ps(a_comp));
      matched = _mm256_cmpeq_epi32_mask(reference_item, *(__m256i *) items);

      if (matched != 0) {
        int matched_index = _tzcnt_u32((uint32_t) matched);
        if (counters[matched_index] < 0)
          // negative means no-error
          counters[matched_index]--;
        else {
          counters[matched_index]++;
          incast[whichcast] += COUNT[choice];
        }
        return;
      }

      // try to find free buckets
      // no need to construct each time
      static const __m256i reference_item_0 = _mm256_set1_epi32(0);
      matched =
          _mm256_cmpeq_epi32_mask(reference_item_0, *(__m256i *) counters);

      if (matched != 0) {
        int matched_index = _tzcnt_u32(static_cast<uint32_t>(matched));
        items[matched_index] = item;
        counters[matched_index] = -1;
        return;
      }

      // get smallest counter's index

      __m256i counter_data = _mm256_abs_epi32(*(__m256i *) counters);

      __m128i low_part = _mm256_extracti128_si256(counter_data, 0);
      __m128i high_part = _mm256_extracti128_si256(counter_data, 1);

      __m128i min0 = _mm_min_epi32(low_part, high_part);
      __m128i min1 = _mm_shuffle_epi32(min0, _MM_SHUFFLE(0, 0, 3, 2));
      __m128i min2 = _mm_min_epi32(min0, min1);
      __m128i min3 = _mm_shuffle_epi32(min2, _MM_SHUFFLE(0, 0, 0, 1));
      __m128i min4 = _mm_min_epi32(min2, min3);
      int min_num = _mm_cvtsi128_si32(min4);

      reference_item = _mm256_set1_epi32(min_num);
      matched = _mm256_cmpeq_epi32_mask(reference_item, counter_data);
      min_pos = _tzcnt_u32(static_cast<uint32_t>(matched));

      if (incast[whichcast] * COUNT[choice] >= int(min_num * factor)) {
        if (counters[min_pos] < 0) {
          uint32_t min_choice = hash(items[min_pos], seed_s) & 1;
          incast[hash(items[min_pos], seed_incast) % counter_num] -=
              COUNT[min_choice] * counters[min_pos];
        }
        items[min_pos] = item;
        counters[min_pos] = min_num + 1;
      }
      incast[whichcast] += COUNT[choice];
    }

    count_type Query(const data_type item,
                     uint32_t seed_s,
                     uint32_t seed_incast) {
      __m256i reference_item = _mm256_set1_epi32(item);
      int matched = _mm256_cmpeq_epi32_mask(reference_item, *(__m256i *) items);

      if (matched != 0) {
        int matched_index = _tzcnt_u32((uint32_t) matched);
        return std::abs(counters[matched_index]);
      }
      return 0;
    }
  };

  WSSIMDMC256(uint32_t _BUCKET_NUM)
      : Abstract((char *) "WavingSketch"), BUCKET_NUM(_BUCKET_NUM) {
    buckets = new(std::align_val_t{64}) Bucket[BUCKET_NUM];
    memset(buckets, 0, BUCKET_NUM * sizeof(Bucket));
    rename("WSSIMDMC<8, 16>");
    // seed_choice = std::clock();
    // sleep(1);
    // seed_incast = std::clock();
    // sleep(1);
    // seed_s = std::clock();
    // fix seed for predictable result
    seed_choice = 123;
    seed_incast = 456;
    seed_s = 789;
  }
  ~WSSIMDMC256() { delete[] buckets; }

  void Insert(const data_type item) {
    uint32_t bucket_pos = hash(item, seed_choice) % BUCKET_NUM;
    buckets[bucket_pos].Insert(item, seed_s, seed_incast);
  }

  count_type Query(const data_type item) {
    uint32_t bucket_pos = hash(item, seed_choice) % BUCKET_NUM;
    return buckets[bucket_pos].Query(item, seed_s, seed_incast);
  }

 private:
  Bucket *buckets;
  uint32_t BUCKET_NUM;
  uint32_t seed_choice;
  uint32_t seed_s;
  uint32_t seed_incast;
};

class WSSIMDMC512 : public Abstract {
 public:
  static constexpr uint32_t slot_num = 16;
  static constexpr uint32_t counter_num = 16;

  struct Bucket {
    data_type items[slot_num];
    count_type counters[slot_num];
    int16_t incast[counter_num];

    void Insert(const data_type item, uint32_t seed_s, uint32_t seed_incast) {
      uint32_t choice = hash(item, seed_s) & 1;
      uint32_t whichcast = hash(item, seed_incast) % counter_num;
      uint32_t min_pos = -1;

      __m256i reference_item;
      int matched;

      // try to find same item
      reference_item = _mm256_set1_epi32(item);
      matched = _mm256_cmpeq_epi32_mask(reference_item, *(__m256i *) items);

      if (matched != 0) {
        int matched_index = _tzcnt_u32((uint32_t) matched);
        if (counters[matched_index] < 0)
          // negative means no-error
          counters[matched_index]--;
        else {
          counters[matched_index]++;
          incast[whichcast] += COUNT[choice];
        }
        return;
      }

      matched =
          _mm256_cmpeq_epi32_mask(reference_item, *(__m256i *) (items + 8));

      if (matched != 0) {
        int matched_index = _tzcnt_u32((uint32_t) matched) + 8;
        if (counters[matched_index] < 0)
          // negative means no-error
          counters[matched_index]--;
        else {
          counters[matched_index]++;
          incast[whichcast] += COUNT[choice];
        }
        return;
      }

      // try to find free buckets
      // no need to construct each time
      static const __m256i reference_item_0 = _mm256_set1_epi32(0);
      matched =
          _mm256_cmpeq_epi32_mask(reference_item_0, *(__m256i *) counters);

      if (matched != 0) {
        int matched_index = _tzcnt_u32(static_cast<uint32_t>(matched));
        items[matched_index] = item;
        counters[matched_index] = -1;
        return;
      }

      matched = _mm256_cmpeq_epi32_mask(reference_item_0,
                                        *(__m256i *) (counters + 8));

      if (matched != 0) {
        int matched_index = _tzcnt_u32(static_cast<uint32_t>(matched)) + 8;
        items[matched_index] = item;
        counters[matched_index] = -1;
        return;
      }

      // get smallest counter's index

      __m256i low_256 = _mm256_abs_epi32(*(__m256i *) counters);
      __m256i high_256 = _mm256_abs_epi32(*(__m256i *) (counters + 8));

      __m256i min_256 = _mm256_min_epi32(low_256, high_256);

      __m128i low_part = _mm256_extracti128_si256(min_256, 0);
      __m128i high_part = _mm256_extracti128_si256(min_256, 1);

      __m128i min0 = _mm_min_epi32(low_part, high_part);
      __m128i min1 = _mm_shuffle_epi32(min0, _MM_SHUFFLE(0, 0, 3, 2));
      __m128i min2 = _mm_min_epi32(min0, min1);
      __m128i min3 = _mm_shuffle_epi32(min2, _MM_SHUFFLE(0, 0, 0, 1));
      __m128i min4 = _mm_min_epi32(min2, min3);
      int min_num = _mm_cvtsi128_si32(min4);

      reference_item = _mm256_set1_epi32(min_num);
      // a_comp = _mm256_cmpeq_epi32(reference_item, low_256);
      // matched = _mm256_movemask_ps(_mm256_castsi256_ps(a_comp));
      matched = _mm256_cmpeq_epi32_mask(reference_item, low_256);
      if (matched != 0) {
        min_pos = _tzcnt_u32(static_cast<uint32_t>(matched));
      } else {
        // a_comp = _mm256_cmpeq_epi32(reference_item, high_256);
        // matched = _mm256_movemask_ps(_mm256_castsi256_ps(a_comp));
        matched = _mm256_cmpeq_epi32_mask(reference_item, high_256);
        min_pos = _tzcnt_u32(static_cast<uint32_t>(matched)) + 8;
      }

      if (incast[whichcast] * COUNT[choice] >= int(min_num * factor)) {
        if (counters[min_pos] < 0) {
          uint32_t min_choice = hash(items[min_pos], seed_s) & 1;
          incast[hash(items[min_pos], seed_incast) % counter_num] -=
              COUNT[min_choice] * counters[min_pos];
        }
        items[min_pos] = item;
        counters[min_pos] = min_num + 1;
      }
      incast[whichcast] += COUNT[choice];
    }

    count_type Query(const data_type item,
                     uint32_t seed_s,
                     uint32_t seed_incast) {
      __m256i reference_item;
      int matched;

      reference_item = _mm256_set1_epi32(item);
      matched = _mm256_cmpeq_epi32_mask(reference_item, *(__m256i *) items);

      if (matched != 0) {
        int matched_index = _tzcnt_u32((uint32_t) matched);
        return std::abs(counters[matched_index]);
      }

      matched =
          _mm256_cmpeq_epi32_mask(reference_item, *(__m256i *) (items + 8));

      if (matched != 0) {
        int matched_index = _tzcnt_u32((uint32_t) matched) + 8;
        return std::abs(counters[matched_index]);
      }
      return 0;
    }
  };

  WSSIMDMC512(uint32_t _BUCKET_NUM)
      : Abstract((char *) "WavingSketch"), BUCKET_NUM(_BUCKET_NUM) {
    buckets = new(std::align_val_t{64}) Bucket[BUCKET_NUM];
    memset(buckets, 0, BUCKET_NUM * sizeof(Bucket));
    rename("WSSIMDMC<16, 16>");
    // seed_choice = std::clock();
    // sleep(1);
    // seed_incast = std::clock();
    // sleep(1);
    // seed_s = std::clock();
    // fix seed for predictable result
    seed_choice = 123;
    seed_incast = 456;
    seed_s = 789;
  }
  ~WSSIMDMC512() { delete[] buckets; }

  void Insert(const data_type item) {
    uint32_t bucket_pos = hash(item, seed_choice) % BUCKET_NUM;
    buckets[bucket_pos].Insert(item, seed_s, seed_incast);
  }

  count_type Query(const data_type item) {
    uint32_t bucket_pos = hash(item, seed_choice) % BUCKET_NUM;
    return buckets[bucket_pos].Query(item, seed_s, seed_incast);
  }

 private:
  Bucket *buckets;
  uint32_t BUCKET_NUM;
  uint32_t seed_choice;
  uint32_t seed_s;
  uint32_t seed_incast;
};

class WSSIMDMC1024 : public Abstract {
 public:
  static constexpr uint32_t slot_num = 32;
  static constexpr uint32_t counter_num = 16;

  struct Bucket {
    data_type items[slot_num];
    count_type counters[slot_num];
    int16_t incast[counter_num];

    void Insert(const data_type item, uint32_t seed_s, uint32_t seed_incast) {
      uint32_t choice = hash(item, seed_s) & 1;
      uint32_t whichcast = hash(item, seed_incast) % counter_num;
      uint32_t min_pos = -1;

      __m256i reference_item;
      int matched;

      // try to find same item
      reference_item = _mm256_set1_epi32(item);
      matched = _mm256_cmpeq_epi32_mask(reference_item, *(__m256i *) items);

      if (matched != 0) {
        int matched_index = _tzcnt_u32((uint32_t) matched);
        if (counters[matched_index] < 0)
          // negative means no-error
          counters[matched_index]--;
        else {
          counters[matched_index]++;
          incast[whichcast] += COUNT[choice];
        }
        return;
      }

      matched =
          _mm256_cmpeq_epi32_mask(reference_item, *(__m256i *) (items + 8));

      if (matched != 0) {
        int matched_index = _tzcnt_u32((uint32_t) matched) + 8;
        if (counters[matched_index] < 0)
          // negative means no-error
          counters[matched_index]--;
        else {
          counters[matched_index]++;
          incast[whichcast] += COUNT[choice];
        }
        return;
      }

      matched =
          _mm256_cmpeq_epi32_mask(reference_item, *(__m256i *) (items + 16));

      if (matched != 0) {
        int matched_index = _tzcnt_u32((uint32_t) matched) + 16;
        if (counters[matched_index] < 0)
          // negative means no-error
          counters[matched_index]--;
        else {
          counters[matched_index]++;
          incast[whichcast] += COUNT[choice];
        }
        return;
      }

      matched =
          _mm256_cmpeq_epi32_mask(reference_item, *(__m256i *) (items + 24));

      if (matched != 0) {
        int matched_index = _tzcnt_u32((uint32_t) matched) + 24;
        if (counters[matched_index] < 0)
          // negative means no-error
          counters[matched_index]--;
        else {
          counters[matched_index]++;
          incast[whichcast] += COUNT[choice];
        }
        return;
      }

      // try to find free buckets
      // no need to construct each time
      static const __m256i reference_item_0 = _mm256_set1_epi32(0);
      matched =
          _mm256_cmpeq_epi32_mask(reference_item_0, *(__m256i *) counters);

      if (matched != 0) {
        int matched_index = _tzcnt_u32(static_cast<uint32_t>(matched));
        items[matched_index] = item;
        counters[matched_index] = -1;
        return;
      }

      matched = _mm256_cmpeq_epi32_mask(reference_item_0,
                                        *(__m256i *) (counters + 8));

      if (matched != 0) {
        int matched_index = _tzcnt_u32(static_cast<uint32_t>(matched)) + 8;
        items[matched_index] = item;
        counters[matched_index] = -1;
        return;
      }

      matched = _mm256_cmpeq_epi32_mask(reference_item_0,
                                        *(__m256i *) (counters + 16));

      if (matched != 0) {
        int matched_index = _tzcnt_u32(static_cast<uint32_t>(matched)) + 16;
        items[matched_index] = item;
        counters[matched_index] = -1;
        return;
      }

      matched = _mm256_cmpeq_epi32_mask(reference_item_0,
                                        *(__m256i *) (counters + 24));

      if (matched != 0) {
        int matched_index = _tzcnt_u32(static_cast<uint32_t>(matched)) + 24;
        items[matched_index] = item;
        counters[matched_index] = -1;
        return;
      }

      // get smallest counter's index

      __m256i a_256 = _mm256_abs_epi32(*(__m256i *) counters);
      __m256i b_256 = _mm256_abs_epi32(*(__m256i *) (counters + 8));
      __m256i c_256 = _mm256_abs_epi32(*(__m256i *) (counters + 16));
      __m256i d_256 = _mm256_abs_epi32(*(__m256i *) (counters + 24));

      __m256i min_ab = _mm256_min_epi32(a_256, b_256);
      __m256i min_cd = _mm256_min_epi32(c_256, d_256);

      __m256i min_256 = _mm256_min_epi32(min_ab, min_cd);

      __m128i low_part = _mm256_extracti128_si256(min_256, 0);
      __m128i high_part = _mm256_extracti128_si256(min_256, 1);

      __m128i min0 = _mm_min_epi32(low_part, high_part);
      __m128i min1 = _mm_shuffle_epi32(min0, _MM_SHUFFLE(0, 0, 3, 2));
      __m128i min2 = _mm_min_epi32(min0, min1);
      __m128i min3 = _mm_shuffle_epi32(min2, _MM_SHUFFLE(0, 0, 0, 1));
      __m128i min4 = _mm_min_epi32(min2, min3);
      int min_num = _mm_cvtsi128_si32(min4);

      reference_item = _mm256_set1_epi32(min_num);
      matched = _mm256_cmpeq_epi32_mask(reference_item, a_256);
      if (matched != 0) {
        min_pos = _tzcnt_u32(static_cast<uint32_t>(matched));
      } else {
        matched = _mm256_cmpeq_epi32_mask(reference_item, b_256);
        if (matched != 0) {
          min_pos = _tzcnt_u32(static_cast<uint32_t>(matched)) + 8;
        } else {
          matched = _mm256_cmpeq_epi32_mask(reference_item, c_256);
          if (matched != 0) {
            min_pos = _tzcnt_u32(static_cast<uint32_t>(matched)) + 16;
          } else {
            matched = _mm256_cmpeq_epi32_mask(reference_item, d_256);
            min_pos = _tzcnt_u32(static_cast<uint32_t>(matched)) + 24;
          }
        }
      }

      if (incast[whichcast] * COUNT[choice] >= int(min_num * factor)) {
        if (counters[min_pos] < 0) {
          uint32_t min_choice = hash(items[min_pos], seed_s) & 1;
          incast[hash(items[min_pos], seed_incast) % counter_num] -=
              COUNT[min_choice] * counters[min_pos];
        }
        items[min_pos] = item;
        counters[min_pos] = min_num + 1;
      }
      incast[whichcast] += COUNT[choice];
    }

    count_type Query(const data_type item,
                     uint32_t seed_s,
                     uint32_t seed_incast) {
      __m256i reference_item;
      int matched;

      reference_item = _mm256_set1_epi32(item);
      matched = _mm256_cmpeq_epi32_mask(reference_item, *(__m256i *) items);

      if (matched != 0) {
        int matched_index = _tzcnt_u32((uint32_t) matched);
        return std::abs(counters[matched_index]);
      }

      matched =
          _mm256_cmpeq_epi32_mask(reference_item, *(__m256i *) (items + 8));

      if (matched != 0) {
        int matched_index = _tzcnt_u32((uint32_t) matched) + 8;
        return std::abs(counters[matched_index]);
      }

      matched =
          _mm256_cmpeq_epi32_mask(reference_item, *(__m256i *) (items + 16));

      if (matched != 0) {
        int matched_index = _tzcnt_u32((uint32_t) matched) + 16;
        return std::abs(counters[matched_index]);
      }

      matched =
          _mm256_cmpeq_epi32_mask(reference_item, *(__m256i *) (items + 24));

      if (matched != 0) {
        int matched_index = _tzcnt_u32((uint32_t) matched) + 24;
        return std::abs(counters[matched_index]);
      }
      return 0;
    }
  };

  WSSIMDMC1024(uint32_t _BUCKET_NUM)
      : Abstract((char *) "WavingSketch"), BUCKET_NUM(_BUCKET_NUM) {
    buckets = new(std::align_val_t{64}) Bucket[BUCKET_NUM];
    memset(buckets, 0, BUCKET_NUM * sizeof(Bucket));
    rename("WSSIMDMC<32, 16>");
    // seed_choice = std::clock();
    // sleep(1);
    // seed_incast = std::clock();
    // sleep(1);
    // seed_s = std::clock();
    // fix seed for predictable result
    seed_choice = 123;
    seed_incast = 456;
    seed_s = 789;
  }
  ~WSSIMDMC1024() { delete[] buckets; }

  void Insert(const data_type item) {
    uint32_t bucket_pos = hash(item, seed_choice) % BUCKET_NUM;
    buckets[bucket_pos].Insert(item, seed_s, seed_incast);
  }

  count_type Query(const data_type item) {
    uint32_t bucket_pos = hash(item, seed_choice) % BUCKET_NUM;
    return buckets[bucket_pos].Query(item, seed_s, seed_incast);
  }

 private:
  Bucket *buckets;
  uint32_t BUCKET_NUM;
  uint32_t seed_choice;
  uint32_t seed_s;
  uint32_t seed_incast;
};