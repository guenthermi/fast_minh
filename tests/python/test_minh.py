from typing import List

import numpy as np
import pytest
from datasketch import MinHash

from fast_minh import HashFamily, minh


@pytest.mark.parametrize(
    'a, b, inputs',
    [
        ([1], [0], ['Token']),
        ([1], [0], ['A', 'set', 'of', 'multiple', 'tokens']),
        ([2, 1], [1, 0], ['Token']),
    ],
)
def test_minh2datasketch(a: List[int], b: List[int], inputs: List[str]):
    m = MinHash(num_perm=len(a), permutations=(np.array(a), np.array(b)))
    for t in inputs:
        m.update(t.encode('utf-8'))
    v_datasketch = m.digest()
    v_fm = minh(inputs, a, b)
    assert len(v_datasketch) == len(v_fm)


@pytest.mark.parametrize(
    'a, b, inputs',
    [
        ([1], [0], ['Token']),
        ([1], [0], ['A', 'set', 'of', 'multiple', 'tokens']),
        ([2, 1], [1, 0], ['Token']),
    ],
)
def test_hash_family2minh(a: List[int], b: List[int], inputs: List[str]):
    hf = HashFamily(a=a, b=b)
    hf_out = hf.minh(inputs)
    mh_out = minh(inputs, a, b)
    for h, m in zip(hf_out, mh_out):
        assert hf_out == mh_out


@pytest.mark.parametrize(
    'inputs, num_perm',
    [
        (['Token'], 20),
        (['A', 'set', 'of', 'multiple', 'tokens'], 30),
        (['Token'], 1_000),
    ],
)
def test_hash_family_auto_init(inputs, num_perm):
    hf = HashFamily(num_perm=num_perm)
    hf_out = hf.minh(inputs)
    assert len(hf_out) == num_perm


@pytest.mark.parametrize(
    'inputs, num_perm', [([['Token'], ['A', 'set', 'of', 'multiple', 'tokens']], 20)]
)
def test_hash_batch(inputs, num_perm):
    hf = HashFamily(num_perm=num_perm)
    hf_out_single = [hf.minh(i) for i in inputs]
    hf_out_batch = hf.minh_batch(inputs)
    for i in range(len(inputs)):
        for j in range(num_perm):
            assert hf_out_single[i][j] == hf_out_batch[i][j]
