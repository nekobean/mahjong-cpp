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
        , wind(Tile::Null)
        , kyoku(1)
        , honba(0)
        , kyotaku(0)
        , self_wind(Tile::East)
    {
    }

  public:
    /* Game rule */
    int rules;
    /* Round wind (場風) */
    int wind;
    /* Kyoku (局) */
    int kyoku;
    /* Honba (本場) */
    int honba;
    /* Kyotaku (供託棒の数) */
    int kyotaku;
    /* List of dora tiles */
    std::vector<int> dora_tiles;
    /* List of uradora tiles */
    std::vector<int> uradora_tiles;
    /* List of dora indicator tiles */
    std::vector<int> dora_indicators;
    /* List of uradora indicator tiles */
    std::vector<int> uradora_indicators;

    int self_wind;
};

} // namespace mahjong

#endif // MAHJONG_CPP_ROUND
