#include "expected_score_calculator.hpp"

#undef NDEBUG
#include <algorithm> // max, fill
#include <cassert>

#include <boost/graph/graph_utility.hpp>

#include "mahjong/core/necessary_tile_calculator.hpp"
#include "mahjong/core/score_calculator.hpp"
#include "mahjong/core/shanten_calculator.hpp"
#include "mahjong/core/unnecessary_tile_calculator.hpp"
#include "mahjong/core/utils.hpp"

namespace mahjong
{

namespace
{

MergedCount to_merged_count(const SeparatedCount &counts)
{
    MergedCount merged = counts;
    merged[Tile::Manzu5] += merged[Tile::RedManzu5];
    merged[Tile::Pinzu5] += merged[Tile::RedPinzu5];
    merged[Tile::Souzu5] += merged[Tile::RedSouzu5];
    return merged;
}

SeparatedCount to_separated_count(const MergedCount &counts, const bool enable_reddora)
{
    SeparatedCount ret{0};
    for (int i = 0; i < 34; ++i) {
        ret[i] = counts[i];
    }

    if (enable_reddora) {
        if (counts[Tile::RedManzu5]) {
            --ret[Tile::Manzu5];
            ++ret[Tile::RedManzu5];
        }
        if (counts[Tile::RedPinzu5]) {
            --ret[Tile::Pinzu5];
            ++ret[Tile::RedPinzu5];
        }
        if (counts[Tile::RedSouzu5]) {
            --ret[Tile::Souzu5];
            ++ret[Tile::RedSouzu5];
        }
    }

    return ret;
}

void draw(Player &player, SeparatedCount &hand_counts, SeparatedCount &wall_counts,
          const int tile)
{
    ++hand_counts[tile];
    --wall_counts[tile];
    player.hand[tile]++;
    if (tile == Tile::RedManzu5) {
        player.hand[Tile::Manzu5]++;
    }
    else if (tile == Tile::RedPinzu5) {
        player.hand[Tile::Pinzu5]++;
    }
    else if (tile == Tile::RedSouzu5) {
        player.hand[Tile::Souzu5]++;
    }
}

void discard(Player &player, SeparatedCount &hand_counts, SeparatedCount &wall_counts,
             const int tile)
{
    --hand_counts[tile];
    ++wall_counts[tile];
    player.hand[tile]--;
    if (tile == Tile::RedManzu5) {
        player.hand[Tile::Manzu5]--;
    }
    else if (tile == Tile::RedPinzu5) {
        player.hand[Tile::Pinzu5]--;
    }
    else if (tile == Tile::RedSouzu5) {
        player.hand[Tile::Souzu5]--;
    }
}

double combination(const int n, const int r)
{
    if (r < 0 || r > n) {
        return 0.0;
    }

    const int k = std::min(r, n - r);
    double value = 1.0;
    for (int i = 1; i <= k; ++i) {
        value *= static_cast<double>(n - k + i);
        value /= static_cast<double>(i);
    }
    return value;
}

std::array<double, 13> calc_uradora_distribution(const MergedCount &wall,
                                                 const MergedCount &hand_and_melds,
                                                 const int num_indicators)
{
    std::array<std::array<double, 13>, 6> dp{};
    dp[0][0] = 1.0;

    for (int tile = 0; tile < 34; ++tile) {
        const int count = wall[ToIndicator[tile]];
        const int gain = hand_and_melds[tile];
        if (count == 0) {
            continue;
        }

        auto next = dp;
        for (int selected = 0; selected < num_indicators; ++selected) {
            for (int uradora = 0; uradora <= 12; ++uradora) {
                if (dp[selected][uradora] == 0.0) {
                    continue;
                }
                const int max_take = std::min(count, num_indicators - selected);
                for (int take = 1; take <= max_take; ++take) {
                    const int next_selected = selected + take;
                    const int next_uradora = std::min(12, uradora + gain * take);
                    next[next_selected][next_uradora] +=
                        dp[selected][uradora] * combination(count, take);
                }
            }
        }
        dp = next;
    }

    const double denominator = combination(
        static_cast<int>(std::accumulate(wall.begin(), wall.begin() + 34, 0)),
        num_indicators);
    assert(denominator > 0.0);

    std::array<double, 13> probabilities{};
    for (int uradora = 0; uradora <= 12; ++uradora) {
        probabilities[uradora] = dp[num_indicators][uradora] / denominator;
    }
    return probabilities;
}

double calc_uradora_score(const ExpectedScoreCalculator::Config &config,
                          const Round &round, const Player &player,
                          const SeparatedCount &hand_counts,
                          const SeparatedCount &wall_counts, const Result &result,
                          const int win_flag)
{
    const int num_indicators = round.dora_indicators.size();

    // 裏ドラ表示牌の抽選では、赤5と通常5を同じ牌として扱う。
    const MergedCount wall = to_merged_count(wall_counts);
    MergedCount hand_and_melds = to_merged_count(hand_counts);
    for (const auto &meld : player.melds) {
        for (auto tile : meld.tiles) {
            ++hand_and_melds[to_no_reddora(tile)];
        }
    }

    // 裏ドラ枚数ごとの確率と、各枚数での点数を掛け合わせる。
    const auto uradora_probabilities =
        calc_uradora_distribution(wall, hand_and_melds, num_indicators);
    const std::vector<int> up_scores =
        ScoreCalculator::get_up_scores(round, player, result, win_flag, 12);

    double score = 0;
    for (int i = 0; i <= 12; ++i) {
        score += up_scores[i] * uradora_probabilities[i];
    }

    return score;
}

double calc_score(const ExpectedScoreCalculator::Config &config, const Round &round,
                  Player &player, SeparatedCount &hand_counts,
                  SeparatedCount &wall_counts, const int shanten_type,
                  const int win_tile, const bool riichi)
{
    // 期待値計算では和了を自摸和了として評価する。
    int win_flag = riichi ? (WinFlag::Tsumo | WinFlag::Riichi) : WinFlag::Tsumo;

    Result result =
        ScoreCalculator::calc_fast(round, player, win_tile, win_flag, shanten_type);

    // 役なしの場合は0点とする。
    if (!result.success) {
        return 0.0;
    }

    // 裏ドラ期待値を計算しない場合は、通常の和了点を返す。
    if (!config.enable_uradora || !(win_flag & WinFlag::Riichi) ||
        round.dora_indicators.empty()) {
        return result.score[0];
    }

    // 役満以上は裏ドラで点数が変わらない。
    if (result.score_title >= ScoreTitle::CountedYakuman) {
        return result.score[0];
    }

    return calc_uradora_score(config, round, player, hand_counts, wall_counts, result,
                              win_flag);
}

template <std::size_t N>
std::vector<double> to_vector(const std::array<double, N> &values, const int t_max)
{
    assert(t_max >= 0 && t_max < static_cast<int>(N));
    return {values.begin(), values.begin() + t_max + 1};
}

int distance(const SeparatedCount &hand, const SeparatedCount &hand_org)
{
    int dist = 0;
    for (int i = 0; i < hand.size(); ++i) {
        dist += std::max(hand[i] - hand_org[i], 0);
    }

    return dist;
}

} // namespace

MergedCount create_wall(const Round &round, const Player &player,
                        const bool enable_reddora)
{
    MergedCount wall{0}, melds{0}, indicators{0};

    for (auto tile : round.dora_indicators) {
        ++indicators[to_no_reddora(tile)];
        if (is_reddora(tile)) {
            ++indicators[tile];
        }
    }

    for (const auto &meld : player.melds) {
        for (auto tile : meld.tiles) {
            ++melds[to_no_reddora(tile)];
            if (is_reddora(tile)) {
                ++melds[tile];
            }
        }
    }

    for (int i = 0; i < 34; ++i) {
        wall[i] = 4 - (player.hand[i] + melds[i] + indicators[i]);
    }
    if (enable_reddora) {
        for (int i = 34; i < 37; ++i) {
            wall[i] = 1 - (player.hand[i] + melds[i] + indicators[i]);
        }
    }

    return wall;
}

ExpectedScoreCalculator::Vertex ExpectedScoreCalculator::draw_node(
    const Config &config, const Round &round, Player &player, Graph &graph,
    Cache &cache1, Cache &cache2, SeparatedCount &hand_counts,
    SeparatedCount &wall_counts, const SeparatedCount &hand_org, const int shanten_org,
    const bool riichi)
{
    // If vertex exists in the cache, return it.
    CacheKey key(hand_counts, riichi);
    if (const auto itr = cache1.find(key); itr != cache1.end()) {
        return itr->second;
    }

    // Calculate necessary tiles.
    auto [type, shanten, wait] = NecessaryTileCalculator::calc(
        player.hand, player.num_melds(), config.shanten_type);

    // 元の手牌から現在の手牌に変化されるのに必要な交換枚数 + 現在の向聴数 < 元の手牌の向聴数 + 追加交換枚数
    // の場合、立直前に限り手変わりを許可する
    bool allow_tegawari =
        config.enable_tegawari && !riichi &&
        distance(hand_counts, hand_org) + shanten < shanten_org + config.extra;
    wait |= (wait & (1LL << Tile::Manzu5)) ? (1LL << Tile::RedManzu5) : 0;
    wait |= (wait & (1LL << Tile::Pinzu5)) ? (1LL << Tile::RedPinzu5) : 0;
    wait |= (wait & (1LL << Tile::Souzu5)) ? (1LL << Tile::RedSouzu5) : 0;

    // Add vertex to graph.
    const Vertex vertex = boost::add_vertex(graph);
    graph[vertex].is_tenpai = shanten == 0;
    cache1[key] = vertex;

    for (int i = 0; i < 37; ++i) {
        bool is_wait = wait & (1LL << i);

        if (wall_counts[i] && (allow_tegawari || is_wait)) {
            const int weight = wall_counts[i];

            draw(player, hand_counts, wall_counts, i);

            const Vertex target =
                discard_node(config, round, player, graph, cache1, cache2, hand_counts,
                             wall_counts, hand_org, shanten_org, riichi);

            if (!boost::edge(vertex, target, graph).second) {
                // 自摸前の時点で聴牌の場合、有効牌自摸後は和了形のため、点数計算を行う
                const double score =
                    (shanten == 0 && is_wait
                         ? calc_score(config, round, player, hand_counts, wall_counts,
                                      type, i, riichi)
                         : 0.0);
                boost::add_edge(vertex, target, {weight, score}, graph);
            }

            discard(player, hand_counts, wall_counts, i);
        }
    }

    return vertex;
}

ExpectedScoreCalculator::Vertex ExpectedScoreCalculator::discard_node(
    const Config &config, const Round &round, Player &player, Graph &graph,
    Cache &cache1, Cache &cache2, SeparatedCount &hand_counts,
    SeparatedCount &wall_counts, const SeparatedCount &hand_org, const int shanten_org,
    const bool riichi)
{
    // If vertex exists in the cache, return it.
    CacheKey key(hand_counts, riichi);
    if (const auto itr = cache2.find(key); itr != cache2.end()) {
        return itr->second;
    }

    // Calculate unnecessary tiles.
    auto [type, shanten, disc] = UnnecessaryTileCalculator::calc(
        player.hand, player.num_melds(), config.shanten_type);

    // 元の手牌から現在の手牌に変化されるのに必要な交換枚数 + 現在の向聴数 < 元の手牌の向聴数 + 追加交換枚数
    // の場合、立直前に限り向聴戻しを許可する
    bool allow_shanten_down =
        config.enable_shanten_down && !riichi &&
        distance(hand_counts, hand_org) + shanten < shanten_org + config.extra;
    disc |= (disc & (1LL << Tile::Manzu5)) ? (1LL << Tile::RedManzu5) : 0;
    disc |= (disc & (1LL << Tile::Pinzu5)) ? (1LL << Tile::RedPinzu5) : 0;
    disc |= (disc & (1LL << Tile::Souzu5)) ? (1LL << Tile::RedSouzu5) : 0;

    // Add vertex to graph.
    const Vertex vertex = boost::add_vertex(graph);
    graph[vertex].is_tenpai = shanten == 0;
    cache2[key] = vertex;

    for (int i = 0; i < 37; ++i) {
        bool is_disc = disc & (1LL << i);

        if (hand_counts[i] && (allow_shanten_down || is_disc)) {
            const bool call_riichi =
                player.is_closed() && shanten == 0 && is_disc ? true : riichi;

            discard(player, hand_counts, wall_counts, i);

            const int weight = wall_counts[i];
            const Vertex source =
                draw_node(config, round, player, graph, cache1, cache2, hand_counts,
                          wall_counts, hand_org, shanten_org, call_riichi);

            draw(player, hand_counts, wall_counts, i);

            if (!boost::edge(source, vertex, graph).second) {
                // 打牌前の時点で向聴数が-1の場合、和了形のため、点数計算を行う
                const double score =
                    (shanten == -1 ? calc_score(config, round, player, hand_counts,
                                                wall_counts, type, i, riichi)
                                   : 0.0);
                boost::add_edge(source, vertex, {weight, score}, graph);
            }
        }
    }

    return vertex;
}

/**
 * @brief Calculate the probability of tenpai, the probability of winning, and the expected score.
 *        https://github.com/nekobean/mahjong-cpp/wiki/%E8%81%B4%E7%89%8C%E7%A2%BA%E7%8E%87%E3%80%81%E5%92%8C%E4%BA%86%E7%A2%BA%E7%8E%87%E3%80%81%E7%82%B9%E6%95%B0%E6%9C%9F%E5%BE%85%E5%80%A4
 * @param config Configulation
 * @param graph Graph
 * @param cache1 List of draw node
 * @param cache2 List of discard node
 */
void ExpectedScoreCalculator::calc_stats(const Config &config, Graph &graph,
                                         const Cache &cache1, const Cache &cache2)
{
    const auto objective_value = [&](const VertexData &state, const int turn) {
        switch (config.objective) {
        case Objective::TenpaiProbability:
            return state.tenpai_prob[turn];
        case Objective::WinProbability:
            return state.win_prob[turn];
        case Objective::ExpectedScore:
            return state.exp_score[turn];
        }
        return state.exp_score[turn];
    };

    const auto win_objective_value = [&](const double score) {
        switch (config.objective) {
        case Objective::TenpaiProbability:
        case Objective::WinProbability:
            return 1.0;
        case Objective::ExpectedScore:
            return score;
        }
        return score;
    };

    for (int t = config.t_max; t >= config.t_min; --t) {
        // draw node
        for (const auto &[_, vertex] : cache1) {
            VertexData &s1 = graph[vertex];
            if (t == config.t_max) {
                if (s1.is_tenpai) {
                    s1.tenpai_prob[t] = 1.0;
                }
                continue;
            }

            for (const auto &edge :
                 boost::make_iterator_range(boost::out_edges(vertex, graph))) {
                const auto &[weight, score] = graph[edge];
                const VertexData &s2 = graph[boost::target(edge, graph)];

                double tenpai_prob = s2.tenpai_prob[t + 1];
                double win_prob = s2.win_prob[t + 1];
                double exp_score = s2.exp_score[t + 1];
                if (score > 0.0 &&
                    win_objective_value(score) >= objective_value(s2, t + 1)) {
                    tenpai_prob = 1.0;
                    win_prob = 1.0;
                    exp_score = score;
                }

                s1.tenpai_prob[t] += weight * (tenpai_prob - s1.tenpai_prob[t + 1]);
                s1.win_prob[t] += weight * (win_prob - s1.win_prob[t + 1]);
                s1.exp_score[t] += weight * (exp_score - s1.exp_score[t + 1]);
            }

            s1.tenpai_prob[t] =
                s1.tenpai_prob[t + 1] + s1.tenpai_prob[t] / (config.sum - t);
            if (s1.is_tenpai) {
                s1.tenpai_prob[t] = 1.0;
            }
            s1.win_prob[t] = s1.win_prob[t + 1] + s1.win_prob[t] / (config.sum - t);
            s1.exp_score[t] = s1.exp_score[t + 1] + s1.exp_score[t] / (config.sum - t);
        }

        // discard node
        for (const auto &[_, vertex] : cache2) {
            VertexData &s1 = graph[vertex];
            const VertexData *best = nullptr;

            for (const auto &edge :
                 boost::make_iterator_range(boost::in_edges(vertex, graph))) {
                const VertexData &s2 = graph[boost::source(edge, graph)];
                if (!best || objective_value(s2, t) > objective_value(*best, t)) {
                    best = &s2;
                }
            }

            if (best) {
                s1.tenpai_prob[t] = best->tenpai_prob[t];
                s1.win_prob[t] = best->win_prob[t];
                s1.exp_score[t] = best->exp_score[t];
            }
        }
    }
}

std::tuple<std::vector<ExpectedScoreCalculator::Stat>, int>
ExpectedScoreCalculator::calc(const Config &config, const Round &round,
                              const Player &player)
{
    const MergedCount wall = create_wall(round, player, config.enable_reddora);

    return calc(config, round, player, wall);
}

std::tuple<int, std::vector<std::tuple<int, int>>>
ExpectedScoreCalculator::get_necessary_tiles(const Config &config, const Player &player,
                                             const MergedCount &wall)
{
    const auto [shanten_type, shanten, tiles] = NecessaryTileCalculator::select(
        player.hand, player.num_melds(), config.shanten_type);

    std::vector<std::tuple<int, int>> necessary_tiles;
    for (const auto tile : tiles) {
        necessary_tiles.emplace_back(tile, wall[tile]);
    }

    return {shanten, necessary_tiles};
}

std::tuple<std::vector<ExpectedScoreCalculator::Stat>, int>
ExpectedScoreCalculator::calc(const Config &_config, const Round &round,
                              const Player &_player, const MergedCount &wall)
{
    Graph graph;
    Cache cache1, cache2;

    Config config = _config;
    assert(config.t_min >= 0);
    assert(config.t_max <= MaxTurn);
    if (config.sum == 0) {
        config.sum = std::accumulate(wall.begin(), wall.begin() + 34, 0);
    }

    Player player = _player;
    // 手牌と山の各牌の枚数を作成する。
    // 赤ドラありの場合、赤なしの5と赤ありの5は別々に管理する。
    SeparatedCount hand_counts = to_separated_count(player.hand, config.enable_reddora);
    SeparatedCount wall_counts = to_separated_count(wall, config.enable_reddora);

    // Calculate shanten number of specified hand.
    const SeparatedCount hand_org = hand_counts;
    const int shanten_org = std::get<1>(
        ShantenCalculator::calc(player.hand, player.num_melds(), config.shanten_type));
    std::vector<Stat> stats;
    const int num_tiles = player.num_tiles() + player.num_melds() * 3;

    // 初期状態では未立直として扱い、打牌ごとに立直するか判定する。
    const bool riichi = false;

    if (num_tiles == 13) {
        // 13枚の場合
        if (config.calc_stats) {
            // 期待値、確率計算を行う場合

            // 13枚の場合は自摸を起点に手牌遷移のグラフを作成する。
            draw_node(config, round, player, graph, cache1, cache2, hand_counts,
                      wall_counts, hand_org, shanten_org, riichi);

            // 確率、期待値を計算する。
            calc_stats(config, graph, cache1, cache2);

            // 結果を取得する。
            if (const auto itr = cache1.find(CacheKey(hand_counts, riichi));
                itr != cache1.end()) {
                const VertexData &state = graph[itr->second];

                // 有効牌の一覧を計算する。
                const auto [shanten2, necessary_tiles] =
                    get_necessary_tiles(config, player, wall);

                stats.emplace_back(Stat{Tile::Null,
                                        to_vector(state.tenpai_prob, config.t_max),
                                        to_vector(state.win_prob, config.t_max),
                                        to_vector(state.exp_score, config.t_max),
                                        necessary_tiles, shanten2});
            }
        }
        else {
            const auto [shanten2, necessary_tiles] =
                get_necessary_tiles(config, player, wall);
            stats.emplace_back(Stat{Tile::Null, {}, {}, {}, necessary_tiles, shanten2});
        }
    }
    else {
        if (config.calc_stats) {
            // 期待値、確率計算を行う場合

            // 14枚の場合は打牌を起点に手牌遷移のグラフを作成する。
            discard_node(config, round, player, graph, cache1, cache2, hand_counts,
                         wall_counts, hand_org, shanten_org, riichi);

            // 確率、期待値を計算する。
            calc_stats(config, graph, cache1, cache2);

            // 結果を取得する。
            auto [discard_type, discard_shanten, discard_tiles] =
                UnnecessaryTileCalculator::calc(player.hand, player.num_melds(),
                                                config.shanten_type);
            discard_tiles |=
                (discard_tiles & (1LL << Tile::Manzu5)) ? (1LL << Tile::RedManzu5) : 0;
            discard_tiles |=
                (discard_tiles & (1LL << Tile::Pinzu5)) ? (1LL << Tile::RedPinzu5) : 0;
            discard_tiles |=
                (discard_tiles & (1LL << Tile::Souzu5)) ? (1LL << Tile::RedSouzu5) : 0;

            for (int i = 0; i < 37; ++i) {
                if (hand_counts[i] > 0) {
                    const bool is_disc = discard_tiles & (1LL << i);
                    const bool call_riichi =
                        player.is_closed() && discard_shanten == 0 && is_disc ? true
                                                                              : riichi;

                    discard(player, hand_counts, wall_counts, i);
                    if (const auto itr =
                            cache1.find(CacheKey(hand_counts, call_riichi));
                        itr != cache1.end()) {
                        const VertexData &state = graph[itr->second];

                        // 有効牌の一覧を計算する。
                        const auto [shanten2, necessary_tiles] =
                            get_necessary_tiles(config, player, wall);

                        stats.emplace_back(
                            Stat{i, to_vector(state.tenpai_prob, config.t_max),
                                 to_vector(state.win_prob, config.t_max),
                                 to_vector(state.exp_score, config.t_max),
                                 necessary_tiles, shanten2});
                    }
                    draw(player, hand_counts, wall_counts, i);
                }
            }
        }
        else {
            for (int i = 0; i < 37; ++i) {
                if (hand_counts[i] > 0) {
                    discard(player, hand_counts, wall_counts, i);
                    const auto [shanten2, necessary_tiles] =
                        get_necessary_tiles(config, player, wall);
                    stats.emplace_back(Stat{i, {}, {}, {}, necessary_tiles, shanten2});
                    draw(player, hand_counts, wall_counts, i);
                }
            }
        }
    }

    const int searched = static_cast<int>(boost::num_vertices(graph));

    return {stats, searched};
}

} // namespace mahjong
