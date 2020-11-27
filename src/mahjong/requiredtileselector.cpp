#include "requiredtileselector.hpp"

#include "syanten.hpp"

namespace mahjong {

std::vector<int> RequiredTileSelector::select_normal(const Hand &hand)
{
    // 現在の向聴数を計算する。
    int syanten = SyantenCalculator::calc_normal(hand);

    return {};
}

std::vector<int> RequiredTileSelector::select_normal2(const Hand &hand)
{
    // 現在の向聴数を計算する。
    int syanten = SyantenCalculator::calc_normal(hand);

    return {};
}

} // namespace mahjong
