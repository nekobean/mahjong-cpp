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
        ge3 = (hand[27] >= 3) + (hand[28] >= 3) + (hand[29] >= 3) + (hand[30] >= 3)
        eq2 = (hand[27] == 2) + (hand[28] == 2) + (hand[29] == 2) + (hand[30] == 2)
        is_established = eq2 == 1 and ge3 == 3

        flatten = flatten_tile34(hand)
        winning_tile = np.random.choice(flatten)
        cases.append((flatten, winning_tile, is_established))

with open(TESTCASE_DIR / "test_score_syosusi.txt", "w") as f:
    for hand, winning_tile, is_established in cases:
        hand_str = " ".join(str(x) for x in hand)
        f.write(f"{hand_str} {winning_tile} {int(is_established)}\n")
