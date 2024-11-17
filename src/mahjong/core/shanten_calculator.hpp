#ifndef MAHJONG_CPP_SHANTEN_CALCULATOR
#define MAHJONG_CPP_SHANTEN_CALCULATOR

#include <array>
#include <cstdint>
#include <tuple>

#include "mahjong/core/table.hpp"
#include "mahjong/types/types.hpp"

namespace mahjong
{
//#define USE_NYANTEN_TABLE

/**
 * @brief Class for calculating shanten number
 */
class ShantenCalculator
{
    using ResultType = std::array<int32_t, 30>;

  public:
    static std::tuple<int, int> calc(const std::vector<int> &hand, const int num_melds,
                                     int type = ShantenFlag::Regular |
                                                ShantenFlag::SevenPairs |
                                                ShantenFlag::ThirteenOrphans);
    static int calc_regular(const std::vector<int> &hand, const int num_melds);
    static int calc_seven_pairs(const std::vector<int> &hand);
    static int calc_thirteen_orphans(const std::vector<int> &hand);

  private:
    static void add1(ResultType &lhs, const Table::TableType &rhs, const int m);
    static void add2(ResultType &lhs, const Table::TableType &rhs, const int m);
};

} // namespace mahjong

#endif /* MAHJONG_CPP_SHANTEN_CALCULATOR */
