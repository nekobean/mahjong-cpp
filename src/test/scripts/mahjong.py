from pathlib import Path


class Tile:
    Null = -1
    Manzu1 = 0
    Manzu2 = 1
    Manzu3 = 2
    Manzu4 = 3
    Manzu5 = 4
    Manzu6 = 5
    Manzu7 = 6
    Manzu8 = 7
    Manzu9 = 8
    Pinzu1 = 9
    Pinzu2 = 10
    Pinzu3 = 11
    Pinzu4 = 12
    Pinzu5 = 13
    Pinzu6 = 14
    Pinzu7 = 15
    Pinzu8 = 16
    Pinzu9 = 17
    Sozu1 = 18
    Sozu2 = 19
    Sozu3 = 20
    Sozu4 = 21
    Sozu5 = 22
    Sozu6 = 23
    Sozu7 = 24
    Sozu8 = 25
    Sozu9 = 26
    Ton = 27
    Nan = 28
    Sya = 29
    Pe = 30
    Haku = 31
    Hatu = 32
    Tyun = 33
    AkaManzu5 = 34
    AkaPinzu5 = 35
    AkaSozu5 = 36


Hai1 = [
    1,
    1 << 3,
    1 << 6,
    1 << 9,
    1 << 12,
    1 << 15,
    1 << 18,
    1 << 21,
    1 << 24,
    1,
    1 << 3,
    1 << 6,
    1 << 9,
    1 << 12,
    1 << 15,
    1 << 18,
    1 << 21,
    1 << 24,
    1,
    1 << 3,
    1 << 6,
    1 << 9,
    1 << 12,
    1 << 15,
    1 << 18,
    1 << 21,
    1 << 24,
    1,
    1 << 3,
    1 << 6,
    1 << 9,
    1 << 12,
    1 << 15,
    1 << 18,
]

TESTCASE_DIR = Path(__file__) / "../../../../data/testcase"


def aka2normal(tile):
    if tile <= Tile.Tyun:
        return tile
    elif tile == Tile.AkaManzu5:
        return Tile.Manzu5
    elif tile == Tile.AkaPinzu5:
        return Tile.Pinzu5
    else:
        return Tile.Sozu5


KanjiNames = [
    "一萬",
    "二萬",
    "三萬",
    "四萬",
    "五萬",
    "六萬",
    "七萬",
    "八萬",
    "九萬",
    "一筒",
    "二筒",
    "三筒",
    "四筒",
    "五筒",
    "六筒",
    "七筒",
    "八筒",
    "九筒",
    "一索",
    "二索",
    "三索",
    "四索",
    "五索",
    "六索",
    "七索",
    "八索",
    "九索",
    "東",
    "南",
    "西",
    "北",
    "白",
    "發",
    "中",
    "赤五萬",
    "赤五筒",
    "赤五索",
]

TileOrder = [
    0,  # 萬子1
    1,
    2,
    3,
    5,
    6,
    7,
    8,
    9,
    10,  # 筒子1
    11,
    12,
    13,
    15,
    16,
    17,
    18,
    19,
    20,  # 索子1
    21,
    22,
    23,
    25,
    26,
    27,
    28,
    29,
    30,  # 東
    31,
    32,
    33,
    34,
    35,
    36,
    4,  # 赤五萬
    14,  # 赤五筒
    24,  # 赤五索
]


def sort_hand(hand):
    return sorted(hand, key=lambda x: TileOrder[x])
