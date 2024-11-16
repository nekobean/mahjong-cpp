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
    MyPlayer() : self_wind(Tile::East)
    {
    }

    std::vector<int> hand;
    std::vector<MeldedBlock> melds;
    int self_wind;
};

} // namespace mahjong

#endif // MAHJONG_CPP_PLAYER
