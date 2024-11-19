#ifndef EXPECTEDVALUECALCULATOR
#define EXPECTEDVALUECALCULATOR

#include "mahjong/core/score_calculator.hpp"
#include "mahjong/types/types.hpp"

#define ENABLE_DRAW_CACHE
#define ENABLE_DISCARD_CACHE
// #define FIX_TEGAWARI_PROB
// #define FIX_SYANTEN_DOWN

namespace mahjong
{

class Candidate
{
  public:
    Candidate(int tile, const std::vector<std::tuple<int, int>> &required_tiles,
              bool syanten_down)
        : tile(tile), required_tiles(required_tiles), syanten_down(syanten_down)
    {
    }

    Candidate(int tile, const std::vector<std::tuple<int, int>> &required_tiles,
              const std::vector<double> &tenpai_probs,
              const std::vector<double> &win_probs,
              const std::vector<double> &exp_values, bool syanten_down)
        : tile(tile)
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

    /*! 向聴戻しになるかどうか */
    bool syanten_down;
};

inline void add_tile(MyPlayer &player, int tile)
{
    if (tile == Tile::RedManzu5) {
        player.hand[Tile::Manzu5]++;
        player.hand[Tile::RedManzu5]++;
    }
    else if (tile == Tile::RedPinzu5) {
        player.hand[Tile::Pinzu5]++;
        player.hand[Tile::RedPinzu5]++;
    }
    else if (tile == Tile::RedSouzu5) {
        player.hand[Tile::Souzu5]++;
        player.hand[Tile::RedSouzu5]++;
    }
    else {
        player.hand[tile]++;
    }
}

inline void remove_tile(MyPlayer &player, int tile)
{
    if (tile == Tile::RedManzu5) {
        player.hand[Tile::Manzu5]--;
        player.hand[Tile::RedManzu5]--;
    }
    else if (tile == Tile::RedPinzu5) {
        player.hand[Tile::Pinzu5]--;
        player.hand[Tile::RedPinzu5]--;
    }
    else if (tile == Tile::RedSouzu5) {
        player.hand[Tile::Souzu5]--;
        player.hand[Tile::RedSouzu5]--;
    }
    else {
        player.hand[tile]--;
    }
}

inline void add_tile(MyPlayer &player, int tile, std::vector<int> &counts)
{
    if (tile == Tile::RedManzu5) {
        player.hand[Tile::Manzu5]++;
        player.hand[Tile::RedManzu5]++;
        counts[Tile::Manzu5]--;
    }
    else if (tile == Tile::RedPinzu5) {
        player.hand[Tile::Pinzu5]++;
        player.hand[Tile::RedPinzu5]++;
        counts[Tile::Pinzu5]--;
    }
    else if (tile == Tile::RedSouzu5) {
        player.hand[Tile::Souzu5]++;
        player.hand[Tile::RedSouzu5]++;
        counts[Tile::Souzu5]--;
    }
    else {
        player.hand[tile]++;
    }
    counts[tile]--;
}

inline void remove_tile(MyPlayer &player, int tile, std::vector<int> &counts)
{
    if (tile == Tile::RedManzu5) {
        player.hand[Tile::Manzu5]--;
        player.hand[Tile::RedManzu5]--;
        counts[Tile::Manzu5]++;
    }
    else if (tile == Tile::RedPinzu5) {
        player.hand[Tile::Pinzu5]--;
        player.hand[Tile::RedPinzu5]--;
        counts[Tile::Pinzu5]++;
    }
    else if (tile == Tile::RedSouzu5) {
        player.hand[Tile::Souzu5]--;
        player.hand[Tile::RedSouzu5]--;
        counts[Tile::Souzu5]++;
    }
    else {
        player.hand[tile]--;
    }
    counts[tile]++;
}

struct CacheKey
{
    CacheKey(const MyPlayer &player, const std::vector<int> &counts, int n_extra_tumo)
        : hmanzu(0)
        , hpinzu(0)
        , hsouzu(0)
        , hhonors(0)
        , manzu(0)
        , pinzu(0)
        , sozu(0)
        , zihai(0)
    {
        hmanzu = std::accumulate(player.hand.begin(), player.hand.begin() + 9, 0,
                                 [](int x, int y) { return x * 8 + y; });
        hpinzu = std::accumulate(player.hand.begin() + 9, player.hand.begin() + 18, 0,
                                 [](int x, int y) { return x * 8 + y; });
        hsouzu = std::accumulate(player.hand.begin() + 18, player.hand.begin() + 27, 0,
                                 [](int x, int y) { return x * 8 + y; });
        hhonors = std::accumulate(player.hand.begin() + 27, player.hand.begin() + 34, 0,
                                  [](int x, int y) { return x * 8 + y; });
        for (size_t i = 0; i < 9; ++i)
            manzu = manzu * 8 + counts[i];
        for (size_t i = 9; i < 18; ++i)
            pinzu = pinzu * 8 + counts[i];
        for (size_t i = 18; i < 27; ++i)
            sozu = sozu * 8 + counts[i];
        for (size_t i = 27; i < 34; ++i)
            zihai = zihai * 8 + counts[i];

        // 未使用のビットをフラグ情報格納に使用する
        zihai |= n_extra_tumo << 21;
        zihai |= counts[Tile::RedManzu5] << 22;
        zihai |= counts[Tile::RedPinzu5] << 23;
        zihai |= counts[Tile::RedSouzu5] << 24;
        zihai |= player.hand[Tile::RedManzu5] << 25;
        zihai |= player.hand[Tile::RedPinzu5] << 26;
        zihai |= player.hand[Tile::RedSouzu5] << 27;
    }

