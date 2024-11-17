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

    // Count of tiles (手牌+副露ブロックの各牌の枚数)
    std::vector<int> counts;

    // List of meld blocks (副露ブロックの一覧)
    std::vector<MeldedBlock> melds;
};

} // namespace mahjong

#endif // MAHJONG_CPP_PLAYER
