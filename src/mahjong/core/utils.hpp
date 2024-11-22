#ifndef MAHJONG_CPP_UTILS
#define MAHJONG_CPP_UTILS

#include <array>

#include "mahjong/types/types.hpp"

#include <spdlog/spdlog.h>

namespace mahjong
{

template <typename T> inline bool check_exclusive(T x)
{
    return !x || !(x & (x - 1));
}

/**
 * @brief Convert to red dora, if tile is 5m, 5p or 5s, otherwise no change.
 *
 * @param tile tile
 * @return red dora if tile is 5m, 5p or 5s
 */
inline int to_reddora(int tile)
{
    if (tile == Tile::RedManzu5) {
        return Tile::Manzu5;
    }
    else if (tile == Tile::RedPinzu5) {
        return Tile::Manzu5;
    }
    else if (tile == Tile::RedSouzu5) {
        return Tile::Souzu5;
    }
    else {
        return tile;
    }
}

inline void check_hand(const Hand &hand)
{
    for (int i = 0; i < 34; ++i) {
        if (hand[i] < 0 || hand[i] > 4) {
            throw std::invalid_argument(
                fmt::format("Invalid tile count found. (tile: {}, count: {})",
                            Tile::Name.at(i), hand[i]));
        }
    }

    for (int i = 34; i < 37; ++i) {
        if (hand[i] < 0 || hand[i] > 1) {
            throw std::invalid_argument(
                fmt::format("Invalid red flag found. (tile: {}, count: {})",
                            Tile::Name.at(i), hand[i]));
        }
    }

    if (hand[Tile::RedManzu5] > hand[Tile::Manzu5]) {
        throw std::invalid_argument("0m flag specified but 5m is not included.");
    }

    if (hand[Tile::RedPinzu5] > hand[Tile::Pinzu5]) {
        throw std::invalid_argument("0p flag specified but 5p is not included.");
    }

    if (hand[Tile::RedSouzu5] > hand[Tile::Souzu5]) {
        throw std::invalid_argument("0s flag specified but 5s is not included.");
    }

    int total = std::accumulate(hand.begin(), hand.begin() + 34, 0);
    if (total > 14) {
        throw std::invalid_argument("More than 14 tiles are used.");
    }

    if (total % 3 == 0) {
        throw std::invalid_argument("The number of tiles divisible by 3.");
    }
}

inline Hand from_array(const std::vector<int> &tiles)
{
    Hand hand{0};

    for (int tile : tiles) {
        if (tile < 0 || tile > 37) {
            throw std::invalid_argument(
                fmt::format("Invalid tile number found. (value: {})", tile));
        }

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

    check_hand(hand);

    return hand;
}

/**
 * @brief Convert to no red dora, if tile is red dora, otherwise no change.
 *
 * @param tile tile
 * @return not red dora tile
 */
inline int to_no_reddora(int tile)
{
    if (tile <= Tile::Red) {
        return tile;
    }
    else if (tile == Tile::RedManzu5) {
        return Tile::Manzu5;
    }
    else if (tile == Tile::RedPinzu5) {
        return Tile::Pinzu5;
    }
    else {
        return Tile::Souzu5;
    }
}

/**
 * @brief 赤牌かどうかを判定する。
 *
 * @param tile 牌
 * @return bool 赤牌かどうか
 */
inline bool is_reddora(int tile)
{
    return tile >= Tile::RedManzu5;
}

/**
 * @brief 萬子かどうかを判定する。
 *
 * @param tile 牌
 * @return bool 萬子かどうか
 */
inline bool is_manzu(int tile)
{
    return (Tile::Manzu1 <= tile && tile <= Tile::Manzu9) || tile == Tile::RedManzu5;
}

/**
 * @brief 筒子かどうかを判定する。
 *
 * @param tile 牌
 * @return bool 筒子かどうか
 */
inline bool is_pinzu(int tile)
{
    return (Tile::Pinzu1 <= tile && tile <= Tile::Pinzu9) || tile == Tile::RedPinzu5;
}

/**
 * @brief 索子かどうかを判定する。
 *
 * @param tile 牌
 * @return bool 索子かどうか
 */
inline bool is_souzu(int tile)
{
    return (Tile::Souzu1 <= tile && tile <= Tile::Souzu9) || tile == Tile::RedSouzu5;
}

/**
 * @brief 数牌かどうかを判定する。
 *
 * @param tile 牌
 * @return bool 数牌かどうか
 */
inline bool is_suits(int tile)
{
    return tile <= Tile::Souzu9 || tile >= Tile::RedManzu5;
}

/**
 * @brief 字牌かどうかを判定する。
 *
 * @param tile 牌
 * @return bool 字牌かどうか
 */
inline bool is_honor(int tile)
{
    return Tile::East <= tile && tile <= Tile::Red;
}

inline bool is_terminal(int tile)
{
    const int n = tile % 9;
    return tile <= Tile::Souzu9 && (n == 0 || n == 8);
}

inline bool is_terminal_or_honor(int tile)
{
    const int n = tile % 9;
    return tile <= Tile::Red && (n == 0 || n == 8 || Tile::East <= tile);
}

inline bool is_simples(int tile)
{
    const int n = tile % 9;
    return tile <= Tile::Souzu9 && n != 0 && n != 8;
}

} // namespace mahjong

#endif /* MAHJONG_CPP_UTILS */
