#pragma once

#include <chrono>
#include <climits>
#include <vector>
#include <iostream>

#include "SS.h"
#include "USS.h"
#include "Count_Heap.h"
#include "WavingSketch.h"

typedef std::chrono::high_resolution_clock::time_point TP;

inline TP now() { return std::chrono::high_resolution_clock::now(); }

data_type *read_data(const char *PATH, const count_type length,
                     count_type *cnt) {
  data_type *items = new data_type[length];
  data_type *it = items;

  FILE *data = fopen(PATH, "rb");

  *cnt = 0;
  while (fread(it++, sizeof(data_type), 1, data) > 0) {
    (*cnt)++;
  }

  fclose(data);

  return items;
}

uint32_t Get_TopK(HashMap mp, uint32_t k) {
  uint32_t size = mp.size();
  uint32_t *num = new uint32_t[size];

  uint32_t pos = 0;
  HashMap::iterator it;
  for (it = mp.begin(); it != mp.end(); ++it) {
    num[pos++] = it->second;
  }
  std::nth_element(num, num + size - k, num + size);
  uint32_t ret = num[size - k];

  delete[] num;
  return ret;
}

void BenchCmp(const char *PATH) {
  std::cout << "Comparison with SS, USS, Count+Heap on ARE, CR and PR"
            << std::endl
            << std::endl;
  count_type cnt;
  data_type *items = read_data(PATH, 100000000, &cnt);

  constexpr int32_t mem_base = 0;
  constexpr int32_t mem_inc = 50000;
  constexpr int32_t mem_var = 6;
  constexpr int32_t cmp_num = 19;

  Abstract *sketches[mem_var][cmp_num];

  for (int i = 0; i < mem_var; ++i) {
    sketches[i][0] = new WavingSketch<4>(
        (i + 1) * mem_inc / sizeof(WavingSketch<4>::Bucket));
    sketches[i][1] = new WavingSketch<8>(
        (i + 1) * mem_inc / sizeof(WavingSketch<8>::Bucket));
    sketches[i][2] = new WavingSketch<16>(
        (i + 1) * mem_inc / sizeof(WavingSketch<16>::Bucket));
    sketches[i][3] = new WavingSketch<32>(
        (i + 1) * mem_inc / sizeof(WavingSketch<32>::Bucket));
    sketches[i][4] = new SS((i + 1) * mem_inc / 100);
    sketches[i][5] = new USS((i + 1) * mem_inc / 100);
    sketches[i][6] = new Count_Heap(2500, ((i + 1) * mem_inc - 120000) / 12, 3);
    sketches[i][7] = new WavingSketchSIMD128(
        (i + 1) * mem_inc / sizeof(WavingSketchSIMD128::Bucket));
    sketches[i][8] = new WavingSketchSIMD256(
        (i + 1) * mem_inc / sizeof(WavingSketchSIMD256::Bucket));
    sketches[i][9] = new WavingSketchSIMD512(
        (i + 1) * mem_inc / sizeof(WavingSketchSIMD512::Bucket));
    sketches[i][10] = new WavingSketchSIMD1024(
        (i + 1) * mem_inc / sizeof(WavingSketchSIMD1024::Bucket));
    sketches[i][11] = new WavingSketchMC<4, 16>(
        (i + 1) * mem_inc / sizeof(WavingSketchMC<4, 16>::Bucket));
    sketches[i][12] = new WavingSketchMC<8, 16>(
        (i + 1) * mem_inc / sizeof(WavingSketchMC<8, 16>::Bucket));
    sketches[i][13] = new WavingSketchMC<16, 16>(
        (i + 1) * mem_inc / sizeof(WavingSketchMC<16, 16>::Bucket));
    sketches[i][14] = new WavingSketchMC<32, 16>(
        (i + 1) * mem_inc / sizeof(WavingSketchMC<32, 16>::Bucket));
    sketches[i][15] = new WSSIMDMC128(
        (i + 1) * mem_inc / sizeof(WSSIMDMC128::Bucket));
    sketches[i][16] = new WSSIMDMC256(
        (i + 1) * mem_inc / sizeof(WSSIMDMC256::Bucket));
    sketches[i][17] = new WSSIMDMC512(
        (i + 1) * mem_inc / sizeof(WSSIMDMC512::Bucket));
    sketches[i][18] = new WSSIMDMC1024(
        (i + 1) * mem_inc / sizeof(WSSIMDMC1024::Bucket));
  }

  // Ground truth
  HashMap mp;
  for (int l = 0; l < cnt; ++l) {
    if (mp.find(items[l]) == mp.end())
      mp[items[l]] = 1;
    else
      mp[items[l]] += 1;
  }
  uint32_t topK = Get_TopK(mp, 2000);

  for (int i = 0; i < mem_var; ++i) {
    std::cout << "Memory size: " << (mem_base + mem_inc * (i + 1)) / 1000
              << "KB" << std::endl
              << std::endl;
    for (int l = 0; l < cnt; ++l) {
      for (int j = 0; j < cmp_num; ++j) {
        sketches[i][j]->Insert(items[l]);
      }
    }

    for (int j = 0; j < cmp_num; ++j) {
      sketches[i][j]->Check(mp, topK);
      delete sketches[i][j];
    }
    std::cout << std::endl;
  }

  delete items;
}

