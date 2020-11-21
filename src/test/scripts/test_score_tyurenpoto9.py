import numpy as np
from mahjong.shanten import Shanten
from tqdm import tqdm
from itertools import product

from utils import *

np.random.seed(0)


def check_ryurenpoto(key):
    return (
        key[0] >= 3
        and key[1] >= 1
        and key[2] >= 1
        and key[3] >= 1
        and key[4] >= 1
        and key[5] >= 1
        and key[6] >= 1
        and key[7] >= 1
        and key[8] >= 3
    )


def is_ryurenpoto9(key):
    return (
        key[0] == 3
        and key[1] == 1
        and key[2] == 1
        and key[3] == 1
        and key[4] == 1
        and key[5] == 1
        and key[6] == 1
        and key[7] == 1
        and key[8] == 3
    )


shanten = Shanten()

cases = []

tyuren_keys = []
for key in tqdm(product(range(5), repeat=9), total=5 ** 9):
    if sum(key) == 14 and check_ryurenpoto(key):
        tyuren_keys.append(list(key))

cases = []
for key in tyuren_keys:
    manzu = list(key) + [0] * 25
    pinzu = [0] * 9 + list(key) + [0] * 16
    sozu = [0] * 18 + list(key) + [0] * 7
    for winning_key in range(9):
        key[winning_key] -= 1
        is_established = is_ryurenpoto9(key)
        key[winning_key] += 1

        flatten = flatten_tile34(manzu)
        cases.append((flatten, winning_key, is_established))

        flatten = flatten_tile34(pinzu)
        cases.append((flatten, winning_key + 9, is_established))

        flatten = flatten_tile34(sozu)
        cases.append((flatten, winning_key + 18, is_established))


with open(TESTCASE_DIR / "test_score_tyurenpoto9.txt", "w") as f:
    for hand, winning_tile, is_established in cases:
        hand_str = " ".join(str(x) for x in hand)
        f.write(f"{hand_str} {winning_tile} {int(is_established)}\n")
