#include "unnecessary_tile_calculator.hpp"

#include <algorithm> // min, max, copy
#include <limits>    // numeric_limits

namespace mahjong
{
/**
 * @brief Calculate the unnecessary tiles.
 *
 * @param[in] hand hand
 * @param[in] type shanten number type
 * @return list of (shanten flag, shanten number, unnecessary tiles)
 */
std::tuple<int, int, std::vector<int>>
UnnecessaryTileCalculator::select(const Hand &hand, const int type)
{
    auto ret = calc(hand, type);

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
 * @brief Calculate the unnecessary tiles.
 *
 * @param[in] hand hand
 * @param[in] type shanten number type
 * @return list of (shanten flag, shanten number, unnecessary tiles)
 */
std::tuple<int, int, int64_t> UnnecessaryTileCalculator::calc(const Hand &hand,
                                                              const int type)
{
    std::tuple<int, int, int64_t> ret = {ShantenFlag::Null,
                                         std::numeric_limits<int>::max(), 0};

    if (type & ShantenFlag::Regular) {
        auto [shanten, disc] = calc_regular(hand);
        if (shanten < std::get<1>(ret)) {
            ret = {ShantenFlag::Regular, shanten, disc};
        }
        else if (shanten == std::get<1>(ret)) {
            std::get<0>(ret) |= ShantenFlag::Regular;
            std::get<2>(ret) |= disc;
        }
    }

    if (type & ShantenFlag::SevenPairs) {
        auto [shanten, disc] = calc_seven_pairs(hand);
        if (shanten < std::get<1>(ret)) {
            ret = {ShantenFlag::SevenPairs, shanten, disc};
        }
        else if (shanten == std::get<1>(ret)) {
            std::get<0>(ret) |= ShantenFlag::SevenPairs;
            std::get<2>(ret) |= disc;
        }
    }

    if (type & ShantenFlag::ThirteenOrphans) {
        auto [shanten, disc] = calc_thirteen_orphans(hand);
        if (shanten < std::get<1>(ret)) {
            ret = {ShantenFlag::ThirteenOrphans, shanten, disc};
        }
        else if (shanten == std::get<1>(ret)) {
            std::get<0>(ret) |= ShantenFlag::ThirteenOrphans;
            std::get<2>(ret) |= disc;
        }
    }

    return ret;
}

/**
 * @brief Calculate the unnecessary tiles for regular hand.
 *
 * @param[in] hand hand
 * @param[in] type shanten number type
 * @return list of (shanten flag, shanten number, unnecessary tiles)
 */
std::tuple<int, int64_t> UnnecessaryTileCalculator::calc_regular(const Hand &hand)
{
    ShantenCalculator::HashType manzu_hash = ShantenCalculator::calc_suits_hash(
        hand.counts.begin(), hand.counts.begin() + 9);
    ShantenCalculator::HashType pinzu_hash = ShantenCalculator::calc_suits_hash(
        hand.counts.begin() + 9, hand.counts.begin() + 18);
    ShantenCalculator::HashType souzu_hash = ShantenCalculator::calc_suits_hash(
        hand.counts.begin() + 18, hand.counts.begin() + 27);
    ShantenCalculator::HashType honors_hash = ShantenCalculator::calc_honors_hash(
        hand.counts.begin() + 27, hand.counts.begin() + 34);
    auto &manzu = ShantenCalculator::suits_table_[manzu_hash];
    auto &pinzu = ShantenCalculator::suits_table_[pinzu_hash];
    auto &souzu = ShantenCalculator::suits_table_[souzu_hash];
    auto &honors = ShantenCalculator::honors_table_[honors_hash];

    int m = 4 - static_cast<int>(hand.melds.size());

    ResultType ret;
    std::copy(honors.begin(), honors.end(), ret.begin());
    add1(ret, souzu, m);
    add1(ret, pinzu, m);
    add1(ret, manzu, m);

    int shanten = static_cast<int>(ret[5 + m]) - 1;
    int64_t disc = ret[25 + m];

    return {shanten, disc};
}

/**
 * @brief Calculate the unnecessary tiles for Seven Pairs.
 *
 * @param[in] hand hand
 * @param[in] type shanten number type
 * @return list of (shanten flag, shanten number, unnecessary tiles)
 */
std::tuple<int, int64_t> UnnecessaryTileCalculator::calc_seven_pairs(const Hand &hand)
{
    int num_pairs = 0;
    int num_types = 0;
    int64_t count1_flag = 0;
    int64_t countge3_flag = 0;

    for (int i = 0; i < 34; ++i) {
        if (hand.counts[i] == 1) {
            ++num_types;
            count1_flag |= INT64_C(1) << i;
        }
        else if (hand.counts[i] == 2) {
            ++num_pairs;
            ++num_types;
        }
        else if (hand.counts[i] >= 3) {
            ++num_pairs;
            ++num_types;
            countge3_flag |= INT64_C(1) << i;
        }
    }

    int shanten = 6 - num_pairs + std::max(0, 7 - num_types);
    int64_t disc = num_types > 7 ? count1_flag | countge3_flag : countge3_flag;

    return {shanten, disc};
}

/**
 * @brief Calculate the unnecessary tiles for Thirteen Orphans.
 *
 * @param[in] hand hand
 * @param[in] type shanten number type
 * @return list of (shanten flag, shanten number, unnecessary tiles)
 */
std::tuple<int, int64_t>
UnnecessaryTileCalculator::calc_thirteen_orphans(const Hand &hand)
{
    static const auto tanyao_tiles = {
        Tile::Manzu2, Tile::Manzu3, Tile::Manzu4, Tile::Manzu5, Tile::Manzu6,
        Tile::Manzu7, Tile::Manzu8, Tile::Pinzu2, Tile::Pinzu3, Tile::Pinzu4,
        Tile::Pinzu5, Tile::Pinzu6, Tile::Pinzu7, Tile::Pinzu8, Tile::Souzu2,
        Tile::Souzu3, Tile::Souzu4, Tile::Souzu5, Tile::Souzu6, Tile::Souzu7,
        Tile::Souzu8};
    static const auto yaochuu_tiles = {
        Tile::Manzu1, Tile::Manzu9, Tile::Pinzu1, Tile::Pinzu9, Tile::Souzu1,
        Tile::Souzu9, Tile::East,   Tile::South,  Tile::West,   Tile::North,
        Tile::White,  Tile::Green,  Tile::Red};

    int num_pairs = 0;
    int num_types = 0;
    int64_t tanyao_flag = 0;
    int64_t count2_flag = 0;
    int64_t countgt2_flag = 0;

    for (const int i : tanyao_tiles) {
        if (hand.counts[i]) {
            tanyao_flag |= INT64_C(1) << i;
        }
    }

    for (int i : yaochuu_tiles) {
        if (hand.counts[i] == 1) {
            ++num_types;
        }
        else if (hand.counts[i] == 2) {
            // 2枚持ちの么九牌は、么九牌の雀頭が2個以上ある場合は不要牌である。
            count2_flag |= INT64_C(1) << i;
            ++num_types;
            ++num_pairs;
        }
        else if (hand.counts[i] > 2) {
            // 3枚以上持ちの么九牌は、不要牌である。
            countgt2_flag |= INT64_C(1) << i;
            ++num_types;
            ++num_pairs;
        }
    }

    int shanten = 13 - num_types - bool(num_pairs);
    uint64_t disc = num_pairs >= 2 ? tanyao_flag | countgt2_flag | count2_flag
                                   : tanyao_flag | countgt2_flag;

    return {shanten, disc};
}

void UnnecessaryTileCalculator::add1(ResultType &lhs,
                                     const ShantenCalculator::TableType &rhs,
                                     const int m)
{
    auto lhs2 = &lhs[20];
    const auto rhs2 = &rhs[20];

    for (int i = m + 5; i >= 5; --i) {
        ResultType::value_type dist = lhs[i] + rhs[0];
        ResultType::value_type disc = (lhs2[i] << 9) | rhs2[0];
        shift(dist, lhs[0] + rhs[i], disc, (lhs2[0] << 9) | rhs2[i]);

        for (int j = 5; j < i; ++j) {
            shift(dist, lhs[j] + rhs[i - j], disc, (lhs2[j] << 9) | rhs2[i - j]);
            shift(dist, lhs[i - j] + rhs[j], disc, (lhs2[i - j] << 9) | rhs2[j]);
        }

        lhs[i] = dist;
        lhs2[i] = disc;
    }

    for (int i = m; i >= 0; --i) {
        ResultType::value_type dist = lhs[i] + rhs[0];
        ResultType::value_type disc = (lhs2[i] << 9) | rhs2[0];

        for (int j = 0; j < i; ++j) {
            shift(dist, lhs[j] + rhs[i - j], disc, (lhs2[j] << 9) | rhs2[i - j]);
        }

        lhs[i] = dist;
        lhs2[i] = disc;
    }
}

void UnnecessaryTileCalculator::add2(ResultType &lhs,
                                     const ShantenCalculator::TableType &rhs,
                                     const int m)
{
    auto lhs2 = &lhs[20];
    const auto rhs2 = &rhs[20];

    int i = m + 5;
    ResultType::value_type dist = lhs[i] + rhs[0];
    ResultType::value_type disc = (lhs2[i] << 9) | rhs2[0];
    shift(dist, lhs[0] + rhs[i], disc, (lhs2[0] << 9) | rhs2[i]);
    for (int j = 5; j < i; ++j) {
        shift(dist, lhs[j] + rhs[i - j], disc, (lhs2[j] << 9) | rhs2[i - j]);
        shift(dist, lhs[i - j] + rhs[j], disc, (lhs2[i - j] << 9) | rhs2[j]);
    }

    lhs[i] = dist;
    lhs2[i] = disc;
}

void UnnecessaryTileCalculator::shift(ResultType::value_type &lv,
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
