from pathlib import Path
import numpy as np
from tqdm import tqdm
from operator import itemgetter
from bs4 import BeautifulSoup
from tqdm.notebook import tqdm

from utils import *

# 牌
tiles134 = np.arange(34).repeat(4)
tiles134[np.where(tiles134 == Tile.Manzu5)[0][0]] = Tile.AkaManzu5  # 赤萬子5
tiles134[np.where(tiles134 == Tile.Pinzu5)[0][0]] = Tile.AkaPinzu5  # 赤筒子5
tiles134[np.where(tiles134 == Tile.Sozu5)[0][0]] = Tile.AkaSozu5  # 赤索子5
ToTile = {i: int(tile) for i, tile in enumerate(tiles134)}

# 役
ToYaku = {
    0: Yaku.Tumo,
    1: Yaku.Reach,
    2: Yaku.Ippatu,
    3: Yaku.Tyankan,
    4: Yaku.Rinsyankaiho,
    5: Yaku.Haiteitumo,
    6: Yaku.Hoteiron,
    7: Yaku.Pinhu,
    8: Yaku.Tanyao,
    9: Yaku.Ipeko,
    10: Yaku.ZikazeTon,
    11: Yaku.ZikazeNan,
    12: Yaku.ZikazeSya,
    13: Yaku.ZikazePe,
    14: Yaku.BakazeTon,
    15: Yaku.BakazeNan,
    16: Yaku.BakazeSya,
    17: Yaku.BakazePe,
    18: Yaku.SangenhaiHaku,
    19: Yaku.SangenhaiHatu,
    20: Yaku.SangenhaiTyun,
    21: Yaku.DoubleReach,
    22: Yaku.Tiitoitu,
    23: Yaku.Tyanta,
    24: Yaku.IkkiTukan,
    25: Yaku.SansyokuDozyun,
    26: Yaku.SansyokuDoko,
    27: Yaku.Sankantu,
    28: Yaku.Toitoiho,
    29: Yaku.Sananko,
    30: Yaku.Syosangen,
    31: Yaku.Honroto,
    32: Yaku.Ryanpeko,
    33: Yaku.Zyuntyanta,
    34: Yaku.Honiso,
    35: Yaku.Tiniso,
    36: Yaku.Renho,
    37: Yaku.Tenho,
    38: Yaku.Tiho,
    39: Yaku.Daisangen,
    40: Yaku.Suanko,
    41: Yaku.SuankoTanki,
    42: Yaku.Tiniso,
    43: Yaku.Ryuiso,
    44: Yaku.Honroto,
    45: Yaku.Tyurenpoto,
    46: Yaku.Tyurenpoto9,
    47: Yaku.Kokusimuso,
    48: Yaku.Kokusimuso13,
    49: Yaku.Daisusi,
    50: Yaku.Syosusi,
    51: Yaku.Sukantu,
    52: Yaku.Dora,
    53: Yaku.UraDora,
    54: Yaku.AkaDora,
}

# 座席
ToSeat = {
    0: SeatType.Zitya,
    1: SeatType.Simotya,
    2: SeatType.Toimen,
    3: SeatType.Kamitya,
}


def convert_taku(s):
    s = int(s)

    vs_person = bool(s & 1)
    aka = bool(s & 0b10)
    kuitan = bool(s & 0b100)
    taku = "半荘" if s & 0b1000 else "東風"
    yonma = "三麻" if s & 0b10000 else "四麻"
    if s & 0b10100000:
        level = "鳳凰"
    elif s & 0b00100000:
        level = "特上"
    elif s & 0b10000000:
        level = "上級"
    else:
        level = "その他"

    return {
        "対人": vs_person,
        "赤有り": aka,
        "喰い断あり": kuitan,
        "卓の種類": taku,
        "人数": yonma,
        "レベル": level,
    }


