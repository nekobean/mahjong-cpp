#include <algorithm>
#include <array>
#include <chrono>
#include <execution>
#include <fstream>
#include <iostream>
#include <map>
#include <numeric>
#include <vector>
#undef NDEBUG
#include <cassert>

#include <boost/filesystem.hpp>
#include <cppitertools/combinations_with_replacement.hpp>
#include <cppitertools/product.hpp>

#include "mahjong/core/shanten_calculator2.hpp"

using namespace mahjong;
using KeyType = uint32_t;
using ValueType = std::array<KeyType, 10>;
using HashType = SyantenCalculator2::HashType;

std::vector<int> range(int n)
{
    std::vector<int> v(n);
    std::iota(v.begin(), v.end(), 0);
    return v;
}

template <class Tuple,
          class T = std::decay_t<std::tuple_element_t<0, std::decay_t<Tuple>>>>
std::vector<T> to_vector(Tuple &&tuple)
{
    return std::apply(
        [](auto &&...elems) {
            return std::vector<T>{std::forward<decltype(elems)>(elems)...};
        },
        std::forward<Tuple>(tuple));
}

/**
 * @brief 和了形に含まれる数牌の組み合わせを列挙する。
 *
 * @return 和了形に含まれる数牌の組み合わせ
 */
std::vector<std::vector<int>> list_suits_win_patterns()
{
    std::vector<std::vector<int>> patterns;

    std::vector<int> num_shuntsu_patterns = range(5);
    std::vector<int> num_koutsu_patterns = range(5);
    std::vector<int> num_head_patterns = range(2);
    std::vector<int> shuntsu_positions = range(7);
    std::vector<int> koutsu_positions = range(9);
    std::vector<int> head_positions = range(9);

    for (auto &&[num_shuntsu, num_koutsu, num_head] :
         iter::product(num_shuntsu_patterns, num_koutsu_patterns, num_head_patterns)) {
        // 順子の数 + 刻子の数 <= 4、雀頭の数 <= 1 を満たす組み合わせを列挙する。
        if (num_shuntsu + num_koutsu > 4) {
            continue;
        }

        // 順子の数、刻子の数、雀頭の数が与えられたとき、数牌の組み合わせを列挙する。
        for (auto &&shuntsu_pos :
             iter::combinations_with_replacement(shuntsu_positions, num_shuntsu)) {
            for (auto &&koutsu_pos :
                 iter::combinations_with_replacement(koutsu_positions, num_koutsu)) {
                for (auto &&head_pos :
                     iter::combinations_with_replacement(head_positions, num_head)) {

                    std::vector<int> pattern(9, 0);

                    for (auto i : shuntsu_pos) {
                        pattern[i] += 1;
                        pattern[i + 1] += 1;
                        pattern[i + 2] += 1;
                    }

                    for (auto i : koutsu_pos) {
                        pattern[i] += 3;
                    }

                    for (auto i : head_pos) {
                        pattern[i] += 2;
                    }

                    if (std::all_of(pattern.begin(), pattern.end(),
                                    [](int x) { return x <= 4; })) {
                        patterns.push_back(pattern);
                    }
                }
            }
        }
    }

    return patterns;
}

/**
 * @brief 和了形に含まれる字牌の組み合わせを列挙する。
 *
 * @return 和了形に含まれる字牌の組み合わせ
 */
std::vector<std::vector<int>> list_honors_win_patterns()
{
    std::vector<std::vector<int>> patterns;

    std::vector<int> num_koutsu_patterns = range(5);
    std::vector<int> num_head_patterns = range(2);
    std::vector<int> koutsu_positions = range(7);
    std::vector<int> head_positions = range(7);

    for (auto &&[num_koutsu, num_head] :
         iter::product(num_koutsu_patterns, num_head_patterns)) {
        // 順子の数、刻子の数、雀頭の数が与えられたとき、数牌の組み合わせを列挙する。
        for (auto &&koutsu_pos :
             iter::combinations_with_replacement(koutsu_positions, num_koutsu)) {
            for (auto &&head_pos :
                 iter::combinations_with_replacement(head_positions, num_head)) {

                std::vector<int> pattern(9, 0);

                for (auto i : koutsu_pos) {
                    pattern[i] += 3;
                }

                for (auto i : head_pos) {
                    pattern[i] += 2;
                }

                if (std::all_of(pattern.begin(), pattern.end(),
                                [](int x) { return x <= 4; })) {
                    patterns.push_back(pattern);
                }
            }
        }
    }

    return patterns;
}

/**
 * @brief 数牌の組み合わせを列挙する。
 *
 * @return 数牌の組み合わせ
 */
std::vector<std::vector<int>> list_suits_patterns()
{
    std::vector<std::vector<int>> patterns;

    std::vector<int> num_shuntsu_patterns = range(5);

    for (auto &&pattern : iter::product<9>(num_shuntsu_patterns)) {
        std::vector<int> pattern_vec = to_vector(pattern);

        if (std::accumulate(pattern_vec.begin(), pattern_vec.end(), 0) <= 14) {
            patterns.push_back(pattern_vec);
        }
    }

    return patterns;
}

/**
 * @brief 字牌の組み合わせを列挙する。
 *
 * @return 字牌の組み合わせ
 */
