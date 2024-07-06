#ifndef ABSTRACT_H
#define ABSTRACT_H

#include <iostream>
#include <random>
#include "data.h"
typedef int32_t count_type;
static const int COUNT[2] = { 1, -1 };
static std::random_device rd;
template<uint32_t DATA_LEN>
class Abstract {
public:
	string name;
	double pr;
	double cr;
	double aae;
	double are;
	string sep;
	int HIT;
	Abstract() { pr = cr = are = aae = 0; }
	virtual void Init(const Data<DATA_LEN>& data) {};
	virtual int Query(const Data<DATA_LEN>& data, HashMap<DATA_LEN>& mp) { return 0; };
	virtual int Query(const Data<DATA_LEN>& data) { return 0; };
	virtual int QueryTopK(const Data<DATA_LEN>& item) { return Query(item); }
	virtual void Check(HashMap<DATA_LEN> mp, Abstract<DATA_LEN>* another, const std::vector<std::ostream*>& outs) {};
	virtual void Init(const Data<DATA_LEN>& from, const Data<DATA_LEN>& to) {};
	virtual void Check(HashMap<DATA_LEN> mp, const std::vector<std::ostream*>& outs) {};
	virtual void Init(const Data<DATA_LEN>& data, uint time) {};
	virtual ~Abstract() {};

	void rename(int slot_num, int counter_num) {
		char* tp = new char[20];
		std::sprintf(tp, "WavingSketch<%d,%d>", slot_num, counter_num);
		name = tp;
	}

	void print_pr(ofstream& out, int num) {
		cout.width(15);
		cout << name << " " << this->pr << endl;
	}
	void printcr(ofstream& out, int num) {
		cout.width(15);
		cout << name << " " << this->cr << endl;
	}
	void print_f1(ofstream& out, int num) {
		cout << name << sep << 2 * this->pr * this->cr / ((this->cr + this->pr < 1e-6) ? 1 : this->cr + pr) << endl;
	}

