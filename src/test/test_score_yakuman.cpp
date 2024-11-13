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

/**
 * @brief Load test cases.
 *
 * @param[out] cases Test cases
 * @return Returns true if loading is successful, otherwise false.
 */
bool load_yakuman_cases(const std::string &filename,
                        std::vector<std::tuple<Hand, int, bool>> &cases)
{
    cases.clear();

    boost::filesystem::path path =
        boost::filesystem::path(CMAKE_TESTCASE_DIR) / filename;

    std::ifstream ifs(path.string());
    if (!ifs) {
        std::cerr << "Failed to open " << path.string() << "." << std::endl;
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

        Hand hand(tiles);
        hand.manzu = std::accumulate(hand.counts.begin(), hand.counts.begin() + 9, 0,
                                     [](int x, int y) { return x * 8 + y; });
        hand.pinzu = std::accumulate(hand.counts.begin() + 9, hand.counts.begin() + 18,
                                     0, [](int x, int y) { return x * 8 + y; });
        hand.souzu = std::accumulate(hand.counts.begin() + 18, hand.counts.begin() + 27,
                                     0, [](int x, int y) { return x * 8 + y; });
        hand.honors = std::accumulate(hand.counts.begin() + 27, hand.counts.end(), 0,
                                      [](int x, int y) { return x * 8 + y; });

        cases.emplace_back(hand, win_tile, is_valid);
    }

    return true;
}