std::vector<std::vector<int>> list_honors_patterns()
{
    std::vector<std::vector<int>> patterns;

    std::vector<int> num_shuntsu_patterns = range(5);

    for (auto &&pattern : iter::product<7>(num_shuntsu_patterns)) {
        std::vector<int> pattern_vec = to_vector(pattern);

        if (std::accumulate(pattern_vec.begin(), pattern_vec.end(), 0) <= 14) {
            patterns.push_back(pattern_vec);
        }
    }

    return patterns;
}

int calc_distance(const std::vector<int> &before, const std::vector<int> &after)
{
    int distance = 0;
    for (size_t i = 0; i < before.size(); ++i) {
        distance += std::max(after[i] - before[i], 0);
    }

    return distance;
}

std::map<HashType, ValueType>
create_table(const std::vector<std::vector<int>> &patterns,
             const std::vector<std::vector<int>> &win_patterns)
{
    std::map<HashType, ValueType> table;
    for (const auto &pattern : patterns) {
        HashType hash =
            pattern.size() == 9
                ? SyantenCalculator2::calc_suits_hash(pattern.begin(), pattern.end())
                : SyantenCalculator2::calc_honors_hash(pattern.begin(), pattern.end());
        table[hash].fill(std::numeric_limits<int>::max());
    }

    // 並列処理を行う
    std::for_each(
        std::execution::par, patterns.begin(), patterns.end(),
        [&](const auto &pattern) {
            HashType hash = pattern.size() == 9
                                ? SyantenCalculator2::calc_suits_hash(pattern.begin(),
                                                                      pattern.end())
                                : SyantenCalculator2::calc_honors_hash(pattern.begin(),
                                                                       pattern.end());
            auto &distances = table[hash];

            for (const auto &win_pattern : win_patterns) {
                int num_win_tiles =
                    std::accumulate(win_pattern.begin(), win_pattern.end(), 0);
                int index = num_win_tiles / 3 + (num_win_tiles % 3 != 0 ? 5 : 0);
                int distance = calc_distance(pattern, win_pattern);
                distances[index] =
                    std::min(distances[index], static_cast<KeyType>(distance));
            }

            for (const auto &win_pattern : win_patterns) {
                int num_win_tiles =
                    std::accumulate(win_pattern.begin(), win_pattern.end(), 0);
                int index = num_win_tiles / 3 + (num_win_tiles % 3 != 0 ? 5 : 0);
                int distance = calc_distance(pattern, win_pattern);

                int dist = distances[index] & 0b1111; // 1~4bit
                int wait = distances[index] >> 4;     // 5~13bit
                int desc = distances[index] >> 13;    // 14~22bit
                if (dist == distance) {
                    for (size_t i = 0; i < pattern.size(); ++i) {
                        if (win_pattern[i] > pattern[i]) {
                            wait |= 1 << i;
                        }
                        else if (win_pattern[i] < pattern[i]) {
                            desc |= 1 << i;
                        }
                    }
                }

                distances[index] = dist | (wait << 4) | (desc << 13);
            }
        });

    return table;
}

bool write_file(const std::string &filename, const std::map<HashType, ValueType> &table)
{
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Table file write table file. (path: " << filename << ")"
                  << std::endl;
        return false;
    }

    for (const auto &[hash, distances] : table) {
        file.write(reinterpret_cast<const char *>(&hash), sizeof(hash));
        for (auto i : distances) {
            file.write(reinterpret_cast<const char *>(&i), sizeof(i));
        }
    }

    file.close();

    std::cout << "Table file written. (path: " << filename << ")" << std::endl;

    return true;
}

void create_shanten_table()
{
    std::cout << "Creating suits table..." << std::endl;
#ifdef NYANTEN
    boost::filesystem::path suits_table_path =
        boost::filesystem::path(CMAKE_CONFIG_DIR) / "suits_table5_nyanten.bin";
#else
    boost::filesystem::path suits_table_path =
        boost::filesystem::path(CMAKE_CONFIG_DIR) / "suits_table5.bin";
#endif
    auto suits_patterns = list_suits_patterns();
    auto suits_win_patterns = list_suits_win_patterns();
    auto suits_table = create_table(suits_patterns, suits_win_patterns);
    write_file(suits_table_path.string(), suits_table);
    std::cout << "suits patterns: " << suits_patterns.size() << std::endl;
    std::cout << "suits win patterns: " << suits_win_patterns.size() << std::endl;

    std::cout << "Creating honors table..." << std::endl;
#ifdef NYANTEN
    boost::filesystem::path honors_table_path =
        boost::filesystem::path(CMAKE_CONFIG_DIR) / "honors_table_nyanten.bin";
#else
    boost::filesystem::path honors_table_path =
        boost::filesystem::path(CMAKE_CONFIG_DIR) / "honors_table.bin";
#endif
    auto honors_patterns = list_honors_patterns();
    auto honors_win_patterns = list_honors_win_patterns();
    auto honors_table = create_table(honors_patterns, honors_win_patterns);
    write_file(honors_table_path.string(), honors_table);
    std::cout << "honors patterns: " << honors_patterns.size() << std::endl;
    std::cout << "honors win patterns: " << honors_win_patterns.size() << std::endl;
}

int main()
{
    auto start = std::chrono::high_resolution_clock::now();
    create_shanten_table();
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Elapsed time: "
              << std::chrono::duration_cast<std::chrono::seconds>(end - start).count()
              << " s" << std::endl;
}
