#undef NDEBUG

#include <algorithm> // find
#include <cassert>
#include <iostream>
#include <vector>

#include <cppitertools/combinations_with_replacement.hpp>
#include <cppitertools/product.hpp>
#include <cppitertools/range.hpp>

#include "mahjong/mahjong.hpp"

using namespace mahjong;

using MeldsAndHand = std::tuple<std::vector<Meld>, Hand>;

/**
 * @brief List patterns of x in {0, 1, 2, 3, 4}^N where sum(x) <= 14.
 *
 * @tparam N Number of tile types
 * @return Patterns
 */
template <int N> std::map<int, std::vector<std::array<int, N>>> list_patterns()
{
    std::map<int, std::vector<std::array<int, N>>> patterns;
    for (auto &&pattern : iter::product<N>(iter::range(5))) {
        // Count total number of tiles.
        const int sum = std::apply([](auto... args) { return (args + ...); }, pattern);
        if (sum > 14) {
            continue;
        }

        // Convert tuple to array.
        std::array<int, N> key;
        std::apply(
            [&key](auto... args) {
                int i = 0;
                ((key[i++] = args), ...);
            },
            pattern);

        patterns[sum].push_back(key);
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
std::vector<std::tuple<std::vector<Meld>, Count>> list_melds(int m)
{
    const auto meld_types = list_meld_types();

    std::vector<std::tuple<std::vector<Meld>, Count>> patterns;
    for (auto &&melds : iter::combinations_with_replacement(meld_types, m)) {
        Count wall;
        wall.fill(4);
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

    std::cout << "Number of melds: " << patterns.size() << std::endl;

    return patterns;
}

/**
 * @brief Concatenate the four suits and honors into a hand.
 *
 * @param manzu Manzu tiles
 * @param pinzu Pinzu tiles
 * @param souzu Souzu tiles
 * @param honors Honors tiles
 * @return Hand
 */
Hand concat(const std::array<int, 9> &manzu, const std::array<int, 9> &pinzu,
            const std::array<int, 9> &souzu, const std::array<int, 7> &honors)
{
    Hand hand{0};
    for (int i = 0; i < 9; ++i) {
        hand[i] = manzu[i];
        hand[i + 9] = pinzu[i];
        hand[i + 18] = souzu[i];
    }

    for (int i = 0; i < 7; ++i) {
        hand[i + 27] = honors[i];
    }

    return hand;
}

/**
 * @brief List all hands with \a n tiles.
 *
 * @param n Number of tiles
 * @return list of hands
 */
std::vector<Hand> list_hands(const int n)
{
    std::vector<Hand> hands;
    auto suits_patterns = list_patterns<9>();
    auto honors_patterns = list_patterns<7>();

    size_t num_hands = 0;
    for (int num_m = 0; num_m <= n; ++num_m) {
        for (int num_p = 0; num_p <= n - num_m; ++num_p) {
            for (int num_s = 0; num_s <= n - num_m - num_p; ++num_s) {
                const int num_z = n - num_m - num_p - num_s;
                assert(num_m + num_p + num_s + num_z == n);

                const auto &manzu = suits_patterns[num_m];
                const auto &pinzu = suits_patterns[num_p];
                const auto &souzu = suits_patterns[num_s];
                const auto &honors = honors_patterns[num_z];
                for (const auto &m : manzu) {
                    for (const auto &p : pinzu) {
                        for (const auto &s : souzu) {
                            for (const auto &h : honors) {
                                //hands.push_back(concat(m, p, s, h));
                                ++num_hands;
                            }
                        }
                    }
                }
            }
        }
    }

    std::cout << "Number of hands: " << num_hands << std::endl;

    return hands;
}

/**
 * @brief List all (melds, hand) patterns where the number of melds is num_melds.
 *
 * @param num_melds Number of melds
 * @return Patterns of (melds, hand)
 */
std::vector<MeldsAndHand> list_hand_and_melds(const int num_melds)
{
    std::vector<MeldsAndHand> patterns;
    const int num_tiles = 14 - 3 * num_melds;
    const auto melds_list = list_melds(num_melds);
    const auto hands_list = list_hands(num_tiles);

    return patterns;
}

int main()
{
    // const auto patterns0 = list_hand_and_melds(0);
    const auto patterns1 = list_hand_and_melds(1);
    const auto patterns2 = list_hand_and_melds(2);
    const auto patterns3 = list_hand_and_melds(3);
    const auto patterns4 = list_hand_and_melds(4);

    return 0;
}
