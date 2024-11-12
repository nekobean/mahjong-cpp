#include <fstream>
#include <iostream>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/dll.hpp>
#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include <catch2/catch.hpp>

#include "mahjong/core/necessary_tile_calculator.hpp"
#include "mahjong/core/shanten_calculator.hpp"
#include "mahjong/mahjong.hpp"

using namespace mahjong;

/**
 * @brief Load test cases.
 *
 * @param[out] cases Test cases
 * @return Returns true if loading is successful, otherwise false.
 */
bool load_testcase(std::vector<Hand> &cases)
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

        std::vector<int> tiles(14);
        for (int i = 0; i < 14; ++i) {
            tiles[i] = std::stoi(tokens[i]);
        }

        cases.emplace_back(tiles);
    }

    spdlog::info("{} testcases loaded.", cases.size());

    return true;
}

TEST_CASE("Required tile selection of regular hand")
{
    std::vector<Hand> cases;
    if (!load_testcase(cases)) {
        return;
    }

    SECTION("Required tile selection of regular hand")
    {
        double avg_tiles = 0;
        for (auto &hand : cases) {
            int shanten = ShantenCalculator::calc_regular(hand);

            std::vector<int> tiles;
            for (int tile = 0; tile < 34; ++tile) {
                if (hand.counts[tile] == 4) {
                    continue;
                }

                hand.counts[tile]++;
                if (shanten > ShantenCalculator::calc_regular(hand)) {
                    tiles.push_back(tile);
                }
                hand.counts[tile]--;
            }

            auto [shanten2, tiles2] = RequiredTileCalculator::select_regular(hand);
            avg_tiles += tiles.size();

            INFO(fmt::format("手牌: {}", hand.to_string()));
            REQUIRE(shanten == shanten2);
            REQUIRE(tiles == tiles2);
        }

        spdlog::info("Average number of tiles: {}", avg_tiles / cases.size());
    };

    BENCHMARK("Required tile selection of regular hand")
    {
        for (const auto &hand : cases) {
            RequiredTileCalculator::select_regular(hand);
        }
    };
}

TEST_CASE("Required tile selection of Chiitoitsu")
{
    std::vector<Hand> cases;
    if (!load_testcase(cases)) {
        return;
    }

    SECTION("Required tile selection of Chiitoitsu")
    {
        double avg_tiles = 0;
        for (auto &hand : cases) {
            int shanten = ShantenCalculator::calc_seven_pairs(hand);

            std::vector<int> tiles;
            for (int tile = 0; tile < 34; ++tile) {
                if (hand.counts[tile] == 4) {
                    continue;
                }

                hand.counts[tile]++;
                if (shanten > ShantenCalculator::calc_seven_pairs(hand)) {
                    tiles.push_back(tile);
                }
                hand.counts[tile]--;
            }

            auto [shanten2, tiles2] = RequiredTileCalculator::select_seven_pairs(hand);
            avg_tiles += tiles.size();

            INFO(fmt::format("手牌: {}", hand.to_string()));
            REQUIRE(shanten == shanten2);
            REQUIRE(tiles == tiles2);
        }

        spdlog::info("Average number of tiles: {}", avg_tiles / cases.size());
    };

    BENCHMARK("Required tile selection of Chiitoitsu")
    {
        for (const auto &hand : cases) {
            RequiredTileCalculator::select_seven_pairs(hand);
        }
    };
}

TEST_CASE("Required tile selection of Kokushimusou")
{
    std::vector<Hand> cases;
    if (!load_testcase(cases)) {
        return;
    }

    SECTION("Required tile selection of Kokushimusou")
    {
        double avg_tiles = 0;
        for (auto &hand : cases) {
            int shanten = ShantenCalculator::calc_thirteen_orphans(hand);

            std::vector<int> tiles;
            for (int tile :
                 {Tile::Manzu1, Tile::Manzu9, Tile::Pinzu1, Tile::Pinzu9, Tile::Souzu1,
                  Tile::Souzu9, Tile::East, Tile::South, Tile::West, Tile::North,
                  Tile::White, Tile::Green, Tile::Red}) {
                if (hand.counts[tile] == 4) {
                    continue;
                }

                hand.counts[tile]++;
                if (shanten > ShantenCalculator::calc_thirteen_orphans(hand)) {
                    tiles.push_back(tile);
                }
                hand.counts[tile]--;
            }

            auto [shanten2, tiles2] =
                RequiredTileCalculator::select_thirteen_orphans(hand);
            avg_tiles += tiles.size();

            INFO(fmt::format("手牌: {}", hand.to_string()));
            REQUIRE(shanten == shanten2);
            REQUIRE(tiles == tiles2);
        }

        spdlog::info("Average number of tiles: {}", avg_tiles / cases.size());
    };

    BENCHMARK("Required tile selection of Kokushimusou")
    {
        for (const auto &hand : cases) {
            RequiredTileCalculator::select_thirteen_orphans(hand);
        }
    };
}

TEST_CASE("Required tile selection")
{
    std::vector<Hand> cases;
    if (!load_testcase(cases)) {
        return;
    }

    SECTION("Required tile selection")
    {
        for (auto &hand : cases) {
            auto [type, shanten] = ShantenCalculator::calc(hand);

            std::vector<int> tiles;
            for (int tile = 0; tile < 34; ++tile) {
                if (hand.counts[tile] == 4) {
                    continue;
                }

                hand.counts[tile]++;
                auto [type_after, shanten_after] = ShantenCalculator::calc(hand);
                if (shanten_after < shanten) {
                    tiles.push_back(tile);
                }
                hand.counts[tile]--;
            }

            auto [type2, shanten2, tiles2] = RequiredTileCalculator::select(hand);

            INFO(fmt::format("手牌: {}", hand.to_string()));
            REQUIRE(type == type2);
            REQUIRE(shanten == shanten2);
            REQUIRE(tiles == tiles2);
        }
    };

    BENCHMARK("Required tile selection")
    {
        for (const auto &hand : cases) {
            RequiredTileCalculator::select(hand);
        }
    };
}
