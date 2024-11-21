#ifndef MAHJONG_CPP_UTILS
#define MAHJONG_CPP_UTILS

#include <array>

#include "mahjong/types/types.hpp"

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

inline Hand from_array(const std::vector<int> &tiles)
{
    Hand hand{0};

    for (int tile : tiles) {
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
