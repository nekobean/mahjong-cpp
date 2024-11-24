import json

import requests
from const import *

from mahjong import from_mpsz


def main():
    # 例: 222567m34p33667s北
    hand = from_mpsz("222567m34p33667s4z")
    print(hand)

    req_data = {
        "enable_reddora": True,
        "enable_uradora": True,
        "enable_shanten_down": True,
        "enable_tegawari": True,
        "enable_riichi": True,
        "round_wind": Tile.East,
        "dora_indicators": [Tile.North],
        "hand": hand,
        "melds": [],
        "seat_wind": Tile.East,
        "version": "0.9.1",
    }

    # Send request to the server
    res = requests.post(
        "http://localhost:50000",
        json.dumps(req_data),
        headers={"Content-Type": "application/json"},
    )
    res_data = res.json()
    # print(res_data)

    # Print result.
    if not res_data["success"]:
        print(f"Failed to calculate. ({res_data['err_msg']})")
        return

    result = res_data["response"]
    print_result(result)


def print_result(ret):
    print("=== Necessary Tiles ===")
    for stat in ret["stats"]:
        tile = Tile.Name[stat["tile"]]
        num_types = len(stat["necessary"])
        num_tiles = sum([x["count"] for x in stat["necessary"]])
        tiles = "".join(
            f"{Tile.Name[x['tile']]}({x['count']})" for x in stat["necessary"]
        )
        print(f"{tile: <3} types: {num_types:<2d} total: {num_tiles:<3d} {tiles}")
    print()

    t_min = ret["config"]["t_min"]
    t_max = ret["config"]["t_max"]

    if ret["config"]["calc_stats"]:
        print("=== Tenpai Probability ===")
        print(f"turn ", end="")
        for stat in ret["stats"]:
            tile = Tile.Name[stat["tile"]]
            print(f"{tile: <7} ", end="")
        print()

        for t in range(t_min, t_max + 1):
            print(f"{t: <5}", end="")
            for stat in ret["stats"]:
                prob = stat["tenpai_prob"][t]
                print(f"{prob:<7.2%} ", end="")
            print()

        print("=== Win Probability ===")
        print(f"turn ", end="")
        for stat in ret["stats"]:
            tile = Tile.Name[stat["tile"]]
            print(f"{tile: <7} ", end="")
        print()

        for t in range(t_min, t_max + 1):
            print(f"{t: <5}", end="")
            for stat in ret["stats"]:
                prob = stat["win_prob"][t]
                print(f"{prob:<7.2%} ", end="")
            print()

        print("=== Expected Score ===")
        print(f"turn ", end="")
        for stat in ret["stats"]:
            tile = Tile.Name[stat["tile"]]
            print(f"{tile: <8} ", end="")
        print()

        for t in range(t_min, t_max + 1):
            print(f"{t: <5}", end="")
            for stat in ret["stats"]:
                prob = stat["exp_score"][t]
                print(f"{prob:<8.2f} ", end="")
            print()

    print("=== Info ===")
    print(
        f"Shanten: {ret['shanten']['all']} "
        f"(Regular: {ret['shanten']['regular']}, "
        f"Seven Pairs: {ret['shanten']['seven_pairs']}, "
        f"Thirteen Orphans: {ret['shanten']['thirteen_orphans']})"
    )

    def time_to_str(us):
        if us < 1000:
            return f"{us} us"
        ms = us / 1000
        if ms < 1000:
            return f"{ms:.2f} ms"
        s = ms / 1000

        return f"{s:.2f} s"

    print(f"Calculation Time: {time_to_str(ret['time'])}")
    print(f"Searched: {ret['searched']} hands")


if __name__ == "__main__":
    main()
