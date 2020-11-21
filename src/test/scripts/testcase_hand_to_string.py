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


def create_string(hand):
    hand = sort_hand(hand)

    manzu = ""
    pinzu = ""
    sozu = ""
    zihai = ""

    for tile in hand:
        if tile == Tile.AkaManzu5:
            manzu += "r5"
        elif tile == Tile.AkaPinzu5:
            pinzu += "r5"
        elif tile == Tile.AkaSozu5:
            sozu += "r5"
        elif tile <= Tile.Manzu9:
            manzu += str(tile + 1)
        elif tile <= Tile.Pinzu9:
            pinzu += str(tile - 8)
        elif tile <= Tile.Sozu9:
            sozu += str(tile - 17)
        else:
            zihai += KanjiNames[tile]

    manzu += "m" if manzu else ""
    pinzu += "p" if pinzu else ""
    sozu += "s" if sozu else ""

    return " ".join([x for x in [manzu, pinzu, sozu, zihai] if x])


with open(TESTCASE_DIR / "testcase_hand_to_string.txt", "w", encoding="utf-8") as f:
    N = 10000
    for i in range(N):
        hand = create_hand(yama, 14)
        hand_str = create_string(hand)

        s = '{} "{}"'.format(" ".join(map(str, hand)), hand_str)
        f.write(s + "\n")
