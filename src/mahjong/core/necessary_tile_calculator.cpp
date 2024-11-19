#include "necessary_tile_calculator.hpp"

#include <algorithm> // max, copy, any_of
#include <limits>    // numeric_limits
#include <numeric>   // accumulate
#include <stdexcept> // invalid_argument

#include <spdlog/spdlog.h>

#include "mahjong/core/string.hpp"

namespace mahjong
{

/**
 * @brief Calculate the necessary tiles.
 *
 * @param[in] hand hand
 * @param[in] type shanten number type
 * @return list of (shanten flag, shanten number, necessary tiles)
 */
std::tuple<int, int, std::vector<int>>
NecessaryTileCalculator::select(const HandType &hand, const int num_melds,
                                const int type)
{
    auto ret = calc(hand, num_melds, type);

    std::vector<int> tiles;
    tiles.reserve(34);
    for (int i = 0; i < 34; ++i) {
        if (std::get<2>(ret) & (INT64_C(1) << i)) {
            tiles.push_back(i);
        }
    }

    return {std::get<0>(ret), std::get<1>(ret), tiles};
}

/**
 * @brief Calculate the necessary tiles.
 *
 * @param[in] hand hand
 * @param[in] type shanten number type
 * @return list of (shanten flag, shanten number, necessary tiles)
 */
std::tuple<int, int, int64_t>
NecessaryTileCalculator::calc(const HandType &hand, const int num_melds, const int type)
{
#ifdef CHECK_ARGUMENTS
    int num_tiles = std::accumulate(hand.begin(), hand.end(), 0) + num_melds * 3;
    bool is_valid_count =
        std::any_of(hand.begin(), hand.end(), [](int x) { return x < 0 || x > 4; });
    if (num_tiles % 3 == 0 || num_tiles > 14 || is_valid_count) {
        throw std::invalid_argument(fmt::format("Invalid hand {} passed. {} {}",
                                                to_mpsz(hand), num_tiles, num_melds));
    }

    if (num_melds < 0 || num_melds > 4) {
        throw std::invalid_argument(
            fmt::format("Invalid num_melds {} passed.", num_melds));
    }

    if (type < 0 || type > 7) {
        throw std::invalid_argument(fmt::format("Invalid type {} passed.", type));
    }
#endif // CHECK_ARGUMENTS

    std::tuple<int, int, int64_t> ret = {ShantenFlag::Null,
                                         std::numeric_limits<int>::max(), 0};

    if (type & ShantenFlag::Regular) {
        auto [shanten, wait] = calc_regular(hand, num_melds);
        if (shanten < std::get<1>(ret)) {
            ret = {ShantenFlag::Regular, shanten, wait};
        }
        else if (shanten == std::get<1>(ret)) {
            std::get<0>(ret) |= ShantenFlag::Regular;
            std::get<2>(ret) |= wait;
        }
    }

    if ((type & ShantenFlag::SevenPairs) && num_melds == 0) {
        auto [shanten, wait] = calc_seven_pairs(hand);
        if (shanten < std::get<1>(ret)) {
            ret = {ShantenFlag::SevenPairs, shanten, wait};
        }
        else if (shanten == std::get<1>(ret)) {
            std::get<0>(ret) |= ShantenFlag::SevenPairs;
            std::get<2>(ret) |= wait;
        }
    }

    if ((type & ShantenFlag::ThirteenOrphans) && num_melds == 0) {
        auto [shanten, wait] = calc_thirteen_orphans(hand);
        if (shanten < std::get<1>(ret)) {
            ret = {ShantenFlag::ThirteenOrphans, shanten, wait};
        }
        else if (shanten == std::get<1>(ret)) {
            std::get<0>(ret) |= ShantenFlag::ThirteenOrphans;
            std::get<2>(ret) |= wait;
        }
    }

    return ret;
}

/**
 * @brief Calculate the necessary tiles for regular hand.
 *
 * @param[in] hand hand
 * @param[in] type shanten number type
 * @return list of (shanten flag, shanten number, necessary tiles)
 */
std::tuple<int, int64_t> NecessaryTileCalculator::calc_regular(const HandType &hand,
                                                               const int num_melds)
{
    Table::HashType manzu_hash = Table::suits_hash(hand.begin(), hand.begin() + 9);
    Table::HashType pinzu_hash = Table::suits_hash(hand.begin() + 9, hand.begin() + 18);
    Table::HashType souzu_hash =
        Table::suits_hash(hand.begin() + 18, hand.begin() + 27);
    Table::HashType honors_hash =
        Table::honors_hash(hand.begin() + 27, hand.begin() + 34);
    auto &manzu = Table::suits_table_[manzu_hash];
    auto &pinzu = Table::suits_table_[pinzu_hash];
    auto &souzu = Table::suits_table_[souzu_hash];
    auto &honors = Table::honors_table_[honors_hash];

    int m = 4 - num_melds;

    ResultType ret;
    std::copy(honors.begin(), honors.end(), ret.begin());
    add1(ret, souzu, m);
    add1(ret, pinzu, m);
    add1(ret, manzu, m);

    int shanten = static_cast<int>(ret[5 + m]) - 1;
    int64_t wait = ret[15 + m];

    return {shanten, wait};
}

/**
 * @brief Calculate the necessary tiles for Seven Pairs.
 *
 * @param[in] hand hand
 * @param[in] type shanten number type
 * @return list of (shanten flag, shanten number, necessary tiles)
 */
std::tuple<int, int64_t> NecessaryTileCalculator::calc_seven_pairs(const HandType &hand)
{
    int num_pairs = 0;
    int num_types = 0;
    int64_t count0_flag = 0;
    int64_t count1_flag = 0;

    for (int i = 0; i < 34; ++i) {
        if (hand[i] == 0) {
            count0_flag |= INT64_C(1) << i;
        }
        else if (hand[i] == 1) {
            ++num_types;
            count1_flag |= INT64_C(1) << i;
        }
        else if (hand[i] >= 2) {
            ++num_pairs;
            ++num_types;
        }
    }

    int shanten = 6 - num_pairs + std::max(0, 7 - num_types);
    int64_t wait;
    if (num_types < 7) {
        wait = count0_flag | count1_flag;
    }
    else if (num_pairs == 7) {
        wait = 0;
    }
    else {
        wait = count1_flag;
    }

    return {shanten, wait};
}

/**
 * @brief Calculate the necessary tiles for Thirteen Orphans.
 *
 * @param[in] hand hand
 * @param[in] type shanten number type
 * @return list of (shanten flag, shanten number, necessary tiles)
 */
std::tuple<int, int64_t>
NecessaryTileCalculator::calc_thirteen_orphans(const HandType &hand)
{
    static const auto yaochuu_tiles = {
        Tile::Manzu1, Tile::Manzu9, Tile::Pinzu1, Tile::Pinzu9, Tile::Souzu1,
        Tile::Souzu9, Tile::East,   Tile::South,  Tile::West,   Tile::North,
        Tile::White,  Tile::Green,  Tile::Red};

    int num_pairs = 0;
    int num_types = 0;
    int64_t count0_flag = 0;
    int64_t count1_flag = 0;
    for (int i : yaochuu_tiles) {
        if (hand[i] == 0) {
            // 手牌に存在しない么九牌は有効牌である。
            count0_flag |= INT64_C(1) << i;
        }
        else if (hand[i] == 1) {
            // 1枚持ちの么九牌は、他に么九牌の雀頭がない場合は有効牌である。
            count1_flag |= INT64_C(1) << i;
            ++num_types;
        }
        else if (hand[i] >= 2) {
            ++num_types;
            ++num_pairs;
        }
    }

    int shanten = 13 - num_types - bool(num_pairs);
    uint64_t wait = num_pairs ? count0_flag : count0_flag | count1_flag;

    return {shanten, wait};
}

void NecessaryTileCalculator::add1(ResultType &lhs, const Table::TableType &rhs,
                                   const int m)
{
    auto lhs2 = &lhs[10];
    const auto rhs2 = &rhs[10];

    for (int i = m + 5; i >= 5; --i) {
        ResultType::value_type dist = lhs[i] + rhs[0];
        ResultType::value_type wait = (lhs2[i] << 9) | rhs2[0];
        shift(dist, lhs[0] + rhs[i], wait, (lhs2[0] << 9) | rhs2[i]);

        for (int j = 5; j < i; ++j) {
            shift(dist, lhs[j] + rhs[i - j], wait, (lhs2[j] << 9) | rhs2[i - j]);
            shift(dist, lhs[i - j] + rhs[j], wait, (lhs2[i - j] << 9) | rhs2[j]);
        }

        lhs[i] = dist;
        lhs2[i] = wait;
    }

    for (int i = m; i >= 0; --i) {
        ResultType::value_type dist = lhs[i] + rhs[0];
        ResultType::value_type wait = (lhs2[i] << 9) | rhs2[0];

        for (int j = 0; j < i; ++j) {
            shift(dist, lhs[j] + rhs[i - j], wait, (lhs2[j] << 9) | rhs2[i - j]);
        }

        lhs[i] = dist;
        lhs2[i] = wait;
    }
}

void NecessaryTileCalculator::add2(ResultType &lhs, const Table::TableType &rhs,
                                   const int m)
{
    auto lhs2 = &lhs[10];
    const auto rhs2 = &rhs[10];

    int i = m + 5;
    ResultType::value_type dist = lhs[i] + rhs[0];
    ResultType::value_type wait = (lhs2[i] << 9) | rhs2[0];
    shift(dist, lhs[0] + rhs[i], wait, (lhs2[0] << 9) | rhs2[i]);
    for (int j = 5; j < i; ++j) {
        shift(dist, lhs[j] + rhs[i - j], wait, (lhs2[j] << 9) | rhs2[i - j]);
        shift(dist, lhs[i - j] + rhs[j], wait, (lhs2[i - j] << 9) | rhs2[j]);
    }

    lhs[i] = dist;
    lhs2[i] = wait;
}

void NecessaryTileCalculator::shift(ResultType::value_type &lv,
                                    const ResultType::value_type rv,
                                    ResultType::value_type &ly,
                                    const ResultType::value_type ry)
{
    if (lv == rv) {
        ly |= ry;
    }
    else if (lv > rv) {
        lv = rv;
        ly = ry;
    }
}

} // namespace mahjong
