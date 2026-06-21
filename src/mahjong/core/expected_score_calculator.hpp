#ifndef MAHJONG_CPP_EXPECTED_SCORE_CALCULATOR
#define MAHJONG_CPP_EXPECTED_SCORE_CALCULATOR

#include <array>
#include <cstdint>
#include <tuple>
#include <vector>

#include <boost/graph/adjacency_list.hpp>
#include <boost/unordered/unordered_flat_map.hpp>

#include "mahjong/types/types.hpp"

namespace mahjong
{
using MergedCount = std::array<int, 37>;
using SeparatedCount = std::array<int, 37>;

MergedCount create_wall(const Round &round, const Player &player, bool enable_reddora);

class ExpectedScoreCalculator
{
  public:
    enum class Objective
    {
        TenpaiProbability = 0,
        WinProbability = 1,
        ExpectedScore = 2,
    };

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
            , objective(Objective::ExpectedScore)
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
        /* objective used to select the best discard */
        Objective objective;
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
    static constexpr int MaxTurn = 18;

    struct CacheKey
    {
        CacheKey(const MergedCount &hand, const bool riichi)
            : manzu(0), pinzu(0), souzu(0), honors(0)
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
            honors |= static_cast<int32_t>(riichi) << 24;
        }

        bool operator==(const CacheKey &other) const
        {
            return manzu == other.manzu && pinzu == other.pinzu &&
                   souzu == other.souzu && honors == other.honors;
        }
        int32_t manzu;
        int32_t pinzu;
        int32_t souzu;
        int32_t honors;
    };

    struct CacheKeyHash
    {
        std::size_t operator()(const CacheKey &key) const noexcept
        {
            std::uint64_t h = key.manzu;
            h = h * 0x9e3779b97f4a7c15ULL + key.pinzu;
            h = h * 0x9e3779b97f4a7c15ULL + key.souzu;
            h = h * 0x9e3779b97f4a7c15ULL + key.honors;
            return static_cast<std::size_t>(h);
        }
    };
    struct VertexData
    {
      public:
        VertexData() = default;
        explicit VertexData(const bool tenpai)
            : is_tenpai(tenpai)
        {
        }

        std::array<double, MaxTurn + 1> tenpai_prob{};
        std::array<double, MaxTurn + 1> win_prob{};
        std::array<double, MaxTurn + 1> exp_score{};
        bool is_tenpai = false;
    };

    using EdgeData = std::tuple<int, double>;
    using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS,
                                        VertexData, EdgeData>;
    using Vertex = Graph::vertex_descriptor;
    using Edge = Graph::edge_descriptor;
    using Cache = boost::unordered_flat_map<CacheKey, Vertex, CacheKeyHash>;

  public:
    ExpectedScoreCalculator() = default;

    static std::tuple<std::vector<Stat>, int>
    calc(const Config &config, const Round &round, const Player &player);

    static std::tuple<std::vector<Stat>, int> calc(const Config &config,
                                                   const Round &round,
                                                   const Player &player,
                                                   const MergedCount &wall);

  private:
    static Vertex draw_node(const Config &config, const Round &round, Player &player,
                            Graph &graph, Cache &cache1, Cache &cache2,
                            SeparatedCount &hand_reds, SeparatedCount &wall_reds,
                            const SeparatedCount &origin_reds, int sht_org,
                            const bool riichi);
    static Vertex discard_node(const Config &config, const Round &round, Player &player,
                               Graph &graph, Cache &cache1, Cache &cache2,
                               SeparatedCount &hand_reds, SeparatedCount &wall_reds,
                               const SeparatedCount &origin_reds, int sht_org,
                               const bool riichi);
    static void calc_stats(const Config &config, Graph &graph, const Cache &cache1,
                           const Cache &cache2);
    static std::tuple<int, std::vector<std::tuple<int, int>>>
    get_necessary_tiles(const Config &config, const Player &player,
                        const MergedCount &wall);
};
} // namespace mahjong

#endif /* MAHJONG_CPP_EXPECTED_SCORE_CALCULATOR */
