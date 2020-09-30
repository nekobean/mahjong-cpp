#include <fstream>
#include <iostream>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/dll.hpp>
#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include <catch2/catch.hpp>

#include "syanten.hpp"

using namespace mahjong;

/**
 * @brief テストケースを読み込む。
 * 
 * @param[out] cases テストケース
 * @return 読み込みに成功した場合は true、そうでない場合は false を返す。
 */
bool load_syanten_case(std::vector<std::tuple<Tehai, int, int, int>> &cases)
{
    cases.clear();

    boost::filesystem::path path =
        boost::dll::program_location().parent_path() / "test_syanten.txt";

    // ファイルを開く。
    std::ifstream ifs(path.string());
    if (!ifs) {
        std::cerr << "Failed to open " << path.string() << "." << std::endl;
        return false;
    }

    // ファイルを読み込む。
    // 形式は <14枚の牌> <一般手の向聴数> <国士無双手の向聴数> <七対子手の向聴数>
    std::string line;
    while (std::getline(ifs, line)) {
        std::vector<std::string> tokens;
        boost::split(tokens, line, boost::is_any_of(" "));

        std::vector<int> tiles(14);
        for (int i = 0; i < 14; ++i)
            tiles[i] = std::stoi(tokens[i]);

        cases.emplace_back(Tehai(tiles), std::stoi(tokens[14]), std::stoi(tokens[15]),
                           std::stoi(tokens[16]));
    }

    return true;
}

TEST_CASE("Calculate Normal Syanten")
{
    std::vector<std::tuple<Tehai, int, int, int>> cases;
    if (!load_syanten_case(cases))
        return;

    SyantenCalculator::initialize();

    SECTION("Normal Syanten")
    {
        for (auto &[tehai, normal, kokusi, tiitoi] : cases)
            REQUIRE(SyantenCalculator::calc_normal(tehai) == normal);
    };

    BENCHMARK("Normal Syanten")
    {
        for (auto &[tehai, normal, kokusi, tiitoi] : cases)
            SyantenCalculator::calc_normal(tehai);
    };
}

TEST_CASE("Calculate Tiitoi Syanten")
{
    std::vector<std::tuple<Tehai, int, int, int>> cases;
    if (!load_syanten_case(cases))
        return;

    SyantenCalculator::initialize();

    SECTION("Tiitoi Syanten")
    {
        for (auto &[tehai, normal, kokusi, tiitoi] : cases)
            REQUIRE(SyantenCalculator::calc_tiitoi(tehai) == tiitoi);
    };

    BENCHMARK("Tiitoi Syanten")
    {
        for (auto &[tehai, normal, kokusi, tiitoi] : cases)
            SyantenCalculator::calc_tiitoi(tehai);
    };
}

TEST_CASE("Calculate Kokusi Syanten")
{
    std::vector<std::tuple<Tehai, int, int, int>> cases;
    if (!load_syanten_case(cases))
        return;

    SyantenCalculator::initialize();

    SECTION("Kokusi Syanten")
    {
        for (auto &[tehai, normal, kokusi, tiitoi] : cases)
            REQUIRE(SyantenCalculator::calc_kokusi(tehai) == kokusi);
    };

    BENCHMARK("Kokusi Syanten")
    {
        for (auto &[tehai, normal, kokusi, tiitoi] : cases)
            SyantenCalculator::calc_kokusi(tehai);
    };
}

TEST_CASE("Calculate All Syanten")
{
    std::vector<std::tuple<Tehai, int, int, int>> cases;
    if (!load_syanten_case(cases))
        return;

    SyantenCalculator::initialize();

    SECTION("All Syanten")
    {
        for (auto &[tehai, normal, kokusi, tiitoi] : cases)
            REQUIRE(SyantenCalculator::calc(tehai) == std::min({normal, kokusi, tiitoi}));
    };

    BENCHMARK("All Syanten")
    {
        for (auto &[tehai, normal, kokusi, tiitoi] : cases)
            SyantenCalculator::calc(tehai);
    };
}

TEST_CASE("Initialize Syanten Table")
{
    BENCHMARK("Syantan table initialization")
    {
        return SyantenCalculator::initialize();
    };
}