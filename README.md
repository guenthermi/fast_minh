# fast_minh

A small Python package for calculating MinHash values, computing approaximated Jaccard similarity, and building LSH indices of MinHash values to perform fast approximated set similarity search.

## Installation

```bash
pip install .
```

## Usage

**Calculating MinHash values:**

To calculate min hash values, you can create a `HashFamily` object which initializes a set of hash fuctions (default: 128).
After that you can use the `HashFamily.minh` function to obtain a set of MinHash values for a given set of strings:

```python
hf = HashFamily()
mh = hf.minh(['test', 'it', 'out'])
```

**Calculate an approximated Jaccard coefficient:**

After calculating multiple minhash values for different sets with the same hash family, you can use the `jaccard` function to determine and approximated similarity score:

```python
from fast_minh import minh, jaccard
hf = HashFamily()
mh1 = hf.minh(['test', 'it', 'out'])
mh2 = hf.minh(['test', 'it', 'again'])
sim = jaccard(mh1, mh2)
```

**MinHash LSH Index:**

To find similar sets of text values fast, you can use an MinHash LSH index.
You can insert sets with the `LshIndex.insert` function and retrieve similar candidates with the `LshIndex.find` method:

```python
from fast_minh import LshIndex
lsh = LshIndex(1, 3)
input_key = 'Key'
input_set = ['A', 'set', 'of', 'multiple', 'tokens']
lsh.insert(input_key, input_set)
out = lsh.find(input_set)
```
