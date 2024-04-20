#include "fast_minh.h"
#include "MurmurHash3.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>

using namespace std;

const int OUTPUT_HASH_SIZE = 4;
const int HASH_SIZE = 16;
const int64_t MERSENNE_PRIME = (1L << 61) - 1L;
const int64_t INT_MOD = UINT_MAX + 1L;
const int SEED = 1;

// min hash function
extern "C" void mhash(const char **inputs, const int inputs_len,
                      const int64_t *a, const int64_t *b, const int num_perm,
                      uint32_t *minh) {
  unsigned char h[HASH_SIZE];
  unsigned char h32[OUTPUT_HASH_SIZE];
  uint64_t candidate;
  fill_n(minh, num_perm, UINT_MAX);
  for (int j = 0; j < num_perm; j++) {
    for (int i = 0; i < inputs_len; i++) {
      MurmurHash3_x64_128((unsigned char *)inputs[i], SEED, strlen(inputs[i]),
                          h);
      copy(h, h + OUTPUT_HASH_SIZE, h32);
      candidate = (((uint64_t)(*((uint32_t *)h32))) * a[j] + b[j]) % INT_MOD;
      if (candidate < minh[j]) {
        minh[j] = candidate;
      }
    }
  }
}

extern "C" void mhash_batch(const char ***inputs, const int num_inputs,
                            const int *inputs_len, const int64_t *a,
                            const int64_t *b, const int num_perm,
                            uint32_t *minh) {
  for (int i = 0; i < num_inputs; i++) {
    mhash(inputs[i], inputs_len[i], a, b, num_perm, minh + (num_perm * i));
  }
}

StorageHash::StorageHash(int s) : s{s} {};

size_t StorageHash::operator()(const uint32_t *value) const {
  size_t hash = value[0];
  for (int i = 1; i < s; i++) {
    hash ^= value[i] + 0x9e3779b9 + (hash << 6) + (hash >> 2);
  }
  return hash;
}

StorageKeyEqual::StorageKeyEqual(int s) : s{s} {};

bool StorageKeyEqual::operator()(const uint32_t *const &lhs,
                                 const uint32_t *const &rhs) const {
  for (int i = 0; i < s; i++) {
    if (lhs[i] != rhs[i]) {
      return false;
    }
  }
  return true;
}

LshIndex::LshIndex(const int l, const int k, const int64_t *a, const int64_t *b,
                   const int num_perm)
    : l{l}, k{k}, a{a}, b{b}, num_perm{num_perm}, hash_func{StorageHash(l)},
      equal_func{StorageKeyEqual(l)} {
  storage = new unordered_map<uint32_t *, unordered_set<string>, StorageHash,
                              StorageKeyEqual>[k];
  for (int i = 0; i < k; i++) {
    storage[i] = unordered_map<uint32_t *, unordered_set<string>, StorageHash,
                               StorageKeyEqual>(1, hash_func, equal_func);
  }
  query_hash = new uint32_t[num_perm];
}
LshIndex::~LshIndex() {
  for (auto &it : hash_vector) {
    delete[] it;
  }
  delete[] storage;
  delete[] query_hash;
}

void LshIndex::insert_set(const char *key, const char **inputs,
                          const int inputs_len) {
  uint32_t *min_hash = new uint32_t[num_perm];
  mhash(inputs, inputs_len, a, b, num_perm, min_hash);
  insert_hash(key, min_hash);
  hash_vector.push_back(min_hash);
}

void LshIndex::insert_hash(const char *key, uint32_t *minh) {

  if (keys.find(key) != keys.end()) {
    stringstream error_message;
    error_message << "Key \"" << key << "\" already exists in the index!";
    throw invalid_argument(error_message.str());
  }

  keys[string(key)] = minh;

  for (int i = 0; i < k; i++) {
    storage[i][(minh + (i * l))].insert(string(key));
  }
}

vector<const char *> &LshIndex::find_keys_from_hash(uint32_t *minh) {
  output_buffer.clear();
  output_set.clear();
  for (int i = 0; i < k; i++) {
    for (auto &s : storage[i][minh + (i * l)]) {
      if (output_set.find(s) == output_set.end()) {
        output_buffer.push_back(s.c_str());
        output_set.insert(s);
      }
    }
  }
  return output_buffer;
}

vector<const char *> &LshIndex::find_keys(const char **inputs,
                                          const int inputs_len) {
  mhash(inputs, inputs_len, a, b, num_perm, query_hash);
  return find_keys_from_hash(query_hash);
}

uint32_t *LshIndex::get_minh(char *key) { return keys[string(key)]; }

extern "C" void get_lsh_index(const int l, const int k, const int64_t *a,
                              const int64_t *b, const int num_perm,
                              void *&index) {
  index = new LshIndex(l, k, a, b, num_perm);
}

extern "C" void delete_lsh_index(LshIndex *index) { delete index; }

extern "C" void insert_key(LshIndex *index, char *key, const char **inputs,
                           const int inputs_len) {
  index->insert_set(key, inputs, inputs_len);
}

extern "C" void get_keys(LshIndex *index, const char **inputs,
                         const int inputs_len, const char **&result,
                         int &size) {
  vector<const char *> &buffer = index->find_keys(inputs, inputs_len);
  result = &buffer[0];
  size = buffer.size();
}
