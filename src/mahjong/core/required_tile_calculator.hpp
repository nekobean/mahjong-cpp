#ifndef MAHJONG_CPP_REQUIRED_TILE_CALCULATOR
#define MAHJONG_CPP_REQUIRED_TILE_CALCULATOR

#include <vector>

#include "mahjong/core/shanten_calculator2.hpp"
#include "mahjong/types/types.hpp"

namespace mahjong
{

class RequiredTileCalculator
{
    using ResultType = std::array<int64_t, 30>;

  public:
    static std::tuple<int, int, std::vector<int>>
    select(const Hand2 &hand, const int type = ShantenType::Regular |
                                               ShantenType::Chiitoitsu |
                                               ShantenType::Kokushimusou);
    static std::tuple<int, std::vector<int>> select_regular(const Hand2 &hand);
    static std::tuple<int, std::vector<int>> select_chiitoitsu(const Hand2 &hand);
    static std::tuple<int, std::vector<int>> select_kokushimusou(const Hand2 &hand);
    static std::tuple<int, int64_t> calc_regular(const Hand2 &hand);
    static std::tuple<int, int64_t> calc_chiitoitsu(const Hand2 &hand);
    static std::tuple<int, int64_t> calc_kokushimusou(const Hand2 &hand);

  private:
    static void add1(ResultType &lhs, const SyantenCalculator2::TableType &rhs,
                     const int m);
    static void add2(ResultType &lhs, const SyantenCalculator2::TableType &rhs,
                     const int m);
    static void shift(ResultType::value_type &lv, const ResultType::value_type rv,
                      ResultType::value_type &ly, const ResultType::value_type ry);
};

} // namespace mahjong

#endif /* MAHJONG_CPP_REQUIRED_TILE_CALCULATOR */
