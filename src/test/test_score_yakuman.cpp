#include <fstream>
#include <iostream>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/dll.hpp>
#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include <catch2/catch.hpp>

#include "mahjong/mahjong.hpp"

using namespace mahjong;

/**
 * @brief テストケースを読み込む。
 *
 * @param[in] filename ファイル名
 * @param[out] cases テストケース
 * @return 読み込みに成功した場合は true、そうでない場合は false を返す。
 */
bool load_yakuman_cases(const std::string &filename,
                        std::vector<std::tuple<Hand, int, bool>> &cases)
{
    cases.clear();

    boost::filesystem::path path = boost::dll::program_location().parent_path() / filename;

    // ファイルを開く。
    std::ifstream ifs(path.string());
    if (!ifs) {
        std::cerr << "Failed to open " << path.string() << "." << std::endl;
        return false;
    }

    // ファイルを読み込む。
    // 形式は `<牌1> <牌2> ... <牌14> <和了牌> <役が成立したかどうか>`
    std::string line;
    while (std::getline(ifs, line)) {
        std::vector<std::string> tokens;
        boost::split(tokens, line, boost::is_any_of(" "));

        std::vector<int> tiles(14);
        for (int i = 0; i < 14; ++i)
            tiles[i] = std::stoi(tokens[i]);

        cases.emplace_back(Hand(tiles), std::stoi(tokens[14]), tokens[15] == "1");
    }

    return true;
}

