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

    // auto win_hands = calculator.get_win_hands();
    // std::sort(win_hands.begin(), win_hands.end(),
    //           [](const std::tuple<Hand, Result> &a, const std::tuple<Hand, Result> &b) {
    //               return std::get<1>(a).score[0] < std::get<1>(b).score[0];
    //           });

    // for (const auto &[hand, result] : win_hands) {
    //     std::cout << fmt::format("手牌: {}, 結果: {}符{}翻 {}点", hand.to_string(),
    //                              result.fu, result.han, result.score[0])
    //               << std::endl;
    // }
    // std::cout << fmt::format("和了形の数: {}", win_hands.size()) << std::endl;

    // const auto &G = calculator.graph();
    // std::map<int, int> counts;
    // for (const auto &v : boost::make_iterator_range(boost::vertices(G))) {
    //     auto node_data = std::static_pointer_cast<HandData>(G[v]);
    //     counts[node_data->syanten]++;
    // }

    // for (auto [syanten, n] : counts) {
    //     std::cout << fmt::format("向聴数: {}, 局面数: {}", syanten, n) << std::endl;
    // }
    // std::cout << fmt::format("局面数: {}", boost::num_vertices(G)) << std::endl;

    // std::cout << fmt::format("手牌: {}, 向聴数: {}", hand.to_string(), syanten)
    //           << std::endl;
    std::cout << fmt::format("{}ms", elapsed_ms) << std::endl;
    // std::cout << fmt::format("評価手: {}", boost::num_vertices(calculator.graph()))
    //           << std::endl;
}
