from const import *


def from_mpsz(s):
    hand = []
    type = None
    for c in reversed(s):
        if c.isspace():
            continue
        if c in "mpsz":
            type = c
        elif c.isdigit():
            tile = int(c) - 1
            if type == "m":
                tile = Tile.RedManzu5 if tile == -1 else tile
            elif type == "p":
                tile = Tile.RedPinzu5 if tile == -1 else tile + 9
            elif type == "s":
                tile = Tile.RedSouzu5 if tile == -1 else tile + 18
            elif type == "z":
                tile = tile + 27

            hand.append(tile)

    check_hand(hand)

    return hand


def check_hand(hand):
    if Tile.RedManzu5 in hand and Tile.Manzu5 not in hand:
        raise ValueError("0m flag specified but 5m is not in the hand.")

    if Tile.RedPinzu5 in hand and Tile.Pinzu5 not in hand:
        raise ValueError("0p flag specified but 5p is not in the hand.")

    if Tile.RedSouzu5 in hand and Tile.Souzu5 not in hand:
        raise ValueError("0s flag specified but 5s is not in the hand.")

    if len(hand) > 14:
        raise ValueError("More than 14 tiles are used.")

    if len(hand) % 3 == 0:
        raise ValueError("The number of tiles divisible by 3.")