def convert_socre_title(score_title, han=0, n_yakuman=0):
    if score_title == 0:
        return ScoreTitle.Null
    elif score_title == 1:
        return ScoreTitle.Mangan
    elif score_title == 2:
        return ScoreTitle.Haneman
    elif score_title == 3:
        return ScoreTitle.Baiman
    elif score_title == 4:
        return ScoreTitle.Sanbaiman
    elif han >= 13:
        return ScoreTitle.KazoeYakuman
    elif n_yakuman == 1:
        return ScoreTitle.Yakuman
    elif n_yakuman == 2:
        return ScoreTitle.TwoYakuman
    elif n_yakuman == 3:
        return ScoreTitle.ThreeYakuman
    elif n_yakuman == 4:
        return ScoreTitle.FourYakuman
    elif n_yakuman == 5:
        return ScoreTitle.FiveYakuman
    elif n_yakuman == 6:
        return ScoreTitle.SixYakuman


def convert_tiles34(s):
    tiles = map(int, s.split(","))
    tiles = list(map(ToTile.__getitem__, tiles))

    return tiles


def parse_score(s):
    s = s.split(",")
    old_score = list(map(int, s[::2]))
    delta_score = list(map(int, s[1::2]))

    old_score = list(map(lambda x: x * 100, old_score))
    delta_score = list(map(lambda x: x * 100, delta_score))
    new_score = [x + y for x, y in zip(old_score, delta_score)]

    return old_score, new_score, delta_score


def convert_yakuman(s):
    yaku_list = []
    yakus = list(map(int, s.split(",")))
    for yaku in yakus:
        yaku_list.append((ToYaku[yaku], 1))

    yaku_list = sorted(yaku_list, key=lambda x: x[0])

    return yaku_list


def convert_yaku(s):
    yaku_list = []
    yakus = list(map(int, s.split(",")))
    for yaku, han in zip(yakus[::2], yakus[1::2]):
        if ToYaku[yaku] == Yaku.UraDora and han == 0:
            continue

        yaku_list.append((ToYaku[yaku], han))

    yaku_list = sorted(yaku_list, key=lambda x: x[0])

    return yaku_list


