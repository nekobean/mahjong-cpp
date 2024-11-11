#include "shanten_calculator2.hpp"

#include <algorithm> // min, max, copy
#include <limits>    // numeric_limits
#include <numeric>   // accumulate
#include <stdexcept> // invalid_argument

#include <boost/dll.hpp>
#include <spdlog/spdlog.h>

#include "mahjong/types/types.hpp"

namespace mahjong
{

SyantenCalculator2::SyantenCalculator2()
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
std::tuple<int, int> SyantenCalculator2::calc(const Hand2 &hand, int type)
{
#ifdef CHECK_ARGUMENT
    if (type < 0 || type > 7) {
        spdlog::warn("Invalid type {} passed.", type);
        throw std::invalid_argument("Invalid type passed.");
    }
#endif

    std::tuple<int, int> ret = {ShantenType::Null, std::numeric_limits<int>::max()};

    if (type & ShantenType::Regular) {
        int shanten = calc_regular(hand);
        if (shanten < std::get<1>(ret)) {
            ret = {ShantenType::Regular, shanten};
        }
        else if (shanten == std::get<1>(ret)) {
            std::get<0>(ret) |= ShantenType::Regular;
        }
    }

    if (type & ShantenType::Chiitoitsu) {
        int shanten = calc_chiitoitsu(hand);
        if (shanten < std::get<1>(ret)) {
            ret = {ShantenType::Chiitoitsu, shanten};
        }
        else if (shanten == std::get<1>(ret)) {
            std::get<0>(ret) |= ShantenType::Chiitoitsu;
        }
    }

    if (type & ShantenType::Kokushimusou) {
        int shanten = calc_kokushimusou(hand);
        if (shanten < std::get<1>(ret)) {
            ret = {ShantenType::Kokushimusou, shanten};
        }
        else if (shanten == std::get<1>(ret)) {
            std::get<0>(ret) |= ShantenType::Kokushimusou;
        }
    }

    return ret;
}

/**
 * @brief Initialize the calculator.
 *
 * @return Returns true if initialization is successful, otherwise false.
 */
bool SyantenCalculator2::initialize()
{
    boost::filesystem::path exe_path = boost::dll::program_location().parent_path();
#ifdef NYANTEN
    boost::filesystem::path suits_table_path = exe_path / "suits_table5_nyanten.bin";
    boost::filesystem::path honors_table_path = exe_path / "honors_table_nyanten.bin";
#else
    boost::filesystem::path suits_table_path = exe_path / "suits_table5.bin";
    boost::filesystem::path honors_table_path = exe_path / "honors_table.bin";
#endif

    return load_table(suits_table_path.string(), suits_table_) &&
           load_table(honors_table_path.string(), honors_table_);
}

void SyantenCalculator2::add1(ResultType &lhs, const TableType &rhs, const int m)
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

void SyantenCalculator2::add2(ResultType &lhs, const TableType &rhs, const int m)
{
    int i = m + 5;
    int32_t dist = std::min(lhs[i] + rhs[0], lhs[0] + rhs[i]);
    for (int j = 5; j < i; ++j) {
        dist = std::min(dist, lhs[j] + rhs[i - j]);
        dist = std::min(dist, lhs[i - j] + rhs[j]);
    }
    lhs[i] = dist;
}

int SyantenCalculator2::calc_regular(const Hand2 &hand)
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
int SyantenCalculator2::calc_chiitoitsu(const Hand2 &hand)
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
int SyantenCalculator2::calc_kokushimusou(const Hand2 &hand)
{
    int num_types = 0;
    bool has_toitsu = false;
    for (int i : {Tile::Manzu1, Tile::Manzu9, Tile::Pinzu1, Tile::Pinzu9, Tile::Sozu1,
                  Tile::Sozu9, Tile::Ton, Tile::Nan, Tile::Sya, Tile::Pe, Tile::Haku,
                  Tile::Hatu, Tile::Tyun}) {
        num_types += hand.counts[i] > 0;
        has_toitsu |= hand.counts[i] >= 2;
    }

    return 13 - num_types - has_toitsu;
}

std::array<SyantenCalculator2::TableType, SyantenCalculator2::SuitsTableSize>
    SyantenCalculator2::suits_table_;
std::array<SyantenCalculator2::TableType, SyantenCalculator2::HonorsTableSize>
    SyantenCalculator2::honors_table_;

static SyantenCalculator2 inst;

} // namespace mahjong