void BenchThp(const char *PATH) {
  std::cout << "Comparison with SS, USS, Count+Heap on Throughput"
            << std::endl
            << std::endl;

  count_type cnt;
  data_type *items = read_data(PATH, 100000000, &cnt);

  constexpr int32_t mem_base = 0;
  constexpr int32_t mem_inc = 50000;
  constexpr int32_t mem_var = 6;
  constexpr int32_t cmp_num = 19;
  constexpr int32_t round = 5;

  Abstract *sketches[mem_var][cmp_num];

  int progress = 0;

  for (int i = 0; i < mem_var; ++i) {
    std::cout << "Memory size: " << (mem_base + mem_inc * (i + 1)) / 1000
              << "KB" << std::endl
              << std::endl;

    double thp[round][cmp_num] = {};
    double avg_thp[cmp_num] = {};

    for (int j = 0; j < round; ++j) {
      sketches[i][0] = new WavingSketch<4>(
          (mem_base + (i + 1) * mem_inc) / sizeof(WavingSketch<4>::Bucket));
      sketches[i][1] = new WavingSketch<8>(
          (mem_base + (i + 1) * mem_inc) / sizeof(WavingSketch<8>::Bucket));
      sketches[i][2] = new WavingSketch<16>(
          (mem_base + (i + 1) * mem_inc) / sizeof(WavingSketch<16>::Bucket));
      sketches[i][3] = new WavingSketch<32>(
          (mem_base + (i + 1) * mem_inc) / sizeof(WavingSketch<32>::Bucket));
      sketches[i][4] = new SS((mem_base + mem_inc * (i + 1)) / 100);
      sketches[i][5] = new USS((mem_base + mem_inc * (i + 1)) / 100);
      sketches[i][6] =
          new Count_Heap(2500, ((i + 1) * mem_inc - 120000) / 12, 3);
      sketches[i][7] = new WavingSketchSIMD128(
          (mem_base + (i + 1) * mem_inc) / sizeof(WavingSketchSIMD128::Bucket));
      sketches[i][8] = new WavingSketchSIMD256(
          (mem_base + (i + 1) * mem_inc) / sizeof(WavingSketchSIMD256::Bucket));
      sketches[i][9] = new WavingSketchSIMD512(
          (mem_base + (i + 1) * mem_inc) / sizeof(WavingSketchSIMD512::Bucket));
      sketches[i][10] = new WavingSketchSIMD1024((mem_base + (i + 1) * mem_inc)
                                                     / sizeof(WavingSketchSIMD1024::Bucket));
      sketches[i][11] = new WavingSketchMC<4, 16>((mem_base + (i + 1) * mem_inc)
                                                      / sizeof(WavingSketchMC<4,
                                                                              16>::Bucket));
      sketches[i][12] = new WavingSketchMC<8, 16>((mem_base + (i + 1) * mem_inc)
                                                      / sizeof(WavingSketchMC<8,
                                                                              16>::Bucket));
      sketches[i][13] =
          new WavingSketchMC<16, 16>((mem_base + (i + 1) * mem_inc)
                                         / sizeof(WavingSketchMC<16,
                                                                 16>::Bucket));
      sketches[i][14] =
          new WavingSketchMC<32, 16>((mem_base + (i + 1) * mem_inc)
                                         / sizeof(WavingSketchMC<32,
                                                                 16>::Bucket));
      sketches[i][15] = new WSSIMDMC128((mem_base + (i + 1) * mem_inc)
                                            / sizeof(WSSIMDMC128::Bucket));
      sketches[i][16] = new WSSIMDMC256((mem_base + (i + 1) * mem_inc)
                                            / sizeof(WSSIMDMC256::Bucket));
      sketches[i][17] = new WSSIMDMC512((mem_base + (i + 1) * mem_inc)
                                            / sizeof(WSSIMDMC512::Bucket));
      sketches[i][18] = new WSSIMDMC1024((mem_base + (i + 1) * mem_inc)
                                             / sizeof(WSSIMDMC1024::Bucket));

      for (int l = 0; l < cmp_num; ++l) {
        TP start, finish;

        start = now();
        for (int m = 0; m < cnt; ++m) {
          sketches[i][l]->Insert(items[m]);
        }
        finish = now();

        thp[j][l] =
            (double) cnt /
                std::chrono::duration_cast<std::chrono::duration<
                    double, std::ratio<1, 1000000> > >(finish - start)
                    .count();
        avg_thp[l] += thp[j][l];

        if (j != round - 1) {
          delete sketches[i][l];
        }
      }
    }

    for (int l = 0; l < cmp_num; ++l) {
      printf("%12s:\tthp = %lf\n", sketches[i][l]->name.c_str(),
             avg_thp[l] / round);
      delete sketches[i][l];
    }
    std::cout << std::endl;
  }

  delete items;
}

