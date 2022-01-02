import random
import json

from utils import *

yama = create_yama()


def create_hand(yama, n):
    yama = yama[:]

    # 先牌する。
    random.shuffle(yama)

    # 配牌を作成する。
    return sorted(yama[:n])


def get_counts(hand):
    counts = [4] * 34 + [1, 1, 1]

    for tile in hand:
        counts[tile] -= 1

        if tile == Tile.AkaManzu5:
            counts[Tile.Manzu5] -= 1
        if tile == Tile.AkaPinzu5:
            counts[Tile.Pinzu5] -= 1
        if tile == Tile.AkaSozu5:
            counts[Tile.Sozu5] -= 1

    return counts


def craete_remove_tile_testcase():
    hand = create_hand(yama, 14)
    counts = get_counts(hand)

    tile = random.choice(hand)
    hand_after = hand[:]
    hand_after.remove(tile)
    counts_after = get_counts(hand_after)

    testcase = {
        "hand_before": hand,
        "counts_before": counts,
        "hand_after": hand_after,
        "counts_after": counts_after,
        "tile": tile,
    }

    return testcase


def craete_add_tile_testcase():
    hand = create_hand(yama, 13)
    counts = get_counts(hand)

    left_tiles = [i for i, n_tiles in enumerate(counts) if n_tiles > 0]

    tile = random.choice(left_tiles)
    hand_after = hand[:]
    hand_after.append(tile)
    hand_after.sort()
    counts_after = get_counts(hand_after)

    testcase = {
        "hand_before": hand,
        "counts_before": counts,
        "hand_after": hand_after,
        "counts_after": counts_after,
        "tile": tile,
    }

    return testcase


random.seed(0)

with open(TESTCASE_DIR / "testcase_remove_tile.json", "w") as f:
    testcases = []
    for i in range(10000):
        testcase = craete_remove_tile_testcase()
        testcases.append(testcase)
    json.dump(testcases, f)

with open(TESTCASE_DIR / "testcase_add_tile.json", "w") as f:
    testcases = []
    for i in range(10000):
        testcase = craete_add_tile_testcase()
        testcases.append(testcase)
    json.dump(testcases, f)