TEST_CASE("緑一色")
{
    ScoreCalculator score;

    std::vector<std::tuple<Hand, int, bool>> cases;
    load_yakuman_cases("test_score_ryuiso.txt", cases);

    SECTION("緑一色")
    {
        for (auto &[hand, win_tile, expected] : cases) {
            bool actual = score.check_ryuiso(hand);
            INFO(fmt::format("手牌: {}, 和了牌: {}", hand.to_string(), Tile::Name.at(win_tile)));
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("緑一色")
    {
        auto &[hand, win_tile, expected] = cases.front();
        score.check_ryuiso(hand);
    };
}

TEST_CASE("大三元")
{
    ScoreCalculator score;

    std::vector<std::tuple<Hand, int, bool>> cases;
    load_yakuman_cases("test_score_daisangen.txt", cases);

    SECTION("大三元")
    {
        for (auto &[hand, win_tile, expected] : cases) {
            bool actual = score.check_daisangen(hand);
            INFO(fmt::format("手牌: {}, 和了牌: {}", hand.to_string(), Tile::Name.at(win_tile)));
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("大三元")
    {
        auto &[hand, win_tile, expected] = cases.front();
        score.check_daisangen(hand);
    };
}

TEST_CASE("小四喜")
{
    ScoreCalculator score;

    std::vector<std::tuple<Hand, int, bool>> cases;
    load_yakuman_cases("test_score_syosusi.txt", cases);

    SECTION("小四喜")
    {
        for (auto &[hand, win_tile, expected] : cases) {
            bool actual = score.check_syosusi(hand);
            INFO(fmt::format("手牌: {}, 和了牌: {}", hand.to_string(), Tile::Name.at(win_tile)));
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("小四喜")
    {
        auto &[hand, win_tile, expected] = cases.front();
        score.check_syosusi(hand);
    };
}

TEST_CASE("字一色")
{
    ScoreCalculator score;

    std::vector<std::tuple<Hand, int, bool>> cases;
    load_yakuman_cases("test_score_tuiso.txt", cases);

    SECTION("字一色")
    {
        for (auto &[hand, win_tile, expected] : cases) {
            bool actual = score.check_tuiso(hand);
            INFO(fmt::format("手牌: {}, 和了牌: {}", hand.to_string(), Tile::Name.at(win_tile)));
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("字一色")
    {
        auto &[hand, win_tile, expected] = cases.front();
        score.check_tuiso(hand);
    };
}

TEST_CASE("九蓮宝燈")
{
    ScoreCalculator score;

    std::vector<std::tuple<Hand, int, bool>> cases;
    load_yakuman_cases("test_score_tyurenpoto.txt", cases);

    SECTION("九蓮宝燈")
    {
        for (auto &[hand, win_tile, expected] : cases) {
            bool actual = score.check_tyurenpoto(hand, win_tile);
            INFO(fmt::format("手牌: {}, 和了牌: {}", hand.to_string(), Tile::Name.at(win_tile)));
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("九蓮宝燈")
    {
        auto &[hand, win_tile, expected] = cases.front();
        score.check_tyurenpoto(hand, win_tile);
    };
}

TEST_CASE("四暗刻")
{
    ScoreCalculator score;

    std::vector<std::tuple<Hand, int, bool>> cases;
    load_yakuman_cases("test_score_suanko.txt", cases);

    SECTION("四暗刻")
    {
        for (auto &[hand, win_tile, expected] : cases) {
            bool actual = score.check_suanko(hand, HandFlag::Tumo);

            INFO(fmt::format("手牌: {}, 和了牌: {}", hand.to_string(), Tile::Name.at(win_tile)));
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("四暗刻")
    {
        auto &[hand, win_tile, expected] = cases.front();
        score.check_suanko(hand, HandFlag::Tumo);
    };
}

TEST_CASE("清老頭")
{
    ScoreCalculator score;

    std::vector<std::tuple<Hand, int, bool>> cases;
    load_yakuman_cases("test_score_tinroto.txt", cases);

    SECTION("清老頭")
    {
        for (auto &[hand, win_tile, expected] : cases) {
            bool actual = score.check_tinroto(hand);
            INFO(fmt::format("手牌: {}, 和了牌: {}", hand.to_string(), Tile::Name.at(win_tile)));
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("清老頭")
    {
        auto &[hand, win_tile, expected] = cases.front();
        score.check_tinroto(hand);
    };
}

TEST_CASE("四槓子")
{
    ScoreCalculator score;

    SECTION("四槓子成立")
    {
        MeldedBlock block1({MeldType::Ankan,
                            {Tile::Manzu1, Tile::Manzu1, Tile::Manzu1, Tile::Manzu1},
                            Tile::Manzu1,
                            PlayerType::Null});
        MeldedBlock block2({MeldType::Minkan,
                            {Tile::Pinzu1, Tile::Pinzu1, Tile::Pinzu1, Tile::Pinzu1},
                            Tile::Pinzu1,
                            PlayerType::Player1});
        MeldedBlock block3({MeldType::Ankan,
                            {Tile::Sozu1, Tile::Sozu1, Tile::Sozu1, Tile::Sozu1},
                            Tile::Sozu1,
                            PlayerType::Null});
        MeldedBlock block4({MeldType::Kakan,
                            {Tile::Ton, Tile::Ton, Tile::Ton, Tile::Ton},
                            Tile::Ton,
                            PlayerType::Player1});
        Hand hand({Tile::Haku, Tile::Haku}, {block1, block2, block3, block4});

        int win_tile = Tile::Haku;
        bool expected = true;
        bool actual = score.check_sukantu(hand);
        INFO(fmt::format("手牌: {}, 和了牌: {}", hand.to_string(), Tile::Name.at(win_tile)));
        REQUIRE(actual == expected);
    };

    SECTION("四槓子不成立")
    {
        MeldedBlock block1({MeldType::Pon,
                            {Tile::Manzu1, Tile::Manzu1, Tile::Manzu1},
                            Tile::Manzu1,
                            PlayerType::Player1});
        MeldedBlock block2({MeldType::Minkan,
                            {Tile::Pinzu1, Tile::Pinzu1, Tile::Pinzu1, Tile::Pinzu1},
                            Tile::Pinzu1,
                            PlayerType::Player1});
        MeldedBlock block3({MeldType::Ankan,
                            {Tile::Sozu1, Tile::Sozu1, Tile::Sozu1, Tile::Sozu1},
                            Tile::Sozu1,
                            PlayerType::Null});
        MeldedBlock block4({MeldType::Kakan,
                            {Tile::Ton, Tile::Ton, Tile::Ton, Tile::Ton},
                            Tile::Ton,
                            PlayerType::Player1});
        Hand hand({Tile::Haku, Tile::Haku}, {block1, block2, block3, block4});

        int win_tile = Tile::Haku;
        bool expected = false;
        bool actual = score.check_sukantu(hand);
        INFO(fmt::format("手牌: {}, 和了牌: {}", hand.to_string(), Tile::Name.at(win_tile)));
        REQUIRE(actual == expected);
    };

    SECTION("四槓子不成立")
    {
        MeldedBlock block1({MeldType::Minkan,
                            {Tile::Pinzu1, Tile::Pinzu1, Tile::Pinzu1, Tile::Pinzu1},
                            Tile::Pinzu1,
                            PlayerType::Player1});
        MeldedBlock block2({MeldType::Ankan,
                            {Tile::Sozu1, Tile::Sozu1, Tile::Sozu1, Tile::Sozu1},
                            Tile::Sozu1,
                            PlayerType::Null});
        MeldedBlock block3({MeldType::Kakan,
                            {Tile::Ton, Tile::Ton, Tile::Ton, Tile::Ton},
                            Tile::Ton,
                            PlayerType::Player1});
        Hand hand({Tile::Haku, Tile::Haku, Tile::Tyun, Tile::Tyun, Tile::Tyun},
                  {block1, block2, block3});

        int win_tile = Tile::Haku;
        bool expected = false;
        bool actual = score.check_sukantu(hand);
        INFO(fmt::format("手牌: {}, 和了牌: {}", hand.to_string(), Tile::Name.at(win_tile)));
        REQUIRE(actual == expected);
    };
}

TEST_CASE("四暗刻単騎")
{
    ScoreCalculator score;

    std::vector<std::tuple<Hand, int, bool>> cases;
    load_yakuman_cases("test_score_suanko_tanki.txt", cases);

    SECTION("四暗刻単騎")
    {
        for (auto &[hand, win_tile, expected] : cases) {
            bool actual = score.check_suanko_tanki(hand, win_tile);
            INFO(fmt::format("手牌: {}, 和了牌: {}", hand.to_string(), Tile::Name.at(win_tile)));
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("四暗刻単騎")
    {
        auto &[hand, win_tile, expected] = cases.front();
        score.check_suanko_tanki(hand, win_tile);
    };
}

TEST_CASE("大四喜")
{
    ScoreCalculator score;

    std::vector<std::tuple<Hand, int, bool>> cases;
    load_yakuman_cases("test_score_daisusi.txt", cases);

    SECTION("大四喜")
    {
        for (auto &[hand, win_tile, expected] : cases) {
            bool actual = score.check_daisusi(hand);
            INFO(fmt::format("手牌: {}, 和了牌: {}", hand.to_string(), Tile::Name.at(win_tile)));
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("大四喜")
    {
        auto &[hand, win_tile, expected] = cases.front();
        score.check_daisusi(hand);
    };
}

TEST_CASE("純正九蓮宝燈")
{
    ScoreCalculator score;

    std::vector<std::tuple<Hand, int, bool>> cases;
    load_yakuman_cases("test_score_tyurenpoto9.txt", cases);

    SECTION("純正九蓮宝燈")
    {
        for (auto &[hand, win_tile, expected] : cases) {
            bool actual = score.check_tyurenpoto9(hand, win_tile);
            INFO(fmt::format("手牌: {}, 和了牌: {}", hand.to_string(), Tile::Name.at(win_tile)));
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("純正九蓮宝燈")
    {
        auto &[hand, win_tile, expected] = cases.front();
        score.check_tyurenpoto(hand, win_tile);
    };
}

TEST_CASE("国士無双13面待ち")
{
    ScoreCalculator score;

    std::vector<std::tuple<Hand, int, bool>> cases;
    load_yakuman_cases("test_kokusi13.txt", cases);

    SECTION("国士無双13面待ち")
    {
        for (auto &[hand, win_tile, expected] : cases) {
            bool actual = score.check_kokusi13(hand, win_tile);
            INFO(fmt::format("手牌: {}, 和了牌: {}", hand.to_string(), Tile::Name.at(win_tile)));
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("国士無双13面待ち")
    {
        auto &[hand, win_tile, expected] = cases.front();
        score.check_kokusi13(hand, win_tile);
    };
}