void BenchCompress(const char *PATH) {
  std::cout << "Comparison with compressed WavingSketch on ARE, CR and PR"
            << std::endl;
  count_type cnt;
  data_type *items = read_data(PATH, 100000000, &cnt);

  // mem_size[i] = mem_base * mem_map[i] (i < mem_var)
  constexpr int32_t mem_base = 10000;
  constexpr int32_t mem_amp[] = {1, 2, 4, 8, 16, 32};
  constexpr int32_t ref_mem_amp = 32;
  constexpr int32_t mem_var = 6;

  std::cout << "The first line uses NO compression" << std::endl;
  std::cout << "The second line compresses from memory size "
            << mem_base * ref_mem_amp / 1000
            << "KB"
            #ifdef NO_ERROR_FIRST
            << ", no-error first"
            #endif
            << std::endl << std::endl;

  WavingSketch<8> *reference = new WavingSketch<8>(
      mem_base / sizeof(WavingSketch<8>::Bucket) * ref_mem_amp);
  WavingSketch<8> *sketches[mem_var];
  WavingSketch<8> *compressed;

  for (int32_t i = 0; i < mem_var; ++i) {
    sketches[i] = new WavingSketch<8>(
        mem_base / sizeof(WavingSketch<8>::Bucket) * mem_amp[i]);
  }

  // ground truth
  HashMap mp;
  for (count_type l = 0; l < cnt; ++l) {
    if (mp.find(items[l]) == mp.end())
      mp[items[l]] = 1;
    else
      mp[items[l]] += 1;
  }
  uint32_t topK = Get_TopK(mp, 2000);

  // insert items
  for (count_type l = 0; l < cnt; ++l) {
    for (auto &sketch: sketches) {
      sketch->Insert(items[l]);
    }
    reference->Insert(items[l]);
  }
  delete items;

  // compress and check
  for (int32_t i = 0; i < mem_var; ++i) {
    std::cout << "Memory size: " << mem_base * mem_amp[i] / 1000 << "KB"
              << std::endl << std::endl;
    compressed = new WavingSketch<8>(
        mem_base / sizeof(WavingSketch<8>::Bucket) * mem_amp[i]);
    uint32_t temp = compress<8>(reference, compressed);
    if (temp == 0) {
      std::cerr << "Error: compress" << std::endl;
      exit(EXIT_FAILURE);
    }
    sketches[i]->Check(mp, static_cast<count_type>(topK));
    compressed->Check(mp, static_cast<count_type>(topK));
    delete sketches[i];
    delete compressed;
    std::cout << std::endl;
  }
}

