#ifndef MAHJONG_CPP_TABLE_STATE_HPP
#define MAHJONG_CPP_TABLE_STATE_HPP

#include <vector>

#include "mahjong/types/tile.hpp"

namespace mahjong
{

/**
 * @brief Shared table state.
 */
struct TableState
{
    /*! Number of deposited riichi sticks. */
    int kyotaku = 0;

    /*! Dora indicator tiles. */
    std::vector<int> dora_indicators;

    /*! Uradora indicator tiles. */
    std::vector<int> uradora_indicators;

    /**
     * @brief Appends dora indicator tiles converted from dora tiles.
     */
    void set_dora(const std::vector<int> &tiles, int game_mode)
    {
        for (const auto tile : tiles) {
            dora_indicators.push_back(Tile::to_indicator(tile, game_mode));
        }
    }

    /**
     * @brief Appends uradora indicator tiles converted from uradora tiles.
     */
    void set_uradora(const std::vector<int> &tiles, int game_mode)
    {
        for (const auto tile : tiles) {
            uradora_indicators.push_back(Tile::to_indicator(tile, game_mode));
        }
    }
};

} // namespace mahjong

#endif // MAHJONG_CPP_TABLE_STATE_HPP
