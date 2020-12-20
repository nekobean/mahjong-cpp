#ifndef MAHJONG_CPP_EXPECTEDVALUECALCULATOR
#define MAHJONG_CPP_EXPECTEDVALUECALCULATOR

#include <boost/graph/adjacency_list.hpp>

#include "requiredtileselector.hpp"
#include "score.hpp"
#include "syanten.hpp"
#include "types/types.hpp"
#include "unnecessarytileselector.hpp"

namespace mahjong {

class Candidate {
public:
    Candidate(int tile, double tenpai_prob, double win_prob, double win_exp)
        : tile(tile)
        , tenpai_prob(tenpai_prob)
        , win_prob(win_prob)
        , win_exp(win_exp)
    {
    }

    int tile;
    double tenpai_prob;
    double win_prob;
    double win_exp;
};

/**
 * @brief ノード
 */
class NodeData {
public:
    NodeData(const Hand &hand, int syanten)
        : hand(hand)
        , syanten(syanten)
    {
    }

    virtual ~NodeData() = default;

    /*! 手牌 */
    Hand hand;

    /*! 向聴数 */
    int syanten;
};

/**
 * @brief 葉ノード
 */
class LeafData : public NodeData {
public:
    LeafData(const Hand &hand, int syanten, const Result &result)
        : NodeData(hand, syanten)
        , result(result)
    {
    }

    /*! 点数計算結果 */
    Result result;
};

/**
 * @brief エッジ
 */
class EdgeData {
public:
    virtual ~EdgeData() = default;
};

/**
 * @brief 打牌
 */
class DiscardData : public EdgeData {
public:
    DiscardData(int tile)
        : tile(tile)
    {
    }

    /*! 牌 */
    int tile;
};

/**
 * @brief 自摸
 */
class DrawData : public EdgeData {
public:
    DrawData(int tile, int num_left_tiles, int total_left_tiles)
        : tile(tile)
        , num_left_tiles(num_left_tiles)
        , total_left_tiles(total_left_tiles)
    {
    }

    /*! 牌 */
    int tile;

    /* その牌の残り枚数 */
    int num_left_tiles;

    /* 有効牌の合計枚数 */
    int total_left_tiles;
};

inline void add_tile(Hand &hand, int tile)
{
    if (tile <= Tile::Manzu9)
        hand.manzu += Bit::tile1[tile];
    else if (tile <= Tile::Pinzu9)
        hand.pinzu += Bit::tile1[tile];
    else if (tile <= Tile::Sozu9)
        hand.sozu += Bit::tile1[tile];
    else
        hand.zihai += Bit::tile1[tile];
}

inline void remove_tile(Hand &hand, int tile)
{
    if (tile <= Tile::Manzu9)
        hand.manzu -= Bit::tile1[tile];
    else if (tile <= Tile::Pinzu9)
        hand.pinzu -= Bit::tile1[tile];
    else if (tile <= Tile::Sozu9)
        hand.sozu -= Bit::tile1[tile];
    else
        hand.zihai -= Bit::tile1[tile];
}

using Graph =
    boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS,
                          std::shared_ptr<NodeData>, std::shared_ptr<EdgeData>>;

class ExpectedValueCalculator {

public:
    std::vector<int> count_left_tiles(const Hand &hand,
                                      const std::vector<int> &dora_tiles);
    int count_num_required_tiles(const std::vector<int> &count,
                                 const std::vector<int> &tiles);

    void calc(const Hand &hand, const ScoreCalculator &score, int syanten_type);
    void discard(Graph &G, Graph::vertex_descriptor parent, int syanten);
    void draw(Graph &G, Graph::vertex_descriptor parent, int syanten);
    std::vector<std::tuple<Hand, Result>> get_win_hands();
    double discard(const Graph &G, Graph::vertex_descriptor parent,
                   int total_left_tiles, int turn);
    double draw(const Graph &G, Graph::vertex_descriptor parent, int total_left_tiles,
                int turn);
    std::vector<Candidate> select(const Graph &G, Graph::vertex_descriptor parent,
                                  int total_left_tiles, int turn);

    Graph &graph();

private:
    Hand hand_;
    std::vector<int> counts_;
    ScoreCalculator score_;
    int syanten_type_;
    Graph G_;

    std::map<Hand, Graph::vertex_descriptor> vert_cache_;
};

} // namespace mahjong

#endif /* MAHJONG_CPP_EXPECTEDVALUECALCULATOR */
