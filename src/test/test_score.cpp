#include <fstream>
#include <iostream>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/dll.hpp>
#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include <catch2/catch.hpp>

#include "score.hpp"
#include "syanten.hpp"

using namespace mahjong;

/**
 * @brief テストケースを読み込む。
 * 
 * @param[in] filename ファイル名
 * @param[out] cases テストケース
 * @return 読み込みに成功した場合は true、そうでない場合は false を返す。
 */
bool load_yakuman_cases(const std::string &filename,
                        std::vector<std::tuple<Tehai, int, bool>> &cases)
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
    // 形式は <14枚の牌> <和了り牌> <役が成立したかどうか>
    std::string line;
    while (std::getline(ifs, line)) {
        std::vector<std::string> tokens;
        boost::split(tokens, line, boost::is_any_of(" "));

        std::vector<int> tiles(14);
        for (int i = 0; i < 14; ++i)
            tiles[i] = std::stoi(tokens[i]);

        cases.emplace_back(Tehai(tiles), std::stoi(tokens[14]), tokens[15] == "1");
    }

    return true;
}

/**
 * @brief 点数計算器を初期化する。
 * 
 * @return ScoreCalculator 点数計算器
 */
ScoreCalculator setup_score_calculator()
{
    ScoreCalculator score;
    score.enable_akahai(true);
    score.enable_kuitan(true);
    score.set_dora({Tile::Pinzu3});
    score.set_bakaze(Tile::Ton);
    score.set_zikaze(Tile::Nan);
    score.set_honba(1);
    score.set_tumibo(2);

    return score;
}

TEST_CASE("供託棒")
{
    SyantenCalculator::initialize();
    ScoreCalculator score = setup_score_calculator();

    score.set_honba(1);
    score.set_tumibo(2);

    SECTION("Calculate kyotaku score")
    {
        score.set_honba(0);
        score.set_tumibo(0);
        REQUIRE(score.calc_kyotaku_score() == 0);

        score.set_honba(1);
        score.set_tumibo(2);
        REQUIRE(score.calc_kyotaku_score() == 300 * 1 + 1000 * 2);

        score.set_honba(5);
        score.set_tumibo(2);
        REQUIRE(score.calc_kyotaku_score() == 300 * 5 + 1000 * 2);
    };
}

