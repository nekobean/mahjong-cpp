import numpy as np
from mahjong.shanten import Shanten
from itertools import product

from utils import *

shanten = Shanten()

yama = create_yama(0)


def create_hand(yama, n):
    yama = yama.copy()

    # 先牌する。
    np.random.shuffle(yama)

    # 配牌を作成する。
    return sorted(yama[:n])


def to_tile37(tiles):
    tile37 = np.zeros(37, dtype=int)

    for tile in tiles:
        if tile == Tile.AkaManzu5:
            tile37[Tile.Manzu5] += 1
        elif tile == Tile.AkaPinzu5:
            tile37[Tile.Pinzu5] += 1
        elif tile == Tile.AkaSozu5:
            tile37[Tile.Sozu5] += 1

        tile37[tile] += 1

    return tile37


cases = []
i = 0
while True:
    hand = create_hand(yama, 14)
    tile37 = to_tile37(hand)

    syanten = shanten.calculate_shanten(tile37, chiitoitsu=False, kokushi=False)

    if syanten == -1:
        continue  # 和了形
    i += 1
    if i == 100000:
        break

    # unnecessary_tiles = []
    # for tile in np.unique(hand):
    #     tile37_after = tile37.copy()

    #     tile37_after[tile] -= 1
    #     if tile == Tile.AkaManzu5:
    #         tile37_after[Tile.Manzu5] -= 1
    #     elif tile == Tile.AkaPinzu5:
    #         tile37_after[Tile.Pinzu5] -= 1
    #     elif tile == Tile.AkaSozu5:
    #         tile37_after[Tile.Sozu5] -= 1

    #     syanten_after = shanten.calculate_shanten(
    #         tile37_after, chiitoitsu=False, kokushi=False
    #     )

    #     if syanten == syanten_after:
    #         unnecessary_tiles.append(tile)

    cases.append(hand)

with open(TESTCASE_DIR / "test_unnecessary_tile_selector.txt", "w") as f:
    for hand in cases:
        s = " ".join(map(str, hand))
        f.write(s + "\n")
