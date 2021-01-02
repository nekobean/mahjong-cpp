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

/**
 * @brief 初期化する。
 */
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

    discard_cache_.resize(5);  // 0(聴牌) ~ 4(4向聴)
    draw_cache_.resize(5);     // 0(聴牌) ~ 4(4向聴)
    discard_cache2_.resize(5); // 0(聴牌) ~ 4(4向聴)
    draw_cache2_.resize(5);    // 0(聴牌) ~ 4(4向聴)
}

/**
 * @brief キャッシュをクリアする。
 */
void ExpectedValueCalculator::clear()
{
    for (auto &cache : discard_cache_)
        cache.clear();
    for (auto &cache : draw_cache_)
        cache.clear();
    for (auto &cache : discard_cache2_)
        cache.clear();
    for (auto &cache : draw_cache2_)
        cache.clear();
    score_cache_.clear();
}

/**
 * @brief 期待値を計算する。
 * @param[in] hand 手牌
 * @param[in] score 点数計算機
 * @param[in] syanten_type 向聴数の種類
 * @param[in] n_extra_tumo 交換枚数 (手変わりまたは向聴戻しを行える回数)
 * @return 各打牌の情報
 */
std::tuple<bool, std::vector<Candidate>>
ExpectedValueCalculator::calc(const Hand &hand, const ScoreCalculator &score,
                              int syanten_type, int n_extra_tumo)
{
    clear();

    score_        = score;
    syanten_type_ = syanten_type;

    // グラフを作成する。
    int n_tiles = hand.num_tiles() + int(hand.melded_blocks.size()) * 3;
    if (n_tiles != 14)
        return {false, {}};

    // 現在の向聴数を計算する。
    auto [_, syanten] = SyantenCalculator::calc(hand, syanten_type_);
    if (syanten == -1 || syanten > 3)
        return {false, {}}; // 手牌が和了形または4向聴以上

    // グラフを作成する。
    std::vector<Candidate> candidates = analyze(n_extra_tumo, syanten, hand);

    // for (size_t i = 0; i < 5; ++i)
    //     spdlog::info("{} 打牌: {}, 自摸: {} 打牌2: {}, 自摸2: {}", i,
    //                  discard_cache_[i].size(), draw_cache_[i].size(),
    //                  discard_cache2_[i].size(), draw_cache2_[i].size());
    // spdlog::info("点数: {}", score_cache_.size());

    return {true, candidates};
}

/**
 * @brief 
 * 
 * @param[in] hand 手牌
 * @param[in] syanten_type 向聴数の種類
 * @param[in] counts 各牌の残り枚数
 * @return (有効牌の合計枚数, 有効牌の一覧)
 */
std::tuple<int, std::vector<std::tuple<int, int>>>
ExpectedValueCalculator::get_required_tiles(const Hand &hand, int syanten_type,
                                            const std::vector<int> &counts)
{
    // 有効牌の一覧を取得する。
    std::vector<int> tiles = RequiredTileSelector::select(hand, syanten_type);

    std::vector<std::tuple<int, int>> required_tiles;
    int sum_required_tiles = 0;
    for (auto tile : tiles) {
        required_tiles.emplace_back(tile, counts[tile]);
        sum_required_tiles += counts[tile];
    }

    return {sum_required_tiles, required_tiles};
}

std::tuple<std::vector<double>, std::vector<double>, std::vector<double>>
ExpectedValueCalculator::draw(int n_extra_tumo, int syanten, Hand &hand,
                              std::vector<int> &counts)
{
    auto &table = draw_cache2_[syanten];

    CacheKey key(hand, counts, n_extra_tumo);
    if (auto itr = table.find(key); itr != table.end())
        return itr->second; // キャッシュが存在する場合

    std::vector<double> tenpai_probs(17, 0);
    std::vector<double> win_probs(17, 0);
    std::vector<double> exp_values(17, 0);

    const DrawTilesCache &cache = get_draw_tiles(hand, syanten);

    int sum_required_tiles = 0;
    for (const auto &tile : cache.hands1)
        sum_required_tiles += counts[tile];

    for (const auto &tile : cache.hands1) {
        int n_required_tiles = counts[tile]; // 有効牌枚数
        if (n_required_tiles == 0)
            continue; // 残り枚数が0枚の場合

        // 手牌に加える
        add_tile(hand, tile);
        counts[tile]--;

        std::vector<double> next_tenpai_probs, next_win_probs, next_exp_values;
        std::vector<int> scores;

        if (syanten == 0) {
            const ScoreCache &cache = get_score(hand, tile);
            scores                  = cache.scores;
        }
        else {
            std::tie(next_tenpai_probs, next_win_probs, next_exp_values) =
                discard(n_extra_tumo, syanten - 1, hand, counts);
        }

        const std::vector<double> &tumo_probs = tumo_probs_table_[n_required_tiles];
        const std::vector<double> &not_tumo_probs =
            not_tumo_probs_table_[sum_required_tiles];

        for (int i = 0; i < 17; ++i) {
            for (int j = i; j < 17; ++j) {
                double prob = tumo_probs[j] * not_tumo_probs[j] / not_tumo_probs[i];

                if (syanten == 1)
                    tenpai_probs[i] += prob;
                else if (j < 16 && syanten > 1)
                    tenpai_probs[i] += prob * next_tenpai_probs[j + 1];

                if (syanten == 0) {
                    win_probs[i] += prob;

                    size_t score_idx = (j == i) + (j == 16); // 一発、海底撈月を考慮
                    if (score_idx >= scores.size())
                        score_idx = scores.size() - 1;

                    exp_values[i] += prob * scores[score_idx];
                }
                else if (j < 16 && syanten > 0) {
                    win_probs[i] += prob * next_win_probs[j + 1];
                    exp_values[i] += prob * next_exp_values[j + 1];
                }
            }
        }

        if (syanten == 0)
            std::fill(tenpai_probs.begin(), tenpai_probs.end(), 1);

        // 手牌から除く
        counts[tile]++;
        remove_tile(hand, tile);
    }

    auto value    = std::make_tuple(tenpai_probs, win_probs, exp_values);
    auto [itr, _] = table.insert_or_assign(key, value);

    return itr->second;
}

