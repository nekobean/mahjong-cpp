#include <fstream>
#include <iostream>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/dll.hpp>
#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include <catch2/catch.hpp>

#include "mahjong/core/score_calculator.hpp"
#include "mahjong/mahjong.hpp"

using namespace mahjong;

using TestCase = std::tuple<HandSeparator::Input, int, bool>;

/**
 * @brief Load test cases.
 *
 * @param[out] cases Test cases
 * @return Returns true if loading is successful, otherwise false.
 */
bool load_yakuman_cases(const std::string &filename, std::vector<TestCase> &cases)
{
    cases.clear();

    boost::filesystem::path path =
        boost::filesystem::path(CMAKE_TESTCASE_DIR) / filename;

    std::ifstream ifs(path.string());
    if (!ifs) {
        spdlog::error("Failed to open {}.", path.string());
        return false;
    }

    // The format is `<tile1> <tile2> ... <tile14> <win tile> <is valid>`
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
        int win_tile = std::stoi(tokens[14]);
        bool is_valid = tokens[15] == "1";

        MyPlayer player(tiles, Tile::East);
        HandSeparator::Input input =
            ScoreCalculator::create_input(player, win_tile, WinFlag::Null);

        cases.emplace_back(input, win_tile, is_valid);
    }

    return true;
}

