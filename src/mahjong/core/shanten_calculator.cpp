#include "shanten_calculator.hpp"

#include <algorithm> // min, max, copy
#include <limits>    // numeric_limits
#include <numeric>   // accumulate
#include <stdexcept> // invalid_argument

#include <boost/dll.hpp>
#include <spdlog/spdlog.h>

#include "mahjong/types/types.hpp"

namespace mahjong
{

ShantenCalculator::ShantenCalculator()
{
    initialize();
}

/**
 * @brief Calculate the shanten number.
 *
 * @param[in] hand The hand
 * @param[in] type The type of shanten number to calculate
 * @return std::tuple<int, int> (Type of shanten number, shanten number)
 */
std::tuple<int, int> ShantenCalculator::calc(const Hand &hand, int type)
{
#ifdef CHECK_ARGUMENT
    if (type < 0 || type > 7) {
        spdlog::warn("Invalid type {} passed.", type);
        throw std::invalid_argument("Invalid type passed.");
    }
#endif

    std::tuple<int, int> ret = {ShantenFlag::Null, std::numeric_limits<int>::max()};

    if (type & ShantenFlag::Regular) {
        int shanten = calc_regular(hand);
        if (shanten < std::get<1>(ret)) {
            ret = {ShantenFlag::Regular, shanten};
        }
        else if (shanten == std::get<1>(ret)) {
            std::get<0>(ret) |= ShantenFlag::Regular;
        }
    }

    if (type & ShantenFlag::SevenPairs) {
        int shanten = calc_seven_pairs(hand);
        if (shanten < std::get<1>(ret)) {
            ret = {ShantenFlag::SevenPairs, shanten};
        }
        else if (shanten == std::get<1>(ret)) {
            std::get<0>(ret) |= ShantenFlag::SevenPairs;
        }
    }

    if (type & ShantenFlag::ThirteenOrphans) {
        int shanten = calc_thirteen_orphans(hand);
        if (shanten < std::get<1>(ret)) {
            ret = {ShantenFlag::ThirteenOrphans, shanten};
        }
        else if (shanten == std::get<1>(ret)) {
            std::get<0>(ret) |= ShantenFlag::ThirteenOrphans;
        }
    }

    return ret;
}

/**
 * @brief Initialize the calculator.
 *
 * @return Returns true if initialization is successful, otherwise false.
 */
bool ShantenCalculator::initialize()
{
    boost::filesystem::path exe_path = boost::dll::program_location().parent_path();
#ifdef USE_NYANTEN_TABLE
    boost::filesystem::path suits_table_path = exe_path / "suits_table_nyanten.bin";
    boost::filesystem::path honors_table_path = exe_path / "honors_table_nyanten.bin";
#else
    boost::filesystem::path suits_table_path = exe_path / "suits_table.bin";
    boost::filesystem::path honors_table_path = exe_path / "honors_table.bin";
#endif

    return load_table(suits_table_path.string(), suits_table_) &&
           load_table(honors_table_path.string(), honors_table_);
}

void ShantenCalculator::add1(ResultType &lhs, const TableType &rhs, const int m)
{
    for (int i = m + 5; i >= 5; --i) {
        int32_t dist = std::min(lhs[i] + rhs[0], lhs[0] + rhs[i]);
        for (int j = 5; j < i; ++j) {
            dist = std::min(dist, lhs[j] + rhs[i - j]);
            dist = std::min(dist, lhs[i - j] + rhs[j]);
        }
        lhs[i] = dist;
    }

    for (int i = m; i >= 0; --i) {
        int32_t dist = lhs[i] + rhs[0];
        for (int j = 0; j < i; ++j) {
            dist = std::min(dist, lhs[j] + rhs[i - j]);
        }
        lhs[i] = dist;
    }
}

void ShantenCalculator::add2(ResultType &lhs, const TableType &rhs, const int m)
{
    int i = m + 5;
    int32_t dist = std::min(lhs[i] + rhs[0], lhs[0] + rhs[i]);
    for (int j = 5; j < i; ++j) {
        dist = std::min(dist, lhs[j] + rhs[i - j]);
        dist = std::min(dist, lhs[i - j] + rhs[j]);
    }
    lhs[i] = dist;
}

int ShantenCalculator::calc_regular(const Hand &hand)
{
    HashType manzu_hash = calc_suits_hash(hand.counts.begin(), hand.counts.begin() + 9);
    HashType pinzu_hash =
        calc_suits_hash(hand.counts.begin() + 9, hand.counts.begin() + 18);
    HashType souzu_hash =
        calc_suits_hash(hand.counts.begin() + 18, hand.counts.begin() + 27);
    HashType honors_hash =
        calc_honors_hash(hand.counts.begin() + 27, hand.counts.end());
    auto &manzu = suits_table_[manzu_hash];
    auto &pinzu = suits_table_[pinzu_hash];
    auto &souzu = suits_table_[souzu_hash];
    auto &honors = honors_table_[honors_hash];
    int m = 4 - static_cast<int>(hand.melds.size());

    ResultType ret;
    std::copy(manzu.begin(), manzu.begin() + 10, ret.begin());
    add1(ret, pinzu, m);
    add1(ret, souzu, m);
    add2(ret, honors, m);

    return ret[5 + m] - 1;
}

/**
 * @brief Calculate the shanten number for Chiitoitsu.
 *
 * @param[in] hand The hand
 * @return int The shanten number
 */
int ShantenCalculator::calc_seven_pairs(const Hand &hand)
{
    int num_types = 0;
    int num_pairs = 0;
    for (size_t i = 0; i < 34; ++i) {
        num_types += hand.counts[i] > 0;
        num_pairs += hand.counts[i] >= 2;
    }

    // 4枚持ちの牌はそのうち2枚しか対子として使用できないため、その分向聴数を増やす。
    return 6 - num_pairs + std::max(0, 7 - num_types);
}

/**
 * @brief Calculate the shanten number for Kokushimusou.
 *
 * @param[in] hand The hand
 * @return int The shanten number
 */
int ShantenCalculator::calc_thirteen_orphans(const Hand &hand)
{
    int num_types = 0;
    bool has_toitsu = false;
    for (int i : {Tile::Manzu1, Tile::Manzu9, Tile::Pinzu1, Tile::Pinzu9, Tile::Souzu1,
                  Tile::Souzu9, Tile::East, Tile::South, Tile::West, Tile::North,
                  Tile::White, Tile::Green, Tile::Red}) {
        num_types += hand.counts[i] > 0;
        has_toitsu |= hand.counts[i] >= 2;
    }

    return 13 - num_types - has_toitsu;
}

std::array<ShantenCalculator::TableType, ShantenCalculator::SuitsTableSize>
    ShantenCalculator::suits_table_;
std::array<ShantenCalculator::TableType, ShantenCalculator::HonorsTableSize>
    ShantenCalculator::honors_table_;

static ShantenCalculator inst;

} // namespace mahjong
