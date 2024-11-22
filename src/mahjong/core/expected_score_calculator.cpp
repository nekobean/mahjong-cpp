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

Count ExpectedScoreCalculator::create_wall(const Round &round, const Player &player,
                                           const bool enable_reddora)
{
    Count wall{0}, melds{0}, indicators{0};

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

    // assert
    if (!enable_reddora) {
        for (int i = 34; i < 37; ++i) {
            assert(wall[i] == 0);
        }
    }
    else {
        if (wall[Tile::RedManzu5]) {
            assert(wall[Tile::Manzu5] >= 1);
        }
        if (wall[Tile::RedPinzu5]) {
            assert(wall[Tile::Pinzu5] >= 1);
        }
        if (wall[Tile::RedSouzu5]) {
            assert(wall[Tile::Souzu5] >= 1);
        }
        if (wall[Tile::Manzu5] == 4) {
            assert(wall[Tile::RedManzu5] == 1);
        }
        if (wall[Tile::Pinzu5] == 4) {
            assert(wall[Tile::RedPinzu5] == 1);
        }
        if (wall[Tile::Souzu5] == 4) {
            assert(wall[Tile::RedSouzu5] == 1);
        }
    }

    // for (int i = 0; i < 34; ++i) {
    //     assert(wall[i] >= 0 && wall[i] <= 4);
    // }
    // for (int i = 34; i < 37; ++i) {
    //     if (enable_reddora) {
    //         assert(wall[i] >= 0 && wall[i] <= 1);
    //     }
    //     else {
    //         assert(wall[i] == 0);
    //     }
    // }

    return wall;
}

