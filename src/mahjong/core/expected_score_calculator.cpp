#include "expected_score_calculator.hpp"

#include <algorithm> // max, fill
#include <cassert>

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

SeparatedCount to_separated_count(const MergedCount &counts)
{
    SeparatedCount ret{0};
    for (int i = 0; i < 34; ++i) {
        ret[i] = counts[i];
    }

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

    return ret;
}

void normalize_red_fives(PlayerState &player)
{
    player.hand[Tile::RedManzu5] = 0;
    player.hand[Tile::RedPinzu5] = 0;
    player.hand[Tile::RedSouzu5] = 0;

    for (auto &meld : player.melds) {
        for (auto &tile : meld.tiles) {
            tile = Tile::to_normal(tile);
        }
    }
}

void normalize_red_fives(TableConfig &table_config, TableState &table_state)
{
    for (auto &tile : table_state.dora_indicators) {
        tile = Tile::to_normal(tile);
    }
    for (auto &tile : table_state.uradora_indicators) {
        tile = Tile::to_normal(tile);
    }
    table_config.rule_flags &= ~RuleFlag::RedDora;
}

void normalize_red_fives(MergedCount &wall)
{
    wall[Tile::RedManzu5] = 0;
    wall[Tile::RedPinzu5] = 0;
    wall[Tile::RedSouzu5] = 0;
}

