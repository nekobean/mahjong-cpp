#include "naiveexpectedvaluecalculator.hpp"

#undef NDEBUG
#include <algorithm>
#include <assert.h>
#include <fstream>
#include <numeric>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/dll.hpp>

#include "mahjong/requiredtileselector.hpp"
#include "mahjong/syanten.hpp"
#include "mahjong/unnecessarytileselector.hpp"
#include "mahjong/utils.hpp"

namespace mahjong
{

NaiveExpectedValueCalculator::NaiveExpectedValueCalculator()
    : calc_syanten_down_(false)
    , calc_tegawari_(false)
    , calc_double_reach_(false)
    , calc_ippatu_(false)
    , calc_haitei_(false)
    , calc_uradora_(false)
    , calc_akatile_tumo_(false)
    , maximize_win_prob_(false)
    , discard_cache_(5) // 0(聴牌) ~ 4(4向聴)
    , draw_cache_(5)    // 0(聴牌) ~ 4(4向聴)
{
    make_uradora_table();
}

/**
 * @brief 期待値を計算する。
 *
 * @param hand 手牌
 * @param score_calculator 点数計算機
 * @param dora_indicators ドラ表示牌の一覧
 * @param syanten_type 向聴数の種類
 * @param flag フラグ
 * @return 各打牌の情報
 */
std::tuple<bool, std::vector<Candidate>>
NaiveExpectedValueCalculator::calc(const Hand &hand, const ScoreCalculator &score_calculator,
                                   const std::vector<int> &dora_indicators, int syanten_type,
                                   int turn, int flag)
{
    std::vector<Candidate> candidates;

    score_calculator_ = score_calculator;
    syanten_type_ = syanten_type;
    dora_indicators_ = dora_indicators;

    calc_syanten_down_ = flag & CalcSyantenDown;
    calc_tegawari_ = flag & CalcTegawari;
    calc_double_reach_ = flag & CalcDoubleReach;
    calc_ippatu_ = flag & CalcIppatu;
    calc_haitei_ = flag & CalcHaiteitumo;
    calc_uradora_ = flag & CalcUradora;
    calc_akatile_tumo_ = flag & CalcAkaTileTumo;
    maximize_win_prob_ = flag & MaximaizeWinProb;

    // 手牌の枚数を数える。
    int n_tiles = hand.num_tiles() + int(hand.melds.size()) * 3;
    if (n_tiles != 14)
        return {false, {}}; // 手牌が14枚ではない場合

    // 現在の向聴数を計算する。
    auto [_, syanten] = SyantenCalculator::calc(hand, syanten_type_);
    if (syanten == -1)
        return {false, {}}; // 手牌が和了形の場合

    // 各牌の残り枚数を数える。
    std::vector<int> counts = count_left_tiles(hand, dora_indicators_);
    int sum_left_tiles = std::accumulate(counts.begin(), counts.begin() + 34, 0);

    // 自摸確率のテーブルを作成する。
    create_prob_table(sum_left_tiles);

    if (syanten > 3) // 3向聴以下は聴牌確率、和了確率、期待値を計算する。
        candidates = analyze(syanten, hand);
    else // 4向聴以上は受入枚数のみ計算する。
        candidates = analyze(0, syanten, hand, turn);

    // キャッシュをクリアする。
    clear_cache();

    return {true, candidates};
}

/**
 * @brief 有効牌の一覧を取得する。
 *
 * @param[in] hand 手牌
 * @param[in] syanten_type 向聴数の種類
 * @param[in] counts 各牌の残り枚数
 * @return 有効牌の一覧
 */
std::vector<std::tuple<int, int>>
NaiveExpectedValueCalculator::get_required_tiles(const Hand &hand, int syanten_type,
                                                 const std::vector<int> &counts)
{
    std::vector<std::tuple<int, int>> required_tiles;

    // 有効牌の一覧を取得する。
    std::vector<int> tiles = RequiredTileSelector::select(hand, syanten_type);

    for (auto tile : tiles)
        required_tiles.emplace_back(tile, counts[tile]);

    return required_tiles;
}

/**
 * @brief 各牌の残り枚数を数える。
 *
 * @param[in] hand 手牌
 * @param[in] dora_indicators ドラ表示牌の一覧
 * @return 各牌の残り枚数
 */
std::vector<int>
NaiveExpectedValueCalculator::count_left_tiles(const Hand &hand,
                                               const std::vector<int> &dora_indicators)
{
    std::vector<int> counts(37, 4);
    counts[Tile::AkaManzu5] = counts[Tile::AkaPinzu5] = counts[Tile::AkaSozu5] = 1;

    // 手牌を除く。
    for (int i = 0; i < 34; ++i)
        counts[i] -= hand.num_tiles(i);
    counts[Tile::AkaManzu5] -= hand.aka_manzu5;
    counts[Tile::AkaPinzu5] -= hand.aka_pinzu5;
    counts[Tile::AkaSozu5] -= hand.aka_sozu5;

    // 副露ブロックを除く。
    for (const auto &block : hand.melds) {
        for (auto tile : block.tiles) {
            counts[aka2normal(tile)]--;
            counts[Tile::AkaManzu5] -= tile == Tile::AkaManzu5;
            counts[Tile::AkaPinzu5] -= tile == Tile::AkaPinzu5;
            counts[Tile::AkaSozu5] -= tile == Tile::AkaSozu5;
        }
    }

    // ドラ表示牌を除く。
    for (auto tile : dora_indicators) {
        counts[aka2normal(tile)]--;
        counts[Tile::AkaManzu5] -= tile == Tile::AkaManzu5;
        counts[Tile::AkaPinzu5] -= tile == Tile::AkaPinzu5;
        counts[Tile::AkaSozu5] -= tile == Tile::AkaSozu5;
    }

    return counts;
}

/**
 * @brief 裏ドラ確率のテーブルを初期化する。
 *
 * @return 初期化に成功した場合は true、そうでない場合は false を返す。
 */
bool NaiveExpectedValueCalculator::make_uradora_table()
{
    if (!uradora_prob_table_.empty())
        return true;

    uradora_prob_table_.resize(6);

    boost::filesystem::path path = boost::dll::program_location().parent_path() / "uradora.txt";
    std::ifstream ifs(path.string());

    std::string line;
    int i = 0;
    while (std::getline(ifs, line)) {
        std::vector<std::string> tokens;
        boost::split(tokens, line, boost::is_any_of(" "));

        for (auto token : tokens)
            uradora_prob_table_[i].push_back(std::stod(token));
        i++;
    }

    return true;
}

/**
 * @brief 自摸確率のテーブルを初期化する。
 *
 * @param[in] n_left_tiles 1巡目時点の残り枚数の合計
 */
void NaiveExpectedValueCalculator::create_prob_table(int n_left_tiles)
{
    // 有効牌の枚数ごとに、この巡目で有効牌を引ける確率のテーブルを作成する
    // tumo_prob_table_[i][j] = 有効牌の枚数が i 枚の場合に j 巡目に有効牌が引ける確率
    tumo_prob_table_.resize(5, std::vector<double>(17));
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 17; ++j)
            tumo_prob_table_[i][j] = double(i) / double(n_left_tiles - j);
    }

    // 有効牌の合計枚数ごとに、これまでの巡目で有効牌が引けなかった確率のテーブルを作成する
    // not_tumo_prob_table_[i][j] = 有効牌の合計枚数が i 枚の場合に j - 1 巡目までに有効牌が引けなかった確率
    not_tumo_prob_table_.resize(n_left_tiles, std::vector<double>(17));
    for (int i = 0; i < n_left_tiles; ++i) {
        not_tumo_prob_table_[i][0] = 1;
        // n_left_tiles - i - j > 0 は残りはすべて有効牌の場合を考慮
        for (int j = 0; j < 16 && n_left_tiles - i - j > 0; ++j) {
            not_tumo_prob_table_[i][j + 1] = not_tumo_prob_table_[i][j] *
                                             double(n_left_tiles - i - j) /
                                             double(n_left_tiles - j);
        }
    }
}

