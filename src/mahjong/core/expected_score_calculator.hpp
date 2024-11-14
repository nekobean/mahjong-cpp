#ifndef MAHJONG_CPP_EXPECTED_SCORE_CALCULATOR
#define MAHJONG_CPP_EXPECTED_SCORE_CALCULATOR

#include <array>
#include <map>
#include <tuple>
#include <valarray>
#include <vector>

#include <boost/container_hash/hash.hpp>
#include <boost/graph/adjacency_list.hpp>

#include "compare/utils.hpp"
#include "mahjong/types/types.hpp"

namespace mahjong
{
/**
 * @brief Expected score calculator
 */
class ExpectedScoreCalculator
{
    struct CacheKey
    {
        CacheKey(const Hand &hand) : manzu(0), pinzu(0), souzu(0), honors(0)
        {
            manzu = std::accumulate(hand.counts.begin(), hand.counts.begin() + 9, 0,
                                    [](int x, int y) { return x * 8 + y; });
            pinzu = std::accumulate(hand.counts.begin() + 9, hand.counts.begin() + 18,
                                    0, [](int x, int y) { return x * 8 + y; });
            souzu = std::accumulate(hand.counts.begin() + 18, hand.counts.begin() + 27,
                                    0, [](int x, int y) { return x * 8 + y; });
            honors = std::accumulate(hand.counts.begin() + 27, hand.counts.begin() + 34,
                                     0, [](int x, int y) { return x * 8 + y; });
            honors |= hand.counts[Tile::RedManzu5] << 21;
            honors |= hand.counts[Tile::RedPinzu5] << 22;
            honors |= hand.counts[Tile::RedSouzu5] << 23;
        }

        bool operator<(const CacheKey &other) const
        {
            return std::make_tuple(manzu, pinzu, souzu, honors) <
                   std::make_tuple(other.manzu, other.pinzu, other.souzu, other.honors);
        }

        int32_t manzu;
        int32_t pinzu;
        int32_t souzu;
        int32_t honors;
    };

    using Graph =
        boost::adjacency_list<boost::listS, boost::vecS, boost::bidirectionalS,
                              std::valarray<double>, int>;
    using Vertex = Graph::vertex_descriptor;
    using Edge = Graph::edge_descriptor;
    using Desc = std::map<CacheKey, Vertex>;

  private:
    Vertex draw(Graph &graph, Desc &desc1, Desc &desc2, Hand &hand, int num,
                const Hand &origin, int sht_org, const Params &params);
    Vertex discard(Graph &graph, Desc &desc1, Desc &desc2, Hand &hand, int num,
                   const Hand &origin, int sht_org, const Params &params);
    void update(Graph &graph, const Desc &desc1, const Desc &desc2,
                const Params &params);

  public:
    std::tuple<std::vector<Stat>, std::size_t> operator()(Hand &hand,
                                                          const Params &params);
};
} // namespace mahjong

#endif /* MAHJONG_CPP_EXPECTED_SCORE_CALCULATOR */
