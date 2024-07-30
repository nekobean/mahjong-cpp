import json
from pprint import pprint

import requests

from mahjong import *


def calc_remaining_tiles(hand_tiles, dora_indicators, melded_blocks):
    # counts[0] ~ counts[34]: 各牌の残り枚数、counts[34] ~ counts[36]: 赤牌が残っているかどうか
    counts = [4 for _ in range(34)] + [1, 1, 1]
    meld_tiles = [tile for meld in melded_blocks for tile in meld["tiles"]]
    visible_tiles = hand_tiles + dora_indicators + meld_tiles

    for tile in visible_tiles:
        counts[tile] -= 1
        if tile == Tile.AkaManzu5:
            counts[Tile.Manzu5] -= 1
        elif tile == Tile.AkaPinzu5:
            counts[Tile.Pinzu5] -= 1
        elif tile == Tile.AkaSozu5:
            counts[Tile.Sozu5] -= 1

    return counts


def print_result(result):
    result_type = result["result_type"]  # 結果の種類
    syanten = result["syanten"]  # 向聴数
    time_us = result["time"]  # 計算時間 (マイクロ秒)

    print(
        f"向聴数: {syanten['syanten']}"
        f" (通常手: {syanten['normal']}, 七対子手: {syanten['tiitoi']}, 国士無双手: {syanten['kokusi']})"
    )
    print(f"計算時間: {time_us / 1e6}秒")

    if result_type == 0:
        #
        # 手牌の枚数が13枚の場合、有効牌、期待値、和了確率、聴牌確率が得られる。
        #
        required_tiles = result["required_tiles"]  # 有効牌
        exp_values = result["exp_values"]  # 期待値 (1~17巡目)
        win_probs = result["win_probs"]  # 和了確率 (1~17巡目)
        tenpai_probs = result["tenpai_probs"]  # 聴牌確率 (1~17巡目)

        tiles = [f"{tile['tile']}: {tile['count']}枚" for tile in required_tiles]
        print(f"  有効牌: {', '.join(tiles)}")

        for turn, (exp, win_prop, tenpai_prop) in enumerate(
            zip(exp_values, win_probs, tenpai_probs), 1
        ):
            print(
                f"  {turn}巡目 期待値: {exp:.0f}点, 和了確率: {win_prop:.1%}, 聴牌確率: {tenpai_prop:.1%}"
            )

    elif result_type == 1:
        #
        # 手牌の枚数が14枚の場合、打牌候補ごとに有効牌、期待値、和了確率、聴牌確率が得られる。
        #
        for candidate in result["candidates"]:
            tile = candidate["tile"]  # 打牌候補
            syanten_down = candidate["syanten_down"]  # 向聴戻しとなる打牌かどうか
            required_tiles = candidate["required_tiles"]  # 有効牌
            exp_values = candidate["exp_values"]  # 期待値 (1~17巡目)
            win_probs = candidate["win_probs"]  # 和了確率 (1~17巡目)
            tenpai_probs = candidate["tenpai_probs"]  # 聴牌確率 (1~17巡目)

            print(f"打牌候補: {Tile.Name[tile]} (向聴落とし: {syanten_down})")

            tiles = [f"{tile['tile']}: {tile['count']}枚" for tile in required_tiles]
            print(f"  有効牌: {', '.join(tiles)}")

            for turn, (exp, win_prop, tenpai_prop) in enumerate(
                zip(exp_values, win_probs, tenpai_probs), 1
            ):
                print(
                    f"  {turn}巡目 期待値: {exp:.0f}点, 和了確率: {win_prop:.1%}, 聴牌確率: {tenpai_prop:.1%}"
                )


