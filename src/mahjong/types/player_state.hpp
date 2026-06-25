#ifndef MAHJONG_CPP_PLAYER_STATE_HPP
#define MAHJONG_CPP_PLAYER_STATE_HPP

#include <numeric>
#include <vector>

#include "mahjong/types/meld.hpp"
#include "mahjong/types/tile.hpp"

namespace mahjong
{

/**
 * @brief Player state.
 */
struct PlayerState
{
    /*! Tile counts in the concealed hand. */
    Hand hand{};

    /*! Melds called by the player. */
    std::vector<Meld> melds;

    /*! Seat wind tile. Must be Tile::East, Tile::South, Tile::West, or Tile::North. */
    int seat_wind = Tile::Null;

    /*! Number of nuki dora tiles. */
    int nuki_count = 0;

    /*! Current player score. */
    int score = 0;

    /**
     * @brief Returns the number of concealed hand tiles.
     */
    int num_tiles() const noexcept
    {
        return std::accumulate(hand.begin(), hand.begin() + 34, 0);
    }

    /**
     * @brief Returns the number of melds.
     */
    int num_melds() const noexcept
    {
        return static_cast<int>(melds.size());
    }

    /**
     * @brief Returns whether the hand is closed.
     */
    bool is_closed() const noexcept
    {
        for (const auto &meld : melds) {
            if (meld.type != MeldType::Ankan) {
                return false;
            }
        }

        return true;
    }
};

} // namespace mahjong

#endif // MAHJONG_CPP_PLAYER_STATE_HPP
