//
// Calculation algorithm is based on the following pages
// - [麻雀アルゴリズム](https://tomohxx.github.io/mahjong-algorithm-book/)
// - [ネット上の向聴数計算アルゴリズムの知見に勝手に補足する](https://zenn.dev/zurukumo/articles/93ae2c381cbe6d)
// - [【麻雀】シャンテン数 高速計算アルゴリズム #C++ - Qiita](https://qiita.com/KamichanR/items/de08c48f92834c0d1f74)
// - [A Fast and Space-Efficient Algorithm for Calculating Deficient Numbers (a.k.a. Shanten Numbers).pdf](https://www.slideshare.net/slideshow/a-fast-and-space-efficient-algorithm-for-calculating-deficient-numbers-a-k-a-shanten-numbers-pdf/269706674)
//
// 2024/11/20: 以下のページのアルゴリズムから上記の方法に変更
// [向聴数を求めるアルゴリズム - あらの（一人）麻雀研究所](https://mahjong.ara.black/etc/shanten/index.htm)
//
#include "shanten_calculator.hpp"

#include <algorithm> // min, max, copy, any_of
#include <limits>    // numeric_limits
#include <numeric>   // accumulate
#include <stdexcept> // invalid_argument

#include <spdlog/spdlog.h>

#include "mahjong/types/types.hpp"

namespace mahjong
{

/**
 * @brief Calculate the shanten number.
 *
 * @param[in] hand The hand
 * @param[in] type The type of shanten number to calculate
 * @return std::tuple<int, int> (Type of shanten number, shanten number)
 */
std::tuple<int, int> ShantenCalculator::calc(const Hand &hand, const int num_melds,
                                             int type)
{
#ifdef CHECK_ARGUMENTS
    int num_tiles = std::accumulate(hand.begin(), hand.end(), 0) + num_melds * 3;
    bool is_valid_count =
        std::any_of(hand.begin(), hand.end(), [](int x) { return x < 0 || x > 4; });
    if (num_tiles % 3 == 0 || num_tiles > 14 || is_valid_count) {
        throw std::invalid_argument(fmt::format("Invalid hand passed."));
    }

    if (num_melds < 0 || num_melds > 4) {
        throw std::invalid_argument(fmt::format("Invalid num_melds passed."));
    }

    if (type < 0 || type > 7) {
        throw std::invalid_argument(fmt::format("Invalid type passed."));
    }
#endif // CHECK_ARGUMENTS

    std::tuple<int, int> ret = {ShantenFlag::Null, 100};

    if (type & ShantenFlag::Regular) {
        int shanten = calc_regular(hand, num_melds);
        if (shanten < std::get<1>(ret)) {
            ret = {ShantenFlag::Regular, shanten};
        }
        else if (shanten == std::get<1>(ret)) {
            std::get<0>(ret) |= ShantenFlag::Regular;
        }
    }

    if ((type & ShantenFlag::SevenPairs) && num_melds == 0) {
        // closed hand only
        int shanten = calc_seven_pairs(hand);
        if (shanten < std::get<1>(ret)) {
            ret = {ShantenFlag::SevenPairs, shanten};
        }
        else if (shanten == std::get<1>(ret)) {
            std::get<0>(ret) |= ShantenFlag::SevenPairs;
        }
    }

    if ((type & ShantenFlag::ThirteenOrphans) && num_melds == 0) {
        // closed hand only
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

int ShantenCalculator::calc_regular(const Hand &hand, const int num_melds)
{
    Table::HashType manzu_hash = Table::suits_hash(hand.begin(), hand.begin() + 9);
    Table::HashType pinzu_hash = Table::suits_hash(hand.begin() + 9, hand.begin() + 18);
    Table::HashType souzu_hash =
        Table::suits_hash(hand.begin() + 18, hand.begin() + 27);
    Table::HashType honors_hash =
        Table::honors_hash(hand.begin() + 27, hand.begin() + 34);
    const auto &manzu = Table::suits_table_[manzu_hash];
    const auto &pinzu = Table::suits_table_[pinzu_hash];
    const auto &souzu = Table::suits_table_[souzu_hash];
    const auto &honors = Table::honors_table_[honors_hash];
    int m = 4 - num_melds;

    ResultType ret;
    std::copy(manzu.begin(), manzu.begin() + 10, ret.begin());
    add1(ret, pinzu, m);
    add1(ret, souzu, m);
    add2(ret, honors, m);

    return ret[5 + m] - 1;
}

/**
 * @brief Calculate the shanten number for Seven Pairs.
 *
 * @param[in] hand The hand
 * @return int The shanten number
 */
int ShantenCalculator::calc_seven_pairs(const Hand &hand)
{
    int num_types = 0;
    int num_pairs = 0;
    for (size_t i = 0; i < 34; ++i) {
        num_types += hand[i] > 0;
        num_pairs += hand[i] >= 2;
    }

    // 4枚持ちの牌はそのうち2枚しか対子として使用できないため、その分向聴数を増やす。
    return 6 - num_pairs + std::max(0, 7 - num_types);
}

/**
 * @brief Calculate the shanten number for Thirteen Orphans.
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
        num_types += hand[i] > 0;
        has_toitsu |= hand[i] >= 2;
    }

    return 13 - num_types - has_toitsu;
}

void ShantenCalculator::add1(ResultType &lhs, const Table::TableType &rhs, const int m)
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

void ShantenCalculator::add2(ResultType &lhs, const Table::TableType &rhs, const int m)
{
    int i = m + 5;
    int32_t dist = std::min(lhs[i] + rhs[0], lhs[0] + rhs[i]);
    for (int j = 5; j < i; ++j) {
        dist = std::min(dist, lhs[j] + rhs[i - j]);
        dist = std::min(dist, lhs[i - j] + rhs[j]);
    }
    lhs[i] = dist;
}

} // namespace mahjong
