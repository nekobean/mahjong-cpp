import numpy as np
from mahjong.shanten import Shanten
from tqdm import tqdm
from itertools import combinations_with_replacement, permutations

from utils import *

np.random.seed(0)
shanten = Shanten()
cases = []
kokusi = [0, 8, 9, 17, 18, 26, 27, 28, 29, 30, 31, 32, 33]

for tile in kokusi:
    flatten = kokusi + [tile]
    for winning_tile in flatten:
        is_established = winning_tile == tile
        cases.append((flatten, winning_tile, is_established))

with open(TESTCASE_DIR / "test_score_kokusi13.txt", "w") as f:
    for hand, winning_tile, is_established in cases:
        hand_str = " ".join(str(x) for x in hand)
        f.write(f"{hand_str} {winning_tile} {int(is_established)}\n")
