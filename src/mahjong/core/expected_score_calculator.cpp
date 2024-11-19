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

using namespace player_impl;
constexpr int L = 64;

namespace mahjong
{
int64_t calc_disc2(const HandType &hand)
{
    int64_t ret = 0LL;

    for (int i = 0; i < K; ++i) {
        if (hand[i] > 0) {
            ret |= 1LL << i;
        }
    }

    return ret;
}

int64_t calc_wait2(const HandType &hand)
{
    int64_t ret = 0LL;

    for (int i = 0; i < K; ++i) {
        if (hand[i] < 4) {
            ret |= 1LL << i;
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

std::vector<int> encode(const std::vector<int> &hand, const std::vector<int> &reds)
{
    std::vector<int> ret(2 * K);

    for (int i = 0; i < K; ++i) {
        ret[i] = hand[i] - reds[i];
        ret[K + i] = reds[i];
    }

    return ret;
}

HandType create_wall(const Round &round, const MyPlayer &player)
{
    HandType wall{0};
    std::fill(wall.begin(), wall.begin() + K, 4);
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

    for (int i = 0; i < K; ++i) {
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

std::vector<int> encode(const HandType &counts)
{
    std::vector<int> ret(2 * K, 0);

    for (int i = 0; i < K; ++i) {
        if (i == Tile::Manzu5) {
            ret[i] = counts[Tile::Manzu5] - counts[Tile::RedManzu5];
            ret[K + i] = counts[Tile::RedManzu5];
        }
        else if (i == Tile::Pinzu5) {
            ret[i] = counts[Tile::Pinzu5] - counts[Tile::RedPinzu5];
            ret[K + i] = counts[Tile::RedPinzu5];
        }
        else if (i == Tile::Souzu5) {
            ret[i] = counts[Tile::Souzu5] - counts[Tile::RedSouzu5];
            ret[K + i] = counts[Tile::RedSouzu5];
        }
        else {
            ret[i] = counts[i];
            ret[K + i] = 0;
        }
    }

    return ret;
}

void draw(std::vector<int> &hand_reds, std::vector<int> &wall_reds, MyPlayer &player,
          const int tile)
{
    ++hand_reds[tile]; // 手札に追加
    --wall_reds[tile]; // 山から削除

    // 手牌更新
    player.hand[tile % K]++; // 手牌から削除
    if (tile == 34 + Tile::Manzu5) {
        player.hand[Tile::RedManzu5]++;
    }
    else if (tile == 34 + Tile::Pinzu5) {
        player.hand[Tile::RedPinzu5]++;
    }
    else if (tile == 34 + Tile::Souzu5) {
        player.hand[Tile::RedSouzu5]++;
    }
}

void discard(std::vector<int> &hand_reds, std::vector<int> &wall_reds, MyPlayer &player,
             const int tile)
{
    --hand_reds[tile]; // 手札から削除
    ++wall_reds[tile]; // 山に追加

    // 手牌更新
    player.hand[tile % K]--; // 手牌から削除
    if (tile == 34 + Tile::Manzu5) {
        player.hand[Tile::RedManzu5]--;
    }
    else if (tile == 34 + Tile::Pinzu5) {
        player.hand[Tile::RedPinzu5]--;
    }
    else if (tile == 34 + Tile::Souzu5) {
        player.hand[Tile::RedSouzu5]--;
    }
}

int ExpectedScoreCalculator::calc_score(MyPlayer &player, const int mode,
                                        const int tile, const Params &params) const
{
    int win_tile2 = tile;
    if (tile == 34 + Tile::Manzu5) {
        win_tile2 = Tile::RedManzu5;
    }
    else if (tile == 34 + Tile::Pinzu5) {
        win_tile2 = Tile::RedPinzu5;
    }
    else if (tile == 34 + Tile::Souzu5) {
        win_tile2 = Tile::RedSouzu5;
    }

    Result result = ScoreCalculator::calc(round_, player, win_tile2,
                                          WinFlag::Riichi | WinFlag::Tsumo);
    if (!result.success) {
        std::cout << to_mpsz(player.hand) << std::endl;
        std::cout << result.err_msg << std::endl;
    }
    assert(result.success);

    return result.score[0];
}

ExpectedScoreCalculator::Vertex ExpectedScoreCalculator::select1(
    Graph &graph, Desc &cache1, Desc &cache2, std::vector<int> &hand_reds,
    std::vector<int> &wall_reds, MyPlayer &player, const std::vector<int> &origin,
    const int sht_org, const Params &params) const
{
    // キャッシュに存在する場合、その頂点を返す。
    CacheKey key(hand_reds);
    if (const auto itr = cache1.find(key); itr != cache1.end()) {
        return itr->second;
    }

    // 頂点を作成する。
    const Vertex vertex =
        boost::add_vertex(std::vector<double>((params.t_max + 1) * 3, 0), graph);
    cache1[key] = vertex;

    // 有効牌を計算する。

    auto [type, shanten, wait] =
        NecessaryTileCalculator::calc(player.hand, 0, params.mode);
    bool allow_tegawari =
        distance(hand_reds, origin) + shanten < sht_org + params.extra;
    int64_t all = allow_tegawari ? calc_wait2(player.hand) : wait;
    all |= all << K;

    for (int i = 0; i < L; ++i) {
        if (wall_reds[i] && (all & (1LL << i))) {
            const int weight = wall_reds[i];

            draw(hand_reds, wall_reds, player, i);

            const auto target = select2(graph, cache1, cache2, hand_reds, wall_reds,
                                        player, origin, sht_org, params);

            if (!boost::edge(vertex, target, graph).second) {
                const int score = (shanten == 0 && (wait & (1LL << i % K))
                                       ? calc_score(player, type, i % K, params)
                                       : 0);

                boost::add_edge(vertex, target, {weight, score}, graph);
            }

            discard(hand_reds, wall_reds, player, i);
        }
    }

    return vertex;
}

ExpectedScoreCalculator::Vertex ExpectedScoreCalculator::select2(
    Graph &graph, Desc &cache1, Desc &cache2, std::vector<int> &hand_reds,
    std::vector<int> &wall_reds, MyPlayer &player, const std::vector<int> &origin,
    const int sht_org, const Params &params) const
{
    // キャッシュに存在する場合、その頂点を返す。
    CacheKey key(hand_reds);
    if (const auto itr = cache2.find(key); itr != cache2.end()) {
        return itr->second;
    }

    // 有効牌を計算する。
    auto [type, shanten, disc] =
        UnnecessaryTileCalculator::calc(player.hand, 0, params.mode);

    bool allow_shanten_down =
        distance(hand_reds, origin) + shanten < sht_org + params.extra;
    int64_t all = allow_shanten_down ? calc_disc2(player.hand) : disc;
    all |= all << K;

    // 頂点を作成する。
    std::vector<double> vertex_data((params.t_max + 1) * 3, 0);
    std::fill(vertex_data.begin(), vertex_data.begin() + (params.t_max + 1),
              shanten <= 0);
    std::fill(vertex_data.begin() + (params.t_max + 1),
              vertex_data.begin() + (params.t_max + 1) * 2, shanten == -1);
    const Vertex vertex = boost::add_vertex(vertex_data, graph);
    cache2[key] = vertex;

    for (int i = 0; i < L; ++i) {
        if (hand_reds[i] && (all & (1LL << i))) {
            discard(hand_reds, wall_reds, player, i);

            const int weight = wall_reds[i];
            const auto source = select1(graph, cache1, cache2, hand_reds, wall_reds,
                                        player, origin, sht_org, params);

            draw(hand_reds, wall_reds, player, i);

            if (!boost::edge(source, vertex, graph).second) {
                const int score =
                    (shanten == -1 ? calc_score(player, type, i % K, params) : 0);

                boost::add_edge(source, vertex, {weight, score}, graph);
            }
        }
    }

    return vertex;
}

void ExpectedScoreCalculator::update(Graph &graph, const Desc &cache1,
                                     const Desc &cache2, const Params &params) const
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
                VertexData &value2 = graph[target];
                double *tenpai_prob2 = value2.data();
                double *win_prob2 = value2.data() + (params.t_max + 1);
                double *exp_value2 = value2.data() + (params.t_max + 1) * 2;

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
                VertexData &value2 = graph[source];
                double *tenpai_prob2 = value2.data();
                double *win_prob2 = value2.data() + (params.t_max + 1);
                double *exp_value2 = value2.data() + (params.t_max + 1) * 2;

                tenpai_prob[t] = std::max(tenpai_prob[t], tenpai_prob2[t]);
                win_prob[t] = std::max(win_prob[t], win_prob2[t]);
                exp_value[t] = std::max(exp_value[t], exp_value2[t]);
            }
        }
    }
}

std::tuple<std::vector<ExpectedScoreCalculator::Stat>, std::size_t>
ExpectedScoreCalculator::calc(const Round &round, MyPlayer &player,
                              const Params &params)
{
    round_ = round;

    assert(params.t_min >= 0);
    assert(params.t_min < params.t_max);
    assert(player.num_tiles() % 3 == 2);

    auto [type, shanten] = ShantenCalculator::calc(player.hand, 0, params.mode);

    Graph graph;
    Desc cache1, cache2;

    auto hand_reds = encode(player.hand);
    auto wall2 = create_wall(round, player);
    auto wall_reds = encode(wall2);

    select2(graph, cache1, cache2, hand_reds, wall_reds, player,
            std::vector<int>{hand_reds}, shanten, params);
    update(graph, cache1, cache2, params);

    // 結果を取得する。
    std::vector<Stat> stats;
    for (int i = 0; i < L; ++i) {
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
                    Stat{i % K, i > K, tenpai_prob, win_prob, exp_value});
            }
            ++hand_reds[i];
        }
    }

    const auto searched = boost::num_vertices(graph);

    return {stats, searched};
}
} // namespace mahjong