void BenchExpand(const char *PATH) {
  std::cout << "Comparison with expanded WavingSketch on ARE, CR and PR"
            << std::endl;
  count_type cnt;
  data_type *items = read_data(PATH, 100000000, &cnt);

  constexpr int32_t mem_base = 10000;
  constexpr int32_t mem_amp[] = {1, 2, 4, 8, 16, 32};
  constexpr int32_t ref_mem_amp = 32;
  constexpr int32_t mem_var = 6;

  std::cout << "The first line uses NO expansion" << std::endl;
  std::cout << "The second line expands to memory size "
            << mem_base * ref_mem_amp / 1000
            << "KB after half of the elements inserted"
            #ifdef NO_INCAST
            << ", discarding incast"
            #endif
            #ifdef COPY_INCAST
            << ", with incast copied"
            #endif
            #ifdef SPLIT_INCAST
            << ", with incast splitted"
            #endif
            #ifdef THRESHOLD_INCAST
            << ", keeping incast when greater than threshold"
            #endif
            << std::endl << std::endl;

  WavingSketch<8> *sketches[mem_var];
  WavingSketch<8> *expaneded_sketches[mem_var];

  for (int32_t i = 0; i < mem_var; ++i) {
    sketches[i] = new WavingSketch<8>(
        mem_base / sizeof(WavingSketch<8>::Bucket) * mem_amp[i]);
    expaneded_sketches[i] = new WavingSketch<8>(
        mem_base / sizeof(WavingSketch<8>::Bucket) * ref_mem_amp);
  }

  // ground truth
  HashMap mp;
  for (count_type l = 0; l < cnt; ++l) {
    if (mp.find(items[l]) == mp.end())
      mp[items[l]] = 1;
    else
      mp[items[l]] += 1;
  }
  uint32_t topK = Get_TopK(mp, 2000);

  // insert the first half of flows
  count_type half = cnt / 2;
  for (count_type l = 0; l < half; ++l) {
    for (auto &sketch: sketches) {
      sketch->Insert(items[l]);
    }
  }

  // expand
  for (int32_t i = 0; i < mem_var; ++i) {
    uint32_t temp = expand(sketches[i], expaneded_sketches[i]);
    if (temp == 0) {
      std::cerr << "Error: expand" << std::endl;
      exit(EXIT_FAILURE);
    }
  }

  // insert the rest
  for (count_type l = half; l < cnt; ++l) {
    for (auto &sketch: sketches) {
      sketch->Insert(items[l]);
    }
    for (auto &sketch: expaneded_sketches) {
      sketch->Insert(items[l]);
    }
  }
  delete items;

  // check
  for (int32_t i = 0; i < mem_var; ++i) {
    std::cout << "Memory size: " << mem_base * mem_amp[i] / 1000 << "KB"
              << std::endl << std::endl;
    sketches[i]->Check(mp, static_cast<count_type>(topK));
    expaneded_sketches[i]->Check(mp, static_cast<count_type>(topK));
    delete sketches[i];
    delete expaneded_sketches[i];
    std::cout << std::endl;
  }
}

void BenchCompressMC(const char *PATH) {
  std::cout << "Comparison with compressed WavingSketchMC on ARE, CR and PR"
            << std::endl;
  count_type cnt;
  data_type *items = read_data(PATH, 100000000, &cnt);

  // mem_size[i] = mem_base * mem_map[i] (i < mem_var)
  constexpr int32_t mem_base = 10000;
  constexpr int32_t mem_amp[] = {1, 2, 4, 8, 16, 32};
  constexpr int32_t ref_mem_amp = 32;
  constexpr int32_t mem_var = 6;

  std::cout << "The first line uses NO compression" << std::endl;
  std::cout << "The second line compresses from memory size "
            << mem_base * ref_mem_amp / 1000
            << "KB"
            #ifdef NO_ERROR_FIRST
            << ", no-error first"
            #endif
            << std::endl << std::endl;

  WavingSketchMC<8, 16> *reference = new WavingSketchMC<8, 16>(
      mem_base / sizeof(WavingSketchMC<8, 16>::Bucket) * ref_mem_amp);
  WavingSketchMC<8, 16> *sketches[mem_var];
  WavingSketchMC<8, 16> *compressed;

  for (int32_t i = 0; i < mem_var; ++i) {
    sketches[i] = new WavingSketchMC<8, 16>(
        mem_base / sizeof(WavingSketchMC<8, 16>::Bucket) * mem_amp[i]);
  }

  // ground truth
  HashMap mp;
  for (count_type l = 0; l < cnt; ++l) {
    if (mp.find(items[l]) == mp.end())
      mp[items[l]] = 1;
    else
      mp[items[l]] += 1;
  }
  uint32_t topK = Get_TopK(mp, 2000);

  // insert items
  for (count_type l = 0; l < cnt; ++l) {
    for (auto &sketch: sketches) {
      sketch->Insert(items[l]);
    }
    reference->Insert(items[l]);
  }
  delete items;

  // compress and check
  for (int32_t i = 0; i < mem_var; ++i) {
    std::cout << "Memory size: " << mem_base * mem_amp[i] / 1000 << "KB"
              << std::endl << std::endl;
    compressed = new WavingSketchMC<8, 16>(
        mem_base / sizeof(WavingSketchMC<8, 16>::Bucket) * mem_amp[i]);
    uint32_t temp = compressMC<8, 16>(reference, compressed);
    if (temp == 0) {
      std::cerr << "Error: compress" << std::endl;
      exit(EXIT_FAILURE);
    }
    sketches[i]->Check(mp, static_cast<count_type>(topK));
    compressed->Check(mp, static_cast<count_type>(topK));
    delete sketches[i];
    delete compressed;
    std::cout << std::endl;
  }
}

