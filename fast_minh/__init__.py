import ctypes
import os
import pathlib
import random
from typing import List, Optional

from fast_minh.utils.estimate import optimal_param

#
if os.path.exists(pathlib.Path().absolute() / 'build'):
    libname = pathlib.Path().absolute() / 'build' / 'libfast_minh.so'
else:
    libname = pathlib.Path(__file__).parent / 'libfast_minh.so'
c_lib = ctypes.CDLL(libname)


def prepare_text_set(inputs: List[str]):
    input_type = ctypes.c_char_p * len(inputs)
    return input_type(*(ctypes.c_char_p(s.encode('utf-8')) for s in inputs))


def prepare_text_set_batch(inputs: List[List[str]]):
    batch_input_type = ctypes.c_void_p * len(inputs)
    return batch_input_type(
        *(
            ctypes.cast(prepare_text_set(input_set), ctypes.c_void_p)
            for input_set in inputs
        )
    )


class HashFamily:
    def __init__(
        self,
        a: Optional[List[int]] = None,
        b: Optional[List[int]] = None,
        num_perm: int = 128,
    ):
        if a or b:
            num_perm = len(a or b)
        self._num_perm = ctypes.c_int(num_perm)
        self._permutation_type = ctypes.c_int64 * num_perm
        self._output_type = ctypes.c_uint32 * num_perm
        self._c_output = self._output_type(*([0] * num_perm))
        if a and b:
            if len(a) != len(b):
                raise ValueError(f'len(a)={len(a)} must be equal to len(b)={len(b)}')
            self._a = self._permutation_type(*a)
            self._b = self._permutation_type(*b)
        else:
            self._a = a or self._permutation_type(
                *(random.randint(0, 2 ** 32 - 1) for x in range(num_perm))
            )
            self._b = b or self._permutation_type(
                *(random.randint(0, 2 ** 32 - 1) for x in range(num_perm))
            )

    @property
    def a(self):
        return self._a.value

    @property
    def b(self):
        return self._b.value

    @property
    def num_perm(self) -> List[int]:
        return self._num_perm.value

    def minh(self, inputs: List[str]) -> List[int]:
        prepared_inputs = prepare_text_set(inputs)
        c_lib.mhash(
            prepared_inputs,
            ctypes.c_int(len(inputs)),
            self._a,
            self._b,
            self._num_perm,
            self._c_output,
        )  # stores result  in self._c_output
        return list(self._c_output)

    def minh_batch(self, inputs: List[List[str]]) -> List[List[int]]:
        prepared_inputs = prepare_text_set_batch(inputs)
        input_lengths_type = ctypes.c_int * len(inputs)
        batch_output_type = self._output_type * len(inputs)
        outputs_batch = batch_output_type(
            *(self._output_type(*([0] * self._num_perm.value)) for i in inputs)
        )
        c_lib.mhash_batch(
            prepared_inputs,
            ctypes.c_int(len(inputs)),
            input_lengths_type(*(len(x) for x in inputs)),
            self._a,
            self._b,
            self._num_perm,
            outputs_batch,
        )
        return [list(x) for x in outputs_batch]


class LshIndex:
    def __init__(
        self,
        k: int,
        d: int,
        a: Optional[List[int]] = None,
        b: Optional[List[int]] = None,
        num_perm: int = 128,
    ):
        """
        LSH index for minhash values.

        :param k: Number of hash tables
        :param d: Length of keys in hash tables
        :param a: Multiplier for permuations
        :param b: Bias for permutations
        :num_perm: Number of permutations
        """
        self.hf = HashFamily(a, b, num_perm)
        self.lsh = ctypes.c_void_p()
        c_lib.get_lsh_index(
            ctypes.c_int(k),
            ctypes.c_int(d),
            self.hf._a,
            self.hf._b,
            self.hf._num_perm,
            ctypes.byref(self.lsh),
        )

    def __del__(self):
        c_lib.delete_lsh_index(self.lsh)

    def insert(self, key: str, inputs: List[str]):  #
        prepared_inputs = prepare_text_set(inputs)
        encoded_key = ctypes.c_char_p(key.encode('utf-8'))
        c_lib.insert_key(
            self.lsh, encoded_key, prepared_inputs, ctypes.c_int(len(inputs))
        )

    def find(self, inputs: List[str]) -> List[str]:
        prepared_inputs = prepare_text_set(inputs)
        output = ctypes.c_void_p()
        output_len = ctypes.c_int()
        c_lib.get_keys(
            self.lsh,
            prepared_inputs,
            ctypes.c_int(len(inputs)),
            ctypes.byref(output),
            ctypes.byref(output_len),
        )
        if output_len.value == 0:
            return []
        else:
            return_type = ctypes.c_char_p * output_len.value
            return_list = ctypes.cast(output, ctypes.POINTER(return_type))[0]
            return [s.decode('utf-8') for s in return_list]


def get_lsh(
    threshold: float = 0.9,
    false_positive_weight: float = 0.5,
    false_negative_weight: float = 0.5,
    num_perm: int = 128,
):
    """
    Creates an LshIndex object with the optimal parameters for k and d.

    :param threshold: Jaccard similarity threshold
    :param false_positive_weight: Weight for false positive
    :param false_negative_weight: Weight for false negative
    :param num_perm: Number of permutations
    :return: LshIndex object
    """
    d, k = optimal_param(
        threshold, num_perm, false_positive_weight, false_negative_weight
    )
    return LshIndex(k, d, num_perm=num_perm)


def minh(inputs: List[str], a: List[int], b: List[int]):
    permutation_type = ctypes.c_int64 * len(a)
    output_type = ctypes.c_uint32 * len(a)
    prepared_inputs = prepare_text_set(inputs)
    num_perm = ctypes.c_int(len(a))
    a = permutation_type(*a)
    b = permutation_type(*b)
    output = output_type(*([0] * len(a)))
    c_lib.mhash(prepared_inputs, ctypes.c_int(len(inputs)), a, b, num_perm, output)
    return list(output)


def jaccard(minh1: List[int], minh2: List[int]):
    count = 0
    for a, b in zip(minh1, minh2):
        if a == b:
            count += 1
    return count / len(minh1)


if __name__ == '__main__':
    inputs = ['Hello', 'World', 'What', 'is', 'going', 'up']
    a = [1]
    b = [0]
    print('here', minh(inputs, a, b)[0])
