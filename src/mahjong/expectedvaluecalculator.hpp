#ifndef MAHJONG_CPP_EXPECTEDVALUECALCULATOR
#define MAHJONG_CPP_EXPECTEDVALUECALCULATOR

#include "score.hpp"
#include "types/types.hpp"

namespace mahjong {

class Candidate {
public:
    Candidate(int tile, int sum_required_tiles,
              const std::vector<std::tuple<int, int>> &required_tiles,
              std::vector<double> tenpai_probs, std::vector<double> win_probs,
              std::vector<double> exp_values, bool syanten_down)
        : tile(tile)
        , sum_required_tiles(sum_required_tiles)
        , required_tiles(required_tiles)
        , tenpai_probs(tenpai_probs)
        , win_probs(win_probs)
        , exp_values(exp_values)
        , syanten_down(syanten_down)
    {
    }

    /*! 打牌 */
    int tile;

    /*! 巡目ごとの聴牌確率 */
    std::vector<double> tenpai_probs;

    /*! 巡目ごとの和了確率 */
    std::vector<double> win_probs;

    /*! 巡目ごとの期待値 */
    std::vector<double> exp_values;

    /*! 有効牌及び枚数の一覧 */
    std::vector<std::tuple<int, int>> required_tiles;

    /*! 有効牌の合計枚数 */
    int sum_required_tiles;

    /*! 向聴戻しになるかどうか */
    bool syanten_down;
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

struct CacheKey {
    CacheKey(const Hand &hand, const std::vector<int> &counts, int n_extra_tumo)
        : hand(hand)
        , manzu(0)
        , pinzu(0)
        , sozu(0)
        , zihai(0)
        , n_extra_tumo(n_extra_tumo)
    {
        for (size_t i = 0; i < 9; ++i)
            manzu = manzu * 8 + counts[i];
        for (size_t i = 9; i < 18; ++i)
            pinzu = pinzu * 8 + counts[i];
        for (size_t i = 18; i < 27; ++i)
            sozu = sozu * 8 + counts[i];
        for (size_t i = 27; i < 34; ++i)
            zihai = zihai * 8 + counts[i];
    }

    Hand hand;
    int manzu;
    int pinzu;
    int sozu;
    int zihai;
    int n_extra_tumo;
};

inline bool operator<(const CacheKey &lhs, const CacheKey &rhs)
{
    return std::make_tuple(lhs.hand.manzu, lhs.hand.pinzu, lhs.hand.sozu,
                           lhs.hand.zihai, lhs.manzu, lhs.pinzu, lhs.sozu, lhs.zihai,
                           lhs.n_extra_tumo) <
           std::make_tuple(rhs.hand.manzu, rhs.hand.pinzu, rhs.hand.sozu,
                           rhs.hand.zihai, rhs.manzu, rhs.pinzu, rhs.sozu, rhs.zihai,
                           rhs.n_extra_tumo);
}

using CacheValue =
    std::tuple<std::vector<double>, std::vector<double>, std::vector<double>>;

struct DrawTilesCache {
    std::vector<int> hands1;
    std::vector<int> hands2;
};

struct ScoreKey {
    ScoreKey(const Hand &hand, int win_tile)
        : hand(hand)
        , win_tile(win_tile)
    {
    }

    Hand hand;
    int win_tile;
};

struct ScoreCache {
    ScoreCache(double score)
        : score(score)
    {
    }

    double score;
};

inline bool operator<(const Hand &lhs, const Hand &rhs)
{
    return std::make_tuple(lhs.manzu, lhs.pinzu, lhs.sozu, lhs.zihai) <
           std::make_tuple(rhs.manzu, rhs.pinzu, rhs.sozu, rhs.zihai);
}

inline bool operator<(const ScoreKey &lhs, const ScoreKey &rhs)
{
    return std::make_tuple(lhs.hand.manzu, lhs.hand.pinzu, lhs.hand.sozu,
                           lhs.hand.zihai, lhs.win_tile) <
           std::make_tuple(rhs.hand.manzu, rhs.hand.pinzu, rhs.hand.sozu,
                           rhs.hand.zihai, rhs.win_tile);
}

class ExpectedValueCalculator {
public:
    ExpectedValueCalculator();

    std::tuple<bool, std::vector<Candidate>> calc(const Hand &hand,
                                                  const ScoreCalculator &score,
                                                  int syanten_type, int n_extra_tumo);

    std::tuple<int, std::vector<std::tuple<int, int>>>
    get_required_tiles(const Hand &hand, int syanten_type,
                       const std::vector<int> &counts);

private:
    void initialize();
    void clear();

    std::vector<Candidate> analyze(int n_extra_tumo, int syanten, const Hand &hand);
    std::tuple<std::vector<double>, std::vector<double>, std::vector<double>>
    discard(int n_extra_tumo, int syanten, Hand &hand, std::vector<int> &counts);
    std::tuple<std::vector<double>, std::vector<double>, std::vector<double>>
    draw(int n_extra_tumo, int syanten, Hand &hand, std::vector<int> &counts);

    std::vector<int> count_left_tiles(const Hand &hand,
                                      const std::vector<int> &dora_tiles);
    const DrawTilesCache &get_draw_tiles(Hand &hand, int syanten);
    const std::vector<int> &get_discard_tiles(Hand &hand, int syanten);
    const ScoreCache &get_score(const Hand &hand, int win_tile);

private:
    /* 向聴数の種類 */
    int syanten_type_;

    /* 点数計算機 */
    ScoreCalculator score_;

    std::vector<std::map<Hand, std::vector<int>>> discard_cache_;
    std::vector<std::map<Hand, DrawTilesCache>> draw_cache_;
    std::vector<std::map<CacheKey, CacheValue>> discard_cache2_;
    std::vector<std::map<CacheKey, CacheValue>> draw_cache2_;
    std::map<ScoreKey, ScoreCache> score_cache_;

    std::vector<std::vector<double>> tumo_probs_table_;
    std::vector<std::vector<double>> not_tumo_probs_table_;
};

} // namespace mahjong

#endif /* MAHJONG_CPP_EXPECTEDVALUECALCULATOR */
