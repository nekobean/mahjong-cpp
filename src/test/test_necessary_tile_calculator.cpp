#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include <cassert>
#include <fstream>
#include <iostream>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/dll.hpp>
#include <catch2/catch.hpp>
#include <spdlog/spdlog.h>

#include "mahjong/mahjong.hpp"

using namespace mahjong;

using TestCase = Hand;

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
        for (int i = 0; i < 13; ++i) { // 13枚だけ読み込む
            int tile = std::stoi(tokens[i]);
            ++hand[Tile::to_normal(tile)];
        }
        assert(std::accumulate(hand.begin(), hand.begin() + 34, 0) == 13);

        cases.emplace_back(hand);
    }

    spdlog::info("{} testcases loaded.", cases.size());

    return true;
}

TEST_CASE("Necessary tile calculator for standard hand")
{
    boost::filesystem::path filepath = boost::filesystem::path(CMAKE_TESTCASE_DIR) /
                                       "test_unnecessary_tile_calculator.txt";

    std::vector<TestCase> cases;
    if (!load_testcase(filepath.string(), cases)) {
        return;
    }

    SECTION("Necessary tile calculator for standard hand")
    {
        double avg_tiles = 0;
        for (auto &hand : cases) {
            int shanten = std::get<1>(ShantenCalculator::calc(
                hand, 0, ShantenFlag::StandardHand, GameMode::Yonma));

            std::vector<int> tiles;
            for (int tile = 0; tile < 34; ++tile) {
                if (hand[tile] == 4) {
                    continue;
                }

                hand[tile]++;
                if (shanten >
                    std::get<1>(ShantenCalculator::calc(
                        hand, 0, ShantenFlag::StandardHand, GameMode::Yonma))) {
                    tiles.push_back(tile);
                }
                hand[tile]--;
            }

            const auto [_, shanten2, tiles2] = NecessaryTileCalculator::select(
                hand, 0, ShantenFlag::StandardHand, GameMode::Yonma);
            avg_tiles += tiles.size();

            INFO(fmt::format("手牌: {}", to_mpsz(hand)));
            REQUIRE(shanten == shanten2);
            REQUIRE(tiles == tiles2);
        }

        spdlog::info("Average number of tiles: {}", avg_tiles / cases.size());
    };

    BENCHMARK("Necessary tile calculator for standard hand")
    {
        for (const auto &hand : cases) {
            NecessaryTileCalculator::select(hand, 0, ShantenFlag::StandardHand,
                                            GameMode::Yonma);
        }
    };
}

TEST_CASE("Necessary tile calculator for Seven Pairs")
{
    boost::filesystem::path filepath = boost::filesystem::path(CMAKE_TESTCASE_DIR) /
                                       "test_unnecessary_tile_calculator.txt";

    std::vector<TestCase> cases;
    if (!load_testcase(filepath.string(), cases)) {
        return;
    }

    SECTION("Necessary tile calculator for Seven Pairs")
    {
        double avg_tiles = 0;
        for (auto &hand : cases) {
            int shanten = std::get<1>(ShantenCalculator::calc(
                hand, 0, ShantenFlag::SevenPairs, GameMode::Yonma));

            std::vector<int> tiles;
            for (int tile = 0; tile < 34; ++tile) {
                if (hand[tile] == 4) {
                    continue;
                }

                hand[tile]++;
                if (shanten > std::get<1>(ShantenCalculator::calc(
                                  hand, 0, ShantenFlag::SevenPairs, GameMode::Yonma))) {
                    tiles.push_back(tile);
                }
                hand[tile]--;
            }

            const auto [_, shanten2, tiles2] = NecessaryTileCalculator::select(
                hand, 0, ShantenFlag::SevenPairs, GameMode::Yonma);
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
            NecessaryTileCalculator::select(hand, 0, ShantenFlag::SevenPairs,
                                            GameMode::Yonma);
        }
    };
}

TEST_CASE("Necessary tile calculator for Thirteen Orphans")
{
    boost::filesystem::path filepath = boost::filesystem::path(CMAKE_TESTCASE_DIR) /
                                       "test_unnecessary_tile_calculator.txt";

    std::vector<TestCase> cases;
    if (!load_testcase(filepath.string(), cases)) {
        return;
    }

    SECTION("Necessary tile calculator for Thirteen Orphans")
    {
        double avg_tiles = 0;
        for (auto &hand : cases) {
            int shanten = std::get<1>(ShantenCalculator::calc(
                hand, 0, ShantenFlag::ThirteenOrphans, GameMode::Yonma));

            std::vector<int> tiles;
            for (int tile :
                 {Tile::Manzu1, Tile::Manzu9, Tile::Pinzu1, Tile::Pinzu9, Tile::Souzu1,
                  Tile::Souzu9, Tile::East, Tile::South, Tile::West, Tile::North,
                  Tile::WhiteDragon, Tile::GreenDragon, Tile::RedDragon}) {
                if (hand[tile] == 4) {
                    continue;
                }

                hand[tile]++;
                if (shanten >
                    std::get<1>(ShantenCalculator::calc(
                        hand, 0, ShantenFlag::ThirteenOrphans, GameMode::Yonma))) {
                    tiles.push_back(tile);
                }
                hand[tile]--;
            }

            const auto [_, shanten2, tiles2] = NecessaryTileCalculator::select(
                hand, 0, ShantenFlag::ThirteenOrphans, GameMode::Yonma);
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
            NecessaryTileCalculator::select(hand, 0, ShantenFlag::ThirteenOrphans,
                                            GameMode::Yonma);
        }
    };
}

TEST_CASE("Necessary tile calculator")
{
    boost::filesystem::path filepath = boost::filesystem::path(CMAKE_TESTCASE_DIR) /
                                       "test_unnecessary_tile_calculator.txt";

    std::vector<TestCase> cases;
    if (!load_testcase(filepath.string(), cases)) {
        return;
    }

    SECTION("Necessary tile calculator")
    {
        for (auto &hand : cases) {
            const auto [type, shanten] =
                ShantenCalculator::calc(hand, 0, ShantenFlag::All, GameMode::Yonma);

            std::vector<int> tiles;
            for (int tile = 0; tile < 34; ++tile) {
                if (hand[tile] == 4) {
                    continue;
                }

                hand[tile]++;
                const auto [type_after, shanten_after] =
                    ShantenCalculator::calc(hand, 0, ShantenFlag::All, GameMode::Yonma);
                if (shanten_after < shanten) {
                    tiles.push_back(tile);
                }
                hand[tile]--;
            }

            const auto [type2, shanten2, tiles2] = NecessaryTileCalculator::select(
                hand, 0, ShantenFlag::All, GameMode::Yonma);

            INFO(fmt::format("手牌: {}", to_mpsz(hand)));
            REQUIRE(type == type2);
            REQUIRE(shanten == shanten2);
            REQUIRE(tiles == tiles2);
        }
    };

    BENCHMARK("Necessary tile calculator")
    {
        for (const auto &hand : cases) {
            NecessaryTileCalculator::select(hand, 0, ShantenFlag::All, GameMode::Yonma);
        }
    };
}
