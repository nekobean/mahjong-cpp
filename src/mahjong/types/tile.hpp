#ifndef MAHJONG_CPP_TILE_HPP
#define MAHJONG_CPP_TILE_HPP

#include <array>
#include <cstdint>
#include <string_view>

#include "mahjong/types/constants.hpp"

namespace mahjong
{

/**
 * @brief Tile types.
 */
namespace Tile
{

using TileMask = std::uint64_t;

inline constexpr TileMask bit(int tile) noexcept
{
    return TileMask{1} << tile;
}

inline constexpr int Null = -1;
inline constexpr int Manzu1 = 0;       /*! 一萬 */
inline constexpr int Manzu2 = 1;       /*! 二萬 */
inline constexpr int Manzu3 = 2;       /*! 三萬 */
inline constexpr int Manzu4 = 3;       /*! 四萬 */
inline constexpr int Manzu5 = 4;       /*! 五萬 */
inline constexpr int Manzu6 = 5;       /*! 六萬 */
inline constexpr int Manzu7 = 6;       /*! 七萬 */
inline constexpr int Manzu8 = 7;       /*! 八萬 */
inline constexpr int Manzu9 = 8;       /*! 九萬 */
inline constexpr int Pinzu1 = 9;       /*! 一筒 */
inline constexpr int Pinzu2 = 10;      /*! 二筒 */
inline constexpr int Pinzu3 = 11;      /*! 三筒 */
inline constexpr int Pinzu4 = 12;      /*! 四筒 */
inline constexpr int Pinzu5 = 13;      /*! 五筒 */
inline constexpr int Pinzu6 = 14;      /*! 六筒 */
inline constexpr int Pinzu7 = 15;      /*! 七筒 */
inline constexpr int Pinzu8 = 16;      /*! 八筒 */
inline constexpr int Pinzu9 = 17;      /*! 九筒 */
inline constexpr int Souzu1 = 18;      /*! 一索 */
inline constexpr int Souzu2 = 19;      /*! 二索 */
inline constexpr int Souzu3 = 20;      /*! 三索 */
inline constexpr int Souzu4 = 21;      /*! 四索 */
inline constexpr int Souzu5 = 22;      /*! 五索 */
inline constexpr int Souzu6 = 23;      /*! 六索 */
inline constexpr int Souzu7 = 24;      /*! 七索 */
inline constexpr int Souzu8 = 25;      /*! 八索 */
inline constexpr int Souzu9 = 26;      /*! 九索 */
inline constexpr int East = 27;        /*! 東 */
inline constexpr int South = 28;       /*! 南 */
inline constexpr int West = 29;        /*! 西 */
inline constexpr int North = 30;       /*! 北 */
inline constexpr int WhiteDragon = 31; /*! 白 */
inline constexpr int GreenDragon = 32; /*! 發 */
inline constexpr int RedDragon = 33;   /*! 中 */
inline constexpr int RedManzu5 = 34;   /*! 赤五萬 */
inline constexpr int RedPinzu5 = 35;   /*! 赤五筒 */
inline constexpr int RedSouzu5 = 36;   /*! 赤五索 */
inline constexpr int Length = 37;

inline constexpr TileMask RedMask = bit(RedManzu5) | bit(RedPinzu5) | bit(RedSouzu5);

inline constexpr TileMask ManzuMask =
    bit(Manzu1) | bit(Manzu2) | bit(Manzu3) | bit(Manzu4) | bit(Manzu5) | bit(Manzu6) |
    bit(Manzu7) | bit(Manzu8) | bit(Manzu9) | bit(RedManzu5);

inline constexpr TileMask PinzuMask =
    bit(Pinzu1) | bit(Pinzu2) | bit(Pinzu3) | bit(Pinzu4) | bit(Pinzu5) | bit(Pinzu6) |
    bit(Pinzu7) | bit(Pinzu8) | bit(Pinzu9) | bit(RedPinzu5);

inline constexpr TileMask SouzuMask =
    bit(Souzu1) | bit(Souzu2) | bit(Souzu3) | bit(Souzu4) | bit(Souzu5) | bit(Souzu6) |
    bit(Souzu7) | bit(Souzu8) | bit(Souzu9) | bit(RedSouzu5);

inline constexpr TileMask SuitMask = ManzuMask | PinzuMask | SouzuMask;

inline constexpr TileMask HonorMask = bit(East) | bit(South) | bit(West) | bit(North) |
                                      bit(WhiteDragon) | bit(GreenDragon) |
                                      bit(RedDragon);

inline constexpr TileMask TerminalMask =
    bit(Manzu1) | bit(Manzu9) | bit(Pinzu1) | bit(Pinzu9) | bit(Souzu1) | bit(Souzu9);

inline constexpr TileMask TerminalOrHonorMask = TerminalMask | HonorMask;

inline constexpr TileMask SimpleMask =
    bit(Manzu2) | bit(Manzu3) | bit(Manzu4) | bit(Manzu5) | bit(Manzu6) | bit(Manzu7) |
    bit(Manzu8) | bit(Pinzu2) | bit(Pinzu3) | bit(Pinzu4) | bit(Pinzu5) | bit(Pinzu6) |
    bit(Pinzu7) | bit(Pinzu8) | bit(Souzu2) | bit(Souzu3) | bit(Souzu4) | bit(Souzu5) |
    bit(Souzu6) | bit(Souzu7) | bit(Souzu8) | bit(RedManzu5) | bit(RedPinzu5) |
    bit(RedSouzu5);

inline constexpr TileMask SanmaDisabledMask = bit(Manzu2) | bit(Manzu3) | bit(Manzu4) |
                                              bit(Manzu5) | bit(Manzu6) | bit(Manzu7) |
                                              bit(Manzu8) | bit(RedManzu5);

/**
 * @brief Returns the tile name.
 */
inline constexpr std::string_view name(int tile) noexcept
{
    switch (tile) {
    case Null:
        return "Null";
    case Manzu1:
        return "1m";
    case Manzu2:
        return "2m";
    case Manzu3:
        return "3m";
    case Manzu4:
        return "4m";
    case Manzu5:
        return "5m";
    case Manzu6:
        return "6m";
    case Manzu7:
        return "7m";
    case Manzu8:
        return "8m";
    case Manzu9:
        return "9m";
    case Pinzu1:
        return "1p";
    case Pinzu2:
        return "2p";
    case Pinzu3:
        return "3p";
    case Pinzu4:
        return "4p";
    case Pinzu5:
        return "5p";
    case Pinzu6:
        return "6p";
    case Pinzu7:
        return "7p";
    case Pinzu8:
        return "8p";
    case Pinzu9:
        return "9p";
    case Souzu1:
        return "1s";
    case Souzu2:
        return "2s";
    case Souzu3:
        return "3s";
    case Souzu4:
        return "4s";
    case Souzu5:
        return "5s";
    case Souzu6:
        return "6s";
    case Souzu7:
        return "7s";
    case Souzu8:
        return "8s";
    case Souzu9:
        return "9s";
    case East:
        return "1z";
    case South:
        return "2z";
    case West:
        return "3z";
    case North:
        return "4z";
    case WhiteDragon:
        return "5z";
    case GreenDragon:
        return "6z";
    case RedDragon:
        return "7z";
    case RedManzu5:
        return "0m";
    case RedPinzu5:
        return "0p";
    case RedSouzu5:
        return "0s";
    default:
        return "Unknown";
    }
}

/**
 * @brief Returns the non-red tile for a tile.
 */
inline constexpr int to_normal(int tile) noexcept
{
    if (tile == RedManzu5) {
        return Manzu5;
    }
    if (tile == RedPinzu5) {
        return Pinzu5;
    }
    if (tile == RedSouzu5) {
        return Souzu5;
    }
    return tile;
}

/**
 * @brief Returns whether the tile is a red five.
 *
 * @pre tile must be in [0, Tile::Length).
 */
inline constexpr bool is_red(int tile) noexcept
{
    return ((RedMask >> tile) & TileMask{1}) != 0;
}

/**
 * @brief Returns whether the tile is a manzu tile.
 *
 * @pre tile must be in [0, Tile::Length).
 */
inline constexpr bool is_manzu(int tile) noexcept
{
    return ((ManzuMask >> tile) & TileMask{1}) != 0;
}

/**
 * @brief Returns whether the tile is a pinzu tile.
 *
 * @pre tile must be in [0, Tile::Length).
 */
inline constexpr bool is_pinzu(int tile) noexcept
{
    return ((PinzuMask >> tile) & TileMask{1}) != 0;
}

/**
 * @brief Returns whether the tile is a souzu tile.
 *
 * @pre tile must be in [0, Tile::Length).
 */
inline constexpr bool is_souzu(int tile) noexcept
{
    return ((SouzuMask >> tile) & TileMask{1}) != 0;
}

/**
 * @brief Returns whether the tile is a suit tile.
 *
 * @pre tile must be in [0, Tile::Length).
 */
inline constexpr bool is_suit(int tile) noexcept
{
    return ((SuitMask >> tile) & TileMask{1}) != 0;
}

/**
 * @brief Returns whether the tile is an honor tile.
 *
 * @pre tile must be in [0, Tile::Length).
 */
inline constexpr bool is_honor(int tile) noexcept
{
    return ((HonorMask >> tile) & TileMask{1}) != 0;
}

/**
 * @brief Returns whether the tile is a terminal tile.
 *
 * @pre tile must be in [0, Tile::Length).
 */
inline constexpr bool is_terminal(int tile) noexcept
{
    return ((TerminalMask >> tile) & TileMask{1}) != 0;
}

/**
 * @brief Returns whether the tile is a terminal or honor tile.
 *
 * @pre tile must be in [0, Tile::Length).
 */
inline constexpr bool is_terminal_or_honor(int tile) noexcept
{
    return ((TerminalOrHonorMask >> tile) & TileMask{1}) != 0;
}

/**
 * @brief Returns whether the tile is a simple tile.
 *
 * @pre tile must be in [0, Tile::Length).
 */
inline constexpr bool is_simple(int tile) noexcept
{
    return ((SimpleMask >> tile) & TileMask{1}) != 0;
}

/**
 * @brief Returns whether the tile is unavailable in sanma.
 *
 * @pre tile must be in [0, Tile::Length).
 */
inline constexpr bool is_sanma_disabled(int tile) noexcept
{
    return ((SanmaDisabledMask >> tile) & TileMask{1}) != 0;
}

using DoraTable = std::array<int, Length>;

inline constexpr DoraTable DoraToIndicatorYonma = {
    Manzu9,      Manzu1,      Manzu2, Manzu3, Manzu4, Manzu5, Manzu6, Manzu7,
    Manzu8,      Pinzu9,      Pinzu1, Pinzu2, Pinzu3, Pinzu4, Pinzu5, Pinzu6,
    Pinzu7,      Pinzu8,      Souzu9, Souzu1, Souzu2, Souzu3, Souzu4, Souzu5,
    Souzu6,      Souzu7,      Souzu8, North,  East,   South,  West,   RedDragon,
    WhiteDragon, GreenDragon, Manzu4, Pinzu4, Souzu4,
};

inline constexpr DoraTable IndicatorToDoraYonma = {
    Manzu2,    Manzu3,      Manzu4, Manzu5, Manzu6, Manzu7, Manzu8, Manzu9,
    Manzu1,    Pinzu2,      Pinzu3, Pinzu4, Pinzu5, Pinzu6, Pinzu7, Pinzu8,
    Pinzu9,    Pinzu1,      Souzu2, Souzu3, Souzu4, Souzu5, Souzu6, Souzu7,
    Souzu8,    Souzu9,      Souzu1, South,  West,   North,  East,   GreenDragon,
    RedDragon, WhiteDragon, Manzu6, Pinzu6, Souzu6,
};

// In sanma, 2m-8m and red 5m are invalid tiles. They map to Null.
// The valid manzu dora cycle is 1m <-> 9m.
inline constexpr DoraTable DoraToIndicatorSanma = {
    Manzu9,      Null,        Null,   Null,   Null,   Null,   Null,   Null,
    Manzu1,      Pinzu9,      Pinzu1, Pinzu2, Pinzu3, Pinzu4, Pinzu5, Pinzu6,
    Pinzu7,      Pinzu8,      Souzu9, Souzu1, Souzu2, Souzu3, Souzu4, Souzu5,
    Souzu6,      Souzu7,      Souzu8, North,  East,   South,  West,   RedDragon,
    WhiteDragon, GreenDragon, Null,   Pinzu4, Souzu4,
};

inline constexpr DoraTable IndicatorToDoraSanma = {
    Manzu9,    Null,        Null,   Null,   Null,   Null,   Null,   Null,
    Manzu1,    Pinzu2,      Pinzu3, Pinzu4, Pinzu5, Pinzu6, Pinzu7, Pinzu8,
    Pinzu9,    Pinzu1,      Souzu2, Souzu3, Souzu4, Souzu5, Souzu6, Souzu7,
    Souzu8,    Souzu9,      Souzu1, South,  West,   North,  East,   GreenDragon,
    RedDragon, WhiteDragon, Null,   Pinzu6, Souzu6,
};

/**
 * @brief Returns the dora indicator for a dora tile.
 */
inline constexpr int to_indicator(int dora, int game_mode) noexcept
{
    return game_mode == GameMode::Sanma ? DoraToIndicatorSanma[dora]
                                        : DoraToIndicatorYonma[dora];
}

/**
 * @brief Returns the dora tile for a dora indicator.
 */
inline constexpr int to_dora(int indicator, int game_mode) noexcept
{
    return game_mode == GameMode::Sanma ? IndicatorToDoraSanma[indicator]
                                        : IndicatorToDoraYonma[indicator];
}

} // namespace Tile

using Hand = std::array<int, Tile::Length>;

} // namespace mahjong

#endif // MAHJONG_CPP_TILE_HPP
