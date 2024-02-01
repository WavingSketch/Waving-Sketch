#pragma once

#include "abstract.h"

// node for double linked list
template<class T>
class Node {
 public:
  T ID;
  Node *prev;
  Node *next;

  Node(T _ID) : ID(_ID), prev(NULL), next(NULL) {}

  void Delete() { Connect(prev, next); }

  void Connect(Node *prev, Node *next) {
    if (prev != NULL)
      prev->next = next;
    if (next != NULL)
      next->prev = prev;
  }
};

class StreamSummary {
 public:
  class StreamBucket;
  class StreamCounter : public Node<data_type> {
   public:
    StreamBucket *parent;
    StreamCounter(data_type _ID) : Node<data_type>(_ID), parent(NULL) {}
  };
  class StreamBucket : public Node<count_type> {
   public:
    StreamCounter *child;
    StreamBucket(count_type _ID = 0)
        : Node<count_type>(_ID), child(NULL) {}
  };

  typedef std::unordered_map<data_type, StreamCounter *> CounterMap;

  StreamSummary(uint32_t _SIZE)
      : SIZE(_SIZE), min_bucket(new StreamBucket()) {}

  ~StreamSummary() {
    StreamBucket *temp_bucket = min_bucket, *next_bucket;

    while (temp_bucket != NULL) {
      next_bucket = (StreamBucket *) temp_bucket->next;

      StreamCounter *temp_counter = temp_bucket->child, *next_counter;
      while (temp_counter != NULL) {
        next_counter = (StreamCounter *) temp_counter->next;
        mp.erase(temp_counter->ID);
        delete temp_counter;
        temp_counter = next_counter;
      }

      delete temp_bucket;
      temp_bucket = next_bucket;
    }
  }

  inline bool isFull() { return mp.size() >= SIZE; }

  inline count_type Query(const data_type &data) {
    return mp.find(data) == mp.end() ? 0 : mp[data]->parent->ID;
  }

  inline StreamBucket *Min_Bucket() {
    return (StreamBucket *) min_bucket->next;
  };

  // min_num of all the node
  inline count_type Min_Num() { return isFull() ? Min_Bucket()->ID : 0; }

  // add the min counter (used for unbias)
  inline void Add_Min() { Add_Data(Min_Bucket()->child->ID); }

  // whether the data is in unordered_map
  bool Add_Data(const data_type &data) {
    if (mp.find(data) == mp.end())
      return false;

    bool del = false;
    StreamCounter *temp = mp[data];
    StreamBucket *prev = temp->parent;
    temp->Delete();
    if (prev->child == temp) {
      prev->child = (StreamCounter *) temp->next;
      if (temp->next == NULL)
        del = true;
    }

    Add_Bucket(temp->parent, temp);

    if (del) {
      prev->Delete();
      delete prev;
    }

    return true;
  }

  // if not in map, add to the list
  void Add_Counter(const data_type &data, bool empty) {
    StreamBucket *prev =
        empty ? min_bucket : (StreamBucket *) min_bucket->next;

    StreamCounter *add = new StreamCounter(data);
    Add_Bucket(prev, add);
    mp[data] = add;

    if (!empty) {
      StreamCounter *temp = Min_Bucket()->child;
      Min_Bucket()->child = (StreamCounter *) temp->next;
      mp.erase(temp->ID);
      temp->Delete();
      if (temp->next == NULL) {
        StreamBucket *bucket = (StreamBucket *) min_bucket->next;
        bucket->Delete();
        delete bucket;
      }
      delete temp;
    }
  }

 private:
  const uint32_t SIZE;
  CounterMap mp;
  StreamBucket *min_bucket;

  // new a new counter next to prev, and add counter to new bucket
  void Add_Bucket(StreamBucket *prev, StreamCounter *counter) {
    if (prev->next == NULL) {
      prev->Connect(prev, new StreamBucket(prev->ID + 1));
    } else if (prev->next->ID - prev->ID > 1) {
      StreamBucket *temp = (StreamBucket *) prev->next;
      StreamBucket *add = new StreamBucket(prev->ID + 1);
      prev->Connect(prev, add);
      add->Connect(add, temp);
    }

    Counter2Bucket(counter, (StreamBucket *) prev->next);
  }

  // add counter to bucket
  void Counter2Bucket(StreamCounter *counter, StreamBucket *bucket) {
    counter->prev = NULL;
    counter->parent = bucket;
    counter->Connect(counter, bucket->child);
    bucket->child = counter;
  }
};