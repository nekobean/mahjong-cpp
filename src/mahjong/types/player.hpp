#ifndef MAHJONG_CPP_PLAYER
#define MAHJONG_CPP_PLAYER

#include <vector>

#include "mahjong/types/const.hpp"
#include "mahjong/types/meld.hpp"

namespace mahjong
{

/**
 * @brief Player
 */
class MyPlayer
{
  public:
    MyPlayer() : wind(Tile::Null)
    {
    }

    // Seat wind (自風)
    int wind;

    // Count of hand tiles (手牌の各牌の枚数)
    std::vector<int> hand;

    // List of meld blocks (副露ブロックの一覧)
    std::vector<MeldedBlock> melds;

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
