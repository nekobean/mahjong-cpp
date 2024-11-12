#include <fstream>
#include <iostream>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/dll.hpp>
#include <boost/spirit/home/x3.hpp>
#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include <catch2/catch.hpp>

#include "mahjong/mahjong.hpp"

using namespace mahjong;

/**
 * @brief テストケースを読み込む。
 *
 * @param[out] cases テストケース
 * @return 読み込みに成功した場合は true、そうでない場合は false を返す。
 */
bool load_constractor_testcase(
    std::vector<std::tuple<std::vector<int>, int, int, int, int, bool, bool, bool>>
        &cases)
{
    cases.clear();

    boost::filesystem::path path =
        boost::filesystem::path(CMAKE_TESTCASE_DIR) / "testcase_hand_constractor.txt";

    // ファイルを開く。
    std::ifstream ifs(path.string());
    if (!ifs) {
        std::cerr << "Failed to open " << path.string() << "." << std::endl;
        return false;
    }

    // ファイルを読み込む。
    // 形式は `<牌1> <牌2> ... <牌14> <萬子のビット値> <筒子のビット値> <索子のビット値>
    //         <字牌のビット値> <赤五萬フラグ> <赤五筒フラグ> <赤五索フラグ>`
    std::string line;
    while (std::getline(ifs, line)) {
        std::vector<std::string> tokens;
        boost::split(tokens, line, boost::is_any_of(" "));

        std::vector<int> tiles(14);
        for (int i = 0; i < 14; ++i)
            tiles[i] = std::stoi(tokens[i]);

        cases.emplace_back(tiles, std::stoi(tokens[14]), std::stoi(tokens[15]),
                           std::stoi(tokens[16]), std::stoi(tokens[17]),
                           std::stoi(tokens[18]), std::stoi(tokens[19]),
                           std::stoi(tokens[20]));
    }

    return true;
}

/**
 * @brief テストケースを読み込む。
 *
 * @param[out] cases テストケース
 * @return 読み込みに成功した場合は true、そうでない場合は false を返す。
 */
bool load_num_tiles_testcase(
    std::vector<std::tuple<std::vector<int>, std::vector<int>, int>> &cases)
{
    cases.clear();

    boost::filesystem::path path =
        boost::filesystem::path(CMAKE_TESTCASE_DIR) / "testcase_hand_num_tiles.txt";

    // ファイルを開く。
    std::ifstream ifs(path.string());
    if (!ifs) {
        std::cerr << "Failed to open " << path.string() << "." << std::endl;
        return false;
    }

    // ファイルを読み込む。
    // 形式は `<牌1> <牌2> ... <牌14> <萬子1の枚数> <萬子2の枚数> ... <赤五萬の枚数>
    // <合計枚数>`
    std::string line;
    while (std::getline(ifs, line)) {
        std::vector<std::string> tokens;
        boost::split(tokens, line, boost::is_any_of(" "));

        std::vector<int> tiles(14);
        for (int i = 0; i < 14; ++i)
            tiles[i] = std::stoi(tokens[i]);

        std::vector<int> counts(37);
        for (int i = 0; i < 37; ++i)
            counts[i] = std::stoi(tokens[14 + i]);

        int n_tiles = std::stoi(tokens[51]);

        cases.emplace_back(tiles, counts, n_tiles);
    }

    return true;
}

/**
 * @brief テストケースを読み込む。
 *
 * @param[out] cases テストケース
 * @return 読み込みに成功した場合は true、そうでない場合は false を返す。
 */
bool load_to_string_testcase(
    std::vector<std::tuple<std::vector<int>, std::string>> &cases)
{
    cases.clear();

    boost::filesystem::path path =
        boost::filesystem::path(CMAKE_TESTCASE_DIR) / "testcase_hand_to_string.txt";

    // ファイルを開く。
    std::ifstream ifs(path.string());
    if (!ifs) {
        std::cerr << "Failed to open " << path.string() << "." << std::endl;
        return false;
    }

    // ファイルを読み込む。
    // 形式は `<牌1> <牌2> ... <牌14> <文字列表記>`
    std::string line;
    while (std::getline(ifs, line)) {
        std::vector<std::string> tokens;

        namespace x3 = boost::spirit::x3;

        auto const quoted = '"' >> *~x3::char_('"') >> '"';
        auto const unquoted = *~x3::char_(' ');
        auto const segments = (quoted | unquoted) % ' ';

        if (!x3::parse(line.begin(), line.end(), segments, tokens))
            return false;

        std::vector<int> tiles(14);
        for (int i = 0; i < 14; ++i)
            tiles[i] = std::stoi(tokens[i]);

        cases.emplace_back(tiles, tokens[14]);
    }

    return true;
}

TEST_CASE("Constract Hand")
{
    SECTION("Hand::Hand()")
    {
        Hand hand;
        REQUIRE(hand.manzu == 0);
        REQUIRE(hand.pinzu == 0);
        REQUIRE(hand.sozu == 0);
        REQUIRE(hand.zihai == 0);
        REQUIRE(!hand.aka_manzu5);
        REQUIRE(!hand.aka_pinzu5);
        REQUIRE(!hand.aka_sozu5);
        REQUIRE(hand.melds.empty());
    }

    SECTION("Hand::Hand(const std::vector<int> &)")
    {
        std::vector<std::tuple<std::vector<int>, int, int, int, int, bool, bool, bool>>
            cases;
        if (!load_constractor_testcase(cases))
            return;

        for (auto &[tiles, manzu, pinzu, sozu, zihai, aka_manzu5, aka_pinzu5,
                    aka_sozu5] : cases) {
            Hand hand(tiles);
            REQUIRE(hand.manzu == manzu);
            REQUIRE(hand.pinzu == pinzu);
            REQUIRE(hand.sozu == sozu);
            REQUIRE(hand.zihai == zihai);
            REQUIRE(hand.aka_manzu5 == aka_manzu5);
            REQUIRE(hand.aka_pinzu5 == aka_pinzu5);
            REQUIRE(hand.aka_sozu5 == aka_sozu5);
            REQUIRE(hand.melds.empty());
        }
    }
}

