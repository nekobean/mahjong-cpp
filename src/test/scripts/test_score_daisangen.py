import numpy as np
from mahjong.shanten import Shanten
from tqdm import tqdm
from itertools import product

from utils import *

np.random.seed(0)


def check_ryuiso(hand):
    return np.isin(hand, [19, 20, 21, 23, 25, 32]).all()


shanten = Shanten()

cases = []

# 大三元
for key in tqdm(product(range(5), repeat=9), total=5 ** 9):
    if sum(key) == 5:
        tiles = list(key) + [0] * 22 + [3, 3, 3]
    else:
        continue

    if shanten.calculate_shanten(tiles) == -1:
        tiles = flatten_tile34(tiles)
        win_tile = np.random.choice(tiles)
        cases.append((tiles, win_tile, True))

# 小三元
for key in tqdm(product(range(5), repeat=9), total=5 ** 9):
    if sum(key) == 6:
        tiles = list(key) + [0] * 22 + np.random.permutation([2, 3, 3]).tolist()
    else:
        continue

    if shanten.calculate_shanten(tiles) == -1:
        tiles = flatten_tile34(tiles)
        win_tile = np.random.choice(tiles)
        cases.append((tiles, win_tile, False))

with open(TESTCASE_DIR / "test_score_daisangen.txt", "w") as f:
    for hand, win_tile, is_established in cases:
        hand_str = " ".join(str(x) for x in hand)
        f.write(f"{hand_str} {win_tile} {int(is_established)}\n")
