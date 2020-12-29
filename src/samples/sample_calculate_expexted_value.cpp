#include "mahjong/mahjong.hpp"

#include <chrono>

using namespace mahjong;

int main(int, char **)
{
    ScoreCalculator score;
    score.set_bakaze(Tile::Ton);
    score.set_zikaze(Tile::Ton);
    score.set_num_tumibo(0);
    score.set_num_kyotakubo(0);
    score.set_dora_tiles({Tile::Haku});

    ExpectedValueCalculator calculator;

    Hand hand({Tile::Manzu2, Tile::Manzu2, Tile::Manzu2, Tile::Manzu5, Tile::Manzu6,
               Tile::Manzu7, Tile::Pinzu3, Tile::Pinzu4, Tile::Pinzu5, Tile::Sozu3,
               Tile::Sozu3, Tile::Sozu6, Tile::Sozu6, Tile::Sozu7});
    // Hand hand({Tile::Manzu2, Tile::Manzu2, Tile::Manzu2, Tile::Manzu5, Tile::Manzu6,
    //            Tile::Manzu7, Tile::Pinzu3, Tile::Pinzu4, Tile::Sozu3, Tile::Sozu3,
    //            Tile::Sozu6, Tile::Sozu6, Tile::Sozu7, Tile::Pe});
    // Hand hand({Tile::Manzu1, Tile::Manzu1, Tile::Manzu2, Tile::Manzu4, Tile::Manzu5,
    //            Tile::Manzu7, Tile::Pinzu9, Tile::Sozu4, Tile::Sozu7, Tile::Sozu9,
    //            Tile::Ton, Tile::Pe, Tile::Pe, Tile::Hatu});

    auto [syanten_type, syanten] = SyantenCalculator::calc(hand, SyantenType::Normal);
    std::cout << syanten << std::endl;

    auto begin = std::chrono::steady_clock::now();
    calculator.calc(hand, score, SyantenType::Normal);
    auto end = std::chrono::steady_clock::now();
    auto elapsed_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
    std::cout << fmt::format("{}ms", elapsed_ms) << std::endl;
}
