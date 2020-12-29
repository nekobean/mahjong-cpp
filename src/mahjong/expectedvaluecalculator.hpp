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
    Candidate(int tile, int total_required_tiles,
              const std::vector<std::tuple<int, int>> &required_tiles,
              double tenpai_prob, double win_prob, double win_exp, bool syanten_down)
        : tile(tile)
        , total_required_tiles(total_required_tiles)
        , required_tiles(required_tiles)
        , tenpai_prob(tenpai_prob)
        , win_prob(win_prob)
        , win_exp(win_exp)
        , syanten_down(syanten_down)
    {
    }

    int tile;
    double tenpai_prob;
    double win_prob;
    double win_exp;
    std::vector<std::tuple<int, int>> required_tiles;
    int total_required_tiles;
    bool syanten_down;
};

/**
 * @brief ノード
 */
class NodeData {
public:
    virtual ~NodeData() = default;
};

/**
 * @brief 手牌
 */
class HandData : public NodeData {
public:
    HandData(const Hand &hand, int syanten, double exp)
        : hand(hand)
        , syanten(syanten)
        , exp(exp)
    {
    }

    /*! 手牌 */
    Hand hand;

    /*! 向聴数 */
    int syanten;

    /* 期待値 */
    double exp;
};

/**
 * @brief 点数
 */
class ScoreData : public NodeData {
public:
    ScoreData(const Result &result)
        : result(result)
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

struct VertCache {
    VertCache(const Hand &hand, int turn)
        : hand(hand)
        , turn(turn)
    {
    }

    bool operator<(const VertCache &rhs) const
    {
        return std::make_tuple(hand.manzu, hand.pinzu, hand.sozu, hand.zihai, turn) <
               std::make_tuple(rhs.hand.manzu, rhs.hand.pinzu, rhs.hand.sozu,
                               rhs.hand.zihai, rhs.turn);
    }

    Hand hand;
    int turn;
};

struct DiscardCache {
    std::vector<int> hands1;
    std::vector<int> hands2;
};

struct DrawCache {
    std::vector<int> hands1;
    std::vector<int> hands2;
};

class ExpectedValueCalculator {

public:
    ExpectedValueCalculator();
    void initialize();

    std::vector<int> count_left_tiles(const Hand &hand,
                                      const std::vector<int> &dora_tiles);
    int count_num_required_tiles(const std::vector<int> &count,
                                 const std::vector<int> &tiles);

    void calc(const Hand &hand, const ScoreCalculator &score, int syanten_type);

    void build_tree_discard_first(int n_left_tumo, int syanten);
    std::tuple<std::vector<double>, std::vector<double>, std::vector<double>>
    build_tree_discard(int n_left_tumo, int syanten, int tumo_tile);
    std::tuple<std::vector<double>, std::vector<double>, std::vector<double>>
    build_tree_draw(int n_left_tumo, int syanten);

private:
    Hand hand_;
    std::vector<int> counts_;
    ScoreCalculator score_;
    int syanten_type_;

    std::map<VertCache, double> vert_cache_;

    std::map<int, int> num_eval_hands_;
    std::map<int, int> num_actual_eval_hands_;

    std::map<Hand, DiscardCache> discard_cache_;
    std::map<Hand, DrawCache> draw_cache_;
    DrawCache get_draw_tiles(Hand &hand, int syanten);
    DiscardCache get_discard_tiles(Hand &hand, int syanten);
    std::vector<std::vector<double>> tumo_probs_table_;
    std::vector<std::vector<double>> not_tumo_probs_table_;
};

} // namespace mahjong

#endif /* MAHJONG_CPP_EXPECTEDVALUECALCULATOR */
