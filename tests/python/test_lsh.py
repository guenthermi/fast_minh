import fast_minh


def test_lsh():
    lsh = fast_minh.LshIndex(1, 3)
    input_key = 'Key'
    input_set = ['A', 'set', 'of', 'multiple', 'tokens']
    lsh.insert(input_key, input_set)
    out = lsh.find(input_set)
    assert len(out) == 1
    assert out[0] == input_key
