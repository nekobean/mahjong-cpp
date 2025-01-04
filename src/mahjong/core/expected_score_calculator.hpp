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
    using CountRed = std::array<int, 37>;

  public:
    struct Config
    {
        Config()
            : t_min(1)
            , t_max(18)
            , sum(0)
            , extra(0)
            , shanten_type(ShantenFlag::All)
            , enable_reddora(true)
            , enable_uradora(true)
            , enable_shanten_down(true)
            , enable_tegawari(true)
            , enable_riichi(false)
            , calc_stats(true)
        {
        }

        /* min turn to be calculated */
        int t_min;
        /* max turn to be calculated */
        int t_max;
        /* number of wall tiles */
        int sum;
        /* search the range of possible (shanten number + extra) exchanges */
        int extra;
        /* calculation shanten type */
        int shanten_type;
        /* enable red dora */
        bool enable_reddora;
        /* enable ura dora */
        bool enable_uradora;
        /* allow shanten down */
        bool enable_shanten_down;
        /* allow tegawari */
        bool enable_tegawari;
        /* call riichi when tenpai */
        bool enable_riichi;
        /* calculate value */
        bool calc_stats;
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
        std::vector<double> exp_score;
        /* list of necessary tiles */
        std::vector<std::tuple<int, int>> necessary_tiles;
        /* shanten */
        int shanten;
    };

  private:
    struct CacheKey
    {
        CacheKey(const Count &hand) : manzu(0), pinzu(0), souzu(0), honors(0)
        {
            manzu = std::accumulate(hand.begin(), hand.begin() + 9, 0,
                                    [](int x, int y) { return x * 8 + y; });
            pinzu = std::accumulate(hand.begin() + 9, hand.begin() + 18, 0,
                                    [](int x, int y) { return x * 8 + y; });
            souzu = std::accumulate(hand.begin() + 18, hand.begin() + 27, 0,
                                    [](int x, int y) { return x * 8 + y; });
            honors = std::accumulate(hand.begin() + 27, hand.begin() + 34, 0,
                                     [](int x, int y) { return x * 8 + y; });
            honors |= hand[Tile::RedManzu5] << 21;
            honors |= hand[Tile::RedPinzu5] << 22;
            honors |= hand[Tile::RedSouzu5] << 23;
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

    struct VertexData
    {
      public:
        VertexData() = default;
        VertexData(const size_t size, const double tenpai_init, const double win_init,
                   const double exp_init)
            : tenpai_prob(size, tenpai_init)
            , win_prob(size, win_init)
            , exp_score(size, exp_init)
        {
        }

        std::vector<double> tenpai_prob;
        std::vector<double> win_prob;
        std::vector<double> exp_score;
    };

    using EdgeData = std::tuple<int, int>;
    using Graph = boost::adjacency_list<boost::listS, boost::vecS,
                                        boost::bidirectionalS, VertexData, EdgeData>;
    using Vertex = Graph::vertex_descriptor;
    using Edge = Graph::edge_descriptor;
    using Cache = std::map<CacheKey, Vertex>;

    // uradora_table[num_indicators][num_doras]
    static std::array<std::array<double, 13>, 6> uradora_table_;

  public:
    ExpectedScoreCalculator();

    static std::tuple<std::vector<Stat>, int>
    calc(const Config &config, const Round &round, const Player &player);

    static std::tuple<std::vector<Stat>, int> calc(const Config &config,
                                                   const Round &round,
                                                   const Player &player,
                                                   const Count &wall);
    static Count create_wall(const Round &round, const Player &player,
                             bool enable_reddora);

  private:
    static CountRed encode(const Count &counts, const bool enable_reddora);
    static int distance(const CountRed &hand, const CountRed &origin);
    static void draw(Player &player, CountRed &hand_reds, CountRed &wall_reds,
                     const int tile);
    static void discard(Player &player, CountRed &hand_reds, CountRed &wall_reds,
                        const int tile);
    static int calc_score(const Config &config, const Round &round, Player &player,
                          CountRed &hand_counts, CountRed &wall_counts,
                          const int shanten_type, const int tile, const bool riichi);
    static Vertex draw_node(const Config &config, const Round &round, Player &player,
                            Graph &graph, Cache &cache1, Cache &cache2,
                            CountRed &hand_reds, CountRed &wall_reds,
                            const CountRed &origin_reds, int sht_org,
                            const bool riichi);
    static Vertex discard_node(const Config &config, const Round &round, Player &player,
                               Graph &graph, Cache &cache1, Cache &cache2,
                               CountRed &hand_reds, CountRed &wall_reds,
                               const CountRed &origin_reds, int sht_org,
                               const bool riichi);
    static void calc_stats(const Config &config, Graph &graph, const Cache &cache1,
                           const Cache &cache2);
    static std::tuple<int, std::vector<std::tuple<int, int>>>
    get_necessary_tiles(const Config &config, const Player &player, const Count &wall);
    static bool load_uradora_table();
};
} // namespace mahjong

#endif /* MAHJONG_CPP_EXPECTED_SCORE_CALCULATOR */
