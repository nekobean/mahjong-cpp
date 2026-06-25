#ifndef MAHJONG_CPP_ROUND_STATE_HPP
#define MAHJONG_CPP_ROUND_STATE_HPP

#include "mahjong/types/constants.hpp"
#include "mahjong/types/tile.hpp"

namespace mahjong
{

/**
 * @brief Round state.
 */
struct RoundState
{
    /*! Round wind tile. */
    int round_wind = Tile::Null;

    /*! 1-based round number within the current round wind. */
    int round_number = 1;

    /*! Honba counter. */
    int honba = 0;

    /*! Dealer player index. */
    int dealer = PlayerIndex::Null;
};

} // namespace mahjong

#endif // MAHJONG_CPP_ROUND_STATE_HPP
