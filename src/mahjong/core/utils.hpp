#ifndef MAHJONG_CPP_UTILS
#define MAHJONG_CPP_UTILS

#include <algorithm> // all_of
#include <vector>

bool check_hand(const std::vector<int> &hand)
{
    if (std::all_of(hand.begin(), hand.end(), [](int x) { return x >= 0 && x <= 4; })) {
        return false;
    }
}

#endif /* MAHJONG_CPP_UTILS */