TEST_CASE("All Green")
{
    std::vector<TestCase> cases;
    if (!load_yakuman_cases("test_score_calculator_all_green.txt", cases)) {
        return;
    }

    SECTION("All Green")
    {
        for (auto &[input, win_tile, expected] : cases) {
            bool actual = ScoreCalculator::check_all_green(input);
            INFO(fmt::format("hand: {}, win tile: {}", to_mpsz(input.hand),
                             Tile::Name.at(win_tile)));
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("All Green")
    {
        auto &[input, win_tile, expected] = cases.front();
        ScoreCalculator::check_all_green(input);
    };
}

TEST_CASE("Big Three Dragons")
{
    std::vector<TestCase> cases;
    if (!load_yakuman_cases("test_score_calculator_big_three_dragons.txt", cases)) {
        return;
    }

    SECTION("Big Three Dragons")
    {
        for (auto &[input, win_tile, expected] : cases) {
            bool actual = ScoreCalculator::check_big_three_dragons(input);
            INFO(fmt::format("hand: {}, win tile: {}", to_mpsz(input.hand),
                             Tile::Name.at(win_tile)));
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("Big Three Dragons")
    {
        auto &[input, win_tile, expected] = cases.front();
        ScoreCalculator::check_big_three_dragons(input);
    };
}

TEST_CASE("Little Four Winds")
{
    std::vector<TestCase> cases;
    if (!load_yakuman_cases("test_score_calculator_little_four_winds.txt", cases)) {
        return;
    }

    SECTION("Little Four Winds")
    {
        for (auto &[input, win_tile, expected] : cases) {
            bool actual = ScoreCalculator::check_little_four_winds(input);
            INFO(fmt::format("hand: {}, win tile: {}", to_mpsz(input.hand),
                             Tile::Name.at(win_tile)));
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("Little Four Winds")
    {
        auto &[input, win_tile, expected] = cases.front();
        ScoreCalculator::check_little_four_winds(input);
    };
}

TEST_CASE("All Honors")
{
    std::vector<TestCase> cases;
    if (!load_yakuman_cases("test_score_calculator_all_honors.txt", cases)) {
        return;
    }

    SECTION("All Honors")
    {
        for (auto &[input, win_tile, expected] : cases) {
            bool actual = ScoreCalculator::check_all_honors(input);
            INFO(fmt::format("hand: {}, win tile: {}", to_mpsz(input.hand),
                             Tile::Name.at(win_tile)));
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("All Honors")
    {
        auto &[input, win_tile, expected] = cases.front();
        ScoreCalculator::check_all_honors(input);
    };
}

TEST_CASE("Nine Gates")
{
    std::vector<TestCase> cases;
    if (!load_yakuman_cases("test_score_calculator_nine_gates.txt", cases)) {
        return;
    }

    SECTION("Nine Gates")
    {
        for (auto &[input, win_tile, expected] : cases) {
            bool actual = ScoreCalculator::check_nine_gates(input);
            INFO(fmt::format("hand: {}, win tile: {}", to_mpsz(input.hand),
                             Tile::Name.at(win_tile)));
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("Nine Gates")
    {
        auto &[input, win_tile, expected] = cases.front();
        ScoreCalculator::check_nine_gates(input);
    };
}

TEST_CASE("True Nine Gates")
{
    std::vector<TestCase> cases;
    if (!load_yakuman_cases("test_score_calculator_true_nine_gates.txt", cases)) {
        return;
    }

    SECTION("True Nine Gates")
    {
        for (auto &[input, win_tile, expected] : cases) {
            bool actual = ScoreCalculator::check_true_nine_gates(input);
            INFO(fmt::format("hand: {}, win tile: {}", to_mpsz(input.hand),
                             Tile::Name.at(win_tile)));
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("True Nine Gates")
    {
        auto &[input, win_tile, expected] = cases.front();
        ScoreCalculator::check_true_nine_gates(input);
    };
}

TEST_CASE("Four Concealed Triplets")
{
    std::vector<TestCase> cases;
    if (!load_yakuman_cases("test_score_calculator_four_concealed_triplets.txt",
                            cases)) {
        return;
    }

    SECTION("Four Concealed Triplets")
    {
        for (auto &[input, win_tile, expected] : cases) {
            HandSeparator::Input input2 = input;
            input2.win_flag = WinFlag::Tsumo;
            bool actual = ScoreCalculator::check_four_concealed_triplets(input2) >= 1;

            INFO(fmt::format("hand: {}, win tile: {}", to_mpsz(input.hand),
                             Tile::Name.at(win_tile)));
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("Four Concealed Triplets")
    {
        auto &[input, win_tile, expected] = cases.front();
        HandSeparator::Input input2 = input;
        input2.win_flag = WinFlag::Tsumo;
        ScoreCalculator::check_four_concealed_triplets(input2);
    };
}

TEST_CASE("Single Wait Four Concealed Triplets")
{
    std::vector<TestCase> cases;
    if (!load_yakuman_cases(
            "test_score_calculator_single_wait_four_concealed_triplets.txt", cases)) {
        return;
    }

    SECTION("Single Wait Four Concealed Triplets")
    {
        for (auto &[input, win_tile, expected] : cases) {
            HandSeparator::Input input2 = input;
            input2.win_flag = WinFlag::Tsumo;
            bool actual = ScoreCalculator::check_four_concealed_triplets(input2) == 2;
            INFO(fmt::format("hand: {}, win tile: {}", to_mpsz(input.hand),
                             Tile::Name.at(win_tile)));
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("Single Wait Four Concealed Triplets")
    {
        auto &[input, win_tile, expected] = cases.front();
        HandSeparator::Input input2 = input;
        input2.win_flag = WinFlag::Tsumo;
        ScoreCalculator::check_four_concealed_triplets(input2);
    };
}

TEST_CASE("All Terminals")
{
    std::vector<TestCase> cases;
    if (!load_yakuman_cases("test_score_calculator_all_terminals.txt", cases)) {
        return;
    }

    SECTION("All Terminals")
    {
        for (auto &[input, win_tile, expected] : cases) {
            bool actual = ScoreCalculator::check_all_terminals(input);
            INFO(fmt::format("hand: {}, win tile: {}", to_mpsz(input.hand),
                             Tile::Name.at(win_tile)));
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("All Terminals")
    {
        auto &[input, win_tile, expected] = cases.front();
        ScoreCalculator::check_all_terminals(input);
    };
}

TEST_CASE("Four Kongs")
{
    SECTION("Four Kongs established")
    {
        Meld block1({MeldType::ClosedKong,
                     {Tile::Manzu1, Tile::Manzu1, Tile::Manzu1, Tile::Manzu1},
                     Tile::Manzu1,
                     PlayerType::Null});
        Meld block2({MeldType::OpenKong,
                     {Tile::Pinzu1, Tile::Pinzu1, Tile::Pinzu1, Tile::Pinzu1},
                     Tile::Pinzu1,
                     PlayerType::Player1});
        Meld block3({MeldType::ClosedKong,
                     {Tile::Souzu1, Tile::Souzu1, Tile::Souzu1, Tile::Souzu1},
                     Tile::Souzu1,
                     PlayerType::Null});
        Meld block4({MeldType::AddedKong,
                     {Tile::East, Tile::East, Tile::East, Tile::East},
                     Tile::East,
                     PlayerType::Player1});
        std::vector<Meld> melds = {block1, block2, block3, block4};
        std::vector<int> tiles = {Tile::White, Tile::White};

        MyPlayer player(tiles, melds, Tile::East);
        HandSeparator::Input input =
            ScoreCalculator::create_input(player, Tile::White, WinFlag::Null);

        bool expected = true;
        bool actual = ScoreCalculator::check_four_kongs(input);
        INFO(fmt::format("hand: {}, win tile: {}", to_mpsz(input.hand),
                         Tile::Name.at(input.win_tile)));
        REQUIRE(actual == expected);
    };

    SECTION("Four Kongs not established")
    {
        Meld block1({MeldType::Pong,
                     {Tile::Manzu1, Tile::Manzu1, Tile::Manzu1},
                     Tile::Manzu1,
                     PlayerType::Player1});
        Meld block2({MeldType::OpenKong,
                     {Tile::Pinzu1, Tile::Pinzu1, Tile::Pinzu1, Tile::Pinzu1},
                     Tile::Pinzu1,
                     PlayerType::Player1});
        Meld block3({MeldType::ClosedKong,
                     {Tile::Souzu1, Tile::Souzu1, Tile::Souzu1, Tile::Souzu1},
                     Tile::Souzu1,
                     PlayerType::Null});
        Meld block4({MeldType::AddedKong,
                     {Tile::East, Tile::East, Tile::East, Tile::East},
                     Tile::East,
                     PlayerType::Player1});
        std::vector<Meld> melds = {block1, block2, block3, block4};
        std::vector<int> tiles = {Tile::White, Tile::White};
        MyPlayer player(tiles, melds, Tile::East);

        HandSeparator::Input input =
            ScoreCalculator::create_input(player, Tile::White, WinFlag::Null);

        int win_tile = Tile::White;
        bool expected = false;
        bool actual = ScoreCalculator::check_four_kongs(input);
        INFO(fmt::format("hand: {}, win tile: {}", to_mpsz(input.hand),
                         Tile::Name.at(input.win_tile)));
        REQUIRE(actual == expected);
    };

    SECTION("Four Kongs not established")
    {
        Meld block1({MeldType::OpenKong,
                     {Tile::Pinzu1, Tile::Pinzu1, Tile::Pinzu1, Tile::Pinzu1},
                     Tile::Pinzu1,
                     PlayerType::Player1});
        Meld block2({MeldType::ClosedKong,
                     {Tile::Souzu1, Tile::Souzu1, Tile::Souzu1, Tile::Souzu1},
                     Tile::Souzu1,
                     PlayerType::Null});
        Meld block3({MeldType::AddedKong,
                     {Tile::East, Tile::East, Tile::East, Tile::East},
                     Tile::East,
                     PlayerType::Player1});
        std::vector<Meld> melds = {block1, block2, block3};
        std::vector<int> tiles = {Tile::White, Tile::White};
        MyPlayer player(tiles, melds, Tile::East);

        HandSeparator::Input input =
            ScoreCalculator::create_input(player, Tile::White, WinFlag::Null);

        int win_tile = Tile::White;
        bool expected = false;
        bool actual = ScoreCalculator::check_four_kongs(input);
        INFO(fmt::format("hand: {}, win tile: {}", to_mpsz(input.hand),
                         Tile::Name.at(input.win_tile)));
        REQUIRE(actual == expected);
    };
}

TEST_CASE("Big Four Winds")
{
    std::vector<TestCase> cases;
    if (!load_yakuman_cases("test_score_calculator_big_four_winds.txt", cases)) {
        return;
    }

    SECTION("Big Four Winds")
    {
        for (auto &[input, win_tile, expected] : cases) {
            bool actual = ScoreCalculator::check_big_four_winds(input);
            INFO(fmt::format("hand: {}, win tile: {}", to_mpsz(input.hand),
                             Tile::Name.at(win_tile)));
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("Big Four Winds")
    {
        auto &[input, win_tile, expected] = cases.front();
        ScoreCalculator::check_big_four_winds(input);
    };
}

TEST_CASE("Thirteen Orphans 13-sided wait")
{
    std::vector<TestCase> cases;
    if (!load_yakuman_cases("test_score_calculator_thirteen_wait_thirteen_orphans.txt",
                            cases)) {
        return;
    }

    SECTION("Thirteen Orphans 13-sided wait")
    {
        for (auto &[input, win_tile, expected] : cases) {
            bool actual = ScoreCalculator::check_thirteen_wait_thirteen_orphans(input);
            INFO(fmt::format("hand: {}, win tile: {}", to_mpsz(input.hand),
                             Tile::Name.at(win_tile)));
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("Thirteen Orphans 13-sided wait")
    {
        auto &[input, win_tile, expected] = cases.front();
        ScoreCalculator::check_thirteen_wait_thirteen_orphans(input);
    };
}
