import numpy as np
from mahjong.shanten import Shanten
from tqdm import tqdm
from itertools import combinations_with_replacement, permutations

from utils import *

np.random.seed(0)
shanten = Shanten()
cases = []

# 風牌のパターン
kazehai = []
for p in combinations_with_replacement([3, 2, 0], 4):
    for p2 in permutations(p):
        kazehai.append(p2)
kazehai = list(set(kazehai))

for key in kazehai:
    hand = [0] * 34
    if any([x == 2 for x in key]):
        # 風牌の雀頭がある場合
        for i in range(14 - sum(key)):
            hand[i] = 1
    else:
        # 風牌の雀頭がない場合
        for i in range(12 - sum(key)):
            hand[i] = 1
        hand[25] = 2

    for i in range(27, 31):
        hand[i] = key[i - 27]

    if shanten.calculate_shanten(hand) == -1:
        is_established = len([hand[i] for i in range(27, 31) if hand[i] == 3]) == 4

        flatten = flatten_tile34(hand)
        win_tile = np.random.choice(flatten)
        cases.append((flatten, win_tile, is_established))

with open(TESTCASE_DIR / "test_score_daisusi.txt", "w") as f:
    for hand, win_tile, is_established in cases:
        hand_str = " ".join(str(x) for x in hand)
        f.write(f"{hand_str} {win_tile} {int(is_established)}\n")
