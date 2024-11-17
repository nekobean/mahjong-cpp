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

using namespace mahjong;

using TestCase = std::tuple<std::vector<int>, int, int, int>;

/**
 * Load a test case from the specified file.
 *
 * @param filepath The path to the file containing the test case data.
 * @param cases list of test cases.
 */
bool load_testcase(std::vector<TestCase> &cases)
{
    cases.clear();

    boost::filesystem::path path =
        boost::filesystem::path(CMAKE_TESTCASE_DIR) / "test_shanten_calculator.txt";

    std::ifstream ifs(path.string());
    if (!ifs) {
        std::cerr << "Failed to open " << path.string() << "." << std::endl;
        return false;
    }

    // The format is `<tile1> <tile2> ... <tile14> <shanten number of regular hand>
    //                <shanten number of Thirteen Orphans> <shanten number of Seven Pairs>`
    std::string line;
    while (std::getline(ifs, line)) {
        if (line.empty()) {
            continue;
        }

        std::vector<std::string> tokens;
        boost::split(tokens, line, boost::is_any_of(" "));

        std::vector<int> tiles(34, 0);
        for (int i = 0; i < 14; ++i) {
            int tile = std::stoi(tokens[i]);
            ++tiles[red2normal(tile)];
        }
        assert(std::accumulate(tiles.begin(), tiles.end(), 0) == 14);
        cases.emplace_back(tiles, std::stoi(tokens[14]), std::stoi(tokens[15]),
                           std::stoi(tokens[16]));
    }

    spdlog::info("{} testcases loaded.", cases.size());

    return true;
}

TEST_CASE("Shanten number of regular hand")
{
    std::vector<TestCase> cases;
    if (!load_testcase(cases)) {
        return;
    }

    SECTION("Shanten number of regular hand")
    {
        for (auto &[hand, regular, thirteen_orphans, seven_pairs] : cases) {
            INFO(fmt::format("手牌: {}", to_mpsz(hand)));
            REQUIRE(ShantenCalculator::calc_regular(hand, 0) == regular);
        }
    };

    BENCHMARK("Shanten number of regular hand")
    {
        for (auto &[hand, regular, thirteen_orphans, seven_pairs] : cases) {
            ShantenCalculator::calc_regular(hand, 0);
        }
    };
}

TEST_CASE("Shanten number of Seven Pairs")
{
    std::vector<TestCase> cases;
    if (!load_testcase(cases)) {
        return;
    }

    SECTION("Shanten number of Seven Pairs")
    {
        for (auto &[hand, regular, thirteen_orphans, seven_pairs] : cases) {
            INFO(fmt::format("手牌: {}", to_mpsz(hand)));
            REQUIRE(ShantenCalculator::calc_seven_pairs(hand) == seven_pairs);
        }
    };

    BENCHMARK("Shanten number of Seven Pairs")
    {
        for (auto &[hand, regular, thirteen_orphans, seven_pairs] : cases) {
            ShantenCalculator::calc_seven_pairs(hand);
        }
    };
}

TEST_CASE("Shanten number of Thirteen Orphans")
{
    std::vector<TestCase> cases;
    if (!load_testcase(cases)) {
        return;
    }

    SECTION("Shanten number of Thirteen Orphans")
    {
        for (auto &[hand, regular, thirteen_orphans, seven_pairs] : cases) {
            INFO(fmt::format("手牌: {}", to_mpsz(hand)));
            REQUIRE(ShantenCalculator::calc_thirteen_orphans(hand) == thirteen_orphans);
        }
    };

    BENCHMARK("Shanten number of Thirteen Orphans")
    {
        for (auto &[hand, regular, thirteen_orphans, seven_pairs] : cases) {
            ShantenCalculator::calc_thirteen_orphans(hand);
        }
    };
}

TEST_CASE("Shanten number")
{
    std::vector<TestCase> cases;
    if (!load_testcase(cases)) {
        return;
    }

    SECTION("Shanten number")
    {
        for (auto &[hand, regular, thirteen_orphans, seven_pairs] : cases) {
            int true_shanten = std::min({regular, thirteen_orphans, seven_pairs});
            int true_type =
                (true_shanten == regular ? ShantenFlag::Regular : 0) |
                (true_shanten == thirteen_orphans ? ShantenFlag::ThirteenOrphans : 0) |
                (true_shanten == seven_pairs ? ShantenFlag::SevenPairs : 0);
            auto [type, syanten] = ShantenCalculator::calc(hand, 0, ShantenFlag::All);

            INFO(fmt::format("手牌: {}", to_mpsz(hand)));
            REQUIRE(syanten == true_shanten);
            REQUIRE(type == true_type);
        }
    };

    BENCHMARK("Shanten number")
    {
        for (auto &[hand, regular, thirteen_orphans, seven_pairs] : cases) {
            ShantenCalculator::calc(hand, 0, ShantenFlag::All);
        }
    };
}