TEST_CASE("緑一色")
{
    SyantenCalculator::initialize();
    ScoreCalculator score = setup_score_calculator();

    std::vector<HuroBlock> huro_blocks;
    std::vector<std::tuple<Tehai, int, bool>> cases;
    load_yakuman_cases("test_ryuiso.txt", cases);

    SECTION("緑一色")
    {
        for (auto &[tehai, agarihai, expected] : cases) {
            bool actual = score.check_ryuiso(tehai, huro_blocks, agarihai);
            INFO("手牌: " << tehai << " " << std::boolalpha << actual << " vs " << expected);
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("緑一色")
    {
        for (auto &[tehai, agarihai, expected] : cases)
            score.check_ryuiso(tehai, huro_blocks, agarihai);
    };
}

TEST_CASE("大三元")
{
    SyantenCalculator::initialize();
    ScoreCalculator score = setup_score_calculator();

    std::vector<HuroBlock> huro_blocks;
    std::vector<std::tuple<Tehai, int, bool>> cases;
    load_yakuman_cases("test_daisangen.txt", cases);

    SECTION("大三元")
    {
        for (auto &[tehai, agarihai, expected] : cases) {
            bool actual = score.check_daisangen(tehai, huro_blocks, agarihai);
            INFO("手牌: " << tehai << " " << std::boolalpha << actual << " vs " << expected);
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("大三元")
    {
        for (auto &[tehai, agarihai, expected] : cases)
            score.check_daisangen(tehai, huro_blocks, agarihai);
    };
}

TEST_CASE("小四喜")
{
    SyantenCalculator::initialize();
    ScoreCalculator score = setup_score_calculator();

    std::vector<HuroBlock> huro_blocks;
    std::vector<std::tuple<Tehai, int, bool>> cases;
    load_yakuman_cases("test_syosusi.txt", cases);

    SECTION("小四喜")
    {
        for (auto &[tehai, agarihai, expected] : cases) {
            bool actual = score.check_syosusi(tehai, huro_blocks, agarihai);
            INFO("手牌: " << tehai << " " << std::boolalpha << actual << " vs " << expected);
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("小四喜")
    {
        for (auto &[tehai, agarihai, expected] : cases)
            score.check_syosusi(tehai, huro_blocks, agarihai);
    };
}

TEST_CASE("字一色")
{
    SyantenCalculator::initialize();
    ScoreCalculator score = setup_score_calculator();

    std::vector<HuroBlock> huro_blocks;
    std::vector<std::tuple<Tehai, int, bool>> cases;
    load_yakuman_cases("test_tuiso.txt", cases);

    SECTION("字一色")
    {
        for (auto &[tehai, agarihai, expected] : cases) {
            bool actual = score.check_tuiso(tehai, huro_blocks, agarihai);
            INFO("手牌: " << tehai << " " << std::boolalpha << actual << " vs " << expected);
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("字一色")
    {
        for (auto &[tehai, agarihai, expected] : cases)
            score.check_tuiso(tehai, huro_blocks, agarihai);
    };
}

TEST_CASE("九蓮宝燈")
{
    SyantenCalculator::initialize();
    ScoreCalculator score = setup_score_calculator();

    std::vector<HuroBlock> huro_blocks;
    std::vector<std::tuple<Tehai, int, bool>> cases;
    load_yakuman_cases("test_tyurenpoto.txt", cases);

    SECTION("九蓮宝燈")
    {
        for (auto &[tehai, agarihai, expected] : cases) {
            bool actual = score.check_tyurenpoto(tehai, huro_blocks, agarihai);
            INFO("手牌: " << tehai << " " << std::boolalpha << actual << " vs " << expected);
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("九蓮宝燈")
    {
        for (auto &[tehai, agarihai, expected] : cases)
            score.check_tyurenpoto(tehai, huro_blocks, agarihai);
    };
}

TEST_CASE("四暗刻")
{
    SyantenCalculator::initialize();
    ScoreCalculator score = setup_score_calculator();

    std::vector<HuroBlock> huro_blocks;
    std::vector<std::tuple<Tehai, int, bool>> cases;
    load_yakuman_cases("test_suanko.txt", cases);

    SECTION("四暗刻")
    {
        for (auto &[tehai, agarihai, expected] : cases) {
            bool actual = score.check_suanko(tehai, huro_blocks, agarihai);
            INFO("手牌: " << tehai << " " << std::boolalpha << actual << " vs " << expected);
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("四暗刻")
    {
        for (auto &[tehai, agarihai, expected] : cases)
            score.check_suanko(tehai, huro_blocks, agarihai);
    };
}

TEST_CASE("清老頭")
{
    SyantenCalculator::initialize();
    ScoreCalculator score = setup_score_calculator();

    std::vector<HuroBlock> huro_blocks;
    std::vector<std::tuple<Tehai, int, bool>> cases;
    load_yakuman_cases("test_tinroto.txt", cases);

    SECTION("清老頭")
    {
        for (auto &[tehai, agarihai, expected] : cases) {
            bool actual = score.check_tinroto(tehai, huro_blocks, agarihai);
            INFO("手牌: " << tehai << " " << std::boolalpha << actual << " vs " << expected);
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("清老頭")
    {
        for (auto &[tehai, agarihai, expected] : cases)
            score.check_tinroto(tehai, huro_blocks, agarihai);
    };
}

TEST_CASE("四槓子")
{
    SyantenCalculator::initialize();
    ScoreCalculator score = setup_score_calculator();

    std::vector<HuroBlock> huro_blocks = {
        {Huro::Ankan, {Tile::Manzu1, Tile::Manzu1, Tile::Manzu1, Tile::Manzu1}, 0, 0},
        {Huro::Minkan, {Tile::Pinzu1, Tile::Pinzu1, Tile::Pinzu1, Tile::Pinzu1}, 0, 0},
        {Huro::Ankan, {Tile::Sozu1, Tile::Sozu1, Tile::Sozu1, Tile::Sozu1}, 0, 0},
        {Huro::Kakan, {Tile::Ton, Tile::Ton, Tile::Ton, Tile::Ton}, 0, 0},
    };
    Tehai tehai({Tile::Manzu1, Tile::Manzu1, Tile::Manzu1, Tile::Pinzu1, Tile::Pinzu1, Tile::Pinzu1,
                 Tile::Sozu1, Tile::Sozu1, Tile::Sozu1, Tile::Ton, Tile::Ton, Tile::Ton, Tile::Haku,
                 Tile::Haku});
    int agarihai = Tile::Sozu1;

    SECTION("四槓子")
    {
        bool expected = true;
        bool actual = score.check_sukantu(tehai, huro_blocks, agarihai);
        INFO("手牌: " << tehai << " " << std::boolalpha << actual << " vs " << expected);
        REQUIRE(actual == expected);
    };
}

TEST_CASE("四暗刻単騎")
{
    SyantenCalculator::initialize();
    ScoreCalculator score = setup_score_calculator();

    std::vector<HuroBlock> huro_blocks;
    std::vector<std::tuple<Tehai, int, bool>> cases;
    load_yakuman_cases("test_suanko_tanki.txt", cases);

    SECTION("四暗刻単騎")
    {
        for (auto &[tehai, agarihai, expected] : cases) {
            bool actual = score.check_suanko_tanki(tehai, huro_blocks, agarihai);
            INFO("手牌: " << tehai << " " << Tile::Names[agarihai] << " " << std::boolalpha
                          << actual << " vs " << expected);
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("四暗刻単騎")
    {
        for (auto &[tehai, agarihai, expected] : cases)
            score.check_suanko_tanki(tehai, huro_blocks, agarihai);
    };
}

TEST_CASE("大四喜")
{
    SyantenCalculator::initialize();
    ScoreCalculator score = setup_score_calculator();

    std::vector<HuroBlock> huro_blocks;
    std::vector<std::tuple<Tehai, int, bool>> cases;
    load_yakuman_cases("test_daisusi.txt", cases);

    SECTION("大四喜")
    {
        for (auto &[tehai, agarihai, expected] : cases) {
            bool actual = score.check_daisusi(tehai, huro_blocks, agarihai);
            INFO("手牌: " << tehai << " " << std::boolalpha << actual << " vs " << expected);
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("大四喜")
    {
        for (auto &[tehai, agarihai, expected] : cases)
            score.check_daisusi(tehai, huro_blocks, agarihai);
    };
}

TEST_CASE("純正九蓮宝燈")
{
    SyantenCalculator::initialize();
    ScoreCalculator score = setup_score_calculator();

    std::vector<HuroBlock> huro_blocks;
    std::vector<std::tuple<Tehai, int, bool>> cases;
    load_yakuman_cases("test_tyurenpoto9.txt", cases);

    SECTION("純正九蓮宝燈")
    {
        for (auto &[tehai, agarihai, expected] : cases) {
            bool actual = score.check_tyurenpoto9(tehai, huro_blocks, agarihai);
            INFO("手牌: " << tehai << " " << std::boolalpha << actual << " vs " << expected);
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("純正九蓮宝燈")
    {
        for (auto &[tehai, agarihai, expected] : cases)
            score.check_tyurenpoto(tehai, huro_blocks, agarihai);
    };
}

TEST_CASE("国士無双13面待ち")
{
    SyantenCalculator::initialize();
    ScoreCalculator score = setup_score_calculator();

    std::vector<HuroBlock> huro_blocks;
    std::vector<std::tuple<Tehai, int, bool>> cases;
    load_yakuman_cases("test_kokusi13.txt", cases);

    SECTION("国士無双13面待ち")
    {
        for (auto &[tehai, agarihai, expected] : cases) {
            bool actual = score.check_kokusi13(tehai, huro_blocks, agarihai);
            INFO("手牌: " << tehai << " " << Tile::Names[agarihai] << " " << std::boolalpha
                          << actual << " vs " << expected);
            REQUIRE(actual == expected);
        }
    };

    BENCHMARK("国士無双13面待ち")
    {
        for (auto &[tehai, agarihai, expected] : cases)
            score.check_kokusi13(tehai, huro_blocks, agarihai);
    };
}