void BenchExpandMC(const char *PATH) {
  std::cout << "Comparison with expanded WavingSketchMC on ARE, CR and PR"
            << std::endl;
  count_type cnt;
  data_type *items = read_data(PATH, 100000000, &cnt);

  constexpr int32_t mem_base = 10000;
  constexpr int32_t mem_amp[] = {1, 2, 4, 8, 16, 32};
  constexpr int32_t ref_mem_amp = 32;
  constexpr int32_t mem_var = 6;

  std::cout << "The first line uses NO expansion" << std::endl;
  std::cout << "The second line expands to memory size " << mem_base * ref_mem_amp / 1000
            << "KB after half of the elements inserted"
            #ifdef NO_INCAST
            << ", discarding incast"
            #endif
            #ifdef COPY_INCAST
            << ", with incast copied"
            #endif
            #ifdef SPLIT_INCAST
            << ", with incast splitted"
            #endif
            #ifdef THRESHOLD_INCAST
            << ", keeping incast when greater than threshold"
            #endif
            << std::endl << std::endl;

  WavingSketchMC<8, 16> *sketches[mem_var];
  WavingSketchMC<8, 16> *expanded_sketches[mem_var];

  for (int32_t i = 0; i < mem_var; ++i) {
    sketches[i] = new WavingSketchMC<8, 16>(
        mem_base / sizeof(WavingSketchMC<8, 16>::Bucket) * mem_amp[i]);
    expanded_sketches[i] = new WavingSketchMC<8, 16>(
        mem_base / sizeof(WavingSketchMC<8, 16>::Bucket) * ref_mem_amp);
  }

  // ground truth
  HashMap mp;
  for (count_type l = 0; l < cnt; ++l) {
    if (mp.find(items[l]) == mp.end())
      mp[items[l]] = 1;
    else
      mp[items[l]] += 1;
  }
  uint32_t topK = Get_TopK(mp, 2000);

  // insert the first half of flows
  count_type half = cnt / 2;
  for (count_type l = 0; l < half; ++l) {
    for (auto &sketch: sketches) {
      sketch->Insert(items[l]);
    }
  }

  // expand
  for (int32_t i = 0; i < mem_var; ++i) {
    uint32_t temp = expandMC<8, 16>(sketches[i], expanded_sketches[i]);
    if (temp == 0) {
      std::cerr << "Error: expand" << std::endl;
      exit(EXIT_FAILURE);
    }
  }

  // insert the rest
  for (count_type l = half; l < cnt; ++l) {
    for (auto &sketch: sketches) {
      sketch->Insert(items[l]);
    }
    for (auto &sketch: expanded_sketches) {
      sketch->Insert(items[l]);
    }
  }
  delete items;

  // check
  for (int32_t i = 0; i < mem_var; ++i) {
    std::cout << "Memory size: " << mem_base * mem_amp[i] / 1000 << "KB"
              << std::endl << std::endl;
    sketches[i]->Check(mp, static_cast<count_type>(topK));
    expanded_sketches[i]->Check(mp, static_cast<count_type>(topK));
    delete sketches[i];
    delete expanded_sketches[i];
    std::cout << std::endl;
  }
}