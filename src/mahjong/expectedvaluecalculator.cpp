#include "expectedvaluecalculator.hpp"

#undef NDEBUG
#include <algorithm>
#include <assert.h>
#include <numeric>

namespace mahjong {

ExpectedValueCalculator::ExpectedValueCalculator()
{
    initialize();
}

void ExpectedValueCalculator::initialize()
{
    tumo_probs_table_.resize(5);
    for (int i = 0; i < 5; ++i) {
        std::vector<double> probs(17);

        for (int j = 0; j < 17; ++j) {
            probs[j] = double(i) / double(121 - j);
        }

        tumo_probs_table_[i] = probs;
    }

    not_tumo_probs_table_.resize(121);
    for (int i = 0; i < 121; ++i) {
        std::vector<double> probs(17);
        probs[0] = 1;

        double prob = 1;
        for (int j = 0; j < 16; ++j) {
            prob *= double(121 - j - i) / double(121 - j);
            probs[j + 1] = prob;
        }

        not_tumo_probs_table_[i] = probs;
    }
}

/**
 * @brief 期待値を計算する。
 * 
 * @param[in] hand 手牌
 * @param[in] score 点数計算機
 * @param[in] syanten_type 向聴数の種類
 */
void ExpectedValueCalculator::calc(const Hand &hand, const ScoreCalculator &score,
                                   int syanten_type)
{
    score_        = score;
    hand_         = hand;
    syanten_type_ = syanten_type;

    // グラフを作成する。
    int n_tiles = hand.num_tiles() + int(hand.melded_blocks.size()) * 3;
    if (n_tiles != 14)
        return;

    // 現在の向聴数を計算する。
    auto [_, syanten] = SyantenCalculator::calc(hand, syanten_type_);
    if (syanten == -1 || syanten == 4)
        return; // 和了形

    // 各牌の残り枚数を数える。
    counts_ = count_left_tiles(hand, score.dora_tiles());

    // グラフを作成する。
    build_tree_discard_first(1, syanten);
}

/**
 * @brief 各牌の残り枚数を数える。
 * 
 * @param[in] hand 手牌
 * @param[in] dora_tiles ドラ牌の一覧
 * @return std::vector<int> 各牌の残り枚数
 */
std::vector<int>
ExpectedValueCalculator::count_left_tiles(const Hand &hand,
                                          const std::vector<int> &dora_tiles)
{
    std::vector<int> counts(34, 4);

    for (int i = 0; i < 34; ++i)
        counts[i] -= hand.num_tiles(i);

    for (const auto &block : hand.melded_blocks) {
        for (auto tile : block.tiles) {
            tile = aka2normal(tile);
            counts[tile]--;
        }
    }

    for (auto tile : dora_tiles)
        counts[tile]--;

    return counts;
}

/**
 * @brief 有効牌の合計枚数を数える。
 * 
 * @param[in] count 各牌の残り枚数
 * @param[in] tiles 有効牌の一覧
 * @return int 有効牌の合計枚数
 */
int ExpectedValueCalculator::count_num_required_tiles(const std::vector<int> &count,
                                                      const std::vector<int> &tiles)
{
    int n_tiles = 0;
    for (auto tile : tiles)
        n_tiles += count[tile];

    return n_tiles;
}

/**
 * @brief グラフ (DAG) を作成する。
 * 
 * @param[in,out] G グラフ
 * @param[in] parent 親ノード
 */
std::tuple<std::vector<double>, std::vector<double>, std::vector<double>>
ExpectedValueCalculator::build_tree_draw(int n_left_tumo, int syanten)
{
    std::vector<double> tenpai_probs(17, 0);
    std::vector<double> win_probs(17, 0);
    std::vector<double> exp_values(17, 0);

    DrawCache cache = get_draw_tiles(hand_, syanten);

    int sum_required_tiles = 0;
    for (const auto &tile : cache.hands1)
        sum_required_tiles += counts_[tile];

    for (const auto &tile : cache.hands1) {
        int n_required_tiles = counts_[tile]; // 有効牌枚数
        if (n_required_tiles == 0)
            continue; // 残り枚数が0枚の場合

        // 手牌に加える
        add_tile(hand_, tile);
        counts_[tile]--;

        auto [next_tenpai_probs, next_win_probs, next_exp_values] =
            build_tree_discard(n_left_tumo, syanten - 1, tile);

        const std::vector<double> &tumo_probs = tumo_probs_table_[n_required_tiles];
        const std::vector<double> &not_tumo_probs =
            not_tumo_probs_table_[sum_required_tiles];

        for (int i = 0; i < 17; ++i) {
            double tenpai_prob = 0;
            double win_prob    = 0;
            double exp_value   = 0;
            for (int j = i; j < 17; ++j) {
                double prob = tumo_probs[j] * not_tumo_probs[j] / not_tumo_probs[i];

                if (syanten == 0)
                    exp_value += prob * next_exp_values.front();
                else if (j < 16 && syanten > 0)
                    exp_value += prob * next_exp_values[j + 1];

                if (syanten == 0)
                    win_prob += prob;
                else if (j < 16 && syanten > 0)
                    win_prob += prob * next_win_probs[j + 1];

                if (syanten == 1)
                    tenpai_prob += prob;
                else if (j < 16 && syanten > 1)
                    tenpai_prob += prob * next_tenpai_probs[j + 1];
            }

            win_probs[i] += win_prob;
            tenpai_probs[i] += tenpai_prob;
            exp_values[i] += exp_value;
        }

        if (syanten == 0)
            std::fill(tenpai_probs.begin(), tenpai_probs.end(), 1);

        // 手牌から除く
        counts_[tile]++;
        remove_tile(hand_, tile);
    }

    return {tenpai_probs, win_probs, exp_values};
}

/**
 * @brief グラフ (DAG) を作成する。
 * 
 * @param[in,out] G グラフ
 * @param[in] parent 親ノード
 */
std::tuple<std::vector<double>, std::vector<double>, std::vector<double>>
ExpectedValueCalculator::build_tree_discard(int n_left_tumo, int syanten, int tumo_tile)
{
    std::vector<double> max_win_probs;
    std::vector<double> max_tenpai_probs;
    std::vector<double> max_exp_values;
    double max_exp = 0;

    if (syanten == -1) {
        Result result = score_.calc(hand_, tumo_tile, HandFlag::Reach | HandFlag::Tumo);
        return {{}, {}, std::vector<double>(17, result.score[0])};
    }

    DiscardCache cache = get_discard_tiles(hand_, syanten);

    for (const auto &tile : cache.hands1) {
        remove_tile(hand_, tile);

        auto [tenpai_probs, win_probs, exp_values] =
            build_tree_draw(n_left_tumo, syanten);

        if (win_probs.front() > max_exp) {
            max_exp          = win_probs.front();
            max_tenpai_probs = tenpai_probs;
            max_win_probs    = win_probs;
            max_exp_values   = exp_values;
        }

        add_tile(hand_, tile);
    }

    return {max_tenpai_probs, max_win_probs, max_exp_values};
}

/**
 * @brief グラフ (DAG) を作成する。
 * 
 * @param[in,out] G グラフ
 * @param[in] parent 親ノード
 */
void ExpectedValueCalculator::build_tree_discard_first(int n_left_tumo, int syanten)
{
    DiscardCache cache = get_discard_tiles(hand_, syanten);

    for (const auto &tile : cache.hands1) {

        remove_tile(hand_, tile);

        auto [tenpai_probs, win_probs, exp_values] =
            build_tree_draw(n_left_tumo, syanten);

        std::cout << "聴牌確率 ";
        std::cout << fmt::format("{}: ", Tile::Name.at(tile));
        for (int i = 0; i < 17; ++i)
            std::cout << fmt::format("{}: {:.2f}% ", i + 1, tenpai_probs[i] * 100);
        std::cout << std::endl;

        std::cout << "和了確率 ";
        std::cout << fmt::format("{}: ", Tile::Name.at(tile));
        for (int i = 0; i < 17; ++i)
            std::cout << fmt::format("{}: {:.2f}% ", i + 1, win_probs[i] * 100);
        std::cout << std::endl;

        std::cout << "期待値 ";
        std::cout << fmt::format("{}: ", Tile::Name.at(tile));
        for (int i = 0; i < 17; ++i)
            std::cout << fmt::format("{}: {:.2f} ", i + 1, exp_values[i]);
        std::cout << std::endl;

        add_tile(hand_, tile);
    }
}

DiscardCache ExpectedValueCalculator::get_discard_tiles(Hand &hand, int syanten)
{
    auto itr = discard_cache_.find(hand);
    if (itr != discard_cache_.end())
        return itr->second;

    DiscardCache cache;
    cache.hands1.reserve(34);
    cache.hands2.reserve(34);

    for (int tile = 0; tile < 34; ++tile) {
        if (!hand.contains(tile))
            continue; // 牌が手牌にない場合

        // 手牌から除く
        remove_tile(hand, tile);

        auto [_, syanten_after] = SyantenCalculator::calc(hand, syanten_type_);
        if (syanten == syanten_after)
            cache.hands1.push_back(tile);
        else
            cache.hands2.push_back(tile);

        // 手牌に戻す
        add_tile(hand, tile);
    }

    discard_cache_.insert_or_assign(hand, cache);

    return cache;
}

DrawCache ExpectedValueCalculator::get_draw_tiles(Hand &hand, int syanten)
{
    auto itr = draw_cache_.find(hand);
    if (itr != draw_cache_.end())
        return itr->second;

    DrawCache cache;
    cache.hands1.reserve(34);
    cache.hands2.reserve(34);

    for (int tile = 0; tile < 34; ++tile) {
        // 手牌から除く
        add_tile(hand, tile);

        auto [_, syanten_after] = SyantenCalculator::calc(hand, syanten_type_);
        if (syanten > syanten_after)
            cache.hands1.push_back(tile);
        else
            cache.hands2.push_back(tile);

        // 手牌に戻す
        remove_tile(hand, tile);
    }

    draw_cache_.insert_or_assign(hand, cache);

    return cache;
}

} // namespace mahjong
