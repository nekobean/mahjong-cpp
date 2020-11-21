from pathlib import Path


class Yaku:
    Null = 0
    Tumo = 1
    Reach = 1 << 1
    Ippatu = 1 << 2
    Tanyao = 1 << 3
    Pinhu = 1 << 4
    Ipeko = 1 << 5
    Tyankan = 1 << 6
    Rinsyankaiho = 1 << 7
    Haiteitumo = 1 << 8
    Hoteiron = 1 << 9
    Dora = 1 << 10
    UraDora = 1 << 11
    AkaDora = 1 << 12
    SangenhaiHaku = 1 << 13
    SangenhaiHatu = 1 << 14
    SangenhaiTyun = 1 << 15
    ZikazeTon = 1 << 16
    ZikazeNan = 1 << 17
    ZikazeSya = 1 << 18
    ZikazePe = 1 << 19
    BakazeTon = 1 << 20
    BakazeNan = 1 << 21
    BakazeSya = 1 << 22
    BakazePe = 1 << 23
    DoubleReach = 1 << 24
    Tiitoitu = 1 << 25
    Toitoiho = 1 << 26
    Sananko = 1 << 27
    SansyokuDoko = 1 << 28
    SansyokuDozyun = 1 << 29
    Honroto = 1 << 30
    IkkiTukan = 1 << 31
    Tyanta = 1 << 32
    Syosangen = 1 << 33
    Sankantu = 1 << 34
    Honiso = 1 << 35
    Zyuntyanta = 1 << 36
    Ryanpeko = 1 << 37
    NagasiMangan = 1 << 38
    Tiniso = 1 << 39
    Tenho = 1 << 40
    Tiho = 1 << 41
    Renho = 1 << 42
    Ryuiso = 1 << 43
    Daisangen = 1 << 44
    Syosusi = 1 << 45
    Tuiso = 1 << 46
    Kokusimuso = 1 << 47
    Tyurenpoto = 1 << 48
    Suanko = 1 << 49
    Tinroto = 1 << 50
    Sukantu = 1 << 51
    SuankoTanki = 1 << 52
    Daisusi = 1 << 53
    Tyurenpoto9 = 1 << 54
    Kokusimuso13 = 1 << 55


class MeldType:
    Null = -1
    Pon = 0
    Ti = 1
    Ankan = 2
    Minkan = 3
    Kakan = 4


class SeatType:
    Null = -1
    Zitya = 0
    Kamitya = 1
    Toimen = 2
    Simotya = 3


class ScoreTitle:
    Null = -1
    Mangan = 0
    Haneman = 1
    Baiman = 2
    Sanbaiman = 3
    KazoeYakuman = 4
    Yakuman = 5
    TwoYakuman = 6
    ThreeYakuman = 7
    FourYakuman = 8
    FiveYakuman = 9
    SixYakuman = 10


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


def flatten_tile34(tiles):
    tiles = sum([[i] * cnt for i, cnt in enumerate(tiles)], [])

    return tiles


DoraHyozi2Dora = {
    0: 1,
    1: 2,
    2: 3,
    3: 4,
    4: 5,
    5: 6,
    6: 7,
    7: 8,
    8: 0,
    9: 10,
    10: 11,
    11: 12,
    12: 13,
    13: 14,
    14: 15,
    15: 16,
    16: 17,
    17: 9,
    18: 19,
    19: 20,
    20: 21,
    21: 22,
    22: 23,
    23: 24,
    24: 25,
    25: 26,
    26: 18,
    27: 28,
    28: 29,
    29: 30,
    30: 27,
    31: 32,
    32: 33,
    33: 31,
    34: 5,
    35: 14,
    36: 23,
}
