#include <fstream>
#include <iostream>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/dll.hpp>
#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include <catch2/catch.hpp>

#include "mahjong/core/shanten_calculator.hpp"
#include "mahjong/mahjong.hpp"

using namespace mahjong;

/**
 * @brief Load test cases.
 *
 * @param[out] cases Test cases
 * @return Returns true if the read was successful, false otherwise.
 */
bool load_testcase(std::vector<std::tuple<Hand, int, int, int>> &cases)
{
    cases.clear();

    boost::filesystem::path path =
        boost::filesystem::path(CMAKE_TESTCASE_DIR) / "test_shanten_calculator.txt";

    std::ifstream ifs(path.string());
    if (!ifs) {
        std::cerr << "Failed to open " << path.string() << "." << std::endl;
        return false;
    }

    // The format is `<tile1> <tile2> ... <tile14> <shanten number of regular hand> <shanten number of Thirteen Orphans> <shanten number of Seven Pairs>`
    std::string line;
    while (std::getline(ifs, line)) {
        std::vector<std::string> tokens;
        boost::split(tokens, line, boost::is_any_of(" "));

        std::vector<int> tiles(34, 0);
        for (int i = 0; i < 14; ++i) {
            int tile = std::stoi(tokens[i]);
            tiles[tile]++;
        }
        Hand hand = Hand::from_array34(tiles);
        int regular_shanten = std::stoi(tokens[14]);
        int kokushi_shanten = std::stoi(tokens[15]);
        int chiitoi_shanten = std::stoi(tokens[16]);
        cases.emplace_back(hand, regular_shanten, kokushi_shanten, chiitoi_shanten);
    }

    spdlog::info("{} testcases loaded.", cases.size());

    return true;
}

TEST_CASE("Shanten number of regular hand")
{
    std::vector<std::tuple<Hand, int, int, int>> cases;
    if (!load_testcase(cases)) {
        return;
    }

    SECTION("Shanten number of regular hand")
    {
        for (auto &[hand, regular, kokushi, chiitoi] : cases) {
            REQUIRE(ShantenCalculator::calc_regular(hand) == regular);
        }
    };

    BENCHMARK("Shanten number of regular hand")
    {
        for (auto &[hand, regular, kokushi, chiitoi] : cases) {
            ShantenCalculator::calc_regular(hand);
        }
    };
}

TEST_CASE("Shanten number of Seven Pairs")
{
    std::vector<std::tuple<Hand, int, int, int>> cases;
    if (!load_testcase(cases)) {
        return;
    }

    SECTION("Shanten number of Seven Pairs")
    {
        for (auto &[hand, regular, kokushi, chiitoi] : cases) {
            REQUIRE(ShantenCalculator::calc_seven_pairs(hand) == chiitoi);
        }
    };

    BENCHMARK("Shanten number of Seven Pairs")
    {
        for (auto &[hand, regular, kokushi, chiitoi] : cases) {
            ShantenCalculator::calc_seven_pairs(hand);
        }
    };
}

TEST_CASE("Shanten number of Thirteen Orphans")
{
    std::vector<std::tuple<Hand, int, int, int>> cases;
    if (!load_testcase(cases)) {
        return;
    }

    SECTION("Shanten number of Thirteen Orphans")
    {
        for (auto &[hand, regular, kokushi, chiitoi] : cases) {
            REQUIRE(ShantenCalculator::calc_thirteen_orphans(hand) == kokushi);
        }
    };

    BENCHMARK("Shanten number of Thirteen Orphans")
    {
        for (auto &[hand, regular, kokushi, chiitoi] : cases) {
            ShantenCalculator::calc_thirteen_orphans(hand);
        }
    };
}

TEST_CASE("Shanten number")
{
    std::vector<std::tuple<Hand, int, int, int>> cases;
    if (!load_testcase(cases)) {
        return;
    }

    SECTION("Shanten number")
    {
        for (auto &[hand, regular, kokushi, chiitoi] : cases) {
            int true_shanten = std::min({regular, kokushi, chiitoi});
            int true_type =
                (true_shanten == regular ? ShantenFlag::Regular : 0) |
                (true_shanten == kokushi ? ShantenFlag::ThirteenOrphans : 0) |
                (true_shanten == chiitoi ? ShantenFlag::SevenPairs : 0);
            auto [type, syanten] = ShantenCalculator::calc(hand);

            REQUIRE(syanten == true_shanten);
            REQUIRE(type == true_type);
        }
    };

    BENCHMARK("Shanten number")
    {
        for (auto &[hand, regular, kokushi, chiitoi] : cases) {
            ShantenCalculator::calc(hand);
        }
    };
}
