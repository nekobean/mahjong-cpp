#include <algorithm>
#include <fstream>
#include <iostream>
#include <random>
#include <vector>

#include "mahjong/mahjong.hpp"

using namespace mahjong;

int count_uradora(const std::vector<int> &yama, const std::vector<int> &hand, int n,
                  std::mt19937 &engine)
{
    std::vector<int> uradora_indicators;
    std::sample(yama.begin(), yama.end(), std::back_inserter(uradora_indicators), n,
                engine);

    int n_uradora = 0;
    for (auto uradora_indicator : uradora_indicators) {
        int uradora = Indicator2Dora.at(uradora_indicator);
        n_uradora += int(std::count(hand.begin(), hand.end(), uradora));
    }

    return n_uradora;
}

int main()
{
    std::vector<int> yama;
    for (int i = 0; i < 136; ++i)
        yama.push_back(i / 4);

    std::mt19937 engine(42);

    std::vector<int> hand = {Tile::Manzu1, Tile::Manzu2, Tile::Manzu3, Tile::Manzu5,
                             Tile::Manzu6, Tile::Manzu7, Tile::Pinzu1, Tile::Pinzu2,
                             Tile::Pinzu3, Tile::Pinzu6, Tile::Pinzu7, Tile::Pinzu8,
                             Tile::Souzu1, Tile::Souzu1};

    // 手牌を山から除く。
    for (auto tile : hand)
        yama.erase(std::find(yama.begin(), yama.end(), tile));

    // 裏ドラが乗った数
    int N = 10000000;
    std::vector<std::vector<double>> stats(
        6, std::vector<double>(13, 0)); //  (0~5枚, 0~12枚以上)
    for (int n = 1; n <= 5; ++n) {
        for (int i = 0; i < N; ++i) {
            int n_uradora = std::min(count_uradora(yama, hand, n, engine), 12);
            stats[n][n_uradora]++;
        }
    }

    for (int n = 1; n <= 5; ++n) {
        for (int i = 0; i <= 12; ++i)
            stats[n][i] /= N;
    }

    std::ofstream ofs("uradora.txt");
    for (int n = 0; n <= 5; ++n) {
        for (int i = 0; i <= 12; ++i)
            ofs << stats[n][i] << " ";
        ofs << std::endl;
    }
}
