import numpy as np

from utils import *

np.random.seed(0)

# 牌山を作成する。
yama = np.arange(34).repeat(4)
yama[np.where(yama == Tile.Manzu5)[0][0]] = Tile.AkaManzu5  # 赤萬子5
yama[np.where(yama == Tile.Pinzu5)[0][0]] = Tile.AkaPinzu5  # 赤筒子5
yama[np.where(yama == Tile.Sozu5)[0][0]] = Tile.AkaSozu5  # 赤索子5


def create_hand(yama, n):
    yama = yama.copy()

    # 先牌する。
    np.random.shuffle(yama)

    # 配牌を作成する。
    return sorted(yama[:n])


def count_hand(hand):
    count = [0] * 37

    for tile in hand:
        tile = aka2normal(tile)
        count[tile] += 1

    count[Tile.AkaManzu5] = count[Tile.Manzu5]
    count[Tile.AkaPinzu5] = count[Tile.Pinzu5]
    count[Tile.AkaSozu5] = count[Tile.Sozu5]

    return count, sum(count[:34])


with open(TESTCASE_DIR / "testcase_hand_num_tiles.txt", "w") as f:
    N = 10000
    for i in range(N):
        hand = create_hand(yama, 14)
        count, n_tiles = count_hand(hand)

        s = "{} {} {}".format(
            " ".join(map(str, hand)), " ".join(map(str, count)), n_tiles
        )
        f.write(s + "\n")