def create_sample_request1():
    ###########################
    # サンプル: 手牌が14枚で面前の場合
    # 例: 222567m34p33667s北
    ###########################
    # 手牌
    hand_tiles = [
        Tile.Manzu2,
        Tile.Manzu2,
        Tile.Manzu2,
        Tile.Manzu5,
        Tile.Manzu6,
        Tile.Manzu7,
        Tile.Pinzu3,
        Tile.Pinzu4,
        Tile.Sozu3,
        Tile.Sozu3,
        Tile.Sozu6,
        Tile.Sozu6,
        Tile.Sozu7,
        Tile.Pe,
    ]
    # 副露牌 (4個まで指定可能)
    melded_blocks = []

    # ドラ表示牌 (4枚まで指定可能)
    dora_indicators = [Tile.Ton]
    # 場風 (東: Tile.Ton, 南: Tile.Nan, 西: Tile.Sya, 北: Tile.Pe)
    bakaze = Tile.Ton
    # 自風 (東: Tile.Ton, 南: Tile.Nan, 西: Tile.Sya, 北: Tile.Pe)
    zikaze = Tile.Ton
    # 計算する向聴数の種類 (通常手: SyantenType.Normal, 七対子手: SyantenType.Tiitoi, 国士無双手: SyantenType.Kokusi)
    syanten_type = SyantenType.Normal
    # 現在の巡目 (1~17巡目の間で指定可能)
    turn = 3
    # 場に見えていない牌の枚数を計算する。
    counts = calc_remaining_tiles(hand_tiles, dora_indicators, melded_blocks)
    # その他、手牌とドラ表示牌以外に場に見えている牌がある場合、それらを引いておけば、山にないものとして計算できる。

    # 期待値を計算する際の設定 (有効にする設定を指定)
    exp_option = (
        ExpOption.CalcSyantenDown  # 向聴落とし考慮
        | ExpOption.CalcTegawari  # 手変わり考慮
        | ExpOption.CalcDoubleReach  # ダブル立直考慮
        | ExpOption.CalcIppatu  # 一発考慮
        | ExpOption.CalcHaiteitumo  # 海底撈月考慮
        | ExpOption.CalcUradora  # 裏ドラ考慮
        | ExpOption.CalcAkaTileTumo  # 赤牌自摸考慮
    )

    # リクエストデータを作成する。
    req_data = {
        "version": "0.9.0",
        "zikaze": bakaze,
        "bakaze": zikaze,
        "turn": turn,
        "syanten_type": syanten_type,
        "dora_indicators": dora_indicators,
        "flag": exp_option,
        "hand_tiles": hand_tiles,
        "melded_blocks": melded_blocks,
        "counts": counts,
    }

    return req_data


def create_sample_request2():
    ###########################
    # サンプル: 手牌が14枚で副露している場合
    # 例: 34p33667s北 [222m][567m]
    # ポンとチーをしている場合
    ###########################
    # 手牌
    hand_tiles = [
        Tile.Pinzu3,
        Tile.Pinzu4,
        Tile.Sozu3,
        Tile.Sozu3,
        Tile.Sozu6,
        Tile.Sozu6,
        Tile.Sozu7,
        Tile.Pe,
    ]
    # 副露牌 (4個まで指定可能)
    melded_blocks = [
        create_meld_block([Tile.Manzu2, Tile.Manzu2, Tile.Manzu2], MeldType.Pon),
        create_meld_block([Tile.Manzu5, Tile.Manzu6, Tile.Manzu7], MeldType.Ti),
    ]

    # ドラ表示牌 (4枚まで指定可能)
    dora_indicators = [Tile.Ton]
    # 場風 (東: Tile.Ton, 南: Tile.Nan, 西: Tile.Sya, 北: Tile.Pe)
    bakaze = Tile.Ton
    # 自風 (東: Tile.Ton, 南: Tile.Nan, 西: Tile.Sya, 北: Tile.Pe)
    zikaze = Tile.Ton
    # 計算する向聴数の種類 (通常手: SyantenType.Normal, 七対子手: SyantenType.Tiitoi, 国士無双手: SyantenType.Kokusi)
    syanten_type = SyantenType.Normal
    # 現在の巡目 (1~17巡目の間で指定可能)
    turn = 3
    # 場に見えていない牌の枚数を計算する。
    counts = calc_remaining_tiles(hand_tiles, dora_indicators, melded_blocks)
    # その他、手牌とドラ表示牌以外に場に見えている牌がある場合、それらを引いておけば、山にないものとして計算できる。

    # 期待値を計算する際の設定 (有効にする設定を指定)
    exp_option = (
        ExpOption.CalcSyantenDown  # 向聴落とし考慮
        | ExpOption.CalcTegawari  # 手変わり考慮
        | ExpOption.CalcDoubleReach  # ダブル立直考慮
        | ExpOption.CalcIppatu  # 一発考慮
        | ExpOption.CalcHaiteitumo  # 海底撈月考慮
        | ExpOption.CalcUradora  # 裏ドラ考慮
        | ExpOption.CalcAkaTileTumo  # 赤牌自摸考慮
    )

    # リクエストデータを作成する。
    req_data = {
        "version": "0.9.0",
        "zikaze": bakaze,
        "bakaze": zikaze,
        "turn": turn,
        "syanten_type": syanten_type,
        "dora_indicators": dora_indicators,
        "flag": exp_option,
        "hand_tiles": hand_tiles,
        "melded_blocks": melded_blocks,
        "counts": counts,
    }

    return req_data