void draw(PlayerState &player, SeparatedCount &hand_counts, SeparatedCount &wall_counts,
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

void discard(PlayerState &player, SeparatedCount &hand_counts,
             SeparatedCount &wall_counts, const int tile)
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
                                                 const int num_indicators,
                                                 const int game_mode)
{
    std::array<std::array<double, 13>, 6> dp{};
    dp[0][0] = 1.0;

    for (int tile = 0; tile < 34; ++tile) {
        const int count = wall[Tile::to_indicator(tile, game_mode)];
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
                          const TableConfig &table_config,
                          const RoundState &round_state,
                          const TableState &table_state, const PlayerState &player,
                          const SeparatedCount &hand_counts,
                          const SeparatedCount &wall_counts, const ScoreResult &result,
                          const int win_flag)
{
    const int num_indicators = table_state.dora_indicators.size();

    // 裏ドラ表示牌の抽選では、赤5と通常5を同じ牌として扱う。
    const MergedCount wall = to_merged_count(wall_counts);
    MergedCount hand_and_melds = to_merged_count(hand_counts);
    for (const auto &meld : player.melds) {
        for (auto tile : meld.tiles) {
            ++hand_and_melds[Tile::to_normal(tile)];
        }
    }

    // 裏ドラ枚数ごとの確率と、各枚数での点数を掛け合わせる。
    const auto uradora_probabilities =
        calc_uradora_distribution(wall, hand_and_melds, num_indicators,
                                  table_config.game_mode);
    const std::vector<int> up_scores = ScoreCalculator::get_up_scores(
        table_config, round_state, table_state, player, result, win_flag, 12);

    double score = 0;
    for (int i = 0; i <= 12; ++i) {
        score += up_scores[i] * uradora_probabilities[i];
    }

    return score;
}

double calc_score(const ExpectedScoreCalculator::Config &config,
                  const TableConfig &table_config, const RoundState &round_state,
                  const TableState &table_state,
                  PlayerState &player, SeparatedCount &hand_counts,
                  SeparatedCount &wall_counts, const int shanten_type,
                  const int win_tile, const bool riichi)
{
    // 期待値計算では和了を自摸和了として評価する。
    int win_flag = riichi ? (WinFlag::Tsumo | WinFlag::Riichi) : WinFlag::Tsumo;

    ScoreResult result = ScoreCalculator::calc_fast(
        table_config, round_state, table_state, player, win_tile, win_flag,
        shanten_type);

    // 役なしの場合は0点とする。
    if (!result.success) {
        return 0.0;
    }

    // 裏ドラ期待値を計算しない場合は、通常の和了点を返す。
    if (!config.enable_uradora || !(win_flag & WinFlag::Riichi) ||
        table_state.dora_indicators.empty()) {
        return result.payments[0];
    }

    // 役満以上は裏ドラで点数が変わらない。
    if (result.score_limit >= ScoreLimit::CountedYakuman) {
        return result.payments[0];
    }

    return calc_uradora_score(config, table_config, round_state, table_state, player,
                              hand_counts, wall_counts, result, win_flag);
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

int64_t add_red5_flags(int64_t tiles)
{
    tiles |= (tiles & (1LL << Tile::Manzu5)) ? (1LL << Tile::RedManzu5) : 0;
    tiles |= (tiles & (1LL << Tile::Pinzu5)) ? (1LL << Tile::RedPinzu5) : 0;
    tiles |= (tiles & (1LL << Tile::Souzu5)) ? (1LL << Tile::RedSouzu5) : 0;
    return tiles;
}

std::tuple<int, std::vector<std::tuple<int, int>>>
get_necessary_tiles(const ExpectedScoreCalculator::Config &config,
                    const PlayerState &player, const MergedCount &wall,
                    const int game_mode)
{
    const auto [shanten_type, shanten, tiles] = NecessaryTileCalculator::select(
        player.hand, player.num_melds(), config.shanten_type, game_mode);

    std::vector<std::tuple<int, int>> necessary_tiles;
    necessary_tiles.reserve(tiles.size());
    for (const auto tile : tiles) {
        necessary_tiles.emplace_back(tile, wall[tile]);
    }

    return {shanten, necessary_tiles};
}

} // namespace

MergedCount create_wall(const TableConfig &table_config, const TableState &table_state,
                        const PlayerState &player, const bool enable_reddora)
{
    MergedCount wall{0}, melds{0}, indicators{0};
    const bool is_sanma = table_config.game_mode == GameMode::Sanma;

    for (auto tile : table_state.dora_indicators) {
        ++indicators[Tile::to_normal(tile)];
        if (Tile::is_red(tile)) {
            ++indicators[tile];
        }
    }

    for (const auto &meld : player.melds) {
        for (auto tile : meld.tiles) {
            ++melds[Tile::to_normal(tile)];
            if (Tile::is_red(tile)) {
                ++melds[tile];
            }
        }
    }

    for (int i = 0; i < 34; ++i) {
        if (is_sanma && Tile::is_sanma_disabled(i)) {
            continue;
        }
        wall[i] = 4 - (player.hand[i] + melds[i] + indicators[i]);
    }
    if (enable_reddora) {
        for (int i = 34; i < 37; ++i) {
            if (is_sanma && Tile::is_sanma_disabled(i)) {
                continue;
            }
            wall[i] = 1 - (player.hand[i] + melds[i] + indicators[i]);
        }
    }

    // Extracted north tiles (nuki dora) are removed from the wall.
    wall[Tile::North] -= player.nuki_count;

    return wall;
}

class ExpectedScoreCalculator::GraphBuilder
{
  public:
    GraphBuilder(const Config &config, const TableConfig &table_config,
                 const RoundState &round_state, const TableState &table_state,
                 PlayerState &player,
                 SeparatedCount &hand_counts, SeparatedCount &wall_counts,
                 const SeparatedCount &hand_org, const int shanten_org)
        : config_(config)
        , table_config_(table_config)
        , round_state_(round_state)
        , table_state_(table_state)
        , player_(player)
        , hand_counts_(hand_counts)
        , wall_counts_(wall_counts)
        , hand_org_(hand_org)
        , shanten_org_(shanten_org)
    {
    }

    Vertex draw_node(bool riichi);
    Vertex discard_node(bool riichi);

    Graph &graph()
    {
        return graph_;
    }
    const Cache &draw_cache() const
    {
        return cache1_;
    }
    const Cache &discard_cache() const
    {
        return cache2_;
    }
    const std::vector<Vertex> &draw_vertices() const
    {
        return draw_vertices_;
    }
    const std::vector<Vertex> &discard_vertices() const
    {
        return discard_vertices_;
    }

  private:
    const Config &config_;
    const TableConfig &table_config_;
    const RoundState &round_state_;
    const TableState &table_state_;
    PlayerState &player_;
    SeparatedCount &hand_counts_;
    SeparatedCount &wall_counts_;
    const SeparatedCount &hand_org_;
    const int shanten_org_;
    Graph graph_;
    Cache cache1_;
    Cache cache2_;
    std::vector<Vertex> draw_vertices_;
    std::vector<Vertex> discard_vertices_;
};

ExpectedScoreCalculator::Vertex
ExpectedScoreCalculator::GraphBuilder::draw_node(const bool riichi)
{
    const CacheKey key(hand_counts_, riichi);
    if (const auto itr = cache1_.find(key); itr != cache1_.end()) {
        return itr->second;
    }

    auto [type, shanten, wait] = NecessaryTileCalculator::calc(
        player_.hand, player_.num_melds(), config_.shanten_type,
        table_config_.game_mode);

    const bool can_extend_search =
        distance(hand_counts_, hand_org_) + shanten < shanten_org_ + config_.extra;
    const bool allow_tegawari = config_.enable_tegawari && !riichi && can_extend_search;
    wait = add_red5_flags(wait);

    const Vertex vertex = graph_.add_vertex();
    graph_[vertex].is_tenpai = shanten == 0;
    cache1_[key] = vertex;
    draw_vertices_.push_back(vertex);

    for (int i = 0; i < 37; ++i) {
        const bool is_wait = wait & (1LL << i);

        if (wall_counts_[i] && (allow_tegawari || is_wait)) {
            const int weight = wall_counts_[i];

            draw(player_, hand_counts_, wall_counts_, i);

            const Vertex target = discard_node(riichi);

            if (!graph_.has_edge(vertex, target)) {
                // 自摸前の時点で聴牌の場合、有効牌自摸後は和了形のため、点数計算を行う
                double score = 0.0;
                if (shanten == 0 && is_wait) {
                    score = calc_score(config_, table_config_, round_state_,
                                       table_state_, player_, hand_counts_, wall_counts_,
                                       type, i, riichi);
                }
                graph_.add_edge(vertex, target, weight, score);
            }

            discard(player_, hand_counts_, wall_counts_, i);
        }
    }

    return vertex;
}

ExpectedScoreCalculator::Vertex
ExpectedScoreCalculator::GraphBuilder::discard_node(const bool riichi)
{
    const CacheKey key(hand_counts_, riichi);
    if (const auto itr = cache2_.find(key); itr != cache2_.end()) {
        return itr->second;
    }

    auto [type, shanten, disc] = UnnecessaryTileCalculator::calc(
        player_.hand, player_.num_melds(), config_.shanten_type,
        table_config_.game_mode);

    const bool can_extend_search =
        distance(hand_counts_, hand_org_) + shanten < shanten_org_ + config_.extra;
    const bool allow_shanten_down =
        config_.enable_shanten_down && !riichi && can_extend_search;
    disc = add_red5_flags(disc);

    const Vertex vertex = graph_.add_vertex();
    graph_[vertex].is_tenpai = shanten == 0;
    cache2_[key] = vertex;
    discard_vertices_.push_back(vertex);

    for (int i = 0; i < 37; ++i) {
        const bool is_disc = disc & (1LL << i);

        if (hand_counts_[i] && (allow_shanten_down || is_disc)) {
            const bool call_riichi =
                player_.is_closed() && shanten == 0 && is_disc ? true : riichi;

            discard(player_, hand_counts_, wall_counts_, i);

            const int weight = wall_counts_[i];
            const Vertex source = draw_node(call_riichi);

            draw(player_, hand_counts_, wall_counts_, i);

            if (!graph_.has_edge(source, vertex)) {
                // 打牌前の時点で向聴数が-1の場合、和了形のため、点数計算を行う
                double score = 0.0;
                if (shanten == -1) {
                    score = calc_score(config_, table_config_, round_state_,
                                       table_state_, player_, hand_counts_, wall_counts_,
                                       type, i, riichi);
                }
                graph_.add_edge(source, vertex, weight, score);
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
ExpectedScoreCalculator::EdgeCsr
ExpectedScoreCalculator::build_edge_csr(const Graph &graph)
{
    const std::size_t vertex_count = graph.num_vertices();
    const std::size_t edge_count = graph.edges.size();
    assert(vertex_count <= std::numeric_limits<std::uint32_t>::max());
    assert(edge_count <= std::numeric_limits<std::uint32_t>::max());

    EdgeCsr edge_csr;
    edge_csr.draw_edge_offsets.assign(vertex_count + 1, 0);
    edge_csr.selection_edge_offsets.assign(vertex_count + 1, 0);

    for (std::size_t vertex_index = 0; vertex_index < vertex_count; ++vertex_index) {
        for (std::uint32_t edge = graph.first_out_edges[vertex_index];
             edge != Graph::NoEdge; edge = graph.edges[edge].next_out) {
            ++edge_csr.draw_edge_offsets[vertex_index + 1];
        }
        for (std::uint32_t edge = graph.first_in_edges[vertex_index];
             edge != Graph::NoEdge; edge = graph.edges[edge].next_in) {
            ++edge_csr.selection_edge_offsets[vertex_index + 1];
        }
    }

    for (std::size_t vertex_index = 0; vertex_index < vertex_count; ++vertex_index) {
        edge_csr.draw_edge_offsets[vertex_index + 1] +=
            edge_csr.draw_edge_offsets[vertex_index];
        edge_csr.selection_edge_offsets[vertex_index + 1] +=
            edge_csr.selection_edge_offsets[vertex_index];
    }

    edge_csr.draw_edges.resize(edge_csr.draw_edge_offsets.back());
    edge_csr.selection_edges.resize(edge_csr.selection_edge_offsets.back());

    std::vector<std::uint32_t> draw_positions = edge_csr.draw_edge_offsets;
    std::vector<std::uint32_t> selection_positions = edge_csr.selection_edge_offsets;

    for (std::size_t vertex_index = 0; vertex_index < vertex_count; ++vertex_index) {
        for (std::uint32_t edge_index = graph.first_out_edges[vertex_index];
             edge_index != Graph::NoEdge;
             edge_index = graph.edges[edge_index].next_out) {
            const EdgeData &edge = graph.edges[edge_index];
            edge_csr.draw_edges[draw_positions[vertex_index]++] =
                DrawEdge{edge.target, edge.weight, edge.score};
        }
        for (std::uint32_t edge_index = graph.first_in_edges[vertex_index];
             edge_index != Graph::NoEdge;
             edge_index = graph.edges[edge_index].next_in) {
            const EdgeData &edge = graph.edges[edge_index];
            edge_csr.selection_edges[selection_positions[vertex_index]++] =
                SelectionEdge{edge.source};
        }
    }

    return edge_csr;
}

void ExpectedScoreCalculator::calc_stats(const Config &config, Graph &graph,
                                         const std::vector<Vertex> &draw_vertices,
                                         const std::vector<Vertex> &discard_vertices,
                                         const EdgeCsr &edge_csr)
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
#ifdef _OPENMP
#pragma omp parallel for schedule(static)
#endif
        for (std::int64_t i = 0; i < static_cast<std::int64_t>(draw_vertices.size());
             ++i) {
            const Vertex vertex = draw_vertices[i];
            VertexData &s1 = graph[vertex];
            if (t == config.t_max) {
                if (s1.is_tenpai) {
                    s1.tenpai_prob[t] = 1.0;
                }
                continue;
            }

            const std::size_t vertex_index = static_cast<std::size_t>(vertex);
            for (std::uint32_t edge_index = edge_csr.draw_edge_offsets[vertex_index];
                 edge_index < edge_csr.draw_edge_offsets[vertex_index + 1];
                 ++edge_index) {
                const DrawEdge &edge = edge_csr.draw_edges[edge_index];
                const VertexData &s2 = graph[edge.target];

                double tenpai_prob = s2.tenpai_prob[t + 1];
                double win_prob = s2.win_prob[t + 1];
                double exp_score = s2.exp_score[t + 1];
                if (edge.score > 0.0 &&
                    win_objective_value(edge.score) >= objective_value(s2, t + 1)) {
                    tenpai_prob = 1.0;
                    win_prob = 1.0;
                    exp_score = edge.score;
                }

                s1.tenpai_prob[t] +=
                    edge.weight * (tenpai_prob - s1.tenpai_prob[t + 1]);
                s1.win_prob[t] += edge.weight * (win_prob - s1.win_prob[t + 1]);
                s1.exp_score[t] += edge.weight * (exp_score - s1.exp_score[t + 1]);
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
#ifdef _OPENMP
#pragma omp parallel for schedule(static)
#endif
        for (std::int64_t i = 0; i < static_cast<std::int64_t>(discard_vertices.size());
             ++i) {
            const Vertex vertex = discard_vertices[i];
            VertexData &s1 = graph[vertex];
            const VertexData *best = nullptr;

            const std::size_t vertex_index = static_cast<std::size_t>(vertex);
            for (std::uint32_t edge_index =
                     edge_csr.selection_edge_offsets[vertex_index];
                 edge_index < edge_csr.selection_edge_offsets[vertex_index + 1];
                 ++edge_index) {
                const SelectionEdge &edge = edge_csr.selection_edges[edge_index];
                const VertexData &s2 = graph[edge.source];
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
ExpectedScoreCalculator::calc(const Config &config, const TableConfig &table_config,
                              const RoundState &round_state,
                              const TableState &table_state, const PlayerState &player)
{
    const MergedCount wall =
        create_wall(table_config, table_state, player, config.enable_reddora);

    return calc(config, table_config, round_state, table_state, player, wall);
}

void ExpectedScoreCalculator::calc_draw_hand(
    const Config &config, const PlayerState &player, const TableConfig &table_config,
    const RoundState &round_state, const TableState &table_state,
    const MergedCount &wall, const SeparatedCount &hand_counts,
    GraphBuilder &graph_builder, std::vector<Stat> &stats)
{
    // 13枚の場合は自摸を起点に手牌遷移のグラフを作成する。
    const Vertex vertex = graph_builder.draw_node(false);

    // 確率、期待値を計算する。
    const EdgeCsr edge_csr = build_edge_csr(graph_builder.graph());
    calc_stats(config, graph_builder.graph(), graph_builder.draw_vertices(),
               graph_builder.discard_vertices(), edge_csr);

    // 結果を取得する。
    const VertexData &state = graph_builder.graph()[vertex];

    // 有効牌の一覧を計算する。
    const auto [shanten2, necessary_tiles] =
        get_necessary_tiles(config, player, wall, table_config.game_mode);

    stats.emplace_back(Stat{Tile::Null, to_vector(state.tenpai_prob, config.t_max),
                            to_vector(state.win_prob, config.t_max),
                            to_vector(state.exp_score, config.t_max), necessary_tiles,
                            shanten2});
}

void ExpectedScoreCalculator::calc_discard_hand(
    const Config &config, PlayerState &player, const TableConfig &table_config,
    const RoundState &round_state, const TableState &table_state,
    const MergedCount &wall, SeparatedCount &hand_counts, SeparatedCount &wall_counts,
    GraphBuilder &graph_builder, std::vector<Stat> &stats)
{
    // 14枚の場合は打牌を起点に手牌遷移のグラフを作成する。
    graph_builder.discard_node(false);

    // 確率、期待値を計算する。
    const EdgeCsr edge_csr = build_edge_csr(graph_builder.graph());
    calc_stats(config, graph_builder.graph(), graph_builder.draw_vertices(),
               graph_builder.discard_vertices(), edge_csr);

    // 結果を取得する。
    auto [discard_type, discard_shanten, discard_tiles] =
        UnnecessaryTileCalculator::calc(player.hand, player.num_melds(),
                                        config.shanten_type, table_config.game_mode);
    discard_tiles = add_red5_flags(discard_tiles);

    for (int i = 0; i < 37; ++i) {
        if (hand_counts[i] > 0) {
            const bool is_disc = discard_tiles & (1LL << i);
            const bool call_riichi =
                player.is_closed() && discard_shanten == 0 && is_disc;

            discard(player, hand_counts, wall_counts, i);
            if (const auto itr =
                    graph_builder.draw_cache().find(CacheKey(hand_counts, call_riichi));
                itr != graph_builder.draw_cache().end()) {
                const VertexData &state = graph_builder.graph()[itr->second];

                const auto [shanten2, necessary_tiles] =
                    get_necessary_tiles(config, player, wall, table_config.game_mode);

                stats.emplace_back(Stat{i, to_vector(state.tenpai_prob, config.t_max),
                                        to_vector(state.win_prob, config.t_max),
                                        to_vector(state.exp_score, config.t_max),
                                        necessary_tiles, shanten2});
            }
            draw(player, hand_counts, wall_counts, i);
        }
    }
}

std::tuple<std::vector<ExpectedScoreCalculator::Stat>, int>
ExpectedScoreCalculator::calc(const Config &_config, const TableConfig &_table_config,
                              const RoundState &_round_state,
                              const TableState &_table_state,
                              const PlayerState &_player, const MergedCount &_wall)
{
    Config config = _config;
    TableConfig table_config = _table_config;
    RoundState round_state = _round_state;
    TableState table_state = _table_state;
    PlayerState player = _player;
    MergedCount wall = _wall;
    assert(config.t_min >= 0);
    assert(config.t_max <= MaxTurn);

    if (!config.enable_reddora) {
        normalize_red_fives(table_config, table_state);
        normalize_red_fives(player);
        normalize_red_fives(wall);
    }

    if (config.sum == 0) {
        config.sum = std::accumulate(wall.begin(), wall.begin() + 34, 0);
    }

    SeparatedCount hand_counts = to_separated_count(player.hand);
    SeparatedCount wall_counts = to_separated_count(wall);
    std::vector<Stat> stats;
    const int num_tiles = player.num_tiles() + player.num_melds() * 3;

    if (!config.calc_stats) {
        if (num_tiles == 13) {
            const auto [shanten, necessary_tiles] =
                get_necessary_tiles(config, player, wall, table_config.game_mode);
            stats.emplace_back(Stat{Tile::Null, {}, {}, {}, necessary_tiles, shanten});
        }
        else {
            for (int i = 0; i < 37; ++i) {
                if (hand_counts[i] > 0) {
                    discard(player, hand_counts, wall_counts, i);
                    const auto [shanten, necessary_tiles] =
                        get_necessary_tiles(config, player, wall,
                                            table_config.game_mode);
                    stats.emplace_back(Stat{i, {}, {}, {}, necessary_tiles, shanten});
                    draw(player, hand_counts, wall_counts, i);
                }
            }
        }

        return {stats, 0};
    }

    const SeparatedCount hand_org = hand_counts;
    const int shanten_org = std::get<1>(ShantenCalculator::calc(
        player.hand, player.num_melds(), config.shanten_type, table_config.game_mode));
    GraphBuilder graph_builder(config, table_config, round_state, table_state, player,
                               hand_counts, wall_counts, hand_org, shanten_org);

    if (num_tiles == 13) {
        calc_draw_hand(config, player, table_config, round_state, table_state, wall,
                       hand_counts, graph_builder, stats);
    }
    else {
        calc_discard_hand(config, player, table_config, round_state, table_state, wall,
                          hand_counts, wall_counts, graph_builder, stats);
    }

    const int searched = static_cast<int>(graph_builder.graph().num_vertices());

    return {stats, searched};
}

} // namespace mahjong
