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
 * @brief Convertion table from dora to dora indicator.
 */
static constexpr std::array<int, 37> ToIndicator = {
    Tile::Manzu9, Tile::Manzu1, Tile::Manzu2, Tile::Manzu3, Tile::Manzu4, Tile::Manzu5,
    Tile::Manzu6, Tile::Manzu7, Tile::Manzu8, Tile::Pinzu9, Tile::Pinzu1, Tile::Pinzu2,
    Tile::Pinzu3, Tile::Pinzu4, Tile::Pinzu5, Tile::Pinzu6, Tile::Pinzu7, Tile::Pinzu8,
    Tile::Souzu9, Tile::Souzu1, Tile::Souzu2, Tile::Souzu3, Tile::Souzu4, Tile::Souzu5,
    Tile::Souzu6, Tile::Souzu7, Tile::Souzu8, Tile::North,  Tile::East,   Tile::South,
    Tile::West,   Tile::Red,    Tile::White,  Tile::Green,  Tile::Manzu4, Tile::Pinzu4,
    Tile::Souzu4};

/**
 * @brief Conversion table from dora indicator to dora.
 */
static constexpr std::array<int, 37> ToDora = {
    Tile::Manzu2, Tile::Manzu3, Tile::Manzu4, Tile::Manzu5, Tile::Manzu6, Tile::Manzu7,
    Tile::Manzu8, Tile::Manzu9, Tile::Manzu1, Tile::Pinzu2, Tile::Pinzu3, Tile::Pinzu4,
    Tile::Pinzu5, Tile::Pinzu6, Tile::Pinzu7, Tile::Pinzu8, Tile::Pinzu9, Tile::Pinzu1,
    Tile::Souzu2, Tile::Souzu3, Tile::Souzu4, Tile::Souzu5, Tile::Souzu6, Tile::Souzu7,
    Tile::Souzu8, Tile::Souzu9, Tile::Souzu1, Tile::South,  Tile::West,   Tile::North,
    Tile::East,   Tile::Green,  Tile::Red,    Tile::White,  Tile::Manzu6, Tile::Pinzu6,
    Tile::Souzu6};
/**
 * @brief 赤なしの牌を赤牌に変換する。
 *
 * @param tile 赤牌
 * @return int 赤なしの牌
 */
inline int normal2red(int tile)
{
    if (tile == Tile::RedManzu5)
        return Tile::Manzu5;
    else if (tile == Tile::RedPinzu5)
        return Tile::Pinzu5;
    else if (tile == Tile::RedSouzu5)
        return Tile::Souzu5;
    else
        return tile;
}

/**
 * @brief 赤牌を赤なしの牌に変換する。
 *
 * @param tile 赤牌
 * @return int 赤なしの牌
 */
inline int red2normal(int tile)
{
    if (tile <= Tile::Red)
        return tile;
    else if (tile == Tile::RedManzu5)
        return Tile::Manzu5;
    else if (tile == Tile::RedPinzu5)
        return Tile::Pinzu5;
    else
        return Tile::Souzu5;
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

} // namespace mahjong
#endif /* MAHJONG_CPP_UTILS */
