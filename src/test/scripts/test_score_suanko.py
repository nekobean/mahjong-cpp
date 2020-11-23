import numpy as np
from mahjong.shanten import Shanten
from tqdm import tqdm
from itertools import product

from utils import *

np.random.seed(0)


def is_suanko(hand):
    return sum([x == 3 for x in key]) == 4


shanten = Shanten()

cases = []
for key in tqdm(product(range(5), repeat=9), total=5 ** 9):
    if sum(key) != 14:
        continue

    hand = list(key) + [0] * 25

    if shanten.calculate_shanten(hand) == -1:
        flatten = flatten_tile34(hand)
        win_tile = np.random.choice(flatten)
        is_established = is_suanko(hand)
        cases.append((flatten, win_tile, is_established))

with open(TESTCASE_DIR / "test_score_suanko.txt", "w") as f:
    for hand, win_tile, is_established in cases:
        hand_str = " ".join(str(x) for x in hand)
        f.write(f"{hand_str} {win_tile} {int(is_established)}\n")
