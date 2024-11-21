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

    void set_dora(const std::vector<int> &tiles)
    {
        for (const auto tile : tiles) {
            dora_indicators.push_back(ToIndicator[tile]);
        }
    }

    void set_uradora(const std::vector<int> &tiles)
    {
        for (const auto tile : tiles) {
            uradora_indicators.push_back(ToIndicator[tile]);
        }
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
    /* List of dora indicator tiles (表ドラ表示牌) */
    std::vector<int> dora_indicators;
    /* List of uradora indicator tiles (裏ドラ表示牌) */
    std::vector<int> uradora_indicators;

    int self_wind;
};

} // namespace mahjong

#endif // MAHJONG_CPP_ROUND
