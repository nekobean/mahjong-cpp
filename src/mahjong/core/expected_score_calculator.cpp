#include "expected_score_calculator.hpp"

#include <boost/range/iterator_range_core.hpp>

#include <numeric>
#undef NDEBUG
#include <cassert>

#include "mahjong/core/necessary_tile_calculator.hpp"
#include "mahjong/core/shanten_calculator.hpp"
#include "mahjong/core/unnecessary_tile_calculator.hpp"

namespace mahjong
{

int64_t discard_all(const Hand &hand)
{
    int64_t ret = 0LL;

    for (int i = 0; i < 34; ++i) {
        if (hand.counts[i] > 0) {
            ret |= 1LL << i;
        }
    }

    return ret;
}

int64_t wait_all(const Hand &hand)
{
    int64_t ret = 0LL;

    for (int i = 0; i < 34; ++i) {
        if (hand.counts[i] < 4) {
            ret |= 1LL << i;
        }
    }

    return ret;
}

int distance(const Hand &hand, const Hand &origin)
{
    return std::inner_product(
        hand.counts.begin(), hand.counts.begin() + 34, origin.counts.begin(), 0,
        std::plus<int>(), [](const int x, const int y) { return std::max(x - y, 0); });
}

ExpectedScoreCalculator::Vertex
ExpectedScoreCalculator::draw(Graph &graph, Desc &desc1, Desc &desc2, Hand &hand,
                              const int num, const Hand &origin, const int sht_org,
                              const Params &params)
{
    CacheKey key(hand);
    if (const auto itr = desc1.find(key); itr != desc1.end()) {
        return itr->second;
    }

    auto [type, shanten, wait] = NecessaryTileCalculator::calc(hand, params.mode);
    bool allow_tegawari = distance(hand, origin) + shanten < sht_org + params.extra;
    wait = allow_tegawari ? wait_all(hand) : wait;

    const Vertex vertex =
        boost::add_vertex(std::valarray<double>(0., params.t_max + 1), graph);
    desc1[key] = vertex;

    for (int i = 0; i < 34; ++i) {
        if (wait & (1LL << i)) {
            // 自摸前の残り枚数
            const int weight = 4 - hand.counts[i];

            // 状態遷移させる。
            ++hand.counts[i];
            const auto target =
                discard(graph, desc1, desc2, hand, num + 1, origin, sht_org, params);
            --hand.counts[i];

            // エッジを追加する。
            // 自摸前の手牌 -自摸枚数-> 自摸後の手牌
            if (!boost::edge(vertex, target, graph).second) {
                boost::add_edge(vertex, target, weight, graph);
            }
        }
    }

    return vertex;
}

ExpectedScoreCalculator::Vertex
ExpectedScoreCalculator::discard(Graph &graph, Desc &desc1, Desc &desc2, Hand &hand,
                                 const int num, const Hand &origin, const int sht_org,
                                 const Params &params)
{
    CacheKey key(hand);
    if (const auto itr = desc2.find(key); itr != desc2.end()) {
        return itr->second;
    }

    auto [type, shanten, disc] = UnnecessaryTileCalculator::calc(hand, params.mode);
    bool allow_shanten_down = distance(hand, origin) + shanten < sht_org + params.extra;
    disc = allow_shanten_down ? discard_all(hand) : disc;

    const Vertex vertex = boost::add_vertex(
        std::valarray<double>(shanten == -1, params.t_max + 1), graph);
    desc2[key] = vertex;

    for (int i = 0; i < 34; ++i) {
        if (disc & (1LL << i)) {
            --hand.counts[i];
            const int weight = 4 - hand.counts[i];
            const auto source =
                draw(graph, desc1, desc2, hand, num - 1, origin, sht_org, params);
            ++hand.counts[i];

            if (!boost::edge(source, vertex, graph).second) {
                boost::add_edge(source, vertex, weight, graph);
            }
        }
    }

    return vertex;
}

void ExpectedScoreCalculator::update(Graph &graph, const Desc &desc1, const Desc &desc2,
                                     const Params &params)
{
    for (int t = params.t_max - 1; t >= params.t_min; --t) {
        for (auto &[hand, vertex] : desc1) {
            std::valarray<double> &prob = graph[vertex];

            for (const auto &edge :
                 boost::make_iterator_range(boost::out_edges(vertex, graph))) {
                const Vertex target = boost::target(edge, graph);

                prob[t] += graph[edge] * (graph[target][t + 1] - prob[t + 1]);
            }

            prob[t] = prob[t + 1] + prob[t] / (params.sum - t);
        }

        for (auto &[hand, vertex] : desc2) {
            std::valarray<double> &prob = graph[vertex];

            for (const auto &edge :
                 boost::make_iterator_range(boost::in_edges(vertex, graph))) {
                const Vertex source = boost::source(edge, graph);

                prob[t] = std::max(prob[t], graph[source][t]);
            }
        }
    }
}

std::tuple<std::vector<Stat>, std::size_t>
ExpectedScoreCalculator::operator()(Hand &hand, const Params &params)
{
    const int num = hand.num_tiles();

    assert(params.t_min >= 0);
    assert(params.t_min < params.t_max);
    assert(num % 3 == 2);

    const auto [type, shanten] = ShantenCalculator::calc(hand, params.mode);

    Graph graph;
    Desc desc1, desc2;
    Hand origin_hand = hand;

    // Create a transition graph.
    discard(graph, desc1, desc2, hand, num, origin_hand, shanten, params);

    // Calculate probabilities and expected values.
    update(graph, desc1, desc2, params);

    std::vector<Stat> stats;
    for (int i = 0; i < 34; ++i) {
        if (hand.counts[i] > 0) {
            --hand.counts[i];
            if (const auto itr = desc1.find(hand); itr != desc1.end()) {
                stats.emplace_back(Stat{i, graph[itr->second]});
            }
            ++hand.counts[i];
        }
    }

    const auto searched = boost::num_vertices(graph);

    return {stats, searched};
}

} // namespace mahjong
