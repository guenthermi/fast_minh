"""
Utility functions to estimate configuration paramters for the LSH index

Source: https://github.com/ekzhu/datasketch/blob/master/datasketch/lsh.py
Commit ID: ab92cbfa05fd65d51dc4826937d8f07042a4bebd
"""

from scipy.integrate import quad as integrate


def _false_positive_probability(threshold, b, r):
    _probability = lambda s: 1 - (1 - s ** float(r)) ** float(b)
    a, err = integrate(_probability, 0.0, threshold)
    return a


def _false_negative_probability(threshold, b, r):
    _probability = lambda s: 1 - (1 - (1 - s ** float(r)) ** float(b))
    a, err = integrate(_probability, threshold, 1.0)
    return a


def optimal_param(threshold, num_perm, false_positive_weight, false_negative_weight):
    """
    Compute the optimal `MinHashLSH` parameter that minimizes the weighted sum
    of probabilities of false positive and false negative.
    """
    min_error = float("inf")
    opt = (0, 0)
    for b in range(1, num_perm + 1):
        max_r = int(num_perm / b)
        for r in range(1, max_r + 1):
            fp = _false_positive_probability(threshold, b, r)
            fn = _false_negative_probability(threshold, b, r)
            error = fp * false_positive_weight + fn * false_negative_weight
            if error < min_error:
                min_error = error
                opt = (b, r)
    return opt