/**
 * @brief キャッシュをクリアする。
 */
void NaiveExpectedValueCalculator::clear_cache()
{
    // デバッグ用
    // for (size_t i = 0; i < 5; ++i)
    //     spdlog::info("向聴数{} 打牌: {}, 自摸: {}", i, discard_cache_[i].size(),
    //                  draw_cache_[i].size());
    std::for_each(discard_cache_.begin(), discard_cache_.end(), [](auto &x) { x.clear(); });
    std::for_each(draw_cache_.begin(), draw_cache_.end(), [](auto &x) { x.clear(); });
}

/**
 * @brief 自摸牌一覧を取得する。
 *
 * @param[in] hand 手牌
 * @param[in] syanten 手牌の向聴数
 * @param[in] counts 各牌の残り枚数
 * @return 自摸牌候補の一覧。各要素は (牌, 残り枚数, 向聴数の変化) を表す。
 */
std::vector<std::tuple<int, int, int>>
NaiveExpectedValueCalculator::get_draw_tiles(Hand &hand, int syanten,
                                             const std::vector<int> &counts)
{
    std::vector<std::tuple<int, int, int>> flags;
    flags.reserve(34);

    for (int tile = 0; tile < 34; ++tile) {
        if (counts[tile] == 0)
            continue;

        add_tile(hand, tile);
        auto [_, syanten_after] = SyantenCalculator::calc(hand, syanten_type_);
        remove_tile(hand, tile);
        int syanten_diff = syanten_after - syanten; // 向聴数の変化

        if (calc_akatile_tumo_ && tile == Tile::Manzu5 && counts[Tile::AkaManzu5] == 1) {
            // 赤五萬が残っている場合
            if (counts[Tile::Manzu5] >= 2) {
                // 普通の牌と赤牌の両方が残っている
                flags.emplace_back(tile, counts[tile] - 1, syanten_diff);
                flags.emplace_back(Tile::AkaManzu5, 1, syanten_diff);
            }
            else if (counts[Tile::Manzu5] == 1) {
                // 赤牌のみ残っている
                flags.emplace_back(Tile::AkaManzu5, 1, syanten_diff);
            }
        }
        else if (calc_akatile_tumo_ && tile == Tile::Pinzu5 && counts[Tile::AkaPinzu5] == 1) {
            // 赤五筒が残っている場合
            if (counts[Tile::Pinzu5] >= 2) {
                // 普通の牌と赤牌の両方が残っている
                flags.emplace_back(tile, counts[tile] - 1, syanten_diff);
                flags.emplace_back(Tile::AkaPinzu5, 1, syanten_diff);
            }
            else if (counts[Tile::Pinzu5] == 1) {
                // 赤牌のみ残っている
                flags.emplace_back(Tile::AkaPinzu5, 1, syanten_diff);
            }
        }
        else if (calc_akatile_tumo_ && tile == Tile::Sozu5 && counts[Tile::AkaSozu5] == 1) {
            // 赤五索が残っている場合
            if (counts[Tile::Sozu5] >= 2) {
                // 普通の牌と赤牌の両方が残っている
                flags.emplace_back(tile, counts[tile] - 1, syanten_diff);
                flags.emplace_back(Tile::AkaSozu5, 1, syanten_diff);
            }
            else if (counts[Tile::Sozu5] == 1) {
                // 赤牌のみ残っている
                flags.emplace_back(Tile::AkaSozu5, 1, syanten_diff);
            }
        }
        else {
            flags.emplace_back(tile, counts[tile], syanten_diff);
        }
    }

    return flags;
}

