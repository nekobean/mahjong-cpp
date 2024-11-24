#include <algorithm> // find
#include <fstream>
#include <iostream>
#include <random>
#include <vector>

#undef NDEBUG
#include <cassert>

#include <boost/filesystem.hpp>

#include "mahjong/mahjong.hpp"

using namespace mahjong;

int count_uradora(const std::vector<int> &yama, const std::vector<int> &hand, int n,
                  std::mt19937 &engine)
{
    // Sample uradora indicators.
    std::vector<int> indicators;
    std::sample(yama.begin(), yama.end(), std::back_inserter(indicators), n, engine);

    int num_doras = 0;
    for (auto tile : indicators) {
        num_doras += int(std::count(hand.begin(), hand.end(), ToDora[tile]));
    }

    return num_doras;
}

int main()
{
    std::vector<int> yama(136);
    for (int i = 0; i < 136; ++i) {
        yama[i] = i / 4;
    }

    std::mt19937 engine(42);
    std::vector<int> hand = {Tile::Manzu1, Tile::Manzu2, Tile::Manzu3, Tile::Manzu5,
                             Tile::Manzu6, Tile::Manzu7, Tile::Pinzu1, Tile::Pinzu2,
                             Tile::Pinzu3, Tile::Pinzu6, Tile::Pinzu7, Tile::Pinzu8,
                             Tile::Souzu1, Tile::Souzu1};

    // Remove hand tiles from yama.
    for (auto tile : hand) {
        yama.erase(std::find(yama.begin(), yama.end(), tile));
    }

    // 裏ドラが乗った数
    const int N = 10000000;
    std::array<std::array<double, 13>, 6> stats{0}; // 0~5枚, 0~12枚以上

    for (int n = 0; n <= 5; ++n) {
        for (int i = 0; i < N; ++i) {
            const int num_uradora = std::min(count_uradora(yama, hand, n, engine), 12);
            ++stats[n][num_uradora];
        }
    }

    for (int n = 0; n <= 5; ++n) {
        for (int i = 0; i <= 12; ++i) {
            stats[n][i] /= N;
        }
    }

    boost::filesystem::path table_path =
        boost::filesystem::path(CMAKE_CONFIG_DIR) / "uradora.bin";

    std::ofstream ofs(table_path.string(), std::ios::binary);
    ofs.write(reinterpret_cast<const char *>(&stats), sizeof(stats));
    ofs.close();

    return 0;
}
