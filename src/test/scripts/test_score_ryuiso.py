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
for key in tqdm(product(range(5), repeat=9), total=5 ** 9):
    if sum(key) == 14:
        hand = [0] * 18 + list(key) + [0] * 7  # 索子14枚
    elif sum(key) == 12:
        hand = [0] * 18 + list(key) + [0, 0, 0, 0, 0, 2, 0]  # 索子12枚 + 發2枚
    elif sum(key) == 11:
        hand = [0] * 18 + list(key) + [0, 0, 0, 0, 0, 3, 0]  # 索子11枚 + 發3枚
    else:
        continue

    if shanten.calculate_shanten(hand) == -1:
        # 和了形の場合
        tiles = flatten_tile34(hand)
        win_tile = np.random.choice(tiles)
        is_established = check_ryuiso(tiles)

        cases.append((tiles, win_tile, is_established))

with open(TESTCASE_DIR / "test_score_ryuiso.txt", "w") as f:
    for hand, win_tile, is_established in cases:
        hand_str = " ".join(str(x) for x in hand)
        f.write(f"{hand_str} {win_tile} {int(is_established)}\n")
