#ifndef MAHJONG_CPP_EXPECTED_SCORE_CALCULATOR
#define MAHJONG_CPP_EXPECTED_SCORE_CALCULATOR

#include <array>
#include <cstdint>
#include <limits>
#include <tuple>
#include <vector>

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
        /* min turn to be calculated */
        int t_min = 1;
        /* max turn to be calculated */
        int t_max = 18;
        /* number of wall tiles */
        int sum = 0;
        /* search the range of possible (shanten number + extra) exchanges */
        int extra = 1;
        /* calculation shanten type */
        int shanten_type = ShantenFlag::All;
        /* enable red dora */
        bool enable_reddora = true;
        /* enable ura dora */
        bool enable_uradora = true;
        /* allow shanten down */
        bool enable_shanten_down = true;
        /* allow tegawari */
        bool enable_tegawari = true;
        /* objective used to select the best discard */
        Objective objective = Objective::ExpectedScore;
        /* calculate value */
        bool calc_stats = true;
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
        explicit VertexData(const bool tenpai) : is_tenpai(tenpai)
        {
        }

        std::array<double, MaxTurn + 1> tenpai_prob{};
        std::array<double, MaxTurn + 1> win_prob{};
        std::array<double, MaxTurn + 1> exp_score{};
        bool is_tenpai = false;
    };

    using Vertex = std::uint32_t;

    struct EdgeData
    {
        Vertex source;
        Vertex target;
        std::uint32_t next_out;
        std::uint32_t next_in;
        int weight;
        double score;
    };

    struct Graph
    {
        static constexpr std::uint32_t NoEdge =
            std::numeric_limits<std::uint32_t>::max();

        Vertex add_vertex()
        {
            const Vertex vertex = static_cast<Vertex>(vertices.size());
            vertices.emplace_back();
            first_out_edges.push_back(NoEdge);
            first_in_edges.push_back(NoEdge);
            return vertex;
        }

        void add_edge(const Vertex source, const Vertex target, const int weight,
                      const double score)
        {
            const auto edge = static_cast<std::uint32_t>(edges.size());
            edges.push_back(EdgeData{source, target, first_out_edges[source],
                                     first_in_edges[target], weight, score});
            first_out_edges[source] = edge;
            first_in_edges[target] = edge;
        }

        bool has_edge(const Vertex source, const Vertex target) const
        {
            for (std::uint32_t edge = first_out_edges[source]; edge != NoEdge;
                 edge = edges[edge].next_out) {
                if (edges[edge].target == target) {
                    return true;
                }
            }
            return false;
        }

        std::size_t num_vertices() const
        {
            return vertices.size();
        }

        VertexData &operator[](const Vertex vertex)
        {
            return vertices[vertex];
        }

        const VertexData &operator[](const Vertex vertex) const
        {
            return vertices[vertex];
        }

        std::vector<VertexData> vertices;
        std::vector<EdgeData> edges;
        std::vector<std::uint32_t> first_out_edges;
        std::vector<std::uint32_t> first_in_edges;
    };

    using Cache = boost::unordered_flat_map<CacheKey, Vertex, CacheKeyHash>;

    struct DrawEdge
    {
        std::uint32_t target;
        int weight;
        double score;
    };

    struct SelectionEdge
    {
        std::uint32_t source;
    };

    struct EdgeCsr
    {
        std::vector<DrawEdge> draw_edges;
        std::vector<SelectionEdge> selection_edges;
        std::vector<std::uint32_t> draw_edge_offsets;
        std::vector<std::uint32_t> selection_edge_offsets;
    };

  public:
    ExpectedScoreCalculator() = default;

    static std::tuple<std::vector<Stat>, int>
    calc(const Config &config, const Round &round, const Player &player);

    static std::tuple<std::vector<Stat>, int> calc(const Config &config,
                                                   const Round &round,
                                                   const Player &player,
                                                   const MergedCount &wall);

  private:
    class GraphBuilder;

    static void calc_draw_hand(const Config &config, const Player &player,
                               const MergedCount &wall,
                               const SeparatedCount &hand_counts,
                               GraphBuilder &graph_builder, std::vector<Stat> &stats);
    static void calc_discard_hand(const Config &config, Player &player,
                                  const MergedCount &wall, SeparatedCount &hand_counts,
                                  SeparatedCount &wall_counts,
                                  GraphBuilder &graph_builder,
                                  std::vector<Stat> &stats);
    static EdgeCsr build_edge_csr(const Graph &graph);
    static void calc_stats(const Config &config, Graph &graph,
                           const std::vector<Vertex> &draw_vertices,
                           const std::vector<Vertex> &discard_vertices,
                           const EdgeCsr &edge_csr);
};
} // namespace mahjong

#endif /* MAHJONG_CPP_EXPECTED_SCORE_CALCULATOR */
