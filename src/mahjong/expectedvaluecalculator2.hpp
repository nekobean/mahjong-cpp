#ifndef MAHJONG_CPP_EXPECTEDVALUECALCULATOR2
#define MAHJONG_CPP_EXPECTEDVALUECALCULATOR2

#include "mahjong/expectedvaluecalculator.hpp"
#include "mahjong/score.hpp"
#include "mahjong/types/types.hpp"
#include <boost/functional/hash.hpp>

namespace mahjong
{

class ExpectedValueCalculator2
{
    /**
     * @brief 期待値 (和了確率) が同じ場合はこの値が高い牌を優先して打牌する。
     */
    static inline const std::vector<int> DiscardPriorities = {
        5, /*! 一萬 */
        4, /*! 二萬 */
        3, /*! 三萬 */
        2, /*! 四萬 */
        1, /*! 五萬 */
        2, /*! 六萬 */
        3, /*! 七萬 */
        4, /*! 八萬 */
        5, /*! 九萬 */
        5, /*! 一筒 */
        4, /*! 二筒 */
        3, /*! 三筒 */
        2, /*! 四筒 */
        1, /*! 五筒 */
        2, /*! 六筒 */
        3, /*! 七筒 */
        4, /*! 八筒 */
        5, /*! 九筒 */
        5, /*! 一索 */
        4, /*! 二索 */
        3, /*! 三索 */
        2, /*! 四索 */
        1, /*! 五索 */
        2, /*! 六索 */
        3, /*! 七索 */
        4, /*! 八索 */
        5, /*! 九索 */
        5, /*! 東 */
        5, /*! 南 */
        5, /*! 西 */
        5, /*! 北 */
        5, /*! 白 */
        5, /*! 発 */
        5, /*! 中 */
    };

  public:
    enum Flag
    {
        Null = 0,
        CalcSyantenDown = 1,      /* 向聴落とし考慮 */
        CalcTegawari = 1 << 1,    /* 手変わり考慮 */
        CalcDoubleReach = 1 << 2, /* ダブル立直考慮 */
        CalcIppatu = 1 << 3,      /* 一発考慮 */
        CalcHaiteitumo = 1 << 4,  /* 海底撈月考慮 */
        CalcUradora = 1 << 5,     /* 裏ドラ考慮 */
        MaximaizeWinProb = 1 << 6, /* 和了確率を最大化 (指定されていない場合は期待値を最大化) */
    };

    ExpectedValueCalculator2();

    std::tuple<bool, std::vector<Candidate>> calc(const Hand &hand,
                                                  const ScoreCalculator &score_calculator,
                                                  const std::vector<int> &dora_indicators,
                                                  int syanten_type, int turn, int flag = 0);
    static std::vector<std::tuple<int, int>> get_required_tiles(const Hand &hand, int syanten_type,
                                                                const std::vector<int> &counts);
    static std::vector<int> count_left_tiles(const Hand &hand,
                                             const std::vector<int> &dora_indicators);

    // private:
    static bool make_uradora_table();
    void create_prob_table(int n_left_tiles);
    void clear_cache();
    std::vector<std::tuple<int, int, int>> get_draw_tiles(Hand &hand, int syanten,
                                                          const std::vector<int> &counts);
    std::vector<int> get_discard_tiles(Hand &hand, int syanten);
    std::vector<double> get_score(const Hand &hand, int win_tile, const std::vector<int> &counts);

    std::vector<Candidate> analyze(int n_extra_tumo, int syanten, const Hand &hand, int turn);
    std::vector<Candidate> analyze(int syanten, const Hand &hand);

    std::tuple<double, double, double> discard(int n_extra_tumo, int syanten, Hand &hand,
                                               std::vector<int> &counts, int turn);
    std::tuple<double, double, double> draw(int n_extra_tumo, int syanten, Hand &hand,
                                            std::vector<int> &counts, int turn);
    std::tuple<double, double, double> draw_without_tegawari(int n_extra_tumo, int syanten,
                                                             Hand &hand, std::vector<int> &counts,
                                                             int turn);
    std::tuple<double, double, double> draw_with_tegawari(int n_extra_tumo, int syanten, Hand &hand,
                                                          std::vector<int> &counts, int turn);

    // private:
    /* 点数計算機 */
    ScoreCalculator score_calculator_;

    /* 向聴数の種類 */
    int syanten_type_;

    /* ドラ表示牌 */
    std::vector<int> dora_indicators_;

    /* 向聴落とし考慮 */
    bool calc_syanten_down_;

    /* 手変わり考慮 */
    bool calc_tegawari_;

    /* ダブル立直考慮 */
    bool calc_double_reach_;

    /* 一発考慮 */
    bool calc_ippatu_;

    /* 海底撈月考慮 */
    bool calc_haitei_;

    /* 裏ドラ考慮 */
    bool calc_uradora_;

    /* 和了確率を最大化 */
    bool maximize_win_prob_;

    /* この巡目で有効牌を引ける確率のテーブル */
    std::vector<std::vector<double>> tumo_prob_table_;

    /* これまでの巡目で有効牌が引けなかった確率のテーブル */
    std::vector<std::vector<double>> not_tumo_prob_table_;

    /* 裏ドラの乗る確率のテーブル */
    static std::vector<std::vector<double>> uradora_prob_table_;

    /* キャッシュ */
    using CacheValue = std::tuple<int, int, int>;
    std::vector<std::map<CacheKey, CacheValue>> discard_cache_;
    std::vector<std::map<CacheKey, CacheValue>> draw_cache_;
};

} // namespace mahjong

#endif /* MAHJONG_CPP_EXPECTEDVALUECALCULATOR2 */