/**
 * @brief 打牌一覧を取得する。
 *
 * @param[in] hand 手牌
 * @param[in] syanten 手牌の向聴数
 * @return 打牌一覧 (向聴戻し: 1、向聴数変化なし: 0、手牌に存在しない: inf)
 */
std::vector<int> NaiveExpectedValueCalculator::get_discard_tiles(Hand &hand, int syanten)
{
    std::vector<int> flags(34, std::numeric_limits<int>::max());

    for (int tile = 0; tile < 34; ++tile) {
        if (hand.contains(tile)) {
            remove_tile(hand, tile);
            auto [_, syanten_after] = SyantenCalculator::calc(hand, syanten_type_);
            add_tile(hand, tile);
            flags[tile] = syanten_after - syanten;
        }
    }

    return flags;
}

/**
 * @brief 手牌の点数を取得する。
 *
 * @param[in] hand 手牌
 * @param[in] win_tile 自摸牌
 * @param[in] counts 各牌の残り枚数
 * @return 点数
 */
std::vector<double> NaiveExpectedValueCalculator::get_score(const Hand &hand, int win_tile,
                                                            const std::vector<int> &counts)
{
    // 非門前の場合は自摸のみ
    int hand_flag = hand.is_menzen() ? (HandFlag::Tumo | HandFlag::Reach) : HandFlag::Tumo;

    // 点数計算を行う。
    Result result = score_calculator_.calc(hand, win_tile, hand_flag);

    // 表ドラの数
    int n_dora = int(dora_indicators_.size());

    // ダブル立直、一発、海底撈月で最大3翻まで増加するので、
    // ベースとなる点数、+1翻の点数、+2翻の点数、+3翻の点数も計算しておく。
    std::vector<double> scores(4, 0);
    if (result.success) {
        // 役ありの場合
        std::vector<int> up_scores = score_calculator_.get_scores_for_exp(result);

        if (calc_uradora_ && n_dora == 1) {
            // 裏ドラ考慮ありかつ表ドラが1枚以上の場合は、厳密に計算する。
            std::vector<double> n_indicators(5, 0);
            int sum_indicators = 0;
            for (int tile = 0; tile < 34; ++tile) {
                int n = hand.num_tiles(tile);
                if (n > 0) {
                    // ドラ表示牌の枚数を数える。
                    n_indicators[n] += counts[Dora2Indicator.at(tile)];
                    sum_indicators += counts[Dora2Indicator.at(tile)];
                }
            }

            // 裏ドラの乗る確率を枚数ごとに計算する。
            std::vector<double> uradora_probs(5, 0);

            // 厳密に計算するなら残り枚数は数えるべきだが、あまり影響がないので121枚で固定
            int n_left_tiles = 121;
            uradora_probs[0] = double(n_left_tiles - sum_indicators) / n_left_tiles;
            for (int i = 1; i < 5; ++i)
                uradora_probs[i] = double(n_indicators[i]) / n_left_tiles;

            for (int base = 0; base < 4; ++base) {
                for (int i = 0; i < 5; ++i) { // 裏ドラ1枚の場合、最大4翻まで乗る可能性がある
                    int han_idx = std::min(base + i, int(up_scores.size() - 1));
                    scores[base] += up_scores[han_idx] * uradora_probs[i];
                }
            }
        }
        else if (calc_uradora_ && n_dora > 1) {
            // 裏ドラ考慮ありかつ表ドラが2枚以上の場合、統計データを利用する。
            for (int base = 0; base < 4; ++base) {
                for (int i = 0; i < 13; ++i) {
                    int han_idx = std::min(base + i, int(up_scores.size() - 1));
                    scores[base] += up_scores[han_idx] * uradora_prob_table_[n_dora][i];
                }
            }
        }
        else {
            // 裏ドラ考慮なしまたは表ドラが0枚の場合
            for (int base = 0; base < 4; ++base) {
                int han_idx = std::min(base, int(up_scores.size() - 1));
                scores[base] += up_scores[han_idx];
            }
        }
    }

    return scores;
}

