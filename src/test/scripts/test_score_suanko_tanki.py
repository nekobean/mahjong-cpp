import numpy as np
from mahjong.shanten import Shanten
from tqdm import tqdm
from itertools import product

from utils import *

np.random.seed(0)


def is_suanko_tanki(hand, win_tile):
    return sum([x == 3 for x in key]) == 4 and hand[win_tile] == 2


shanten = Shanten()

cases = []
for key in tqdm(product(range(5), repeat=9), total=5 ** 9):
    if sum(key) != 14:
        continue

    hand = list(key) + [0] * 25

    if shanten.calculate_shanten(hand) == -1:
        for win_tile in np.unique(key):
            flatten = flatten_tile34(hand)
            is_established = is_suanko_tanki(hand, win_tile)
            cases.append((flatten, win_tile, is_established))

with open(TESTCASE_DIR / "test_score_suanko_tanki.txt", "w") as f:
    for hand, win_tile, is_established in cases:
        hand_str = " ".join(str(x) for x in hand)
        f.write(f"{hand_str} {win_tile} {int(is_established)}\n")
