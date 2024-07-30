# fmt:off
class Tile:
    Null = -1
    Manzu1 = 0     # 一萬
    Manzu2 = 1     # 二萬
    Manzu3 = 2     # 三萬
    Manzu4 = 3     # 四萬
    Manzu5 = 4     # 五萬
    Manzu6 = 5     # 六萬
    Manzu7 = 6     # 七萬
    Manzu8 = 7     # 八萬
    Manzu9 = 8     # 九萬
    Pinzu1 = 9     # 一筒
    Pinzu2 = 10    # 二筒
    Pinzu3 = 11    # 三筒
    Pinzu4 = 12    # 四筒
    Pinzu5 = 13    # 五筒
    Pinzu6 = 14    # 六筒
    Pinzu7 = 15    # 七筒
    Pinzu8 = 16    # 八筒
    Pinzu9 = 17    # 九筒
    Sozu1 = 18     # 一索
    Sozu2 = 19     # 二索
    Sozu3 = 20     # 三索
    Sozu4 = 21     # 四索
    Sozu5 = 22     # 五索
    Sozu6 = 23     # 六索
    Sozu7 = 24     # 七索
    Sozu8 = 25     # 八索
    Sozu9 = 26     # 九索
    Ton = 27       # 東
    Nan = 28       # 南
    Sya = 29       # 西
    Pe = 30        # 北
    Haku = 31      # 白
    Hatu = 32      # 発
    Tyun = 33      # 中
    AkaManzu5 = 34 # 赤五萬
    AkaPinzu5 = 35 # 赤五筒
    AkaSozu5 = 36  # 赤五索

    Name = [
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
        "発",
        "中",
        "赤五萬",
        "赤五筒",
        "赤五索"
    ]
# fmt:on


# fmt:off
class SyantenType:
    Normal = 1 # 通常手
    Tiitoi = 2 # 七対子手
    Kokusi = 4 # 国士無双手
# fmt:on

# fmt:off
class ExpOption:
    CalcSyantenDown = 1       # 向聴落とし考慮
    CalcTegawari = 1 << 1     # 手変わり考慮
    CalcDoubleReach = 1 << 2  # ダブル立直考慮
    CalcIppatu = 1 << 3       # 一発考慮
    CalcHaiteitumo = 1 << 4   # 海底撈月考慮
    CalcUradora = 1 << 5      # 裏ドラ考慮
    CalcAkaTileTumo = 1 << 6  # 赤牌自摸考慮
    MaximaizeWinProb = 1 << 7 # 和了確率を最大化 (指定されていない場合は期待値を最大化)
# fmt:on


class MeldType:
    Null = -1
    Pon = 1
    Ti = 2
    Ankan = 3
    Minkan = 4
    Kakan = 5

    Name = ["ポン", "チー", "暗槓", "明槓", "加槓"]


def infer_meld_type(tiles, type):
    if len(tiles) == 3:
        if tiles[0] + 1 == tiles[1] and tiles[1] + 1 == tiles[2]:
            return MeldType.Ti
        elif tiles[0] == tiles[1] and tiles[1] == tiles[2]:
            return MeldType.Pon
        else:
            raise ValueError("Invalid tiles pattern for meld.")
    elif len(tiles) == 4:
        if tiles[0] == tiles[1] and tiles[1] == tiles[2] and tiles[2] == tiles[3]:
            return MeldType.Minkan  # 明槓と仮定
        else:
            raise ValueError("Invalid tiles pattern for meld.")

    raise ValueError("Number of tiles must be 3 or 4.")


def create_meld_block(tiles, type=MeldType.Null):
    if type == MeldType.Null:
        type = infer_meld_type(tiles, type)
    elif type == MeldType.Ti:
        assert (
            len(tiles) == 3 and tiles[0] + 1 == tiles[1] and tiles[1] + 1 == tiles[2]
        ), "Invalid tiles pattern for meld."
    elif type == MeldType.Pon:
        assert (
            len(tiles) == 3 and tiles[0] == tiles[1] and tiles[1] == tiles[2]
        ), "Invalid tiles pattern for meld."
    elif type in [MeldType.Ankan, MeldType.Minkan, MeldType.Kakan]:
        assert (
            len(tiles) == 4
            and tiles[0] == tiles[1]
            and tiles[1] == tiles[2]
            and tiles[2] == tiles[3]
        ), "Invalid tiles pattern for meld."

    # discard_tile (鳴いた牌) と from (誰から鳴かれたか) は計算には関係ないので、適当に埋める。
    meld_block = {"type": type, "tiles": tiles, "discarded_tile": tiles[0], "from": 0}

    return meld_block
