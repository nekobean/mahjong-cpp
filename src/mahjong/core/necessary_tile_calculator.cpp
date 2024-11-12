#include "necessary_tile_calculator.hpp"

namespace mahjong
{

/**
 * @brief 有効牌を選択する。
 *
 * @param[in] hand 手牌
 * @param[in] type 計算対象の向聴数の種類
 * @return std::vector<int> 牌一覧
 */
std::tuple<int, int, std::vector<int>> RequiredTileCalculator::select(const Hand &hand,
                                                                      const int type)
{
    std::tuple<int, int, int64_t> ret = {ShantenFlag::Null,
                                         std::numeric_limits<int>::max(), 0};

    if (type & ShantenFlag::Regular) {
        auto [shanten, wait] = calc_regular(hand);
        if (shanten < std::get<1>(ret)) {
            ret = {ShantenFlag::Regular, shanten, wait};
        }
        else if (shanten == std::get<1>(ret)) {
            std::get<0>(ret) |= ShantenFlag::Regular;
            std::get<2>(ret) |= wait;
        }
    }

    if (type & ShantenFlag::SevenPairs) {
        auto [shanten, wait] = calc_chiitoitsu(hand);
        if (shanten < std::get<1>(ret)) {
            ret = {ShantenFlag::SevenPairs, shanten, wait};
        }
        else if (shanten == std::get<1>(ret)) {
            std::get<0>(ret) |= ShantenFlag::SevenPairs;
            std::get<2>(ret) |= wait;
        }
    }

    if (type & ShantenFlag::ThirteenOrphans) {
        auto [shanten, wait] = calc_kokushimusou(hand);
        if (shanten < std::get<1>(ret)) {
            ret = {ShantenFlag::ThirteenOrphans, shanten, wait};
        }
        else if (shanten == std::get<1>(ret)) {
            std::get<0>(ret) |= ShantenFlag::ThirteenOrphans;
            std::get<2>(ret) |= wait;
        }
    }

    std::vector<int> tiles;
    tiles.reserve(34);
    for (int i = 0; i < 34; ++i) {
        if (std::get<2>(ret) & (INT64_C(1) << i)) {
            tiles.push_back(i);
        }
    }

    return {std::get<0>(ret), std::get<1>(ret), tiles};
}

std::tuple<int, std::vector<int>>
RequiredTileCalculator::select_regular(const Hand &hand)
{
    auto [shanten, wait] = calc_regular(hand);

    std::vector<int> tiles;
    tiles.reserve(34);
    for (int i = 0; i < 34; ++i) {
        if (wait & (INT64_C(1) << i)) {
            tiles.push_back(i);
        }
    }

    return {shanten, tiles};
}

std::tuple<int, std::vector<int>>
RequiredTileCalculator::select_chiitoitsu(const Hand &hand)
{
    auto [shanten, wait] = calc_chiitoitsu(hand);

    std::vector<int> tiles;
    tiles.reserve(34);
    for (int i = 0; i < 34; ++i) {
        if (wait & (INT64_C(1) << i)) {
            tiles.push_back(i);
        }
    }

    return {shanten, tiles};
}

std::tuple<int, std::vector<int>>
RequiredTileCalculator::select_kokushimusou(const Hand &hand)
{
    auto [shanten, wait] = calc_kokushimusou(hand);

    std::vector<int> tiles;
    tiles.reserve(13);
    for (int i = 0; i < 34; ++i) {
        if (wait & (INT64_C(1) << i)) {
            tiles.push_back(i);
        }
    }

    return {shanten, tiles};
}

std::tuple<int, int64_t> RequiredTileCalculator::calc_regular(const Hand &hand)
{
    ShantenCalculator::HashType manzu_hash = ShantenCalculator::calc_suits_hash(
        hand.counts.begin(), hand.counts.begin() + 9);
    ShantenCalculator::HashType pinzu_hash = ShantenCalculator::calc_suits_hash(
        hand.counts.begin() + 9, hand.counts.begin() + 18);
    ShantenCalculator::HashType souzu_hash = ShantenCalculator::calc_suits_hash(
        hand.counts.begin() + 18, hand.counts.begin() + 27);
    ShantenCalculator::HashType honors_hash = ShantenCalculator::calc_honors_hash(
        hand.counts.begin() + 27, hand.counts.end());
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
    int64_t wait = ret[15 + m];

    return {shanten, wait};
}

std::tuple<int, int64_t> RequiredTileCalculator::calc_chiitoitsu(const Hand &hand)
{
    int num_pairs = 0;
    int num_types = 0;
    int64_t count0_flag = 0;
    int64_t count1_flag = 0;

    for (int i = 0; i < 34; ++i) {
        if (hand.counts[i] == 0) {
            count0_flag |= INT64_C(1) << i;
        }
        else if (hand.counts[i] == 1) {
            ++num_types;
            count1_flag |= INT64_C(1) << i;
        }
        else if (hand.counts[i] >= 2) {
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

std::tuple<int, int64_t> RequiredTileCalculator::calc_kokushimusou(const Hand &hand)
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
        if (hand.counts[i] == 0) {
            // 手牌に存在しない么九牌は有効牌である。
            count0_flag |= INT64_C(1) << i;
        }
        else if (hand.counts[i] == 1) {
            // 1枚持ちの么九牌は、他に么九牌の雀頭がない場合は有効牌である。
            count1_flag |= INT64_C(1) << i;
            ++num_types;
        }
        else if (hand.counts[i] >= 2) {
            ++num_types;
            ++num_pairs;
        }
    }

    int shanten = 13 - num_types - bool(num_pairs);
    uint64_t wait = num_pairs ? count0_flag : count0_flag | count1_flag;

    return {shanten, wait};
}

void RequiredTileCalculator::add1(ResultType &lhs,
                                  const ShantenCalculator::TableType &rhs, const int m)
{
    for (int i = m + 5; i >= 5; --i) {
        ResultType::value_type dist = lhs[i] + rhs[0];
        ResultType::value_type wait = (lhs[i + 10] << 9) | rhs[10];
        shift(dist, lhs[0] + rhs[i], wait, (lhs[10] << 9) | rhs[i + 10]);

        for (int j = 5; j < i; ++j) {
            shift(dist, lhs[j] + rhs[i - j], wait,
                  (lhs[j + 10] << 9) | rhs[i - j + 10]);
            shift(dist, lhs[i - j] + rhs[j], wait,
                  (lhs[i - j + 10] << 9) | rhs[j + 10]);
        }

        lhs[i] = dist;
        lhs[i + 10] = wait;
    }

    for (int i = m; i >= 0; --i) {
        ResultType::value_type dist = lhs[i] + rhs[0];
        ResultType::value_type wait = (lhs[i + 10] << 9) | rhs[10];

        for (int j = 0; j < i; ++j) {
            shift(dist, lhs[j] + rhs[i - j], wait,
                  (lhs[j + 10] << 9) | rhs[i - j + 10]);
        }

        lhs[i] = dist;
        lhs[i + 10] = wait;
    }
}

void RequiredTileCalculator::add2(ResultType &lhs,
                                  const ShantenCalculator::TableType &rhs, const int m)
{
    int i = m + 5;
    ResultType::value_type dist = lhs[i] + rhs[0];
    ResultType::value_type wait = (lhs[i + 10] << 9) | rhs[10];
    shift(dist, lhs[0] + rhs[i], wait, (lhs[10] << 9) | rhs[i + 10]);
    for (int j = 5; j < i; ++j) {
        shift(dist, lhs[j] + rhs[i - j], wait, (lhs[j + 10] << 9) | rhs[i - j + 10]);
        shift(dist, lhs[i - j] + rhs[j], wait, (lhs[i - j + 10] << 9) | rhs[j + 10]);
    }

    lhs[i] = dist;
    lhs[i + 10] = wait;
}

void RequiredTileCalculator::shift(ResultType::value_type &lv,
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