/**
 * @brief 自摸する。(手変わりを考慮しない)
 *
 * @param[in] n_extra_tumo
 * @param[in] syanten 向聴数
 * @param[in] hand 手牌
 * @param[in] counts 各牌の残り枚数
 * @return (各巡目の聴牌確率, 各巡目の和了確率, 各巡目の期待値)
 */
std::tuple<double, double, double>
NaiveExpectedValueCalculator::draw_without_tegawari(int n_extra_tumo, int syanten, Hand &hand,
                                                    std::vector<int> &counts, int turn)
{
    assert(syanten > -1);
    // 自摸候補を取得する。
    std::vector<std::tuple<int, int, int>> flags = get_draw_tiles(hand, syanten, counts);

    // 有効牌の合計枚数を計算する。
    int sum_required_tiles = 0;
    for (auto &[tile, count, diff] : flags) {
        if (diff == -1) // 有効牌の場合
            sum_required_tiles += count;
    }

    double tenpai_prob = 0, win_prob = 0, exp_value = 0;
    for (auto &[tile, count, diff] : flags) {
        if (diff != -1) // 有効牌以外の場合
            continue;

        const std::vector<double> &tumo_probs = tumo_prob_table_[count];
        const std::vector<double> &not_tumo_probs = not_tumo_prob_table_[sum_required_tiles];

        // 手牌に加える
        add_tile(hand, tile, counts);

        std::vector<double> scores;
        if (syanten == 0) {
            scores = get_score(hand, tile, counts);
        }

        for (int t = turn - 1; t <= 16; ++t) {
            // 現在の巡目が turn の場合に t 巡目に有効牌を引く確率
            double not_prob = not_tumo_probs[t] / not_tumo_probs[turn - 1];
            double prob = tumo_probs[t] * not_prob;

            double tumo_prob2 = double(count) / (121 - t);
            double not_prob2 = 1;
            for (int j = turn - 1; j < t; j++)
                not_prob2 *= double(121 - j - sum_required_tiles) / (121 - j);
            //std::cout << fmt::format("tumo_prob t={}, {} vs {}\n", t, tumo_probs[t], prob2);
            assert(std::abs(tumo_probs[t] - tumo_prob2) < 10e-10);
            // std::cout << fmt::format("not_tumo_prob turn={}, t={}, {} vs {}\n", turn, t, not_prob,
            //                          not_prob2);
            assert(std::abs(not_prob - not_prob2) < 10e-10);

            double next_tenpai_prob = 0, next_win_prob = 0, next_exp_value = 0;
            if (t < 16 && syanten > 0) {
                // 次の巡目がある場合
                std::tie(next_tenpai_prob, next_win_prob, next_exp_value) =
                    discard(n_extra_tumo, syanten - 1, hand, counts, t + 2);
            }

            if (syanten == 1) // 1向聴の場合は次で聴牌
                tenpai_prob += prob;
            else if (t < 16 && syanten > 1) // 2向聴以上で16巡目以下の場合
            {
                tenpai_prob += prob * next_tenpai_prob;
                // if (syanten - (16 - t) > 1) {
                //     // 聴牌までたどり着くことが不可能
                //     assert(next_tenpai_prob == 0);
                // }
                // else {
                //     assert(next_tenpai_prob > 0);
                // }
            }

            // scores[0] == 0 の場合は役なしなので、和了確率、期待値は0
            if (syanten == 0 && scores[0] != 0) { // 聴牌の場合は次で和了
                // 1巡目で聴牌の場合はダブル立直成立
                bool win_double_reach = turn == 1 && calc_double_reach_;
                // 1巡目で聴牌し、次の巡目で和了の場合は一発成立
                bool win_ippatu = t == turn - 1 && calc_ippatu_;
                // 最後の巡目で和了の場合は海底撈月成立
                bool win_haitei = t == 16 && calc_haitei_;

                win_prob += prob;
                exp_value += prob * scores[win_double_reach + win_ippatu + win_haitei];
            }
            else if (t < 16 && syanten > 0) { // 聴牌以上で16巡目以下の場合
                win_prob += prob * next_win_prob;
                exp_value += prob * next_exp_value;
            }
        }

        // 手牌から除く
        remove_tile(hand, tile, counts);
    }

    return {tenpai_prob, win_prob, exp_value};
}

