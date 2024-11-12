#ifndef MAHJONG_CPP_TILES
#define MAHJONG_CPP_TILES

#include <string>
#include <vector>

#include "mahjong/types/const.hpp"

namespace mahjong
{
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

#endif /* MAHJONG_CPP_TILES */