std::tuple<std::vector<double>, std::vector<double>, std::vector<double>>
ExpectedValueCalculator::discard(int n_extra_tumo, int syanten, Hand &hand,
                                 std::vector<int> &counts)
{
    auto &table = discard_cache2_[syanten];

    CacheKey key(hand, counts, n_extra_tumo);
    if (auto itr = table.find(key); itr != table.end())
        return itr->second; // キャッシュが存在する場合

    // 打牌候補を取得する。
    const std::vector<int> &flags = get_discard_tiles(hand, syanten);

    // 期待値が最大となる打牌を選択する。
    std::vector<double> max_tenpai_probs, max_win_probs, max_exp_values;
    for (int tile = 0; tile < 34; ++tile) {
        if (flags[tile] == 1) {
            // 向聴数が変化しない打牌
            remove_tile(hand, tile);
            auto [tenpai_probs, win_probs, exp_values] =
                draw(n_extra_tumo, syanten, hand, counts);
            add_tile(hand, tile);

            if (max_exp_values.empty() || exp_values.front() > max_exp_values.front()) {
                max_tenpai_probs = tenpai_probs;
                max_win_probs    = win_probs;
                max_exp_values   = exp_values;
            }
        }
        else if (flags[tile] == 2 && n_extra_tumo > 0) {
            // 向聴戻しになる打牌
            remove_tile(hand, tile);
            auto [tenpai_probs, win_probs, exp_values] =
                draw(n_extra_tumo - 1, syanten + 1, hand, counts);
            add_tile(hand, tile);

            if (max_exp_values.empty() || exp_values.front() > max_exp_values.front()) {
                max_tenpai_probs = tenpai_probs;
                max_win_probs    = win_probs;
                max_exp_values   = exp_values;
            }
        }
    }

    auto value    = std::make_tuple(max_tenpai_probs, max_win_probs, max_exp_values);
    auto [itr, _] = table.insert_or_assign(key, value);

    return itr->second;
}

std::vector<Candidate> ExpectedValueCalculator::analyze(int n_extra_tumo, int syanten,
                                                        const Hand &_hand)
{
    std::vector<Candidate> candidates;
    Hand hand = _hand;

    // 各牌の残り枚数を数える。
    std::vector<int> counts = count_left_tiles(hand, score_.dora_tiles());

    const std::vector<int> &flags = get_discard_tiles(hand, syanten);

    for (int tile = 0; tile < 34; ++tile) {
        if (flags[tile] == 1) {
            remove_tile(hand, tile);

            auto [sum_required_tiles, required_tiles] =
                get_required_tiles(hand, syanten_type_, counts);

            auto [tenpai_probs, win_probs, exp_values] =
                draw(n_extra_tumo, syanten, hand, counts);

            add_tile(hand, tile);

            candidates.emplace_back(tile, sum_required_tiles, required_tiles,
                                    tenpai_probs, win_probs, exp_values, false);
        }
        else if (flags[tile] == 2 && n_extra_tumo > 0) {
            remove_tile(hand, tile);

            auto [sum_required_tiles, required_tiles] =
                get_required_tiles(hand, syanten_type_, counts);

            auto [tenpai_probs, win_probs, exp_values] =
                draw(n_extra_tumo - 1, syanten + 1, hand, counts);

            add_tile(hand, tile);

            candidates.emplace_back(tile, sum_required_tiles, required_tiles,
                                    tenpai_probs, win_probs, exp_values, true);
        }
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
const std::vector<int> &ExpectedValueCalculator::get_discard_tiles(Hand &hand,
                                                                   int syanten)
{
    auto &cache = discard_cache_[syanten];

    if (auto itr = cache.find(hand); itr != cache.end())
        return itr->second; // キャッシュが存在する場合

    std::vector<int> flags(34, 0);
    for (int tile = 0; tile < 34; ++tile) {
        if (hand.contains(tile)) {
            remove_tile(hand, tile);
            auto [_, syanten_after] = SyantenCalculator::calc(hand, syanten_type_);
            add_tile(hand, tile);

            flags[tile] = syanten == syanten_after ? 1 : 2;
        }
    }

    auto [itr, _] = cache.insert_or_assign(hand, flags);

    return itr->second;
}

/**
 * @brief 自摸牌一覧を取得する。
 * 
 * @param[in] hand 手牌
 * @param[in] syanten 手牌の向聴数
 * @return 自摸牌一覧
 */
const DrawTilesCache &ExpectedValueCalculator::get_draw_tiles(Hand &hand, int syanten)
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
const ScoreCache &ExpectedValueCalculator::get_score(const Hand &hand, int win_tile)
{
    ScoreKey key(hand, win_tile);
    if (auto itr = score_cache_.find(key); itr != score_cache_.end())
        return itr->second; // キャッシュが存在する場合

    Result result = score_.calc(hand, win_tile, HandFlag::Reach | HandFlag::Tumo);
    std::vector<int> scores = score_.get_scores_for_exp(result);

    ScoreCache cache(scores);

    auto [itr, _] = score_cache_.insert_or_assign(key, cache);

    return itr->second;
}

} // namespace mahjong
