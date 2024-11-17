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

} // namespace mahjong
#endif /* MAHJONG_CPP_UTILS */