/**
 * @brief 自摸する。(手変わりを考慮する)
 *
 * @param[in] n_extra_tumo
 * @param[in] syanten 向聴数
 * @param[in] hand 手牌
 * @param[in] counts 各牌の残り枚数
 * @return (各巡目の聴牌確率, 各巡目の和了確率, 各巡目の期待値)
 *
 * この関数が呼ばれた時点で向聴戻しは行われていない
 */
std::tuple<double, double, double>
NaiveExpectedValueCalculator::draw_with_tegawari(int n_extra_tumo, int syanten, Hand &hand,
                                                 std::vector<int> &counts, int turn)
{
    assert(syanten > -1);
    // 自摸候補を取得する。
    std::vector<std::tuple<int, int, int>> flags = get_draw_tiles(hand, syanten, counts);

    // 有効牌の合計枚数を計算する。
    double s = 0;
    int s2 = 0;
    int s3 = 0;
    // 残り牌を集計
    for (int i = 0; i < 34; ++i) {
        s2 += counts[i];
    }

    for (auto &[tile, count, syanten_diff] : flags) {
        s += double(count) / s2;
        s3 += count;
    }

    // if (s2 != 121 - turn - 1) {
    //     std::cout << fmt::format("121 - turn - 1={}, s={}, diff={}", 121 - turn - 1, s3,
    //                              (121 - turn - 1) - s3)
    //               << std::endl;
    // }

    // if (std::abs(s - 1) > 10e-10) {
    //     std::cout << fmt::format("t={}, sum={}", turn - 1, s2) << std::endl;
    //     std::cout << s << " " << std::endl;
    //     for (auto &[tile, count, syanten_diff] : flags) {
    //         std::cout << fmt::format("count={}, prob={:.8f}", count,
    //                                  tumo_prob_table_[count][turn - 1])
    //                   << std::endl;
    //     }
    //     assert(std::abs(s - 1) < 10e-10);
    // }

    assert(std::abs(s - 1) < 10e-10);

    double tenpai_prob = 0, win_prob = 0, exp_value = 0;
    for (auto &[tile, count, syanten_diff] : flags) {
        assert(syanten_diff == -1 || syanten_diff == 0);
        if (syanten_diff != -1)
            continue; // 有効牌以外の場合

        const std::vector<double> &tumo_probs = tumo_prob_table_[count];

        // 手牌に加える
        add_tile(hand, tile, counts);

        std::vector<double> scores;
        if (syanten == 0) {
            scores = get_score(hand, tile, counts);
        }

        int t = turn - 1;
        double next_tenpai_prob = 0, next_win_prob = 0, next_exp_value = 0;
        if (t < 16 && syanten > 0) {
            std::tie(next_tenpai_prob, next_win_prob, next_exp_value) =
                discard(n_extra_tumo, syanten - 1, hand, counts, turn + 1);
        }

#ifdef TEGAWARI_PROB
        double tumo_prob = double(count) / s3; // 確率が1を超えないようにするため
#else
        double tumo_prob = tumo_probs[t];
#endif

        if (syanten == 1) // 1向聴の場合は次で聴牌
            tenpai_prob += tumo_prob;
        else if (t < 16 && syanten > 1)
            tenpai_prob += tumo_prob * next_tenpai_prob;

        if (syanten == 0) { // 聴牌の場合は次で和了
            // 1巡目で聴牌の場合はダブル立直成立
            bool win_double_reach = turn == 1 && calc_double_reach_;
            // 1巡目で聴牌し、次の巡目で和了の場合は一発成立
            bool win_ippatu = t == turn - 1 && calc_ippatu_;
            // 最後の巡目で和了の場合は海底撈月成立
            bool win_haitei = t == 16 && calc_haitei_;

            win_prob += tumo_prob;
            exp_value += tumo_prob * scores[win_double_reach + win_ippatu + win_haitei];
        }
        else if (t < 16 && syanten > 0) {
            win_prob += tumo_prob * next_win_prob;
            exp_value += tumo_prob * next_exp_value;
        }

        // 手牌から除く
        remove_tile(hand, tile, counts);
    }

    for (auto &[tile, count, syanten_diff] : flags) {
        if (syanten_diff != 0)
            continue; // 有効牌の場合

        const std::vector<double> &tumo_probs = tumo_prob_table_[count];
        int t = turn - 1;
#ifdef TEGAWARI_PROB
        double tumo_prob = double(count) / s3; // 確率が1を超えないようにするため
#else
        double tumo_prob = tumo_probs[t];
#endif

        // 手牌に加える
        add_tile(hand, tile, counts);

        double next_tenpai_prob = 0, next_win_prob = 0, next_exp_value = 0;
        if (t < 16) {
            auto [next_tenpai_prob, next_win_prob, next_exp_value] =
                discard(n_extra_tumo + 1, syanten, hand, counts, turn + 1);
            tenpai_prob += tumo_prob * next_tenpai_prob;
            win_prob += tumo_prob * next_win_prob;
            exp_value += tumo_prob * next_exp_value;
        }

        // 手牌から除く
        remove_tile(hand, tile, counts);
    }

    return {tenpai_prob, win_prob, exp_value};
}