TEST_CASE("Ryuuiisou (All Green)")
{
    ScoreCalculator score;

    std::vector<std::tuple<Hand, int, bool>> cases;
    load_yakuman_cases("test_score_ryuuiisou.txt", cases);

    SECTION("Ryuuiisou (All Green)")
    {
        for (auto &[hand, win_tile, expected] : cases) {
            bool actual = score.check_all_green(hand);
            INFO(fmt::format("hand: {}, win tile: {}", hand.to_string(),
                             Tile::Name.at(win_tile)));
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("Ryuuiisou (All Green)")
    {
        auto &[hand, win_tile, expected] = cases.front();
        score.check_all_green(hand);
    };
}

TEST_CASE("Daisangen (Big Three Dragons)")
{
    ScoreCalculator score;

    std::vector<std::tuple<Hand, int, bool>> cases;
    load_yakuman_cases("test_score_daisangen.txt", cases);

    SECTION("Daisangen (Big Three Dragons)")
    {
        for (auto &[hand, win_tile, expected] : cases) {
            bool actual = score.check_big_three_dragons(hand);
            INFO(fmt::format("hand: {}, win tile: {}", hand.to_string(),
                             Tile::Name.at(win_tile)));
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("Daisangen (Big Three Dragons)")
    {
        auto &[hand, win_tile, expected] = cases.front();
        score.check_big_three_dragons(hand);
    };
}

TEST_CASE("Shousuushii (Little Four Winds)")
{
    ScoreCalculator score;

    std::vector<std::tuple<Hand, int, bool>> cases;
    load_yakuman_cases("test_score_syosusi.txt", cases);

    SECTION("Shousuushii (Little Four Winds)")
    {
        for (auto &[hand, win_tile, expected] : cases) {
            bool actual = score.check_little_four_winds(hand);
            INFO(fmt::format("hand: {}, win tile: {}", hand.to_string(),
                             Tile::Name.at(win_tile)));
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("Shousuushii (Little Four Winds)")
    {
        auto &[hand, win_tile, expected] = cases.front();
        score.check_little_four_winds(hand);
    };
}

TEST_CASE("Tsuuiisou (All Honors)")
{
    ScoreCalculator score;

    std::vector<std::tuple<Hand, int, bool>> cases;
    load_yakuman_cases("test_score_tuiso.txt", cases);

    SECTION("Tsuuiisou (All Honors)")
    {
        for (auto &[hand, win_tile, expected] : cases) {
            bool actual = score.check_all_honors(hand);
            INFO(fmt::format("hand: {}, win tile: {}", hand.to_string(),
                             Tile::Name.at(win_tile)));
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("Tsuuiisou (All Honors)")
    {
        auto &[hand, win_tile, expected] = cases.front();
        score.check_all_honors(hand);
    };
}

TEST_CASE("Chuuren Poutou (Nine Gates)")
{
    ScoreCalculator score;

    std::vector<std::tuple<Hand, int, bool>> cases;
    load_yakuman_cases("test_score_tyurenpoto.txt", cases);

    SECTION("Chuuren Poutou (Nine Gates)")
    {
        for (auto &[hand, win_tile, expected] : cases) {
            bool actual = score.check_nine_gates(hand, win_tile);
            INFO(fmt::format("hand: {}, win tile: {}", hand.to_string(),
                             Tile::Name.at(win_tile)));
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("Chuuren Poutou (Nine Gates)")
    {
        auto &[hand, win_tile, expected] = cases.front();
        score.check_nine_gates(hand, win_tile);
    };
}

TEST_CASE("Junsei Chuuren Poutou (Pure Nine Gates)")
{
    ScoreCalculator score;

    std::vector<std::tuple<Hand, int, bool>> cases;
    load_yakuman_cases("test_score_tyurenpoto9.txt", cases);

    SECTION("Junsei Chuuren Poutou (Pure Nine Gates)")
    {
        for (auto &[hand, win_tile, expected] : cases) {
            bool actual = score.check_true_nine_gates(hand, win_tile);
            INFO(fmt::format("hand: {}, win tile: {}", hand.to_string(),
                             Tile::Name.at(win_tile)));
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("Junsei Chuuren Poutou (Pure Nine Gates)")
    {
        auto &[hand, win_tile, expected] = cases.front();
        score.check_true_nine_gates(hand, win_tile);
    };
}

TEST_CASE("Suuankou (Four Closed Triplets)")
{
    ScoreCalculator score;

    std::vector<std::tuple<Hand, int, bool>> cases;
    load_yakuman_cases("test_score_suanko.txt", cases);

    SECTION("Suuankou (Four Closed Triplets)")
    {
        for (auto &[hand, win_tile, expected] : cases) {
            bool actual = score.check_four_concealed_triplets(hand, WinFlag::Tsumo,
                                                              win_tile) >= 1;

            INFO(fmt::format("hand: {}, win tile: {}", hand.to_string(),
                             Tile::Name.at(win_tile)));
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("Suuankou (Four Closed Triplets)")
    {
        auto &[hand, win_tile, expected] = cases.front();
        score.check_four_concealed_triplets(hand, WinFlag::Tsumo, win_tile);
    };
}

TEST_CASE("Suuankou Tanki (Four Closed Triplets Single Wait)")
{
    ScoreCalculator score;

    std::vector<std::tuple<Hand, int, bool>> cases;
    load_yakuman_cases("test_score_suanko_tanki.txt", cases);

    SECTION("Suuankou Tanki (Four Closed Triplets)")
    {
        for (auto &[hand, win_tile, expected] : cases) {
            bool actual = score.check_four_concealed_triplets(hand, WinFlag::Tsumo,
                                                              win_tile) == 2;
            INFO(fmt::format("hand: {}, win tile: {}", hand.to_string(),
                             Tile::Name.at(win_tile)));
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("Suuankou Tanki (Four Closed Triplets)")
    {
        auto &[hand, win_tile, expected] = cases.front();
        score.check_four_concealed_triplets(hand, WinFlag::Tsumo, win_tile);
    };
}

TEST_CASE("Chinroutou (All Terminals)")
{
    ScoreCalculator score;

    std::vector<std::tuple<Hand, int, bool>> cases;
    load_yakuman_cases("test_score_tinroto.txt", cases);

    SECTION("Chinroutou (All Terminals)")
    {
        for (auto &[hand, win_tile, expected] : cases) {
            bool actual = score.check_all_terminals(hand);
            INFO(fmt::format("hand: {}, win tile: {}", hand.to_string(),
                             Tile::Name.at(win_tile)));
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("Chinroutou (All Terminals)")
    {
        auto &[hand, win_tile, expected] = cases.front();
        score.check_all_terminals(hand);
    };
}

TEST_CASE("Suukantsu (Four Kongs)")
{
    ScoreCalculator score;

    SECTION("Suukantsu (Four Kongs) established")
    {
        MeldedBlock block1({MeldType::ClosedKong,
                            {Tile::Manzu1, Tile::Manzu1, Tile::Manzu1, Tile::Manzu1},
                            Tile::Manzu1,
                            PlayerType::Null});
        MeldedBlock block2({MeldType::OpenKong,
                            {Tile::Pinzu1, Tile::Pinzu1, Tile::Pinzu1, Tile::Pinzu1},
                            Tile::Pinzu1,
                            PlayerType::Player1});
        MeldedBlock block3({MeldType::ClosedKong,
                            {Tile::Souzu1, Tile::Souzu1, Tile::Souzu1, Tile::Souzu1},
                            Tile::Souzu1,
                            PlayerType::Null});
        MeldedBlock block4({MeldType::AddedKong,
                            {Tile::East, Tile::East, Tile::East, Tile::East},
                            Tile::East,
                            PlayerType::Player1});
        Hand hand({Tile::White, Tile::White}, {block1, block2, block3, block4});

        int win_tile = Tile::White;
        bool expected = true;
        bool actual = score.check_four_kongs(hand);
        INFO(fmt::format("hand: {}, win tile: {}", hand.to_string(),
                         Tile::Name.at(win_tile)));
        REQUIRE(actual == expected);
    };

    SECTION("Suukantsu (Four Kongs) not established")
    {
        MeldedBlock block1({MeldType::Pong,
                            {Tile::Manzu1, Tile::Manzu1, Tile::Manzu1},
                            Tile::Manzu1,
                            PlayerType::Player1});
        MeldedBlock block2({MeldType::OpenKong,
                            {Tile::Pinzu1, Tile::Pinzu1, Tile::Pinzu1, Tile::Pinzu1},
                            Tile::Pinzu1,
                            PlayerType::Player1});
        MeldedBlock block3({MeldType::ClosedKong,
                            {Tile::Souzu1, Tile::Souzu1, Tile::Souzu1, Tile::Souzu1},
                            Tile::Souzu1,
                            PlayerType::Null});
        MeldedBlock block4({MeldType::AddedKong,
                            {Tile::East, Tile::East, Tile::East, Tile::East},
                            Tile::East,
                            PlayerType::Player1});
        Hand hand({Tile::White, Tile::White}, {block1, block2, block3, block4});

        int win_tile = Tile::White;
        bool expected = false;
        bool actual = score.check_four_kongs(hand);
        INFO(fmt::format("hand: {}, win tile: {}", hand.to_string(),
                         Tile::Name.at(win_tile)));
        REQUIRE(actual == expected);
    };

    SECTION("Suukantsu (Four Kongs) not established")
    {
        MeldedBlock block1({MeldType::OpenKong,
                            {Tile::Pinzu1, Tile::Pinzu1, Tile::Pinzu1, Tile::Pinzu1},
                            Tile::Pinzu1,
                            PlayerType::Player1});
        MeldedBlock block2({MeldType::ClosedKong,
                            {Tile::Souzu1, Tile::Souzu1, Tile::Souzu1, Tile::Souzu1},
                            Tile::Souzu1,
                            PlayerType::Null});
        MeldedBlock block3({MeldType::AddedKong,
                            {Tile::East, Tile::East, Tile::East, Tile::East},
                            Tile::East,
                            PlayerType::Player1});
        Hand hand({Tile::White, Tile::White, Tile::Red, Tile::Red, Tile::Red},
                  {block1, block2, block3});

        int win_tile = Tile::White;
        bool expected = false;
        bool actual = score.check_four_kongs(hand);
        INFO(fmt::format("hand: {}, win tile: {}", hand.to_string(),
                         Tile::Name.at(win_tile)));
        REQUIRE(actual == expected);
    };
}

TEST_CASE("Daisuushii (Big Four Winds)")
{
    ScoreCalculator score;

    std::vector<std::tuple<Hand, int, bool>> cases;
    load_yakuman_cases("test_score_daisusi.txt", cases);

    SECTION("Daisuushii (Big Four Winds)")
    {
        for (auto &[hand, win_tile, expected] : cases) {
            bool actual = score.check_big_four_winds(hand);
            INFO(fmt::format("hand: {}, win tile: {}", hand.to_string(),
                             Tile::Name.at(win_tile)));
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("Daisuushii (Big Four Winds)")
    {
        auto &[hand, win_tile, expected] = cases.front();
        score.check_big_four_winds(hand);
    };
}

TEST_CASE("Thirteen Orphans13 (Thirteen Orphans 13-sided wait)")
{
    ScoreCalculator score;

    std::vector<std::tuple<Hand, int, bool>> cases;
    load_yakuman_cases("test_score_kokusi13.txt", cases);

    SECTION("Thirteen Orphans13 (Thirteen Orphans 13-sided wait)")
    {
        for (auto &[hand, win_tile, expected] : cases) {
            bool actual = score.check_thirteen_wait_thirteen_orphans(hand, win_tile);
            INFO(fmt::format("hand: {}, win tile: {}", hand.to_string(),
                             Tile::Name.at(win_tile)));
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("Thirteen Orphans13 (Thirteen Orphans 13-sided wait)")
    {
        auto &[hand, win_tile, expected] = cases.front();
        score.check_thirteen_wait_thirteen_orphans(hand, win_tile);
    };
}
