#include "mahjong/mahjong.hpp"

#include <chrono>

using namespace mahjong;

int main(int, char **)
{
    ScoreCalculator score;
    score.set_bakaze(Tile::Ton);
    score.set_zikaze(Tile::Nan);
    score.set_num_tumibo(0);
    score.set_num_kyotakubo(0);
    score.set_dora_tiles({Tile::Pe});

    ExpectedValueCalculator calculator;

    Hand hand({Tile::Manzu9, Tile::Manzu9, Tile::Manzu9, Tile::Manzu8, Tile::Manzu8,
               Tile::Pinzu3, Tile::Pinzu5, Tile::Pinzu5, Tile::Tyun, Tile::Pinzu9,
               Tile::Sozu2, Tile::Sozu4, Tile::Haku, Tile::Hatu});

    auto [syanten_type, syanten] = SyantenCalculator::calc(hand, SyantenType::Normal);

    std::cout << fmt::format("手牌: {}, 向聴数: {}", hand.to_string(), syanten)
              << std::endl;

    auto begin = std::chrono::steady_clock::now();
    calculator.calc(hand, score, SyantenType::Normal);
    auto end = std::chrono::steady_clock::now();
    auto elapsed_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
    std::cout << fmt::format("{}ms", elapsed_ms) << std::endl;

    auto win_hands = calculator.get_win_hands();
    std::sort(win_hands.begin(), win_hands.end(),
              [](const std::tuple<Hand, Result> &a, const std::tuple<Hand, Result> &b) {
                  return std::get<1>(a).score[0] < std::get<1>(b).score[0];
              });

    // for (const auto &[hand, result] : win_hands) {
    //     std::cout << fmt::format("手牌: {}, 結果: {}符{}翻 {}点", hand.to_string(),
    //                              result.fu, result.han, result.score[0])
    //               << std::endl;
    // }
    std::cout << fmt::format("和了形の数: {}", win_hands.size()) << std::endl;
}