/**
 * @brief 自摸する。
 *
 * @param[in] n_extra_tumo
 * @param[in] syanten 向聴数
 * @param[in] hand 手牌
 * @param[in] counts 各牌の残り枚数
 * @return (各巡目の聴牌確率, 各巡目の和了確率, 各巡目の期待値)
 */
std::tuple<double, double, double> NaiveExpectedValueCalculator::draw(int n_extra_tumo, int syanten,
                                                                      Hand &hand,
                                                                      std::vector<int> &counts,
                                                                      int turn)
{
    if (calc_tegawari_ && n_extra_tumo == 0)
        return draw_with_tegawari(n_extra_tumo, syanten, hand, counts, turn);
    else
        return draw_without_tegawari(n_extra_tumo, syanten, hand, counts, turn);
}

/**
 * @brief 打牌する。
 *
 * @param[in] n_extra_tumo
 * @param[in] syanten 向聴数
 * @param[in] hand 手牌
 * @param[in] counts 各牌の残り枚数
 * @return (各巡目の聴牌確率, 各巡目の和了確率, 各巡目の期待値)
 */
std::tuple<double, double, double> NaiveExpectedValueCalculator::discard(int n_extra_tumo,
                                                                         int syanten, Hand &hand,
                                                                         std::vector<int> &counts,
                                                                         int turn)
{
    assert(syanten > -1);

    // 打牌候補を取得する。
    const std::vector<int> flags = get_discard_tiles(hand, syanten);

    // 期待値が最大となる打牌を選択する。
    double max_tenpai_prob, max_win_prob, max_exp_value;
    double tenpai_prob, win_prob, exp_value;
    int max_tile = -1;
    double max_value = -1;
    for (int tile = 0; tile < 34; ++tile) {
        int discard_tile = tile;
        // 赤牌以外が残っている場合はそちらを先に捨てる。
        if (tile == Tile::Manzu5 && hand.aka_manzu5 && hand.num_tiles(Tile::Manzu5) == 1)
            discard_tile = Tile::AkaManzu5;
        else if (tile == Tile::Pinzu5 && hand.aka_pinzu5 && hand.num_tiles(Tile::Pinzu5) == 1)
            discard_tile = Tile::AkaPinzu5;
        else if (tile == Tile::Sozu5 && hand.aka_sozu5 && hand.num_tiles(Tile::Sozu5) == 1)
            discard_tile = Tile::AkaSozu5;

        if (flags[tile] == 0) {
            // 向聴数が変化しない打牌
            remove_tile(hand, discard_tile);
            std::tie(tenpai_prob, win_prob, exp_value) =
                draw(n_extra_tumo, syanten, hand, counts, turn);
            add_tile(hand, discard_tile);
        }
        else if (calc_syanten_down_ && n_extra_tumo == 0 && flags[tile] == 1) {
            // 向聴戻ししたら最短でも聴牌できなくなる場合
            // if (turn + syanten > 18)
            //     continue; //

            // 向聴戻しになる打牌
            remove_tile(hand, discard_tile);
            std::tie(tenpai_prob, win_prob, exp_value) =
                draw(n_extra_tumo + 1, syanten + 1, hand, counts, turn);
            add_tile(hand, discard_tile);
        }
        else {
            // 手牌に存在しない牌、または向聴落としが無効な場合に向聴落としとなる牌
            continue;
        }

        // 向聴戻し、手変わりは巡目がずれるので、1つ手前にずらす。(このやり方で正しいのか要検証)

        // 和了確率は下2桁まで一致していれば同じ、期待値は下0桁まで一致していれば同じとみなす。
        double value = maximize_win_prob_ ? int(win_prob * 10000) : int(exp_value);
        if ((value == max_value && DiscardPriorities[max_tile] < DiscardPriorities[tile]) ||
            value > max_value) {
            // 値が同等なら、DiscardPriorities が高い牌を優先して選択する。
            max_tenpai_prob = tenpai_prob;
            max_win_prob = win_prob;
            max_exp_value = exp_value;

            max_value = value;
            max_tile = tile;
        }
    }

    return {max_tenpai_prob, max_win_prob, max_exp_value};
}