    int hmanzu;
    int hpinzu;
    int hsouzu;
    int hhonors;
    int manzu;
    int pinzu;
    int sozu;
    int zihai;
};

inline bool operator<(const CacheKey &lhs, const CacheKey &rhs)
{
    return std::make_tuple(lhs.hmanzu, lhs.hpinzu, lhs.hsouzu, lhs.hhonors, lhs.manzu,
                           lhs.pinzu, lhs.sozu, lhs.zihai) <
           std::make_tuple(rhs.hmanzu, rhs.hpinzu, rhs.hsouzu, rhs.hhonors, rhs.manzu,
                           rhs.pinzu, rhs.sozu, rhs.zihai);
}

class ExpectedValueCalculator
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
        1, /*! 赤五萬 */
        1, /*! 赤五筒 */
        1, /*! 赤五索 */
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
        CalcAkaTileTumo = 1 << 6, /* 赤牌自摸考慮 */
        MaximaizeWinProb =
            1 << 7, /* 和了確率を最大化 (指定されていない場合は期待値を最大化) */
    };

    Round params;
    void set_params(const Round &params)
    {
        this->params = params;
    }

    ExpectedValueCalculator();

    std::tuple<bool, std::vector<Candidate>>
    calc(const MyPlayer &player, const std::vector<int> &dora_indicators,
         int syanten_type, int flag = 0);
    std::tuple<bool, std::vector<Candidate>>
    calc(const MyPlayer &player, const std::vector<int> &dora_indicators,
         int syanten_type, const std::vector<int> &counts, int flag = 0);

    static std::vector<std::tuple<int, int>>
    get_required_tiles(const MyPlayer &player, int syanten_type,
                       const std::vector<int> &counts);
    static std::vector<int> count_left_tiles(const MyPlayer &player,
                                             const std::vector<int> &dora_indicators);

    // private:
    static bool make_uradora_table();
    void create_prob_table(int n_left_tiles);
    void clear_cache();
    std::vector<std::tuple<int, int, int>>
    get_draw_tiles(MyPlayer &player, int syanten, const std::vector<int> &counts);
    std::vector<std::tuple<int, int>> get_discard_tiles(MyPlayer &player, int syanten);
    std::vector<double> get_score(const MyPlayer &player, int win_tile,
                                  const std::vector<int> &counts);

    std::vector<Candidate> analyze_discard(int n_extra_tumo, int syanten,
                                           MyPlayer player, std::vector<int> counts);
    std::vector<Candidate> analyze_discard(int syanten, MyPlayer player,
                                           std::vector<int> counts);
    std::vector<Candidate> analyze_draw(int n_extra_tumo, int syanten, MyPlayer player,
                                        std::vector<int> counts);
    std::vector<Candidate> analyze_draw(int syanten, MyPlayer player,
                                        std::vector<int> counts);

    std::tuple<std::vector<double>, std::vector<double>, std::vector<double>>
    discard(int n_extra_tumo, int syanten, MyPlayer &player, std::vector<int> &counts);
    std::tuple<std::vector<double>, std::vector<double>, std::vector<double>>
    draw(int n_extra_tumo, int syanten, MyPlayer &player, std::vector<int> &counts);
    std::tuple<std::vector<double>, std::vector<double>, std::vector<double>>
    draw_without_tegawari(int n_extra_tumo, int syanten, MyPlayer &player,
                          std::vector<int> &counts);
    std::tuple<std::vector<double>, std::vector<double>, std::vector<double>>
    draw_with_tegawari(int n_extra_tumo, int syanten, MyPlayer &player,
                       std::vector<int> &counts);

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

    /* 赤牌自摸考慮 */
    bool calc_akatile_tumo_;

    /* 和了確率を最大化 */
    bool maximize_win_prob_;

    /* 自摸回数の最大値 */
    int max_tumo_;

    /* この巡目で有効牌を引ける確率のテーブル */
    std::vector<std::vector<double>> tumo_prob_table_;

    /* これまでの巡目で有効牌が引けなかった確率のテーブル */
    std::vector<std::vector<double>> not_tumo_prob_table_;

    /* 裏ドラの乗る確率のテーブル */
    static std::vector<std::vector<double>> uradora_prob_table_;

    int N_;

    /* キャッシュ */
    using CacheValue =
        std::tuple<std::vector<double>, std::vector<double>, std::vector<double>>;
    std::vector<std::map<CacheKey, CacheValue>> discard_cache_;
    std::vector<std::map<CacheKey, CacheValue>> draw_cache_;
};

} // namespace mahjong

#endif // EXPECTEDVALUECALCULATOR
