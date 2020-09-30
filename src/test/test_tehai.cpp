#include <fstream>
#include <iostream>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/dll.hpp>
#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include <catch2/catch.hpp>

#include "types.hpp"

using namespace mahjong;

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
    // 形式は <14枚の牌> <一般手の向聴数> <国士の向聴数> <七対子の向聴数>
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

TEST_CASE("Calculate Tehai")
{
    SECTION("Tehai::to_kanji_string()")
    {
        std::vector<int> tiles = {0, 0, 6, 8, 8, 27, 27, 28, 31, 31, 31, 31, 32, 33};
        Tehai tehai(tiles);
        REQUIRE(tehai.to_kanji_string() == "一萬一萬七萬九萬九萬 東東南白白白白發中");
    }

    std::vector<int> tiles = {0, 0, 6, 8, 8, 27, 27, 28, 31, 31, 31, 31, 32, 33};
    BENCHMARK("Tehai::Tehai()")
    {
        Tehai tehai(tiles);
    };
}
