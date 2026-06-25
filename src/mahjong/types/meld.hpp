#ifndef MAHJONG_CPP_MELD_HPP
#define MAHJONG_CPP_MELD_HPP

#include <vector>

#include "mahjong/types/constants.hpp"
#include "mahjong/types/tile.hpp"

namespace mahjong
{

/**
 * @brief Meld.
 */
struct Meld
{
    /*! Meld type. */
    int type = MeldType::Null;

    /*! Tiles in the meld. */
    std::vector<int> tiles;

    /*! Discarded tile used to make the call. */
    int discarded_tile = Tile::Null;

    /*! Relative seat of the player who discarded the called tile. */
    int from = SeatType::Null;
};

} // namespace mahjong

#endif // MAHJONG_CPP_MELD_HPP
