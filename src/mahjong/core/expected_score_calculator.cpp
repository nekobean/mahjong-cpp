#include "expected_score_calculator.hpp"

#undef NDEBUG
#include <cassert>
#include <numeric>

#include <boost/graph/graph_utility.hpp>

#include "mahjong/core/necessary_tile_calculator.hpp"
#include "mahjong/core/score_calculator.hpp"
#include "mahjong/core/shanten_calculator.hpp"
#include "mahjong/core/string.hpp"
#include "mahjong/core/unnecessary_tile_calculator.hpp"

namespace mahjong
{

int64_t calc_disc2(const Hand &hand)
{
    int64_t ret = 0LL;

    for (int i = 0; i < 34; ++i) {
        if (hand[i] > 0) {
            ret |= 1LL << i;
        }
    }

    return ret;
}

int64_t calc_wait2(const Hand &hand)
{
    int64_t ret = 0LL;

    for (int i = 0; i < 34; ++i) {
        if (hand[i] < 4) {
            ret |= 1LL << i;
        }
    }

    return ret;
}

std::vector<int> encode(const std::vector<int> &hand, const std::vector<int> &reds)
{
    std::vector<int> ret(2 * 34);

    for (int i = 0; i < 34; ++i) {
        ret[i] = hand[i] - reds[i];
        ret[34 + i] = reds[i];
    }

    return ret;
}

Hand create_wall(const Round &round, const Player &player)
{
    Hand wall{0};
    std::fill(wall.begin(), wall.begin() + 34, 4);
    wall[Tile::RedManzu5] = 1;
    wall[Tile::RedPinzu5] = 1;
    wall[Tile::RedSouzu5] = 1;

    for (auto tile : round.dora_indicators) {
        if (tile == Tile::RedManzu5) {
            wall[Tile::Manzu5]--;
            wall[Tile::RedManzu5]--;
        }
        else if (tile == Tile::RedPinzu5) {
            wall[Tile::Pinzu5]--;
            wall[Tile::RedPinzu5]--;
        }
        else if (tile == Tile::RedSouzu5) {
            wall[Tile::Souzu5]--;
            wall[Tile::RedSouzu5]--;
        }
        else {
            wall[tile]--;
        }
    }

    for (int i = 0; i < 34; ++i) {
        if (i == Tile::Manzu5) {
            wall[i] -= player.hand[Tile::Manzu5];
            wall[Tile::RedManzu5] -= player.hand[Tile::RedManzu5];
        }
        else if (i == Tile::Pinzu5) {
            wall[i] -= player.hand[Tile::Pinzu5];
            wall[Tile::RedPinzu5] -= player.hand[Tile::RedPinzu5];
        }
        else if (i == Tile::Souzu5) {
            wall[i] -= player.hand[Tile::Souzu5];
            wall[Tile::RedSouzu5] -= player.hand[Tile::RedSouzu5];
        }
        else {
            wall[i] -= player.hand[i];
        }
    }

    for (const auto &meld : player.melds) {
        for (auto tile : meld.tiles) {
            if (tile == Tile::RedManzu5) {
                wall[tile] -= 1;
                wall[Tile::Manzu5] -= 1;
            }
            else if (tile == Tile::RedPinzu5) {
                wall[tile] -= 1;
                wall[Tile::Pinzu5] -= 1;
            }
            else if (tile == Tile::RedSouzu5) {
                wall[tile] -= 1;
                wall[Tile::Souzu5] -= 1;
            }
            else {
                wall[tile] -= 1;
            }
        }
    }

    return wall;
}

std::vector<int> encode(const Hand &counts)
{
    std::vector<int> ret(2 * 34, 0);

    for (int i = 0; i < 34; ++i) {
        if (i == Tile::Manzu5) {
            ret[i] = counts[Tile::Manzu5] - counts[Tile::RedManzu5];
            ret[34 + i] = counts[Tile::RedManzu5];
        }
        else if (i == Tile::Pinzu5) {
            ret[i] = counts[Tile::Pinzu5] - counts[Tile::RedPinzu5];
            ret[34 + i] = counts[Tile::RedPinzu5];
        }
        else if (i == Tile::Souzu5) {
            ret[i] = counts[Tile::Souzu5] - counts[Tile::RedSouzu5];
            ret[34 + i] = counts[Tile::RedSouzu5];
        }
        else {
            ret[i] = counts[i];
            ret[34 + i] = 0;
        }
    }

    return ret;
}

int distance(const std::vector<int> &hand, const std::vector<int> &origin)
{
    return std::inner_product(
        hand.begin(), hand.end(), origin.begin(), 0, std::plus<int>(),
        [](const int x, const int y) { return std::max(x - y, 0); });
}

void ExpectedScoreCalculator::draw(Player &player, std::vector<int> &hand_reds,
                                   std::vector<int> &wall_reds, const int tile) const
{
    ++hand_reds[tile];
    --wall_reds[tile];

    // Update hand
    player.hand[tile % 34]++;
    if (tile == Tile::Manzu5 + 34) {
        player.hand[Tile::RedManzu5]++;
    }
    else if (tile == Tile::Pinzu5 + 34) {
        player.hand[Tile::RedPinzu5]++;
    }
    else if (tile == Tile::Souzu5 + 34) {
        player.hand[Tile::RedSouzu5]++;
    }
}

void ExpectedScoreCalculator::discard(Player &player, std::vector<int> &hand_reds,
                                      std::vector<int> &wall_reds, const int tile) const
{
    --hand_reds[tile];
    ++wall_reds[tile];

    // Update hand
    player.hand[tile % 34]--;
    if (tile == Tile::Manzu5 + 34) {
        player.hand[Tile::RedManzu5]--;
    }
    else if (tile == Tile::Pinzu5 + 34) {
        player.hand[Tile::RedPinzu5]--;
    }
    else if (tile == Tile::Souzu5 + 34) {
        player.hand[Tile::RedSouzu5]--;
    }
}

int ExpectedScoreCalculator::calc_score(const Params &params, const Round &round,
                                        Player &player, const int mode,
                                        const int tile) const
{
    int win_tile = tile;
    if (tile == Tile::Manzu5 + 34) {
        win_tile = Tile::RedManzu5;
    }
    else if (tile == Tile::Pinzu5 + 34) {
        win_tile = Tile::RedPinzu5;
    }
    else if (tile == Tile::Souzu5 + 34) {
        win_tile = Tile::RedSouzu5;
    }

    Result result = ScoreCalculator::calc(round, player, win_tile,
                                          WinFlag::Riichi | WinFlag::Tsumo);
    if (!result.success) {
        spdlog::error("Failed to calculate score.\n{}", to_string(result));
    }
    int score = result.score[0];

    return score;
}

ExpectedScoreCalculator::Vertex ExpectedScoreCalculator::select1(
    const Params &params, const Round &round, Player &player, Graph &graph,
    Desc &cache1, Desc &cache2, std::vector<int> &hand_reds,
    std::vector<int> &wall_reds, const std::vector<int> &origin,
    const int sht_org) const
{
    // If vertex exists in the cache, return it.
    CacheKey key(hand_reds);
    if (const auto itr = cache1.find(key); itr != cache1.end()) {
        return itr->second;
    }

    // Calculate necessary tiles.
    auto [type, shanten, wait] =
        NecessaryTileCalculator::calc(player.hand, 0, params.mode);
    bool allow_tegawari =
        distance(hand_reds, origin) + shanten < sht_org + params.extra;
    int64_t all = allow_tegawari ? calc_wait2(player.hand) : wait;
    all |= all << 34;

    // Add vertex to graph.
    const Vertex vertex =
        boost::add_vertex(std::vector<double>((params.t_max + 1) * 3, 0), graph);
    cache1[key] = vertex;

    for (int i = 0; i < 64; ++i) {
        if (wall_reds[i] && (all & (1LL << i))) {
            const int weight = wall_reds[i];

            draw(player, hand_reds, wall_reds, i);

            const auto target = select2(params, round, player, graph, cache1, cache2,
                                        hand_reds, wall_reds, origin, sht_org);

            if (!boost::edge(vertex, target, graph).second) {
                const int score = (shanten == 0 && (wait & (1LL << i % 34))
                                       ? calc_score(params, round, player, type, i % 34)
                                       : 0);

                boost::add_edge(vertex, target, {weight, score}, graph);
            }

            discard(player, hand_reds, wall_reds, i);
        }
    }

    return vertex;
}

ExpectedScoreCalculator::Vertex ExpectedScoreCalculator::select2(
    const Params &params, const Round &round, Player &player, Graph &graph,
    Desc &cache1, Desc &cache2, std::vector<int> &hand_reds,
    std::vector<int> &wall_reds, const std::vector<int> &origin,
    const int sht_org) const
{
    // If vertex exists in the cache, return it.
    CacheKey key(hand_reds);
    if (const auto itr = cache2.find(key); itr != cache2.end()) {
        return itr->second;
    }

    // Calculate unnecessary tiles.
    auto [type, shanten, disc] =
        UnnecessaryTileCalculator::calc(player.hand, 0, params.mode);
    bool allow_shanten_down =
        distance(hand_reds, origin) + shanten < sht_org + params.extra;
    int64_t all = allow_shanten_down ? calc_disc2(player.hand) : disc;
    all |= all << 34;

    // Add vertex to graph.
    std::vector<double> vertex_data((params.t_max + 1) * 3, 0);
    std::fill(vertex_data.begin(), vertex_data.begin() + (params.t_max + 1),
              shanten <= 0);
    std::fill(vertex_data.begin() + (params.t_max + 1),
              vertex_data.begin() + (params.t_max + 1) * 2, shanten == -1);
    const Vertex vertex = boost::add_vertex(vertex_data, graph);
    cache2[key] = vertex;

    for (int i = 0; i < 64; ++i) {
        if (hand_reds[i] && (all & (1LL << i))) {
            discard(player, hand_reds, wall_reds, i);

            const int weight = wall_reds[i];
            const auto source = select1(params, round, player, graph, cache1, cache2,
                                        hand_reds, wall_reds, origin, sht_org);

            draw(player, hand_reds, wall_reds, i);

            if (!boost::edge(source, vertex, graph).second) {
                const int score =
                    (shanten == -1 ? calc_score(params, round, player, type, i % 34)
                                   : 0);

                boost::add_edge(source, vertex, {weight, score}, graph);
            }
        }
    }

    return vertex;
}

void ExpectedScoreCalculator::update(const Params &params, Graph &graph,
                                     const Desc &cache1, const Desc &cache2) const
{
    for (int t = params.t_max - 1; t >= params.t_min; --t) {
        for (auto &[_, vertex] : cache1) {
            VertexData &value = graph[vertex];
            double *tenpai_prob = value.data();
            double *win_prob = value.data() + (params.t_max + 1);
            double *exp_value = value.data() + (params.t_max + 1) * 2;

            for (auto [first, last] = boost::out_edges(vertex, graph); first != last;
                 ++first) {
                const auto target = boost::target(*first, graph);
                const auto [weight, score] = graph[*first];
                const VertexData &value2 = graph[target];
                const double *tenpai_prob2 = value2.data();
                const double *win_prob2 = value2.data() + (params.t_max + 1);
                const double *exp_value2 = value2.data() + (params.t_max + 1) * 2;

                tenpai_prob[t] += weight * (tenpai_prob2[t + 1] - tenpai_prob[t + 1]);
                win_prob[t] += weight * (win_prob2[t + 1] - win_prob[t + 1]);
                exp_value[t] +=
                    weight * (std::max(static_cast<double>(score), exp_value2[t + 1]) -
                              exp_value[t + 1]);
            }

            // tenpai_prob[t] = tenpai_prob[t + 1] + tenpai_prob[t] / (params.sum - t);
            // win_prob[t] = win_prob[t + 1] + win_prob[t] / (params.sum - t);
            // exp_value[t] = exp_value[t + 1] + exp_value[t] / (params.sum - t);
            tenpai_prob[t] = tenpai_prob[t + 1] + tenpai_prob[t] / params.sum;
            win_prob[t] = win_prob[t + 1] + win_prob[t] / params.sum;
            exp_value[t] = exp_value[t + 1] + exp_value[t] / params.sum;
        }

        for (auto &[_, vertex] : cache2) {
            VertexData &value = graph[vertex];
            double *tenpai_prob = value.data();
            double *win_prob = value.data() + (params.t_max + 1);
            double *exp_value = value.data() + (params.t_max + 1) * 2;

            for (auto [first, last] = boost::in_edges(vertex, graph); first != last;
                 ++first) {
                const auto source = boost::source(*first, graph);
                const VertexData &value2 = graph[source];
                const double *tenpai_prob2 = value2.data();
                const double *win_prob2 = value2.data() + (params.t_max + 1);
                const double *exp_value2 = value2.data() + (params.t_max + 1) * 2;

                tenpai_prob[t] = std::max(tenpai_prob[t], tenpai_prob2[t]);
                win_prob[t] = std::max(win_prob[t], win_prob2[t]);
                exp_value[t] = std::max(exp_value[t], exp_value2[t]);
            }
        }
    }
}

std::tuple<std::vector<ExpectedScoreCalculator::Stat>, std::size_t>
ExpectedScoreCalculator::calc(const Params &params, const Round &round, Player &player)
{
    Graph graph;
    Desc cache1, cache2;

    assert(params.t_min >= 0);
    assert(params.t_min < params.t_max);
    assert(player.num_tiles() % 3 == 2);

    // Calculate shanten number of specified hand.
    auto [type, shanten] = ShantenCalculator::calc(player.hand, 0, params.mode);

    auto hand_reds = encode(player.hand);
    auto wall2 = create_wall(round, player);
    auto wall_reds = encode(wall2);

    // Build hand transition graph.
    select2(params, round, player, graph, cache1, cache2, hand_reds, wall_reds,
            std::vector<int>{hand_reds}, shanten);

    // Calculate expected values and probabilities.
    update(params, graph, cache1, cache2);

    // 結果を取得する。
    std::vector<Stat> stats;
    for (int i = 0; i < 64; ++i) {
        if (hand_reds[i] > 0) {
            --hand_reds[i];
            if (const auto itr = cache1.find(hand_reds); itr != cache1.end()) {
                const VertexData &value = graph[itr->second];
                std::vector<double> tenpai_prob(value.begin(),
                                                value.begin() + (params.t_max + 1));
                std::vector<double> win_prob(value.begin() + (params.t_max + 1),
                                             value.begin() + (params.t_max + 1) * 2);
                std::vector<double> exp_value(value.begin() + (params.t_max + 1) * 2,
                                              value.end());

                stats.emplace_back(
                    Stat{i % 34, i > 34, tenpai_prob, win_prob, exp_value});
            }
            ++hand_reds[i];
        }
    }

    const auto searched = boost::num_vertices(graph);

    return {stats, searched};
}
} // namespace mahjong
