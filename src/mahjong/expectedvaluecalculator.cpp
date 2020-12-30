#include "expectedvaluecalculator.hpp"

#undef NDEBUG
#include <algorithm>
#include <assert.h>
#include <numeric>

#include "requiredtileselector.hpp"
#include "syanten.hpp"

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

    discard_cache_.resize(5);
    draw_cache_.resize(5);
}

/**
 * @brief 期待値を計算する。
 * 
 * @param[in] hand 手牌
 * @param[in] score 点数計算機
 * @param[in] syanten_type 向聴数の種類
 */
std::tuple<bool, std::vector<Candidate>>
ExpectedValueCalculator::calc(const Hand &hand, const ScoreCalculator &score,
                              int syanten_type)
{
    for (auto &cache : discard_cache_)
        cache.clear();
    for (auto &cache : draw_cache_)
        cache.clear();
    score_cache_.clear();

    score_        = score;
    hand_         = hand;
    syanten_type_ = syanten_type;

    // グラフを作成する。
    int n_tiles = hand.num_tiles() + int(hand.melded_blocks.size()) * 3;
    if (n_tiles != 14)
        return {false, {}};

    // 現在の向聴数を計算する。
    auto [_, syanten] = SyantenCalculator::calc(hand, syanten_type_);
    if (syanten == -1 || syanten > 3)
        return {false, {}};

    // 各牌の残り枚数を数える。
    counts_ = count_left_tiles(hand, score.dora_tiles());

    // グラフを作成する。
    std::vector<Candidate> candidates = analyze(1, syanten);

    return {true, candidates};
}

/**
 * @brief 手牌の有効牌の一覧を取得する。
 * 
 * @param[in] hand 手牌
 * @param[in] syanten_type 向聴数の種類
 * @return (有効牌の合計枚数, 有効牌の一覧)
 */
std::tuple<int, std::vector<std::tuple<int, int>>>
ExpectedValueCalculator::get_required_tiles(const Hand &hand, int syanten_type)
{
    // 有効牌の一覧を取得する。
    std::vector<int> tiles = RequiredTileSelector::select(hand, syanten_type);

    std::vector<std::tuple<int, int>> required_tiles;
    int sum_required_tiles = 0;
    for (auto tile : tiles) {
        required_tiles.emplace_back(tile, counts_[tile]);
        sum_required_tiles += counts_[tile];
    }

    return {sum_required_tiles, required_tiles};
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

    DrawTilesCache &cache = get_draw_tiles(hand_, syanten);

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
    if (syanten == -1) {
        ScoreCache &cache = get_score(hand_, tumo_tile);
        return {{}, {}, std::vector<double>(17, cache.score)};
    }

    std::vector<double> max_win_probs;
    std::vector<double> max_tenpai_probs;
    std::vector<double> max_exp_values;

    DiscardTilesCache cache = get_discard_tiles(hand_, syanten);

    for (const auto &tile : cache.hands1) {
        remove_tile(hand_, tile);

        auto [tenpai_probs, win_probs, exp_values] =
            build_tree_draw(n_left_tumo, syanten);

        if (max_exp_values.empty() || exp_values.front() > max_exp_values.front()) {
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
std::vector<Candidate> ExpectedValueCalculator::analyze(int n_left_tumo, int syanten)
{
    std::vector<Candidate> candidates;

    DiscardTilesCache &cache = get_discard_tiles(hand_, syanten);

    for (const auto &tile : cache.hands1) {
        remove_tile(hand_, tile);

        auto [sum_required_tiles, required_tiles] =
            get_required_tiles(hand_, syanten_type_);

        auto [tenpai_probs, win_probs, exp_values] =
            build_tree_draw(n_left_tumo, syanten);

        candidates.emplace_back(tile, sum_required_tiles, required_tiles, tenpai_probs,
                                win_probs, exp_values, false);

        add_tile(hand_, tile);
    }

    return candidates;
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
 * @brief 打牌一覧を取得する。
 * 
 * @param[in] hand 手牌
 * @param[in] syanten 手牌の向聴数
 * @return 打牌一覧
 */
DiscardTilesCache &ExpectedValueCalculator::get_discard_tiles(Hand &hand, int syanten)
{
    auto &table = discard_cache_[syanten];

    assert(syanten > -1);

    if (auto itr = table.find(hand); itr != table.end())
        return itr->second; // キャッシュが存在する場合

    DiscardTilesCache cache;
    cache.hands1.reserve(34);
    cache.hands2.reserve(34);

    for (int tile = 0; tile < 34; ++tile) {
        if (!hand.contains(tile))
            continue; // 牌が手牌にない場合

        remove_tile(hand, tile);

        auto [_, syanten_after] = SyantenCalculator::calc(hand, syanten_type_);
        if (syanten == syanten_after)
            cache.hands1.push_back(tile);
        else
            cache.hands2.push_back(tile);

        add_tile(hand, tile);
    }

    auto [itr, _] = table.insert_or_assign(hand, cache);

    return itr->second;
}

/**
 * @brief 自摸牌一覧を取得する。
 * 
 * @param[in] hand 手牌
 * @param[in] syanten 手牌の向聴数
 * @return 自摸牌一覧
 */
DrawTilesCache &ExpectedValueCalculator::get_draw_tiles(Hand &hand, int syanten)
{
    auto &table = draw_cache_[syanten];

    if (auto itr = table.find(hand); itr != table.end())
        return itr->second; // キャッシュが存在する場合

    DrawTilesCache cache;
    cache.hands1.reserve(34);
    cache.hands2.reserve(34);

    for (int tile = 0; tile < 34; ++tile) {
        add_tile(hand, tile);

        auto [_, syanten_after] = SyantenCalculator::calc(hand, syanten_type_);
        if (syanten > syanten_after)
            cache.hands1.push_back(tile);
        else
            cache.hands2.push_back(tile);

        remove_tile(hand, tile);
    }

    auto [itr, _] = table.insert_or_assign(hand, cache);

    return itr->second;
}

/**
 * @brief 手牌の点数を取得する。
 * 
 * @param[in] hand 手牌
 * @param[in] win_tile 自摸牌
 * @return 点数
 */
ScoreCache &ExpectedValueCalculator::get_score(const Hand &hand, int win_tile)
{
    ScoreKey key(hand, win_tile);
    if (auto itr = score_cache_.find(key); itr != score_cache_.end())
        return itr->second; // キャッシュが存在する場合

    Result result = score_.calc(hand, win_tile, HandFlag::Reach | HandFlag::Tumo);

    ScoreCache cache(result.score[0]);

    auto [itr, _] = score_cache_.insert_or_assign(key, cache);

    return itr->second;
}

} // namespace mahjong
