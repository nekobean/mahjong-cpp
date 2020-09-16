
#include "syanten.hpp"

#include <fstream>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/dll.hpp>
#include <spdlog/spdlog.h>

using namespace mahjong;

int main(int, char **)
{
    // 初期化が必要
    SyantenCalculator::initialize();

    boost::filesystem::path path =
        boost::dll::program_location().parent_path() / "test_syanten.txt";

    // ファイルを開く。
    std::ifstream ifs(path.string());
    if (!ifs) {
        spdlog::error("Failed to open {}.", path.string());
        return 1;
    }

    const int N = 100;

    // ファイルを読み込む。
    // 形式は <14枚の牌> <一般手の向聴数> <国士の向聴数> <七対子の向聴数>
    std::vector<Tehai> tehais;
    std::vector<std::tuple<int, int, int>> results;

    std::string line;
    while (std::getline(ifs, line)) {
        std::vector<std::string> tokens;
        boost::split(tokens, line, boost::is_any_of(" "));

        std::vector<int> tiles(14);
        for (int i = 0; i < 14; ++i)
            tiles[i] = std::stoi(tokens[i]);

        tehais.emplace_back(tiles);
        results.emplace_back(std::stoi(tokens[14]), std::stoi(tokens[15]), std::stoi(tokens[16]));
    }

    {
        std::vector<Tehai> tehais;
        tehais.reserve(1000000);
        auto begin = std::chrono::steady_clock::now();
        std::vector<int> tiles = {0, 0, 6, 8, 8, 27, 27, 28, 31, 31, 31, 31, 32, 33};
        Tehai tehai(tiles);
        int cnt = 0;
        for (int i = 0; i < 1000000; ++i)
            if (tehai.to_kanji_string() != "一萬一萬七萬九萬九萬 東東南白白白白發中")
                cnt++;

        auto end = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
        spdlog::info("total time: {}ms", elapsed);
    }

    // 一般手
    ///////////////////////////////////

    {
        auto begin = std::chrono::steady_clock::now();
        int actual, expected;
        for (int i = 0; i < N; ++i) {
            for (size_t i = 0; i < tehais.size(); ++i) {
                actual = SyantenCalculator::calc_normal(tehais[i]);
                expected = std::get<0>(results[i]);

                if (actual != expected)
                    spdlog::warn("wrong result found. tehai: {}, expected: {}, actual: {}",
                                 tehais[i].to_string(), expected, actual);
            }
        }
        auto end = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
        spdlog::info("[Normal Syanten] N: {}, total time: {}ms, time/N: {:.6f}ms",
                     tehais.size() * N, elapsed, double(elapsed) / (tehais.size() * N));
    }

    // 七対子手
    ///////////////////////////////////

    {
        auto begin = std::chrono::steady_clock::now();
        int actual, expected;
        for (int i = 0; i < N; ++i) {
            for (size_t i = 0; i < tehais.size(); ++i) {
                actual = SyantenCalculator::calc_tiitoi(tehais[i]);
                expected = std::get<2>(results[i]);

                if (actual != expected)
                    spdlog::warn("wrong result found. tehai: {}, expected: {}, actual: {}",
                                 tehais[i].to_string(), expected, actual);
            }
        }
        auto end = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
        spdlog::info("[Tiitoi Synten] N: {}, total time: {}ms, time/N: {:.6f}ms", tehais.size() * N,
                     elapsed, double(elapsed) / (tehais.size() * N));
    }

    // 国士手
    ///////////////////////////////////

    {
        auto begin = std::chrono::steady_clock::now();
        int actual, expected;
        for (int i = 0; i < N; ++i) {
            for (size_t i = 0; i < tehais.size(); ++i) {
                actual = SyantenCalculator::calc_kokushi(tehais[i]);
                expected = std::get<1>(results[i]);

                if (actual != expected)
                    spdlog::warn("wrong result found. tehai: {}, expected: {}, actual: {}",
                                 tehais[i].to_string(), expected, actual);
            }
        }
        auto end = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
        spdlog::info("[Kokushi Synten] N: {}, total time: {}ms, time/N: {:.6f}ms",
                     tehais.size() * N, elapsed, double(elapsed) / (tehais.size() * N));
    }

    return 0;
}
