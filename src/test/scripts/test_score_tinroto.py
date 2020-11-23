import numpy as np
from mahjong.shanten import Shanten
from itertools import combinations

from utils import *

np.random.seed(0)


def is_tinroto(hand):
    return all([x in [0, 8, 9, 17, 18, 26] for x in key])


shanten = Shanten()

cases = []
for key in combinations(range(34), 5):
    hand = [0] * 34
    for tile in key[:4]:
        hand[tile] = 3
    hand[key[4]] = 2

    flatten = flatten_tile34(hand)
    win_tile = np.random.choice(flatten)
    is_established = is_tinroto(hand)
    cases.append((flatten, win_tile, is_established))

with open(TESTCASE_DIR / "test_score_tinroto.txt", "w") as f:
    for hand, win_tile, is_established in cases:
        hand_str = " ".join(str(x) for x in hand)
        f.write(f"{hand_str} {win_tile} {int(is_established)}\n")