Count ExpectedScoreCalculator::encode(const Count &counts, const bool enable_reddora)
{
    Count ret{0};
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

int ExpectedScoreCalculator::distance(const Count &hand, const Count &hand_org)
{
    int dist = 0;
    for (int i = 0; i < hand.size(); ++i) {
        dist += std::max(hand[i] - hand_org[i], 0);
    }

    return dist;
}

void ExpectedScoreCalculator::draw(Player &player, Count &hand_counts,
                                   Count &wall_counts, const int tile)
{
    ++hand_counts[tile];
    --wall_counts[tile];

    // Update hand
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

void ExpectedScoreCalculator::discard(Player &player, Count &hand_counts,
                                      Count &wall_counts, const int tile)
{
    --hand_counts[tile];
    ++wall_counts[tile];

    // Update hand
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

int ExpectedScoreCalculator::calc_score(const Config &config, const Round &round,
                                        Player &player, const int shanten_type,
                                        const int win_tile)
{
    // Result result = ScoreCalculator::calc(round, player, win_tile,
    //                                       WinFlag::Riichi | WinFlag::Tsumo);
    Result result = ScoreCalculator::calc_fast(
        round, player, win_tile, WinFlag::Riichi | WinFlag::Tsumo, shanten_type);
    assert(result.success);
    int score = result.score[0];

    return score;
}

ExpectedScoreCalculator::Vertex
ExpectedScoreCalculator::select1(const Config &config, const Round &round,
                                 Player &player, Graph &graph, Cache &cache1,
                                 Cache &cache2, Count &hand_counts, Count &wall_counts,
                                 const Count &hand_org, const int shanten_org)
{
    // If vertex exists in the cache, return it.
    CacheKey key(hand_counts);
    if (const auto itr = cache1.find(key); itr != cache1.end()) {
        return itr->second;
    }

    // Calculate necessary tiles.
    auto [type, shanten, wait] = NecessaryTileCalculator::calc(
        player.hand, player.num_melds(), config.shanten_type);
    bool allow_tegawari =
        config.enable_tegawari &&
        distance(hand_counts, hand_org) + shanten < shanten_org + config.extra;
    wait |= (wait & (1LL << Tile::Manzu5)) ? (1LL << Tile::RedManzu5) : 0;
    wait |= (wait & (1LL << Tile::Pinzu5)) ? (1LL << Tile::RedPinzu5) : 0;
    wait |= (wait & (1LL << Tile::Souzu5)) ? (1LL << Tile::RedSouzu5) : 0;

    // Add vertex to graph.
    const Vertex vertex =
        boost::add_vertex(std::vector<double>((config.t_max + 1) * 3, 0), graph);
    cache1[key] = vertex;

    for (int i = 0; i < 37; ++i) {
        bool is_wait = wait & (1LL << i);

        if (wall_counts[i] && (allow_tegawari || is_wait)) {
            const int weight = wall_counts[i];

            draw(player, hand_counts, wall_counts, i);

            const Vertex target =
                select2(config, round, player, graph, cache1, cache2, hand_counts,
                        wall_counts, hand_org, shanten_org);

            if (!boost::edge(vertex, target, graph).second) {
                const int score = (shanten == 0 && is_wait
                                       ? calc_score(config, round, player, type, i)
                                       : 0);
                boost::add_edge(vertex, target, {weight, score}, graph);
            }

            discard(player, hand_counts, wall_counts, i);
        }
    }

    return vertex;
}

ExpectedScoreCalculator::Vertex
ExpectedScoreCalculator::select2(const Config &config, const Round &round,
                                 Player &player, Graph &graph, Cache &cache1,
                                 Cache &cache2, Count &hand_counts, Count &wall_counts,
                                 const Count &hand_org, const int shanten_org)
{
    // If vertex exists in the cache, return it.
    CacheKey key(hand_counts);
    if (const auto itr = cache2.find(key); itr != cache2.end()) {
        return itr->second;
    }

    // Calculate unnecessary tiles.
    auto [type, shanten, disc] = UnnecessaryTileCalculator::calc(
        player.hand, player.num_melds(), config.shanten_type);
    bool allow_shanten_down =
        config.enable_shanten_down &&
        distance(hand_counts, hand_org) + shanten < shanten_org + config.extra;
    disc |= (disc & (1LL << Tile::Manzu5)) ? (1LL << Tile::RedManzu5) : 0;
    disc |= (disc & (1LL << Tile::Pinzu5)) ? (1LL << Tile::RedPinzu5) : 0;
    disc |= (disc & (1LL << Tile::Souzu5)) ? (1LL << Tile::RedSouzu5) : 0;

    // Add vertex to graph.
    std::vector<double> vertex_data((config.t_max + 1) * 3, 0);
    std::fill(vertex_data.begin(), vertex_data.begin() + (config.t_max + 1),
              shanten <= 0);
    std::fill(vertex_data.begin() + (config.t_max + 1),
              vertex_data.begin() + (config.t_max + 1) * 2, shanten == -1);
    const Vertex vertex = boost::add_vertex(vertex_data, graph);
    cache2[key] = vertex;

    for (int i = 0; i < 37; ++i) {
        bool is_disc = disc & (1LL << i);

        if (hand_counts[i] && (allow_shanten_down || is_disc)) {
            discard(player, hand_counts, wall_counts, i);

            const int weight = wall_counts[i];
            const Vertex source =
                select1(config, round, player, graph, cache1, cache2, hand_counts,
                        wall_counts, hand_org, shanten_org);

            draw(player, hand_counts, wall_counts, i);

            if (!boost::edge(source, vertex, graph).second) {
                const int score =
                    (shanten == -1 ? calc_score(config, round, player, type, i) : 0);
                boost::add_edge(source, vertex, {weight, score}, graph);
            }
        }
    }

    return vertex;
}

void ExpectedScoreCalculator::calc_values(const Config &config, Graph &graph,
                                          const Cache &cache1, const Cache &cache2,
                                          const int sum)
{
    for (int t = config.t_max - 1; t >= config.t_min; --t) {
        for (auto &[_, vertex] : cache1) {
            VertexData &value = graph[vertex];
            double *tenpai_prob = value.data();
            double *win_prob = value.data() + (config.t_max + 1);
            double *exp_value = value.data() + (config.t_max + 1) * 2;

            for (auto [first, last] = boost::out_edges(vertex, graph); first != last;
                 ++first) {
                const auto target = boost::target(*first, graph);
                const auto [weight, score] = graph[*first];
                const VertexData &value2 = graph[target];
                const double *tenpai_prob2 = value2.data();
                const double *win_prob2 = value2.data() + (config.t_max + 1);
                const double *exp_value2 = value2.data() + (config.t_max + 1) * 2;

                tenpai_prob[t] += weight * (tenpai_prob2[t + 1] - tenpai_prob[t + 1]);
                win_prob[t] += weight * (win_prob2[t + 1] - win_prob[t + 1]);
                exp_value[t] +=
                    weight * (std::max(static_cast<double>(score), exp_value2[t + 1]) -
                              exp_value[t + 1]);
            }

            // tenpai_prob[t] = tenpai_prob[t + 1] + tenpai_prob[t] / (sum - t);
            // win_prob[t] = win_prob[t + 1] + win_prob[t] / (sum - t);
            // exp_value[t] = exp_value[t + 1] + exp_value[t] / (sum - t);
            tenpai_prob[t] = tenpai_prob[t + 1] + tenpai_prob[t] / sum;
            win_prob[t] = win_prob[t + 1] + win_prob[t] / sum;
            exp_value[t] = exp_value[t + 1] + exp_value[t] / sum;
        }

        for (auto &[_, vertex] : cache2) {
            VertexData &value = graph[vertex];
            double *tenpai_prob = value.data();
            double *win_prob = value.data() + (config.t_max + 1);
            double *exp_value = value.data() + (config.t_max + 1) * 2;

            for (auto [first, last] = boost::in_edges(vertex, graph); first != last;
                 ++first) {
                const auto source = boost::source(*first, graph);
                const VertexData &value2 = graph[source];
                const double *tenpai_prob2 = value2.data();
                const double *win_prob2 = value2.data() + (config.t_max + 1);
                const double *exp_value2 = value2.data() + (config.t_max + 1) * 2;

                tenpai_prob[t] = std::max(tenpai_prob[t], tenpai_prob2[t]);
                win_prob[t] = std::max(win_prob[t], win_prob2[t]);
                exp_value[t] = std::max(exp_value[t], exp_value2[t]);
            }
        }
    }
}

std::tuple<std::vector<ExpectedScoreCalculator::Stat>, int>
ExpectedScoreCalculator::calc(const Config &config, const Round &round,
                              const Player &player)
{
    const Count wall = create_wall(round, player, config.enable_reddora);

    return calc(config, round, player, wall);
}

std::tuple<std::vector<ExpectedScoreCalculator::Stat>, int>
ExpectedScoreCalculator::calc(const Config &config, const Round &round,
                              const Player &player, const Count &wall)
{
    Graph graph;
    Cache cache1, cache2;

    Player player_copy = player;
    Count hand_counts = encode(player.hand, config.enable_reddora);
    Count wall_counts = encode(wall, config.enable_reddora);

    const int sum = config.sum;

    // Calculate shanten number of specified hand.
    const Count hand_org = hand_counts;
    const int shanten_org = std::get<1>(
        ShantenCalculator::calc(player.hand, player.num_melds(), config.shanten_type));

    std::vector<Stat> stats;
    const int num_tiles = player.num_tiles() + player.num_melds() * 3;
    if (num_tiles == 13) {
        // Build hand transition graph.
        select1(config, round, player_copy, graph, cache1, cache2, hand_counts,
                wall_counts, hand_org, shanten_org);

        // Calculate expected values and probabilities.
        calc_values(config, graph, cache1, cache2, sum);

        // 結果を取得する。
        if (const auto itr = cache1.find(hand_counts); itr != cache1.end()) {
            const VertexData &value = graph[itr->second];
            std::vector<double> tenpai_prob(value.begin(),
                                            value.begin() + (config.t_max + 1));
            std::vector<double> win_prob(value.begin() + (config.t_max + 1),
                                         value.begin() + (config.t_max + 1) * 2);
            std::vector<double> exp_value(value.begin() + (config.t_max + 1) * 2,
                                          value.end());

            const auto [shanten_type, shanten, tiles] = NecessaryTileCalculator::select(
                player.hand, player.num_melds(), config.shanten_type);
            std::vector<std::tuple<int, int>> necessary_tiles;
            for (const auto tile : tiles) {
                necessary_tiles.emplace_back(tile, wall[tile]);
            }

            stats.emplace_back(
                Stat{Tile::Null, tenpai_prob, win_prob, exp_value, necessary_tiles});
        }
    }
    else {
        // Build hand transition graph.
        select2(config, round, player_copy, graph, cache1, cache2, hand_counts,
                wall_counts, hand_org, shanten_org);

        // Calculate expected values and probabilities.
        calc_values(config, graph, cache1, cache2, sum);

        // 結果を取得する。
        for (int i = 0; i < 37; ++i) {
            if (hand_counts[i] > 0) {
                --hand_counts[i];
                if (const auto itr = cache1.find(hand_counts); itr != cache1.end()) {
                    const VertexData &value = graph[itr->second];
                    std::vector<double> tenpai_prob(value.begin(),
                                                    value.begin() + (config.t_max + 1));
                    std::vector<double> win_prob(value.begin() + (config.t_max + 1),
                                                 value.begin() +
                                                     (config.t_max + 1) * 2);
                    std::vector<double> exp_value(
                        value.begin() + (config.t_max + 1) * 2, value.end());

                    const auto [shanten_type, shanten, tiles] =
                        NecessaryTileCalculator::select(player.hand, player.num_melds(),
                                                        config.shanten_type);
                    std::vector<std::tuple<int, int>> necessary_tiles;
                    for (const auto tile : tiles) {
                        necessary_tiles.emplace_back(tile, wall[tile]);
                    }

                    stats.emplace_back(
                        Stat{i, tenpai_prob, win_prob, exp_value, necessary_tiles});
                }
                ++hand_counts[i];
            }
        }
    }

    const int searched = static_cast<int>(boost::num_vertices(graph));

    return {stats, searched};
}

} // namespace mahjong
