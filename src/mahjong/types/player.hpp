#ifndef MAHJONG_CPP_PLAYER
#define MAHJONG_CPP_PLAYER

#include <vector>

#include "mahjong/types/const.hpp"
#include "mahjong/types/hand.hpp"
#include "mahjong/types/meld.hpp"

namespace mahjong
{

/**
 * @brief Player
 */
class MyPlayer
{
  public:
    MyPlayer() : hand{0}, wind(Tile::Null)
    {
    }

    // Count of hand tiles (手牌の各牌の枚数)
    HandType hand;

    // List of meld blocks (副露ブロックの一覧)
    std::vector<MeldedBlock> melds;

    // Seat wind (自風)
    int wind;

    int num_tiles() const
    {
        return std::accumulate(hand.begin(), hand.begin() + 34, 0);
    }

    int num_melds() const
    {
        return static_cast<int>(melds.size());
    }

    bool is_closed() const
    {
        return melds.empty();
    }
};

} // namespace mahjong

#endif // MAHJONG_CPP_PLAYER
