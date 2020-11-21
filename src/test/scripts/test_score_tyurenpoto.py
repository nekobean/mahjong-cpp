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


shanten = Shanten()

cases = []

for key in tqdm(product(range(5), repeat=9), total=5 ** 9):
    if sum(key) == 14:
        is_established = check_ryurenpoto(key)
        manzu = list(key) + [0] * 25  # 萬子の清一形
        pinzu = [0] * 9 + list(key) + [0] * 16  # 筒子の清一形
        sozu = [0] * 18 + list(key) + [0] * 7  # 索子の清一形
    else:
        continue

    if shanten.calculate_shanten(manzu) == -1:
        # 萬子の九連宝灯
        flatten = flatten_tile34(manzu)
        winning_tile = np.random.choice(flatten)
        cases.append((flatten, winning_tile, is_established))

        # 筒子の九連宝灯
        flatten = flatten_tile34(pinzu)
        winning_tile = np.random.choice(flatten)
        cases.append((flatten, winning_tile, is_established))

        # 索子の九連宝灯
        flatten = flatten_tile34(sozu)
        winning_tile = np.random.choice(flatten)
        cases.append((flatten, winning_tile, is_established))


with open(TESTCASE_DIR / "test_score_tyurenpoto.txt", "w") as f:
    for hand, winning_tile, is_established in cases:
        hand_str = " ".join(str(x) for x in hand)
        f.write(f"{hand_str} {winning_tile} {int(is_established)}\n")
