#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#undef NDEBUG

#include <cassert>
#include <fstream>
#include <iostream>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/dll.hpp>
#include <catch2/catch.hpp>
#include <spdlog/spdlog.h>

#include "mahjong/core/shanten_calculator.hpp"
#include "mahjong/core/string.hpp"
#include "mahjong/core/unnecessary_tile_calculator.hpp"

using namespace mahjong;

using TestCase = Hand;

/**
 * Load a test case from the specified file.
 *
 * @param filepath The path to the file containing the test case data.
 * @param cases list of test cases.
 * @return true if the test case is loaded successfully, false otherwise.
 */
bool load_testcase(const std::string &filepath, std::vector<TestCase> &cases)
{
    cases.clear();

    std::ifstream ifs(filepath);
    if (!ifs) {
        spdlog::error("Failed to open {}.", filepath);
        return false;
    }

    // The format is `<tile1> <tile2> ... <tile14>`
    std::string line;
    while (std::getline(ifs, line)) {
        if (line.empty()) {
            continue;
        }

        std::vector<std::string> tokens;
        boost::split(tokens, line, boost::is_any_of(" "));

        Hand hand{0};
        for (int i = 0; i < 14; ++i) {
            int tile = std::stoi(tokens[i]);
            ++hand[to_no_reddora(tile)];
        }
        assert(std::accumulate(hand.begin(), hand.begin() + 34, 0) == 14);

        cases.push_back(hand);
    }

    spdlog::info("{} testcases loaded.", cases.size());

    return true;
}

TEST_CASE("Unnecessary tile calculator for regular hand")
{
    boost::filesystem::path filepath = boost::filesystem::path(CMAKE_TESTCASE_DIR) /
                                       "test_unnecessary_tile_calculator.txt";

    std::vector<TestCase> cases;
    if (!load_testcase(filepath.string(), cases)) {
        return;
    }

    SECTION("Unnecessary tile calculator for regular hand")
    {
        for (auto &hand : cases) {
            int shanten = ShantenCalculator::calc_regular(hand, 0);

            std::vector<int> tiles;
            for (int tile = 0; tile < 34; ++tile) {
                if (hand[tile] > 0) {
                    hand[tile]--;
                    if (shanten == ShantenCalculator::calc_regular(hand, 0)) {
                        tiles.push_back(tile);
                    }
                    hand[tile]++;
                }
            }

            auto [_, shanten2, tiles2] =
                UnnecessaryTileCalculator::select(hand, 0, ShantenFlag::Regular);

            INFO(fmt::format("手牌: {}", to_mpsz(hand)));
            REQUIRE(shanten == shanten2);
            REQUIRE(tiles == tiles2);
        }
    };

    BENCHMARK("Unnecessary tile calculator for regular hand")
    {
        for (const auto &hand : cases) {
            UnnecessaryTileCalculator::select(hand, 0, ShantenFlag::Regular);
        }
    };
}

TEST_CASE("Unnecessary tile calculator for Seven Pairs")
{
    boost::filesystem::path filepath = boost::filesystem::path(CMAKE_TESTCASE_DIR) /
                                       "test_unnecessary_tile_calculator.txt";

    std::vector<TestCase> cases;
    if (!load_testcase(filepath.string(), cases)) {
        return;
    }

    SECTION("Unnecessary tile calculator for Seven Pairs")
    {
        for (auto &hand : cases) {
            int shanten = ShantenCalculator::calc_seven_pairs(hand);

            std::vector<int> tiles;
            for (int tile = 0; tile < 34; ++tile) {
                if (hand[tile] > 0) {
                    hand[tile]--;
                    if (shanten == ShantenCalculator::calc_seven_pairs(hand)) {
                        tiles.push_back(tile);
                    }
                    hand[tile]++;
                }
            }

            auto [_, shanten2, tiles2] =
                UnnecessaryTileCalculator::select(hand, 0, ShantenFlag::SevenPairs);

            INFO(fmt::format("手牌: {}", to_mpsz(hand)));
            REQUIRE(shanten == shanten2);
            REQUIRE(tiles == tiles2);
        }
    };

    BENCHMARK("Unnecessary tile calculator for Seven Pairs")
    {
        for (const auto &hand : cases) {
            UnnecessaryTileCalculator::select(hand, 0, ShantenFlag::SevenPairs);
        }
    };
}

TEST_CASE("Unnecessary tile calculator for Thirteen Orphans")
{
    boost::filesystem::path filepath = boost::filesystem::path(CMAKE_TESTCASE_DIR) /
                                       "test_unnecessary_tile_calculator.txt";

    std::vector<TestCase> cases;
    if (!load_testcase(filepath.string(), cases)) {
        return;
    }

    SECTION("Unnecessary tile calculator for Thirteen Orphans")
    {
        for (auto &hand : cases) {
            int shanten = ShantenCalculator::calc_thirteen_orphans(hand);

            std::vector<int> tiles;
            for (int tile = 0; tile < 34; ++tile) {
                if (hand[tile] > 0) {
                    hand[tile]--;
                    if (shanten == ShantenCalculator::calc_thirteen_orphans(hand)) {
                        tiles.push_back(tile);
                    }
                    hand[tile]++;
                }
            }

            auto [_, shanten2, tiles2] = UnnecessaryTileCalculator::select(
                hand, 0, ShantenFlag::ThirteenOrphans);

            INFO(fmt::format("手牌: {}", to_mpsz(hand)));
            REQUIRE(shanten == shanten2);
            REQUIRE(tiles == tiles2);
        }
    };

    BENCHMARK("Unnecessary tile calculator for Thirteen Orphans")
    {
        for (const auto &hand : cases) {
            UnnecessaryTileCalculator::select(hand, 0, ShantenFlag::ThirteenOrphans);
        }
    };
}

TEST_CASE("Unnecessary tile selection")
{
    boost::filesystem::path filepath = boost::filesystem::path(CMAKE_TESTCASE_DIR) /
                                       "test_unnecessary_tile_calculator.txt";

    std::vector<TestCase> cases;
    if (!load_testcase(filepath.string(), cases)) {
        return;
    }

    SECTION("Unnecessary tile selection")
    {
        for (auto &hand : cases) {
            auto [type, shanten] = ShantenCalculator::calc(hand, 0, ShantenFlag::All);

            std::vector<int> tiles;
            for (int tile = 0; tile < 34; ++tile) {
                if (hand[tile] > 0) {
                    hand[tile]--;
                    auto [type_after, shanten_after] =
                        ShantenCalculator::calc(hand, 0, ShantenFlag::All);
                    if (shanten == shanten_after) {
                        tiles.push_back(tile);
                    }
                    hand[tile]++;
                }
            }

            auto [type2, shanten2, tiles2] =
                UnnecessaryTileCalculator::select(hand, 0, ShantenFlag::All);

            INFO(fmt::format("手牌: {}", to_mpsz(hand)));
            REQUIRE(type == type2);
            REQUIRE(shanten == shanten2);
            REQUIRE(tiles == tiles2);
        }
    };

    BENCHMARK("Unnecessary tile selection")
    {
        for (const auto &hand : cases) {
            UnnecessaryTileCalculator::select(hand, 0, ShantenFlag::All);
        }
    };
}
