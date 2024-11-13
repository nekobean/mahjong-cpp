#ifndef MAHJONG_CPP_NECESSARY_TILE_CALCULATOR
#define MAHJONG_CPP_NECESSARY_TILE_CALCULATOR

#include <array>
#include <cstdint>
#include <tuple>
#include <vector>

#include "mahjong/core/shanten_calculator.hpp"
#include "mahjong/types/types.hpp"

namespace mahjong
{

class RequiredTileCalculator
{
    using ResultType = std::array<int64_t, 30>;

  public:
    static std::tuple<int, int, std::vector<int>>
    select(const Hand &hand, const int type = ShantenFlag::Regular |
                                              ShantenFlag::SevenPairs |
                                              ShantenFlag::ThirteenOrphans);
    static std::tuple<int, int, int64_t> calc(const Hand &hand, const int type);
    static std::tuple<int, int64_t> calc_regular(const Hand &hand);
    static std::tuple<int, int64_t> calc_seven_pairs(const Hand &hand);
    static std::tuple<int, int64_t> calc_thirteen_orphans(const Hand &hand);

  private:
    static void add1(ResultType &lhs, const ShantenCalculator::TableType &rhs,
                     const int m);
    static void add2(ResultType &lhs, const ShantenCalculator::TableType &rhs,
                     const int m);
    static void shift(ResultType::value_type &lv, const ResultType::value_type rv,
                      ResultType::value_type &ly, const ResultType::value_type ry);
};

} // namespace mahjong

#endif /* MAHJONG_CPP_NECESSARY_TILE_CALCULATOR */
