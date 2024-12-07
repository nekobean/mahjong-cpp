#undef NDEBUG

#include <algorithm>
#include <cassert>
#include <future>
#include <iostream>
#include <thread>
#include <vector>

#include <cppitertools/combinations.hpp>
#include <cppitertools/combinations_with_replacement.hpp>
#include <cppitertools/product.hpp>
#include <cppitertools/range.hpp>
#include <spdlog/spdlog.h>

#include "mahjong/mahjong.hpp"

using namespace mahjong;

using MeldsAndHand = std::tuple<std::vector<Meld>, Hand>;

/**
 * @brief List suits patterns.
 *
 * @tparam N Number of tile types
 * @param counts Number of remaining tiles
 * @return Patterns
 */
std::map<int, size_t> list_suits_patterns(const std::vector<int> &counts)
{
    std::map<int, size_t> patterns;
    for (auto &&pattern :
         iter::product(iter::range(counts[0] + 1), iter::range(counts[1] + 1),
                       iter::range(counts[2] + 1), iter::range(counts[3] + 1),
                       iter::range(counts[4] + 1), iter::range(counts[5] + 1),
                       iter::range(counts[6] + 1), iter::range(counts[7] + 1),
                       iter::range(counts[8] + 1))) {

        // Count total number of tiles.
        const int sum = std::apply([](auto... args) { return (args + ...); }, pattern);
        if (sum > 14) {
            continue;
        }

        patterns[sum]++;
    }

    return patterns;
}

/**
 * @brief List honors patterns.
 *
 * @tparam N Number of tile types
 * @param counts Number of remaining tiles
 * @return Patterns
 */
std::map<int, size_t> list_honors_patterns(const std::vector<int> &counts)
{
    std::map<int, size_t> patterns;
    for (auto &&pattern :
         iter::product(iter::range(counts[0] + 1), iter::range(counts[1] + 1),
                       iter::range(counts[2] + 1), iter::range(counts[3] + 1),
                       iter::range(counts[4] + 1), iter::range(counts[5] + 1),
                       iter::range(counts[6] + 1))) {

        // Count total number of tiles.
        const int sum = std::apply([](auto... args) { return (args + ...); }, pattern);
        if (sum > 14) {
            continue;
        }

        patterns[sum]++;
    }

    return patterns;
}

/**
 * @brief List meld types.
 *
 * @return Meld types
 */
std::vector<Meld> list_meld_types()
{
    std::vector<Meld> patterns;
    for (const int meld_type : {MeldType::Chow, MeldType::Pong, MeldType::OpenKong}) {
        for (int i = 0; i < 34; ++i) {
            if (meld_type == MeldType::Chow && (i % 9 >= 7 || i >= 27)) {
                continue;
            }

            std::vector<int> tiles;
            if (meld_type == MeldType::Chow) {
                tiles = {i, i + 1, i + 2};
            }
            else if (meld_type == MeldType::Pong) {
                tiles = {i, i, i};
            }
            else if (meld_type == MeldType::OpenKong) {
                tiles = {i, i, i, i};
            }

            patterns.emplace_back(meld_type, tiles);
        }
    }

    return patterns;
}

/**
 * @brief List all patterns of melds where the number of melds is m.
 *
 * @param m Number of melds
 * @return List of melds
 */
std::vector<std::tuple<std::vector<Meld>, std::vector<int>>> list_melds(int m)
{
    const auto meld_types = list_meld_types();

    std::vector<std::tuple<std::vector<Meld>, std::vector<int>>> patterns;
    for (auto &&melds : iter::combinations_with_replacement(meld_types, m)) {
        std::vector<int> wall(34, 4);
        for (const auto &meld : melds) {
            for (const auto tile : meld.tiles) {
                --wall[tile];
            }
        }

        if (std::any_of(wall.begin(), wall.end(), [](const int x) { return x < 0; })) {
            continue;
        }

        patterns.emplace_back(std::vector<Meld>(melds.begin(), melds.end()), wall);
    }

    return patterns;
}

/**
 * @brief List all hands with \a n tiles.
 *
 * @param n Number of tiles
 * @param wall Wall
 * @return Number of hands
 */
size_t list_hands(const int n, const std::vector<int> &wall)
{
    auto manzu_patterns =
        list_suits_patterns(std::vector<int>(wall.begin(), wall.begin() + 9));
    auto pinzu_patterns =
        list_suits_patterns(std::vector<int>(wall.begin() + 9, wall.begin() + 18));
    auto souzu_patterns =
        list_suits_patterns(std::vector<int>(wall.begin() + 18, wall.begin() + 27));
    auto honors_patterns =
        list_honors_patterns(std::vector<int>(wall.begin() + 27, wall.begin() + 34));

    size_t num_hands = 0;
    for (int num_m = 0; num_m <= n; ++num_m) {
        for (int num_p = 0; num_p <= n - num_m; ++num_p) {
            for (int num_s = 0; num_s <= n - num_m - num_p; ++num_s) {
                const int num_z = n - num_m - num_p - num_s;
                assert(num_m + num_p + num_s + num_z == n);
                const auto manzu = manzu_patterns[num_m];
                const auto pinzu = pinzu_patterns[num_p];
                const auto souzu = souzu_patterns[num_s];
                const auto honors = honors_patterns[num_z];
                num_hands += manzu * pinzu * souzu * honors;
            }
        }
    }

    return num_hands;
}

/**
 * @brief List all (melds, hand) patterns where the number of melds is num_melds.
 *
 * @param num_melds Number of melds
 * @return Patterns of (melds, hand)
 */
void list_hand_and_melds(const int num_melds)
{
    const int num_tiles = 14 - 3 * num_melds;
    const auto melds_list = list_melds(num_melds);

    std::vector<std::future<size_t>> futures;

    for (const auto &[melds, wall] : melds_list) {
        futures.push_back(std::async(std::launch::async, list_hands, num_tiles, wall));
    }

    size_t hand_and_melds = 0;
    for (auto &f : futures) {
        hand_and_melds += f.get();
    }

    spdlog::info("num_melds: {}, meld patterns: {}, hand+melds patterns: {}", num_melds,
                 melds_list.size(), hand_and_melds);
}

int main()
{
    list_hand_and_melds(0);
    list_hand_and_melds(1);
    list_hand_and_melds(2);
    list_hand_and_melds(3);
    list_hand_and_melds(4);
    // num_melds: 0 meld patterns: 1 hand+melds patterns: 326520504500
    // num_melds: 1 meld patterns: 89 hand+melds patterns: 592886913600
    // num_melds: 2, meld patterns: 3840, hand+melds patterns: 303564382392
    // ...
    // ...

    return 0;
}