def create_sample_request3():
    ###########################
    # サンプル: 手牌が13枚
    # 例: 222567m34p33667s
    ###########################
    # 手牌
    hand_tiles = [
        Tile.Manzu2,
        Tile.Manzu2,
        Tile.Manzu2,
        Tile.Manzu5,
        Tile.Manzu6,
        Tile.Manzu7,
        Tile.Pinzu3,
        Tile.Pinzu4,
        Tile.Sozu3,
        Tile.Sozu3,
        Tile.Sozu6,
        Tile.Sozu6,
        Tile.Sozu7,
    ]
    # 副露牌 (4個まで指定可能)
    melded_blocks = []

    # ドラ表示牌 (4枚まで指定可能)
    dora_indicators = [Tile.Ton]
    # 場風 (東: Tile.Ton, 南: Tile.Nan, 西: Tile.Sya, 北: Tile.Pe)
    bakaze = Tile.Ton
    # 自風 (東: Tile.Ton, 南: Tile.Nan, 西: Tile.Sya, 北: Tile.Pe)
    zikaze = Tile.Ton
    # 計算する向聴数の種類 (通常手: SyantenType.Normal, 七対子手: SyantenType.Tiitoi, 国士無双手: SyantenType.Kokusi)
    syanten_type = SyantenType.Normal
    # 現在の巡目 (1~17巡目の間で指定可能)
    turn = 3
    # 場に見えていない牌の枚数を計算する。
    counts = calc_remaining_tiles(hand_tiles, dora_indicators, melded_blocks)
    # その他、手牌とドラ表示牌以外に場に見えている牌がある場合、それらを引いておけば、山にないものとして計算できる。

    # 期待値を計算する際の設定 (有効にする設定を指定)
    exp_option = (
        ExpOption.CalcSyantenDown  # 向聴落とし考慮
        | ExpOption.CalcTegawari  # 手変わり考慮
        | ExpOption.CalcDoubleReach  # ダブル立直考慮
        | ExpOption.CalcIppatu  # 一発考慮
        | ExpOption.CalcHaiteitumo  # 海底撈月考慮
        | ExpOption.CalcUradora  # 裏ドラ考慮
        | ExpOption.CalcAkaTileTumo  # 赤牌自摸考慮
    )

    # リクエストデータを作成する。
    req_data = {
        "version": "0.9.0",
        "zikaze": bakaze,
        "bakaze": zikaze,
        "turn": turn,
        "syanten_type": syanten_type,
        "dora_indicators": dora_indicators,
        "flag": exp_option,
        "hand_tiles": hand_tiles,
        "melded_blocks": melded_blocks,
        "counts": counts,
    }

    return req_data


def main():
    ########################################
    # 計算実行
    ########################################
    req_data = create_sample_request1()

    # dict -> json
    payload = json.dumps(req_data)
    # リクエストを送信する。
    res = requests.post(
        "http://localhost:8888", payload, headers={"Content-Type": "application/json"}
    )
    res_data = res.json()

    ########################################
    # 結果出力
    ########################################
    if not res_data["success"]:
        raise RuntimeError(f"計算の実行に失敗しました。(理由: {res_data['err_msg']})")

    result = res_data["response"]
    print_result(result)


if __name__ == "__main__":
    main()
