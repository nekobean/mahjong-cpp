import numpy as np

from mahjong import *

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


def encode_hand(hand):
    manzu, pinzu, sozu, zihai = 0, 0, 0, 0
    aka_manzu5, aka_pinzu5, aka_sozu5 = 0, 0, 0

    for tile in hand:
        if tile == Tile.AkaManzu5:
            aka_manzu5 = 1
            tile = Tile.Manzu5
        elif tile == Tile.AkaPinzu5:
            aka_pinzu5 = 1
            tile = Tile.Pinzu5
        elif tile == Tile.AkaSozu5:
            aka_sozu5 = 1
            tile = Tile.Sozu5

        if tile <= Tile.Manzu9:
            manzu += Hai1[tile]
        elif tile <= Tile.Pinzu9:
            pinzu += Hai1[tile]
        elif tile <= Tile.Sozu9:
            sozu += Hai1[tile]
        else:
            zihai += Hai1[tile]

    return manzu, pinzu, sozu, zihai, aka_manzu5, aka_pinzu5, aka_sozu5


with open(TESTCASE_DIR / "testcase_hand_constractor.txt", "w") as f:
    N = 10000
    for i in range(N):
        hand = create_hand(yama, 14)
        encoded = encode_hand(hand)

        s = " ".join(map(str, hand)) + " " + " ".join(map(str, encoded))
        f.write(s + "\n")
