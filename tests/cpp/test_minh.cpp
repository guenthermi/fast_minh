#include "fast_minh.h"
#include <algorithm>
#include <gtest/gtest.h>
#include <iostream>

#define GTEST_COUT std::cerr << "[          ] [ INFO ] "

struct LshParams {
  int64_t *a;
  int64_t *b;
  int l;
  int k;
  int num_perm;
};

int64_t A[] = {1};
int64_t B[] = {0};
const int NUM_PERM = 1;

const char *INPUTS_1[] = {"A", "set", "of", "multiple", "tokens"};
const char *INPUTS_2[] = {"Token"};
const char **MULTI_INPUTS[] = {INPUTS_1, INPUTS_2};
const int MULTI_INPUTS_LEN[] = {5, 1};

class LshTestF : public ::testing::TestWithParam<int> {

protected:
  LshParams create_lsh_param_multi_perm(int k, int l, const int num_perm) {
    int64_t *a = new int64_t[num_perm];
    int64_t *b = new int64_t[num_perm];
    for (int i = 0; i < num_perm; i++) {
      a[i] = rand();
      b[i] = rand();
    }
    return {a, b, 1, 3, num_perm};
  }

  void SetUp() override {
    lsh_params[0] = {A, B, 1, 1, NUM_PERM};
    lsh_params[1] = create_lsh_param_multi_perm(1, 3, 128);
  }

  void TearDown() override {
    delete[] lsh_params[1].a;
    delete[] lsh_params[1].b;
  }

  LshParams lsh_params[2];
};

TEST(MHashTest, SimpleMHash) {

  uint32_t min_hash = 0;

  mhash(INPUTS_1, 3, A, B, NUM_PERM, &min_hash);

  EXPECT_NE(min_hash, 0);
}

TEST(MHashTest, BatchMHash) {
  uint32_t *min_hashs = new uint32_t[2];
  std::fill_n(min_hashs, 2, 0);
  mhash_batch(MULTI_INPUTS, 2, MULTI_INPUTS_LEN, A, B, 1, min_hashs);
  EXPECT_NE(min_hashs[0], 0);
  EXPECT_NE(min_hashs[1], 0);
  delete[] min_hashs;
}

TEST_P(LshTestF, CreateLshIndex) {

  LshParams params = lsh_params[GetParam()];

  LshIndex index(params.l, params.k, params.a, params.b, params.num_perm);
  EXPECT_EQ(index.a, params.a);
  EXPECT_EQ(index.b, params.b);
  EXPECT_EQ(index.num_perm, params.num_perm);
}

TEST_P(LshTestF, GetLshIndex) {
  LshParams params = lsh_params[GetParam()];
  void *ref;
  get_lsh_index(params.l, params.k, params.a, params.b, params.num_perm, ref);
  LshIndex *index = static_cast<LshIndex *>(ref);
  EXPECT_EQ(index->a, params.a);
  EXPECT_EQ(index->b, params.b);
  EXPECT_EQ(index->num_perm, params.num_perm);
  delete index;
}

TEST_P(LshTestF, InsertSet) {
  const char *key = "Key";
  LshParams params = lsh_params[0];
  LshIndex index(params.l, params.k, params.a, params.b, params.num_perm);
  index.insert_set(key, INPUTS_1, 5);
  uint32_t minh = 0;
  mhash(INPUTS_1, 5, params.a, params.b, params.num_perm, &minh);
  vector<const char *> res = index.find_keys_from_hash(&minh);
  EXPECT_EQ(res.size(), 1);
}

TEST_P(LshTestF, FindKeys) {
  LshParams params = lsh_params[GetParam()];
  LshIndex index(params.l, params.k, params.a, params.b, params.num_perm);
  index.insert_set("Key", INPUTS_1, 5);
  vector<const char *> res = index.find_keys(INPUTS_1, 5);
  EXPECT_GT(res.size(), 0);
}

TEST_P(LshTestF, get_keys) {
  LshParams params = lsh_params[GetParam()];
  LshIndex index(params.l, params.k, params.a, params.b, params.num_perm);
  index.insert_set("Key", INPUTS_1, 5);
  const char **result = nullptr;
  int result_size = 0;
  get_keys(&index, INPUTS_1, 5, result, result_size);
  EXPECT_GT(result_size, 0);
}

INSTANTIATE_TEST_CASE_P(LshTest, LshTestF, ::testing::Values(0, 1));
