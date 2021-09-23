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

    boost::filesystem::path path =
        boost::dll::program_location().parent_path() / "test_unnecessary_tile_selector.txt";

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

TEST_CASE("一般手の不要牌を選択する")
{
    std::vector<Hand> cases;
    if (!load_testcase(cases))
        return;

    BENCHMARK("一般手の不要牌を選択する")
    {
        for (const auto &hand : cases)
            UnnecessaryTileSelector::select_normal(hand);
    };
}

TEST_CASE("七対子手の不要牌を選択する")
{
    std::vector<Hand> cases;
    if (!load_testcase(cases))
        return;

    BENCHMARK("七対子手の不要牌を選択する")
    {
        for (const auto &hand : cases)
            UnnecessaryTileSelector::select_tiitoi(hand);
    };
}

TEST_CASE("国士手の不要牌を選択する")
{
    std::vector<Hand> cases;
    if (!load_testcase(cases))
        return;

    BENCHMARK("国士手の不要牌を選択する")
    {
        for (const auto &hand : cases)
            UnnecessaryTileSelector::select_kokusi(hand);
    };
}
