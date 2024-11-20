#ifndef MAHJONG_CPP_EXPECTED_SCORE_CALCULATOR
#define MAHJONG_CPP_EXPECTED_SCORE_CALCULATOR

#include <map>
#include <tuple>
#include <vector>

#include "compare/player_impl.hpp"
#include "compare/utils.hpp"

#include <boost/graph/adjacency_list.hpp>

#include "mahjong/types/types.hpp"

namespace mahjong
{
class ExpectedScoreCalculator
{
  public:
    struct Params
    {
        int t_min = 0; // 計算対象の巡目の最小値
        int t_max = 0; // 計算対象の巡目の最大値
        int sum = 0;   // 山の枚数
        int extra = 0; // 現在の手牌からこの回数分は牌を交換できるものとする
        int mode = 7;                     // 計算対象の向聴数の種類
        bool enable_uradora = true;       // 裏ドラを考慮するか
        bool enable_reddora = true;       // 赤ドラを考慮するか
        bool enable_shanten_down = true;  // 向聴落としを考慮するか
        bool enable_tegawari = true;      // 手替わりを考慮するか
        bool enable_double_riichi = true; // ダブル立直を考慮するか
        bool enable_ippatsu = true;       // 一発を考慮するか
        bool enable_under_the_sea = true; // 海底撈月を考慮するか
    };

    struct Stat
    {
        const int tile;                        // 打牌
        const bool is_red;                     // 赤ドラか
        const std::vector<double> tenpai_prob; // 和了確率
        const std::vector<double> win_prob;    // 和了確率
        const std::vector<double> exp_value;   // 和了確率
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
    std::tuple<std::vector<Stat>, std::size_t> calc(const Round &round, Player &player,
                                                    const Params &params);

  private:
    void draw(Player &player, std::vector<int> &hand_reds, std::vector<int> &wall_reds,
              const int tile) const;
    void discard(Player &player, std::vector<int> &hand_reds,
                 std::vector<int> &wall_reds, const int tile) const;
    int calc_score(const Params &params, Player &player, const int mode,
                   const int tile) const;

    Vertex select1(Graph &graph, Desc &cache1, Desc &cache2,
                   std::vector<int> &hand_reds, std::vector<int> &wall_reds,
                   Player &player, const std::vector<int> &origin_reds, int sht_org,
                   const Params &params) const;
    Vertex select2(Graph &graph, Desc &cache1, Desc &cache2,
                   std::vector<int> &hand_reds, std::vector<int> &wall_reds,
                   Player &player, const std::vector<int> &origin_reds, int sht_org,
                   const Params &params) const;
    void update(Graph &graph, const Desc &cache1, const Desc &cache2,
                const Params &params) const;

  private:
    Round round_;
};
} // namespace mahjong

#endif
