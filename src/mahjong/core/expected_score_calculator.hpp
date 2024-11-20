#ifndef MAHJONG_CPP_EXPECTED_SCORE_CALCULATOR
#define MAHJONG_CPP_EXPECTED_SCORE_CALCULATOR

#include <map>
#include <tuple>
#include <vector>

#include <boost/graph/adjacency_list.hpp>

#include "mahjong/types/types.hpp"

namespace mahjong
{
class ExpectedScoreCalculator
{
  public:
    struct Params
    {
        Params()
            : t_min(0)
            , t_max(18)
            , sum(121)
            , extra(0)
            , mode(7)
            , enable_shanten_down(true)
            , enable_tegawari(true)
            , enable_reddora(true)
            , enable_uradora(true)
        {
        }

        /* min turn to be calculated */
        int t_min = 0;
        /* max turn to be calculated */
        int t_max = 18;
        /* number of wall tiles */
        int sum = 0;
        /* search the range of possible (shanten number + extra) exchanges */
        int extra = 0;
        /* calculation mode */
        int mode = 7;
        /* enable red dora */
        bool enable_reddora = true;
        /* enable ura dora */
        bool enable_uradora = true;
        /* allow shanten down */
        bool enable_shanten_down = true;
        /* allow tegawari */
        bool enable_tegawari = true;
    };

    struct Stat
    {
        /* tile */
        int tile;
        /* tenpai probability */
        std::vector<double> tenpai_prob;
        /* win probability */
        std::vector<double> win_prob;
        /* expected score */
        std::vector<double> exp_value;
        /* list of necessary tiles */
        std::vector<std::tuple<int, int>> necessary_tiles;
    };

  private:
    struct CacheKey
    {
        CacheKey(const std::vector<int> &hand) : manzu(0), pinzu(0), souzu(0), honors(0)
        {
            manzu = std::accumulate(hand.begin(), hand.begin() + 9, 0,
                                    [](int x, int y) { return x * 8 + y; });
            pinzu = std::accumulate(hand.begin() + 9, hand.begin() + 18, 0,
                                    [](int x, int y) { return x * 8 + y; });
            souzu = std::accumulate(hand.begin() + 18, hand.begin() + 27, 0,
                                    [](int x, int y) { return x * 8 + y; });
            honors = std::accumulate(hand.begin() + 27, hand.begin() + 34, 0,
                                     [](int x, int y) { return x * 8 + y; });
            honors |= hand[Tile::Manzu5 + 34] << 21;
            honors |= hand[Tile::Pinzu5 + 34] << 22;
            honors |= hand[Tile::Souzu5 + 34] << 23;
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

    using VertexData = std::vector<double>;
    using EdgeData = std::pair<int, int>;
    using Graph = boost::adjacency_list<boost::listS, boost::vecS,
                                        boost::bidirectionalS, VertexData, EdgeData>;
    using Vertex = Graph::vertex_descriptor;
    using Edge = Graph::edge_descriptor;
    using Desc = std::map<CacheKey, Vertex>;

  public:
    static std::tuple<std::vector<Stat>, int> calc(const Params &params,
                                                   const Round &round, Player &player);

  private:
    static int distance(const std::vector<int> &hand, const std::vector<int> &origin);
    static void draw(Player &player, std::vector<int> &hand_reds,
                     std::vector<int> &wall_reds, const int tile);
    static void discard(Player &player, std::vector<int> &hand_reds,
                        std::vector<int> &wall_reds, const int tile);
    static int calc_score(const Params &params, const Round &round, Player &player,
                          const int mode, const int tile);
    static Vertex select1(const Params &params, const Round &round, Player &player,
                          Graph &graph, Desc &cache1, Desc &cache2,
                          std::vector<int> &hand_reds, std::vector<int> &wall_reds,
                          const std::vector<int> &origin_reds, int sht_org);
    static Vertex select2(const Params &params, const Round &round, Player &player,
                          Graph &graph, Desc &cache1, Desc &cache2,
                          std::vector<int> &hand_reds, std::vector<int> &wall_reds,
                          const std::vector<int> &origin_reds, int sht_org);
    static void calc_values(const Params &params, Graph &graph, const Desc &cache1,
                            const Desc &cache2);
};
} // namespace mahjong

#endif
