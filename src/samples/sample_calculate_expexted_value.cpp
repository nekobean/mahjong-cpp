#include "mahjong/mahjong.hpp"

#include <chrono>

using namespace mahjong;

int main(int, char **)
{
    ExpectedValueCalculator calculator;

    // 点数計算の設定
    ScoreCalculator score;
    score.set_bakaze(Tile::Ton);
    score.set_zikaze(Tile::Ton);
    score.set_num_tumibo(0);
    score.set_num_kyotakubo(0);
    score.set_dora_tiles({Tile::Haku});

    Hand hand1({Tile::Manzu2, Tile::Manzu2, Tile::Manzu2, Tile::Manzu5, Tile::Manzu6,
                Tile::Manzu7, Tile::Pinzu3, Tile::Pinzu4, Tile::Pinzu5, Tile::Sozu3,
                Tile::Sozu3, Tile::Sozu6, Tile::Sozu6, Tile::Sozu7});
    Hand hand2({Tile::Manzu2, Tile::Manzu2, Tile::Manzu2, Tile::Manzu5, Tile::Manzu6,
                Tile::Manzu7, Tile::Pinzu3, Tile::Pinzu4, Tile::Sozu3, Tile::Sozu3,
                Tile::Sozu6, Tile::Sozu6, Tile::Sozu7, Tile::Pe});
    Hand hand3({Tile::Manzu1, Tile::Manzu1, Tile::Manzu2, Tile::Manzu4, Tile::Manzu5,
                Tile::Manzu7, Tile::Pinzu9, Tile::Sozu3, Tile::Sozu7, Tile::Sozu9,
                Tile::Ton, Tile::Pe, Tile::Pe, Tile::Hatu});

    // 向聴戻しが有効なケース
    Hand hand4({Tile::Manzu2, Tile::Manzu3, Tile::Manzu4, Tile::Manzu5, Tile::Pinzu1,
                Tile::Pinzu1, Tile::Pinzu3, Tile::Pinzu4, Tile::Pinzu7, Tile::Pinzu9,
                Tile::Sozu6, Tile::Sozu7, Tile::Sozu7, Tile::Sozu8});

    MeldedBlock block(MeldType::Kakan, {Tile::Ton, Tile::Ton, Tile::Ton, Tile::Ton});
    Hand hand5({Tile::Manzu2, Tile::Manzu3, Tile::Manzu4, Tile::Manzu5, Tile::Pinzu1,
                Tile::Pinzu1, Tile::Pinzu3, Tile::Pinzu4, Tile::Pinzu7, Tile::Pinzu9,
                Tile::Sozu6},
               {block});

    int turn         = 1;
    int n_extra_tumo = 0;
    Hand hand        = hand5;

    // 向聴数を計算する。
    auto [syanten_type, syanten] = SyantenCalculator::calc(hand, SyantenType::Normal);

    // 期待値を計算する。
    auto begin = std::chrono::steady_clock::now();
    auto [success, candidates] =
        calculator.calc(hand, score, SyantenType::Normal, n_extra_tumo);
    auto end = std::chrono::steady_clock::now();
    auto elapsed_ms =
        std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();

    // 期待値が高い順にソートする。
    std::sort(candidates.begin(), candidates.end(),
              [](const Candidate &a, const Candidate &b) {
                  return a.exp_values.front() > b.exp_values.front();
              });

    // 結果を出力する。
    if (!success) {
        std::cout << "エラー" << std::endl;
        return 1;
    }

    std::cout << fmt::format("手牌: {}, 向聴数: {}, 巡目: {}", hand.to_string(),
                             syanten, turn)
              << std::endl;

    for (const auto &candidate : candidates) {
        std::cout << fmt::format("[打 {}{}]", Tile::Name.at(candidate.tile),
                                 candidate.syanten_down ? " (向聴戻し)" : "")
                  << " ";

        std::cout << fmt::format(
            "有効牌: {}種{}枚, 聴牌確率: {:.2f}%, 和了確率: "
            "{:.2f}%, 期待値: {:.2f} ",
            candidate.required_tiles.size(), candidate.sum_required_tiles,
            candidate.tenpai_probs[turn - 1] * 100, candidate.win_probs[turn - 1] * 100,
            candidate.exp_values[turn - 1]);

        std::cout << "有効牌";
        for (const auto [tile, n] : candidate.required_tiles)
            std::cout << fmt::format(" {}", Tile::Name.at(tile));
        std::cout << std::endl;

        // std::cout << "巡目ごとの聴牌確率、和了確率、期待値" << std::endl;
        // for (size_t i = 0; i < 17; ++i) {
        //     std::cout << fmt::format("{:<2}巡目 聴牌確率: {:>5.2f}%, 和了確率: "
        //                              "{:>5.2f}%, 期待値: {:.2f}",
        //                              i + 1, candidate.tenpai_probs[i] * 100,
        //                              candidate.win_probs[i] * 100,
        //                              candidate.exp_values[i])
        //               << std::endl;
        // }
    }

    std::cout << fmt::format("計算時間: {}us", elapsed_ms) << std::endl;
}
