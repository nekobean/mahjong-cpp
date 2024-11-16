#ifndef MAHJONG_CPP_ROUND
#define MAHJONG_CPP_ROUND

#include <vector>

#include "mahjong/types/const.hpp"

namespace mahjong
{

/**
 * @brief Round
 */
class Round
{
  public:
    Round()
        : rules(RuleFlag::RedDora | RuleFlag::OpenTanyao)
        , round_wind(Tile::East)
        , num_bonus_sticks(0)
        , num_deposit_sticks(0)
        , self_wind(Tile::East)
    {
    }

  public:
    /* Game rule */
    int rules;
    /* Round wind */
    int round_wind;
    /* Number of bonus sticks */
    int num_bonus_sticks;
    /* Number of deposit sticks */
    int num_deposit_sticks;
    /* List of dora tiles */
    std::vector<int> dora_tiles;
    /* List of uradora tiles */
    std::vector<int> uradora_tiles;
    /* List of dora indicator tiles */
    std::vector<int> dora_indicators;

    int self_wind;
};

} // namespace mahjong

#endif // MAHJONG_CPP_ROUND