def perse_syuntu(m, player):
    t = (m & 0b1111110000000000) >> 10

    # 構成牌
    min_tile = (((t // 3) // 7) * 9 + ((t // 3) % 7)) * 4
    tile1 = min_tile + ((m & 0b11000) >> 3)
    tile2 = min_tile + 4 + ((m & 0b1100000) >> 5)
    tile3 = min_tile + 8 + ((m & 0b110000000) >> 7)
    tiles = [ToTile[tile1], ToTile[tile2], ToTile[tile3]]

    # 鳴いた牌
    discarded_tile = tiles[t % 3]

    # 鳴かれたプレイヤー
    from_ = ToSeat[m & 0b11]

    block = {
        "type": MeldType.Ti,
        "tiles": tiles,
        "discarded_tile": discarded_tile,
        "from": to_abs_seat(from_, player),
    }

    return block


def perse_kotu(m, player):
    t = (m & 0xFE00) >> 9

    # 構成牌
    min_tile = (t // 3) * 4
    unused = (m & 0x60) >> 5
    if unused == 0:
        tiles = [min_tile + 1, min_tile + 2, min_tile + 3]
    elif unused == 1:
        tiles = [min_tile, min_tile + 2, min_tile + 3]
    elif unused == 2:
        tiles = [min_tile, min_tile + 1, min_tile + 3]
    else:
        tiles = [min_tile, min_tile + 1, min_tile + 2]
    tiles = list(map(ToTile.__getitem__, tiles))

    # 鳴いた牌
    discarded_tile = tiles[t % 3]

    # 鳴かれたプレイヤー
    from_ = ToSeat[m & 0b11]

    block = {
        "type": MeldType.Pon,
        "tiles": tiles,
        "discarded_tile": discarded_tile,
        "from": to_abs_seat(from_, player),
    }

    return block


def parse_kakan(m, player):
    t = (m & 0xFE00) >> 9

    # 構成牌
    min_tile = (t // 3) * 4
    unused = (m & 0b1100000) >> 5
    if unused == 0:
        tiles = [min_tile + 1, min_tile + 2, min_tile + 3, min_tile]
    elif unused == 1:
        tiles = [min_tile, min_tile + 2, min_tile + 3, min_tile + 1]
    elif unused == 2:
        tiles = [min_tile, min_tile + 1, min_tile + 3, min_tile + 2]
    else:
        tiles = [min_tile, min_tile + 1, min_tile + 2, min_tile + 3]
    tiles = list(map(ToTile.__getitem__, tiles))

    # 鳴いた牌
    discarded_tile = tiles[t % 3]

    # 鳴かれたプレイヤー
    from_ = ToSeat[m & 0b11]

    block = {
        "type": MeldType.Kakan,
        "tiles": tiles,
        "discarded_tile": discarded_tile,
        "from": to_abs_seat(from_, player),
    }

    return block


def to_abs_seat(rel_seat, abs_seat):
    #   0
    # 1   3
    #   2
    table = [
        [0, 3, 2, 1],
        [1, 0, 3, 2],
        [2, 1, 0, 3],
        [3, 2, 1, 0],
    ]

    return table[abs_seat][rel_seat]


def perse_minkan_or_ankan(m, player):
    # 鳴かれたプレイヤー
    from_ = ToSeat[m & 0b11]

    # 種類
    meldtype = MeldType.Ankan if from_ == SeatType.Zitya else MeldType.Minkan

    # 構成牌
    min_tile = ((m & 0xFF00) >> 8) - (((m & 0xFF00) >> 8) % 4)
    tiles = [min_tile, min_tile + 1, min_tile + 2, min_tile + 3]
    tiles = list(map(ToTile.__getitem__, tiles))

    # 鳴いた牌
    discarded_tile = tiles[0] if meldtype == MeldType.Minkan else -1

    block = {
        "type": meldtype,
        "tiles": tiles,
        "discarded_tile": discarded_tile,
        "from": to_abs_seat(from_, player),
    }

    return block


def parse_meld(s, player):
    s = list(map(int, s.split(",")))

    melded_blocks = []
    for m in s:
        if m & 0b100:
            melded_blocks.append(perse_syuntu(m, player))
        elif m & 0b1000:
            melded_blocks.append(perse_kotu(m, player))
        elif m & 0b10000:
            melded_blocks.append(parse_kakan(m, player))
        elif m & 0b100000:
            raise ValueError("三麻じゃない")
        else:
            melded_blocks.append(perse_minkan_or_ankan(m, player))

    return melded_blocks


def convert_flag(yaku_list, is_tumo):
    flag = 0
    if is_tumo:
        flag |= Yaku.Tumo

    yaku_list = set(x for x, _ in yaku_list)
    special_yaku = {
        Yaku.Tenho,
        Yaku.Tiho,
        Yaku.Renho,
        Yaku.Reach,
        Yaku.DoubleReach,
        Yaku.Ippatu,
        Yaku.Tyankan,
        Yaku.Rinsyankaiho,
        Yaku.Haiteitumo,
        Yaku.Hoteiron,
    }

    for yaku in special_yaku & yaku_list:
        flag |= yaku

    return flag


def parse_agari(tag, bakaze, zikaze_list, path):
    win_player = int(tag["who"])
    lose_player = int(tag["fromWho"])

    # 場況
    zikaze = zikaze_list[int(tag["who"])]
    n_tumibo, n_kyotakubo = map(int, tag["ba"].split(","))
    dora_tiles = convert_tiles34(tag["doraHai"])
    dora_tiles = [DoraHyozi2Dora[x] for x in dora_tiles]

    uradora_tiles = (
        convert_tiles34(tag["doraHaiUra"]) if tag.has_attr("doraHaiUra") else []
    )
    uradora_tiles = [DoraHyozi2Dora[x] for x in uradora_tiles]
    # 入力
    hand_tiles = convert_tiles34(tag["hai"])
    melded_blocks = parse_meld(tag["m"], win_player) if tag.has_attr("m") else []
    winning_tile = ToTile[int(tag["machi"])]
    flag = 0

    # 結果
    yakuman = False
    hu, base_score, score_title = map(int, tag["ten"].split(","))
    if tag.has_attr("yaku"):
        yaku_list = convert_yaku(tag["yaku"])
        han = sum(x for _, x in yaku_list)
        score_title = convert_socre_title(score_title, han=han)
    else:
        yaku_list = convert_yakuman(tag["yakuman"])
        han = -1
        hu = -1
        yakuman = True
        score_title = convert_socre_title(score_title, n_yakuman=len(yaku_list))

    flag = convert_flag(yaku_list, win_player == lose_player)
    _, _, delta_score = parse_score(tag["sc"])

    payments = [-x for x in delta_score if x < 0]
    income = next(x for x in delta_score if x > 0)

    is_host = zikaze == Tile.Ton
    is_tumo = win_player == lose_player

    if is_tumo and is_host:
        # 親ツモ
        non_host_payment = payments[0]
        score = [income, non_host_payment]
    elif is_tumo and not is_host:
        # 子ツモ
        host_payment = max(payments)
        non_host_payment = min(payments)
        score = [income, host_payment, non_host_payment]
    elif not is_tumo:
        # ロン
        payment = payments[0]
        score = [income, payment]

    data = {
        # 場況
        "bakaze": bakaze,
        "zikaze": zikaze,
        "num_tumibo": n_tumibo,
        "num_kyotakubo": n_kyotakubo,
        "dora_tiles": dora_tiles,
        "uradora_tiles": uradora_tiles,
        # 和了り情報
        "hand_tiles": hand_tiles,
        "melded_blocks": melded_blocks,
        "winning_tile": winning_tile,
        "flag": flag,
        # 点数
        "han": han,
        "hu": hu,
        "score_title": score_title,
        "yaku_list": yaku_list,
        # 点数
        "win_player": win_player,
        "lose_player": lose_player,
        "score": score,
        "url": path.name,
    }

    return data, yakuman


def parse_init(tag):
    ba, honba, tumibo, dice1, dice2, dora = list(map(int, tag["seed"].split(",")))
    bakaze = Tile.Ton if ba <= 3 else Tile.Nan
    kyoku = ba % 4 + 1

    if kyoku == 1:
        zikaze_list = [Tile.Ton, Tile.Nan, Tile.Sya, Tile.Pe]
    elif kyoku == 2:
        zikaze_list = [Tile.Pe, Tile.Ton, Tile.Nan, Tile.Sya]
    elif kyoku == 3:
        zikaze_list = [Tile.Sya, Tile.Pe, Tile.Ton, Tile.Nan]
    elif kyoku == 4:
        zikaze_list = [Tile.Nan, Tile.Sya, Tile.Pe, Tile.Ton]

    return {"bakaze": bakaze, "局": kyoku, "zikaze_list": zikaze_list}


def parse_mjlog(path):
    print(path)
    soup = BeautifulSoup(open(path), "lxml-xml")
    root = soup.find("mjloggm")

    data = []

    for child in root:
        if child.name == "GO":
            taku = convert_taku(child["type"])
            if taku["人数"] == "三麻":
                return False
        if child.name == "INIT":
            init_data = parse_init(child)
        elif child.name == "AGARI":
            agari_data, yakuman = parse_agari(
                child, init_data["bakaze"], init_data["zikaze_list"], path
            )

            if not yakuman:
                data.append(agari_data)

    return data


data_dir = Path(r"E:\mahjong")
mj_paths = sorted(data_dir.glob("**/*.mjlog"))
print(f"{len(mj_paths)} mjlog files found.")

data = []
for x in tqdm(mj_paths[:500]):
    d = parse_mjlog(x)
    if d:
        data += d

import json

with open(TESTCASE_DIR / "test_score_normal_yaku.json", "w") as f:
    json.dump(data, f, indent=4)

