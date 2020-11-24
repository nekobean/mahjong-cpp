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
bool load_testcase(std::vector<std::tuple<Hand, int, int, int>> &cases)
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
    // 形式は `<牌1> <牌2> ... <牌14> <一般手の向聴数> <国士無双手の向聴数> <七対子手の向聴数>`
    std::string line;
    while (std::getline(ifs, line)) {
        std::vector<std::string> tokens;
        boost::split(tokens, line, boost::is_any_of(" "));

        std::vector<int> tiles(14);
        for (int i = 0; i < 14; ++i)
            tiles[i] = std::stoi(tokens[i]);

        cases.emplace_back(Hand(tiles), std::stoi(tokens[14]), std::stoi(tokens[15]),
                           std::stoi(tokens[16]));
    }

    std::cout << fmt::format("{} testcases loaded.", cases.size()) << std::endl;

    return true;
}

TEST_CASE("Calculate Normal Syanten")
{
    std::vector<std::tuple<Hand, int, int, int>> cases;
    if (!load_testcase(cases))
        return;

    SyantenCalculator::initialize();

    SECTION("Normal Syanten")
    {
        for (auto &[hand, normal, kokusi, tiitoi] : cases)
            REQUIRE(SyantenCalculator::calc_normal(hand) == normal);
    };

    BENCHMARK("Normal Syanten")
    {
        for (auto &[hand, normal, kokusi, tiitoi] : cases)
            SyantenCalculator::calc_normal(hand);
    };
}

TEST_CASE("Calculate Tiitoi Syanten")
{
    std::vector<std::tuple<Hand, int, int, int>> cases;
    if (!load_testcase(cases))
        return;

    SyantenCalculator::initialize();

    SECTION("Tiitoi Syanten")
    {
        for (auto &[hand, normal, kokusi, tiitoi] : cases)
            REQUIRE(SyantenCalculator::calc_tiitoi(hand) == tiitoi);
    };

    BENCHMARK("Tiitoi Syanten")
    {
        for (auto &[hand, normal, kokusi, tiitoi] : cases)
            SyantenCalculator::calc_tiitoi(hand);
    };
}

TEST_CASE("Calculate Kokusi Syanten")
{
    std::vector<std::tuple<Hand, int, int, int>> cases;
    if (!load_testcase(cases))
        return;

    SyantenCalculator::initialize();

    SECTION("Kokusi Syanten")
    {
        for (auto &[hand, normal, kokusi, tiitoi] : cases)
            REQUIRE(SyantenCalculator::calc_kokusi(hand) == kokusi);
    };

    BENCHMARK("Kokusi Syanten")
    {
        for (auto &[hand, normal, kokusi, tiitoi] : cases)
            SyantenCalculator::calc_kokusi(hand);
    };
}

TEST_CASE("Calculate All Syanten")
{
    std::vector<std::tuple<Hand, int, int, int>> cases;
    if (!load_testcase(cases))
        return;

    SyantenCalculator::initialize();

    SECTION("All Syanten")
    {
        for (auto &[hand, normal, kokusi, tiitoi] : cases) {
            auto [syanten_type, syanten] = SyantenCalculator::calc(hand);
            REQUIRE(syanten == std::min({normal, kokusi, tiitoi}));
        }
    };

    BENCHMARK("All Syanten")
    {
        for (auto &[hand, normal, kokusi, tiitoi] : cases)
            SyantenCalculator::calc(hand);
    };
}

TEST_CASE("Initialize Syanten Table")
{
    BENCHMARK("Syantan table initialization")
    {
        return SyantenCalculator::initialize();
    };
}
