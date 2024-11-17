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

#include "mahjong/core/necessary_tile_calculator.hpp"
#include "mahjong/core/shanten_calculator.hpp"
#include "mahjong/core/string.hpp"

using namespace mahjong;

using TestCase = std::vector<int>;

/**
 * Load a test case from the specified file.
 *
 * @param filepath The path to the file containing the test case data.
 * @param cases list of test cases.
 */
bool load_testcase(std::vector<TestCase> &cases)
{
    cases.clear();

    boost::filesystem::path path = boost::filesystem::path(CMAKE_TESTCASE_DIR) /
                                   "test_unnecessary_tile_selector.txt";

    std::ifstream ifs(path.string());
    if (!ifs) {
        std::cerr << "Failed to open " << path.string() << "." << std::endl;
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

        std::vector<int> tiles(34, 0);
        for (int i = 0; i < 13; ++i) { // 13枚だけ読み込む
            int tile = std::stoi(tokens[i]);
            ++tiles[red2normal(tile)];
        }
        assert(std::accumulate(tiles.begin(), tiles.end(), 0) == 13);

        cases.emplace_back(tiles);
    }

    spdlog::info("{} testcases loaded.", cases.size());

    return true;
}

TEST_CASE("Necessary tile calculator for regular hand")
{
    std::vector<TestCase> cases;
    if (!load_testcase(cases)) {
        return;
    }

    SECTION("Necessary tile calculator for regular hand")
    {
        double avg_tiles = 0;
        for (auto &hand : cases) {
            int shanten = ShantenCalculator::calc_regular(hand, 0);

            std::vector<int> tiles;
            for (int tile = 0; tile < 34; ++tile) {
                if (hand[tile] == 4) {
                    continue;
                }

                hand[tile]++;
                if (shanten > ShantenCalculator::calc_regular(hand, 0)) {
                    tiles.push_back(tile);
                }
                hand[tile]--;
            }

            auto [_, shanten2, tiles2] =
                NecessaryTileCalculator::select(hand, 0, ShantenFlag::Regular);
            avg_tiles += tiles.size();

            INFO(fmt::format("手牌: {}", to_mpsz(hand)));
            REQUIRE(shanten == shanten2);
            REQUIRE(tiles == tiles2);
        }

        spdlog::info("Average number of tiles: {}", avg_tiles / cases.size());
    };

    BENCHMARK("Necessary tile calculator for regular hand")
    {
        for (const auto &hand : cases) {
            NecessaryTileCalculator::select(hand, 0, ShantenFlag::Regular);
        }
    };
}

TEST_CASE("Necessary tile calculator for Seven Pairs")
{
    std::vector<TestCase> cases;
    if (!load_testcase(cases)) {
        return;
    }

    SECTION("Necessary tile calculator for Seven Pairs")
    {
        double avg_tiles = 0;
        for (auto &hand : cases) {
            int shanten = ShantenCalculator::calc_seven_pairs(hand);

            std::vector<int> tiles;
            for (int tile = 0; tile < 34; ++tile) {
                if (hand[tile] == 4) {
                    continue;
                }

                hand[tile]++;
                if (shanten > ShantenCalculator::calc_seven_pairs(hand)) {
                    tiles.push_back(tile);
                }
                hand[tile]--;
            }

            auto [_, shanten2, tiles2] =
                NecessaryTileCalculator::select(hand, 0, ShantenFlag::SevenPairs);
            avg_tiles += tiles.size();

            INFO(fmt::format("手牌: {}", to_mpsz(hand)));
            REQUIRE(shanten == shanten2);
            REQUIRE(tiles == tiles2);
        }

        spdlog::info("Average number of tiles: {}", avg_tiles / cases.size());
    };

    BENCHMARK("Necessary tile calculator for Seven Pairs")
    {
        for (const auto &hand : cases) {
            NecessaryTileCalculator::select(hand, 0, ShantenFlag::SevenPairs);
        }
    };
}

TEST_CASE("Necessary tile calculator for Thirteen Orphans")
{
    std::vector<TestCase> cases;
    if (!load_testcase(cases)) {
        return;
    }

    SECTION("Necessary tile calculator for Thirteen Orphans")
    {
        double avg_tiles = 0;
        for (auto &hand : cases) {
            int shanten = ShantenCalculator::calc_thirteen_orphans(hand);

            std::vector<int> tiles;
            for (int tile :
                 {Tile::Manzu1, Tile::Manzu9, Tile::Pinzu1, Tile::Pinzu9, Tile::Souzu1,
                  Tile::Souzu9, Tile::East, Tile::South, Tile::West, Tile::North,
                  Tile::White, Tile::Green, Tile::Red}) {
                if (hand[tile] == 4) {
                    continue;
                }

                hand[tile]++;
                if (shanten > ShantenCalculator::calc_thirteen_orphans(hand)) {
                    tiles.push_back(tile);
                }
                hand[tile]--;
            }

            auto [_, shanten2, tiles2] =
                NecessaryTileCalculator::select(hand, 0, ShantenFlag::ThirteenOrphans);
            avg_tiles += tiles.size();

            INFO(fmt::format("手牌: {}", to_mpsz(hand)));
            REQUIRE(shanten == shanten2);
            REQUIRE(tiles == tiles2);
        }

        spdlog::info("Average number of tiles: {}", avg_tiles / cases.size());
    };

    BENCHMARK("Necessary tile calculator for Thirteen Orphans")
    {
        for (const auto &hand : cases) {
            NecessaryTileCalculator::select(hand, 0, ShantenFlag::ThirteenOrphans);
        }
    };
}

TEST_CASE("Necessary tile calculator")
{
    std::vector<TestCase> cases;
    if (!load_testcase(cases)) {
        return;
    }

    SECTION("Necessary tile calculator")
    {
        for (auto &hand : cases) {
            auto [type, shanten] = ShantenCalculator::calc(hand, 0, ShantenFlag::All);

            std::vector<int> tiles;
            for (int tile = 0; tile < 34; ++tile) {
                if (hand[tile] == 4) {
                    continue;
                }

                hand[tile]++;
                auto [type_after, shanten_after] =
                    ShantenCalculator::calc(hand, 0, ShantenFlag::All);
                if (shanten_after < shanten) {
                    tiles.push_back(tile);
                }
                hand[tile]--;
            }

            auto [type2, shanten2, tiles2] =
                NecessaryTileCalculator::select(hand, 0, ShantenFlag::All);

            INFO(fmt::format("手牌: {}", to_mpsz(hand)));
            REQUIRE(type == type2);
            REQUIRE(shanten == shanten2);
            REQUIRE(tiles == tiles2);
        }
    };

    BENCHMARK("Necessary tile calculator")
    {
        for (const auto &hand : cases) {
            NecessaryTileCalculator::select(hand, 0, ShantenFlag::All);
        }
    };
}