	void Check(const HashMap<DATA_LEN>& mp, count_type HIT, FILE* file) {
		typename HashMap<DATA_LEN>::iterator it;
		count_type value = 0, all = 0, hit = 0, size = 0;
		double aae = 0, are = 0, cr = 0, pr = 0;
		for (it = mp.begin(); it != mp.end(); ++it) {
			value = QueryTopK(it->first);
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

		fprintf(file, "%s: %d\nAAE: %lf\nARE: %lf\nCR: %lf\nPR: %lf\n", name, all,
			aae, are, cr, pr);
	}
	// Heavy Hitter
	void Check(const HashMap<DATA_LEN>& mp, count_type HIT, const std::vector<std::ostream*>& outs) {
		typename HashMap<DATA_LEN>::const_iterator it;
		count_type value = 0, all = 0, hit = 0, size = 0, flagtrue = 0;
		double aae = 0, are = 0, cr = 0, pr = 0, rt = 0;
		for (it = mp.begin(); it != mp.end(); ++it) {
			value = QueryTopK(it->first);
			if (it->second > HIT) {
				all++;
				if (value < 0) {
					value = -value;
					flagtrue += 1;
				}
				if (value > HIT) {
					hit += 1;
					aae += abs(it->second - value);
					are += abs(it->second - value) / (double)it->second;
				}
			}
			if (value > HIT)
			{
				size += 1;
			}
		}

		aae /= hit;
		are /= hit;
		cr = hit / (double)all;
		pr = hit / (double)size;

		printf(
			"%s:\tARE: %f\tCR: %f\tPR: %f\tF1: %f\n",
			name.c_str(), are, cr, pr, 2 * pr * cr / ((cr + pr < 1e-6) ? 1 : cr + pr));
		*outs[0] << are << ",";
		*outs[1] << cr << ",";
		*outs[2] << pr << ",";
		*outs[3] << 2 * pr * cr / ((cr + pr < 1e-6) ? 1 : cr + pr) << ",";
	}
	void CheckSubset(HashMap<DATA_LEN> mp, count_type HIT, int32_t epochs, int32_t batch_size, const std::vector<std::ostream*>& outs)
	{
		std::vector<std::pair<count_type, count_type>> flows;
		for (auto& k_v : mp)
		{
			if (k_v.second > HIT)
			{
				count_type value = Query(k_v.first);
				if (value != k_v.second)
				{
					count_type value = Query(k_v.first);
				}
				flows.push_back(std::make_pair(k_v.second, value));
			}
		}
		double sum_are = 0, sum_aae = 0, avg_are = 0, avg_aae = 0, avg_hit = 0;
		for (uint32_t epoch = 0; epoch < epochs; epoch++)
		{
			std::random_shuffle(flows.begin(), flows.end());
			double real_sum = 0, hit = 0, predict_sum = 0, predict_average = 0;
			for (size_t i = 0; i < batch_size; i++)
			{
				real_sum += flows[i].first;
				predict_sum += flows[i].second;
				if (flows[i].second > HIT)
				{
					hit++;
					predict_average += flows[i].second;
				}
			}
			predict_average /= std::max(1.0, hit);
			double real_average = real_sum / batch_size;
			sum_are += abs(real_sum - predict_sum) / real_sum;
			sum_aae += abs(real_sum - predict_sum);
			avg_are += abs(real_average - predict_average) / real_average;
			avg_aae += abs(real_average - predict_average);
			avg_hit += hit;
		}
		sum_are /= epochs;
		sum_aae /= epochs;
		avg_are /= epochs;
		avg_aae /= epochs;
		avg_hit /= epochs;
		*outs[0] << sum_are << ",";
		*outs[1] << sum_aae << ",";
		*outs[2] << avg_are << ",";
		*outs[3] << avg_aae << ",";
		printf(
			"%s:\tHIT:%f sum_ARE: %f\tsum_AAE: %f\tavg_ARE: %f\tavg_AAE: %f\n",
			name.c_str(), avg_hit, sum_are, sum_aae, avg_are, avg_aae);
	}

	static void DistributeCheck(const HashMap<DATA_LEN>& mp, count_type HIT, HashMap<DATA_LEN>& data2block, Abstract<DATA_LEN>** sketches, const std::vector<std::ostream*>& outs)
	{
		std::vector<double> result(4, 0);
		DistributeCheck(mp, HIT, data2block, sketches, result);
		double are = result[0];
		double cr = result[1];
		double pr = result[2];
		printf(
			"%s:\tARE: %f\tCR: %f\tPR: %f\tF1: %f\n",
			sketches[0]->name.c_str(), are, cr, pr, 2 * pr * cr / ((cr + pr < 1e-6) ? 1 : cr + pr));
		*outs[0] << are << ",";
		*outs[1] << cr << ",";
		*outs[2] << pr << ",";
		*outs[3] << 2 * pr * cr / ((cr + pr < 1e-6) ? 1 : cr + pr) << ",";
	}
	static void DistributeCheck(const HashMap<DATA_LEN>& mp, count_type HIT, HashMap<DATA_LEN>& data2block, Abstract<DATA_LEN>** sketches, std::vector<double>& outs)
	{
		typename HashMap<DATA_LEN>::const_iterator it;
		count_type value = 0, all = 0, hit = 0, size = 0, flagtrue = 0;
		double aae = 0, are = 0, cr = 0, pr = 0, rt = 0;
		for (it = mp.begin(); it != mp.end(); ++it) {
			int pos = data2block[it->first];
			value = sketches[pos]->QueryTopK(it->first);
			if (it->second > HIT) {
				all++;
				if (value < 0) {
					value = -value;
					flagtrue += 1;
				}
				if (value > HIT) {
					hit += 1;
					aae += abs(it->second - value);
					are += abs(it->second - value) / (double)it->second;
				}
			}
			if (value > HIT)
			{
				size += 1;
			}
		}

		aae /= hit;
		are /= hit;
		cr = hit / (double)all;
		pr = hit / (double)size;

		outs[0] = are;
		outs[1] = cr;
		outs[2] = pr;
		outs[3] = 2 * pr * cr / ((cr + pr < 1e-6) ? 1 : cr + pr);
	}
};

#endif  // ABSTRACT_H
