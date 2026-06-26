#include <fstream>
#include <iostream>
#include <tuple>
#include <vector>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/dll.hpp>
#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include <catch2/catch.hpp>

#include "mahjong/mahjong.hpp"

using namespace mahjong;

using TestCase = std::tuple<PlayerState, int, bool>;

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

        cases.emplace_back(PlayerState{to_hand(tiles), {}, Tile::East}, win_tile,
                           is_valid);
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
        for (auto &[player, win_tile, expected] : cases) {
            bool actual =
                score_calculator_detail::check_all_green(
                    score_calculator_detail::merge_hand(player)) != Yaku::None;
            INFO(fmt::format("hand: {}, win tile: {}", to_mpsz(player.hand),
                             Tile::name(win_tile)));
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("All Green")
    {
        auto &[player, win_tile, expected] = cases.front();
        score_calculator_detail::check_all_green(
            score_calculator_detail::merge_hand(player));
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
        for (auto &[player, win_tile, expected] : cases) {
            bool actual = score_calculator_detail::check_three_dragons(
                              score_calculator_detail::merge_hand(player)) ==
                          Yaku::BigThreeDragons;
            INFO(fmt::format("hand: {}, win tile: {}", to_mpsz(player.hand),
                             Tile::name(win_tile)));
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("Big Three Dragons")
    {
        auto &[player, win_tile, expected] = cases.front();
        score_calculator_detail::check_three_dragons(
            score_calculator_detail::merge_hand(player));
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
        for (auto &[player, win_tile, expected] : cases) {
            bool actual = score_calculator_detail::check_four_winds(
                              score_calculator_detail::merge_hand(player)) ==
                          Yaku::LittleFourWinds;
            INFO(fmt::format("hand: {}, win tile: {}", to_mpsz(player.hand),
                             Tile::name(win_tile)));
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("Little Four Winds")
    {
        auto &[player, win_tile, expected] = cases.front();
        score_calculator_detail::check_four_winds(
            score_calculator_detail::merge_hand(player));
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
        for (auto &[player, win_tile, expected] : cases) {
            bool actual =
                score_calculator_detail::check_all_honors(
                    score_calculator_detail::merge_hand(player)) != Yaku::None;
            INFO(fmt::format("hand: {}, win tile: {}", to_mpsz(player.hand),
                             Tile::name(win_tile)));
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("All Honors")
    {
        auto &[player, win_tile, expected] = cases.front();
        score_calculator_detail::check_all_honors(
            score_calculator_detail::merge_hand(player));
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
        for (auto &[player, win_tile, expected] : cases) {
            bool actual = score_calculator_detail::check_nine_gates(
                              player, score_calculator_detail::merge_hand(player),
                              win_tile) != Yaku::None;
            INFO(fmt::format("hand: {}, win tile: {}", to_mpsz(player.hand),
                             Tile::name(win_tile)));
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("Nine Gates")
    {
        auto &[player, win_tile, expected] = cases.front();
        score_calculator_detail::check_nine_gates(
            player, score_calculator_detail::merge_hand(player), win_tile);
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
        for (auto &[player, win_tile, expected] : cases) {
            bool actual = score_calculator_detail::check_nine_gates(
                              player, score_calculator_detail::merge_hand(player),
                              win_tile) == Yaku::TrueNineGates;
            INFO(fmt::format("hand: {}, win tile: {}", to_mpsz(player.hand),
                             Tile::name(win_tile)));
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("True Nine Gates")
    {
        auto &[player, win_tile, expected] = cases.front();
        score_calculator_detail::check_nine_gates(
            player, score_calculator_detail::merge_hand(player), win_tile);
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
        for (auto &[player, win_tile, expected] : cases) {
            const bool actual = score_calculator_detail::check_four_concealed_triplets(
                                    player, score_calculator_detail::merge_hand(player),
                                    win_tile, WinFlag::Tsumo) != Yaku::None;

            INFO(fmt::format("hand: {}, win tile: {}", to_mpsz(player.hand),
                             Tile::name(win_tile)));
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("Four Concealed Triplets")
    {
        auto &[player, win_tile, expected] = cases.front();
        score_calculator_detail::check_four_concealed_triplets(
            player, score_calculator_detail::merge_hand(player), win_tile,
            WinFlag::Tsumo);
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
        for (auto &[player, win_tile, expected] : cases) {
            const bool actual =
                (score_calculator_detail::check_four_concealed_triplets(
                     player, score_calculator_detail::merge_hand(player), win_tile,
                     WinFlag::Tsumo) &
                 Yaku::SingleWaitFourConcealedTriplets) != 0;
            INFO(fmt::format("hand: {}, win tile: {}", to_mpsz(player.hand),
                             Tile::name(win_tile)));
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("Single Wait Four Concealed Triplets")
    {
        auto &[player, win_tile, expected] = cases.front();
        score_calculator_detail::check_four_concealed_triplets(
            player, score_calculator_detail::merge_hand(player), win_tile,
            WinFlag::Tsumo);
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
        for (auto &[player, win_tile, expected] : cases) {
            bool actual =
                score_calculator_detail::check_all_terminals(
                    score_calculator_detail::merge_hand(player)) == Yaku::AllTerminals;
            INFO(fmt::format("hand: {}, win tile: {}", to_mpsz(player.hand),
                             Tile::name(win_tile)));
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("All Terminals")
    {
        auto &[player, win_tile, expected] = cases.front();
        score_calculator_detail::check_all_terminals(
            score_calculator_detail::merge_hand(player));
    };
}

TEST_CASE("Four Kongs")
{
    SECTION("Four Kongs established")
    {
        Meld block1({MeldType::Ankan,
                     {Tile::Manzu1, Tile::Manzu1, Tile::Manzu1, Tile::Manzu1},
                     Tile::Manzu1,
                     PlayerIndex::Null});
        Meld block2({MeldType::Daiminkan,
                     {Tile::Pinzu1, Tile::Pinzu1, Tile::Pinzu1, Tile::Pinzu1},
                     Tile::Pinzu1,
                     PlayerIndex::Player1});
        Meld block3({MeldType::Ankan,
                     {Tile::Souzu1, Tile::Souzu1, Tile::Souzu1, Tile::Souzu1},
                     Tile::Souzu1,
                     PlayerIndex::Null});
        Meld block4({MeldType::Kakan,
                     {Tile::East, Tile::East, Tile::East, Tile::East},
                     Tile::East,
                     PlayerIndex::Player1});
        std::vector<Meld> melds = {block1, block2, block3, block4};
        std::vector<int> tiles = {Tile::WhiteDragon, Tile::WhiteDragon};

        PlayerState player{to_hand(tiles), melds, Tile::East};
        bool expected = true;
        bool actual = score_calculator_detail::check_kongs(player) == Yaku::FourKans;
        INFO(fmt::format("hand: {}, win tile: {}", to_mpsz(player.hand),
                         Tile::name(Tile::WhiteDragon)));
        REQUIRE(actual == expected);
    };

    SECTION("Four Kongs not established")
    {
        Meld block1({MeldType::Pon,
                     {Tile::Manzu1, Tile::Manzu1, Tile::Manzu1},
                     Tile::Manzu1,
                     PlayerIndex::Player1});
        Meld block2({MeldType::Daiminkan,
                     {Tile::Pinzu1, Tile::Pinzu1, Tile::Pinzu1, Tile::Pinzu1},
                     Tile::Pinzu1,
                     PlayerIndex::Player1});
        Meld block3({MeldType::Ankan,
                     {Tile::Souzu1, Tile::Souzu1, Tile::Souzu1, Tile::Souzu1},
                     Tile::Souzu1,
                     PlayerIndex::Null});
        Meld block4({MeldType::Kakan,
                     {Tile::East, Tile::East, Tile::East, Tile::East},
                     Tile::East,
                     PlayerIndex::Player1});
        std::vector<Meld> melds = {block1, block2, block3, block4};
        std::vector<int> tiles = {Tile::WhiteDragon, Tile::WhiteDragon};
        PlayerState player{to_hand(tiles), melds, Tile::East};

        int win_tile = Tile::WhiteDragon;
        bool expected = false;
        bool actual = score_calculator_detail::check_kongs(player) == Yaku::FourKans;
        INFO(fmt::format("hand: {}, win tile: {}", to_mpsz(player.hand),
                         Tile::name(win_tile)));
        REQUIRE(actual == expected);
    };

    SECTION("Four Kongs not established")
    {
        Meld block1({MeldType::Daiminkan,
                     {Tile::Pinzu1, Tile::Pinzu1, Tile::Pinzu1, Tile::Pinzu1},
                     Tile::Pinzu1,
                     PlayerIndex::Player1});
        Meld block2({MeldType::Ankan,
                     {Tile::Souzu1, Tile::Souzu1, Tile::Souzu1, Tile::Souzu1},
                     Tile::Souzu1,
                     PlayerIndex::Null});
        Meld block3({MeldType::Kakan,
                     {Tile::East, Tile::East, Tile::East, Tile::East},
                     Tile::East,
                     PlayerIndex::Player1});
        std::vector<Meld> melds = {block1, block2, block3};
        std::vector<int> tiles = {Tile::WhiteDragon, Tile::WhiteDragon};
        PlayerState player{to_hand(tiles), melds, Tile::East};

        int win_tile = Tile::WhiteDragon;
        bool expected = false;
        bool actual = score_calculator_detail::check_kongs(player) == Yaku::FourKans;
        INFO(fmt::format("hand: {}, win tile: {}", to_mpsz(player.hand),
                         Tile::name(win_tile)));
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
        for (auto &[player, win_tile, expected] : cases) {
            bool actual =
                score_calculator_detail::check_four_winds(
                    score_calculator_detail::merge_hand(player)) == Yaku::BigFourWinds;
            INFO(fmt::format("hand: {}, win tile: {}", to_mpsz(player.hand),
                             Tile::name(win_tile)));
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("Big Four Winds")
    {
        auto &[player, win_tile, expected] = cases.front();
        score_calculator_detail::check_four_winds(
            score_calculator_detail::merge_hand(player));
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
        for (auto &[player, win_tile, expected] : cases) {
            bool actual = score_calculator_detail::check_thirteen_wait_thirteen_orphans(
                score_calculator_detail::merge_hand(player), win_tile);
            INFO(fmt::format("hand: {}, win tile: {}", to_mpsz(player.hand),
                             Tile::name(win_tile)));
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("Thirteen Orphans 13-sided wait")
    {
        auto &[player, win_tile, expected] = cases.front();
        score_calculator_detail::check_thirteen_wait_thirteen_orphans(
            score_calculator_detail::merge_hand(player), win_tile);
    };
}
