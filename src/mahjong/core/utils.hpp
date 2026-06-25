#ifndef MAHJONG_CPP_UTILS_HPP
#define MAHJONG_CPP_UTILS_HPP

#include <numeric>
#include <stdexcept>
#include <vector>

#include "mahjong/types/types.hpp"

#include <spdlog/spdlog.h>

namespace mahjong
{

template <typename T> inline bool check_exclusive(T x)
{
    return !x || !(x & (x - 1));
}

inline void check_hand(const Hand &hand)
{
    for (int i = 0; i < 34; ++i) {
        if (hand[i] < 0 || hand[i] > 4) {
            throw std::invalid_argument(
                fmt::format(u8"Invalid tile count found. (tile: {}, count: {})",
                            Tile::name(i), hand[i]));
        }
    }

    for (int i = 34; i < 37; ++i) {
        if (hand[i] < 0 || hand[i] > 1) {
            throw std::invalid_argument(
                fmt::format(u8"Invalid red flag found. (tile: {}, count: {})",
                            Tile::name(i), hand[i]));
        }
    }

    if (hand[Tile::RedManzu5] > hand[Tile::Manzu5]) {
        throw std::invalid_argument(u8"0m flag specified but 5m is not included.");
    }

    if (hand[Tile::RedPinzu5] > hand[Tile::Pinzu5]) {
        throw std::invalid_argument(u8"0p flag specified but 5p is not included.");
    }

    if (hand[Tile::RedSouzu5] > hand[Tile::Souzu5]) {
        throw std::invalid_argument(u8"0s flag specified but 5s is not included.");
    }

    int total = std::accumulate(hand.begin(), hand.begin() + 34, 0);
    if (total > 14) {
        throw std::invalid_argument(u8"More than 14 tiles are used.");
    }

    if (total % 3 == 0) {
        throw std::invalid_argument(u8"The number of tiles divisible by 3.");
    }
}

/**
 * @brief Create a hand from a list of tiles.
 *
 * Red fives are counted both as red fives and as normal fives.
 *
 * @pre Each tile must be in [0, Tile::Length).
 */
inline Hand to_hand(const std::vector<int> &tiles) noexcept
{
    Hand hand{};

    for (const int tile : tiles) {
        if (tile == Tile::RedManzu5) {
            ++hand[Tile::Manzu5];
        }
        else if (tile == Tile::RedPinzu5) {
            ++hand[Tile::Pinzu5];
        }
        else if (tile == Tile::RedSouzu5) {
            ++hand[Tile::Souzu5];
        }

        ++hand[tile];
    }

    return hand;
}

inline Hand from_array(const std::vector<int> &tiles)
{
    for (const int tile : tiles) {
        if (tile < 0 || tile >= Tile::Length) {
            throw std::invalid_argument(
                fmt::format(u8"Invalid tile number found. (value: {})", tile));
        }
    }

    Hand hand = to_hand(tiles);
    check_hand(hand);

    return hand;
}

inline bool has_sanma_disabled_tiles(const Hand &hand)
{
    for (int tile = Tile::Manzu2; tile <= Tile::Manzu8; ++tile) {
        if (hand[tile] > 0) {
            return true;
        }
    }

    return hand[Tile::RedManzu5] > 0;
}

} // namespace mahjong

#endif // MAHJONG_CPP_UTILS_HPP
