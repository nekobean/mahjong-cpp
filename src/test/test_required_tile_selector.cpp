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
 * @param[out] cases テストケース
 * @return 読み込みに成功した場合は true、そうでない場合は false を返す。
 */
bool load_testcase(std::vector<Hand> &cases)
{
    cases.clear();

    boost::filesystem::path path = boost::dll::program_location().parent_path() /
                                   "test_unnecessary_tile_selector.txt";

    // ファイルを開く。
    std::ifstream ifs(path.string());
    if (!ifs) {
        std::cerr << "Failed to open " << path.string() << "." << std::endl;
        return false;
    }

    // ファイルを読み込む。
    // 形式は `<牌1> <牌2> ... <牌14>`
    std::string line;
    while (std::getline(ifs, line)) {
        std::vector<std::string> tokens;
        boost::split(tokens, line, boost::is_any_of(" "));

        std::vector<int> tiles(14);
        for (int i = 0; i < 14; ++i)
            tiles[i] = std::stoi(tokens[i]);

        cases.emplace_back(tiles);
    }

    return true;
}

// TEST_CASE("検証用")
// {
//     SyantenCalculator::initialize();

//     std::vector<Hand> cases;
//     if (!load_testcase(cases))
//         return;

//     std::vector<std::vector<int>> rets1;
//     for (const auto &hand : cases) {
//         auto ret = RequiredTileSelector::select_kokusi(hand);
//         rets1.push_back(ret);
//     }

//     std::vector<std::vector<int>> rets2;
//     for (const auto &hand : cases) {
//         auto ret = RequiredTileSelector::select_kokusi2(hand);
//         rets2.push_back(ret);
//     }

//     for (size_t i = 0; i < cases.size(); ++i) {
//         INFO(fmt::format("手牌: {}", cases[i].to_string()));
//         REQUIRE(rets1[i].size() == rets2[i].size());
//         for (size_t j = 0; j < rets1[i].size(); ++j)
//             REQUIRE(rets1[i][j] == rets2[i][j]);
//     }
// }

TEST_CASE("一般手の有効牌を選択する")
{
    SyantenCalculator::initialize();

    std::vector<Hand> cases;
    if (!load_testcase(cases))
        return;

    BENCHMARK("一般手の有効牌を選択する")
    {
        for (const auto &hand : cases)
            RequiredTileSelector::select_normal(hand);
    };
}

TEST_CASE("七対子手の有効牌を選択する")
{
    SyantenCalculator::initialize();

    std::vector<Hand> cases;
    if (!load_testcase(cases))
        return;

    BENCHMARK("七対子手の有効牌を選択する")
    {
        for (const auto &hand : cases)
            RequiredTileSelector::select_tiitoi(hand);
    };
}

TEST_CASE("国士手の有効牌を選択する")
{
    SyantenCalculator::initialize();

    std::vector<Hand> cases;
    if (!load_testcase(cases))
        return;

    BENCHMARK("国士手の有効牌を選択する")
    {
        for (const auto &hand : cases)
            RequiredTileSelector::select_kokusi(hand);
    };
}
