#ifndef MAHJONG_CPP_MELD
#define MAHJONG_CPP_MELD

#include <utility>
#include <vector>

#include "mahjong/types/constants.hpp"

namespace mahjong
{

/**
 * @brief Meld block.
 */
struct Meld
{
    Meld() = default;

    Meld(const int type, std::vector<int> tiles) : type(type), tiles(std::move(tiles))
    {
    }

    Meld(const int type, std::vector<int> tiles, const int discarded_tile,
         const int from)
        : type(type)
        , tiles(std::move(tiles))
        , discarded_tile(discarded_tile)
        , from(from)
    {
    }

    /*! Meld type. */
    int type = MeldType::Null;

    /*! Tiles contained in the meld. */
    std::vector<int> tiles;

    /*! Discarded tile used for the call. */
    int discarded_tile = Tile::Null;

    /*! Relative seat of the player who discarded the called tile. */
    int from = SeatType::Null;
};

} // namespace mahjong

#endif // MAHJONG_CPP_MELD
