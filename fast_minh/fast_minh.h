
#ifndef FAST_MINH_H
#define FAST_MINH_H

#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std;

struct StorageHash {
  StorageHash(int s = 1);
  int s;
  size_t operator()(const uint32_t *value) const;
};

struct StorageKeyEqual {
  StorageKeyEqual(int s = 1);
  int s;
  bool operator()(const uint32_t *const &lhs, const uint32_t *const &rhs) const;
};

class LshIndex {
public:
  const int64_t *a;
  const int64_t *b;
  const int num_perm;
  LshIndex(const int l, const int k, const int64_t *a, const int64_t *b,
           const int num_perm);
  ~LshIndex();

  void insert_set(const char *key, const char **inputs, const int inputs_len);
  void insert_hash(const char *key, uint32_t *minh);
  vector<const char *> &find_keys_from_hash(uint32_t *minh);
  vector<const char *> &find_keys(const char **inputs, const int inputs_len);
  uint32_t *get_minh(char *key);

private:
  const int l;
  const int k;
  vector<const char *> output_buffer;
  unordered_set<string> output_set;
  unordered_map<uint32_t *, unordered_set<string>, StorageHash, StorageKeyEqual>
      *storage; // string size l array size k
  vector<uint32_t *>
      hash_vector; // stores all hash value for garbadge collection
  unordered_map<string, uint32_t *>
      keys; // use this if keys are required // string size l*k
  StorageHash hash_func;
  StorageKeyEqual equal_func;
  uint32_t *query_hash;
};

extern "C" void mhash(const char **inputs, const int inputs_len,
                      const int64_t *a, const int64_t *b, const int num_perm,
                      uint32_t *minh);

extern "C" void mhash_batch(const char ***inputs, const int num_inputs,
                            const int *inputs_len, const int64_t *a,
                            const int64_t *b, const int num_perm,
                            uint32_t *minh);

extern "C" void get_lsh_index(const int l, const int k, const int64_t *a,
                              const int64_t *b, const int num_perm,
                              void *&index);

extern "C" void delete_lsh_index(LshIndex *index);

extern "C" void insert_key(LshIndex *index, char *key, const char **inputs,
                           const int inputs_len);

extern "C" void get_keys(LshIndex *index, const char **inputs,
                         const int inputs_len, const char **&result, int &size);

#endif