/**
 * @brief 手牌の推移パターンを和了まですべて解析する。
 *
 * @param [in]n_extra_tumo
 * @param [in]syanten 向聴数
 * @param [in]_hand 手牌
 * @return std::vector<Candidate> 打牌候補の一覧
 */
std::vector<Candidate> NaiveExpectedValueCalculator::analyze(int n_extra_tumo, int syanten,
                                                             const Hand &_hand, int turn)
{
    assert(syanten >= -1);

    std::vector<Candidate> candidates;
    Hand hand = _hand;

    // 各牌の残り枚数を数える。
    std::vector<int> counts = count_left_tiles(hand, dora_indicators_);

    // 打牌候補を取得する。
    const std::vector<int> flags = get_discard_tiles(hand, syanten);

    for (int tile = 0; tile < 34; ++tile) {
        int discard_tile = tile;
        // 赤牌以外が残っている場合はそちらを先に捨てる。
        if (tile == Tile::Manzu5 && hand.aka_manzu5 && hand.num_tiles(Tile::Manzu5) == 1)
            discard_tile = Tile::AkaManzu5;
        else if (tile == Tile::Pinzu5 && hand.aka_pinzu5 && hand.num_tiles(Tile::Pinzu5) == 1)
            discard_tile = Tile::AkaPinzu5;
        else if (tile == Tile::Sozu5 && hand.aka_sozu5 && hand.num_tiles(Tile::Sozu5) == 1)
            discard_tile = Tile::AkaSozu5;

        if (flags[tile] == 0) {
            remove_tile(hand, discard_tile);

            auto required_tiles = get_required_tiles(hand, syanten_type_, counts);

            auto [tenpai_prob, win_prob, exp_value] =
                draw(n_extra_tumo, syanten, hand, counts, turn);

            if (syanten == 0)
                tenpai_prob = 1;

            add_tile(hand, discard_tile);

            std::vector<double> tenpai_probs(17), win_probs(17), exp_values(17);
            tenpai_probs[turn - 1] = tenpai_prob;
            win_probs[turn - 1] = win_prob;
            exp_values[turn - 1] = exp_value;
            candidates.emplace_back(discard_tile, required_tiles, tenpai_probs, win_probs,
                                    exp_values, false);
        }
        else if (calc_syanten_down_ && flags[tile] == 1 && n_extra_tumo == 0 && syanten < 3) {
            remove_tile(hand, discard_tile);

            auto required_tiles = get_required_tiles(hand, syanten_type_, counts);

            auto [tenpai_prob, win_prob, exp_value] =
                draw(n_extra_tumo + 1, syanten + 1, hand, counts, turn);

            add_tile(hand, discard_tile);

            std::vector<double> tenpai_probs(17), win_probs(17), exp_values(17);
            tenpai_probs[turn - 1] = tenpai_prob;
            win_probs[turn - 1] = win_prob;
            exp_values[turn - 1] = exp_value;

            candidates.emplace_back(discard_tile, required_tiles, tenpai_probs, win_probs,
                                    exp_values, true);
        }
    }

    return candidates;
}

/**
 * @brief 手牌の推移パターンを1手先まで解析する。
 *
 * @param [in]syanten 向聴数
 * @param [in]_hand 手牌
 * @return std::vector<Candidate> 打牌候補の一覧
 */
std::vector<Candidate> NaiveExpectedValueCalculator::analyze(int syanten, const Hand &_hand)
{
    std::vector<Candidate> candidates;
    Hand hand = _hand;

    // 各牌の残り枚数を数える。
    std::vector<int> counts = count_left_tiles(hand, dora_indicators_);

    std::vector<int> tiles = UnnecessaryTileSelector::select(hand, syanten_type_);

    for (auto tile : tiles) {
        remove_tile(hand, tile);
        auto required_tiles = get_required_tiles(hand, syanten_type_, counts);
        add_tile(hand, tile);
        candidates.emplace_back(tile, required_tiles);
    }

    return candidates;
}

std::vector<std::vector<double>> NaiveExpectedValueCalculator::uradora_prob_table_;

} // namespace mahjong