TEST_CASE("is_menzen()")
{
    SECTION("鳴いていない場合は門前")
    {
        Hand hand({Tile::Manzu1, Tile::Manzu1, Tile::Manzu2, Tile::Manzu2,
                   Tile::RedManzu5, Tile::Manzu6, Tile::Manzu7, Tile::Manzu8,
                   Tile::Manzu9, Tile::Pinzu1, Tile::Pinzu1, Tile::Pinzu2,
                   Tile::Pinzu2});

        REQUIRE(hand.is_menzen());
    }

    SECTION("副露ブロックに暗槓以外がない場合は門前")
    {
        MeldedBlock block1 = {
            MeldType::Pon, {Tile::Pinzu1, Tile::Pinzu1, Tile::Pinzu1}, Tile::Pinzu1, 0};
        MeldedBlock block2 = {MeldType::Ankan,
                              {Tile::Pinzu8, Tile::Pinzu8, Tile::Pinzu8, Tile::Pinzu8},
                              Tile::Pinzu1,
                              1};
        Hand hand({Tile::Manzu1, Tile::Manzu1, Tile::Manzu2, Tile::Manzu2, Tile::Manzu6,
                   Tile::Manzu7, Tile::Manzu8},
                  {block1, block2});

        REQUIRE(!hand.is_menzen());
    }

    SECTION("副露ブロックに暗槓以外がある場合は門前でない")
    {
        MeldedBlock block1 = {MeldType::Ankan,
                              {Tile::Pinzu1, Tile::Pinzu1, Tile::Pinzu1},
                              Tile::Pinzu1,
                              0};
        MeldedBlock block2 = {MeldType::Ankan,
                              {Tile::Pinzu8, Tile::Pinzu8, Tile::Pinzu8, Tile::Pinzu8},
                              Tile::Pinzu1,
                              1};
        Hand hand({Tile::Manzu1, Tile::Manzu1, Tile::Manzu2, Tile::Manzu2, Tile::Manzu6,
                   Tile::Manzu7, Tile::Manzu8},
                  {block1, block2});

        REQUIRE(hand.is_menzen());
    }
}

TEST_CASE("is_melded()")
{
    SECTION("鳴いていない")
    {
        Hand hand({Tile::Manzu1, Tile::Manzu1, Tile::Manzu2, Tile::Manzu2,
                   Tile::RedManzu5, Tile::Manzu6, Tile::Manzu7, Tile::Manzu8,
                   Tile::Manzu9, Tile::Pinzu1, Tile::Pinzu1, Tile::Pinzu2,
                   Tile::Pinzu2});

        REQUIRE(!hand.is_melded());
    }

    SECTION("副露ブロックに暗槓以外がない")
    {
        MeldedBlock block1 = {
            MeldType::Pon, {Tile::Pinzu1, Tile::Pinzu1, Tile::Pinzu1}, Tile::Pinzu1, 0};
        MeldedBlock block2 = {MeldType::Ankan,
                              {Tile::Pinzu8, Tile::Pinzu8, Tile::Pinzu8, Tile::Pinzu8},
                              Tile::Pinzu1,
                              1};
        Hand hand({Tile::Manzu1, Tile::Manzu1, Tile::Manzu2, Tile::Manzu2, Tile::Manzu6,
                   Tile::Manzu7, Tile::Manzu8},
                  {block1, block2});

        REQUIRE(hand.is_melded());
    }

    SECTION("副露ブロックに暗槓以外がある")
    {
        MeldedBlock block1 = {MeldType::Ankan,
                              {Tile::Pinzu1, Tile::Pinzu1, Tile::Pinzu1},
                              Tile::Pinzu1,
                              0};
        MeldedBlock block2 = {MeldType::Ankan,
                              {Tile::Pinzu8, Tile::Pinzu8, Tile::Pinzu8, Tile::Pinzu8},
                              Tile::Pinzu1,
                              1};
        Hand hand({Tile::Manzu1, Tile::Manzu1, Tile::Manzu2, Tile::Manzu2, Tile::Manzu6,
                   Tile::Manzu7, Tile::Manzu8},
                  {block1, block2});

        REQUIRE(hand.is_melded());
    }
}

TEST_CASE("num_tiles()")
{
    std::vector<std::tuple<std::vector<int>, std::vector<int>, int>> cases;
    if (!load_num_tiles_testcase(cases))
        return;

    for (auto &[tiles, counts, n_tiles] : cases) {
        Hand hand(tiles);

        for (int i = 0; i < 37; ++i)
            REQUIRE(hand.num_tiles(i) == counts[i]);

        REQUIRE(hand.num_tiles() == n_tiles);
    }
}

TEST_CASE("to_string()")
{
    std::vector<std::tuple<std::vector<int>, std::string>> cases;
    if (!load_to_string_testcase(cases))
        return;

    for (auto &[tiles, str] : cases) {
        Hand hand(tiles);
        REQUIRE(hand.to_string() == str);
    }
}
