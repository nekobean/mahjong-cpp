#ifndef EXPECTEDVALUECALCULATOR
#define EXPECTEDVALUECALCULATOR

#include "score.hpp"
#include "types/types.hpp"
#include <boost/functional/hash.hpp>

#define ENABLE_DRAW_CACHE
#define ENABLE_DISCARD_CACHE
// #define FIX_TEGAWARI_PROB
// #define FIX_SYANTEN_DOWN

namespace mahjong
{

class Candidate
{
  public:
    Candidate(int tile, const std::vector<std::tuple<int, int>> &required_tiles)
        : tile(tile), required_tiles(required_tiles), syanten_down(false)
    {
    }

    Candidate(int tile, const std::vector<std::tuple<int, int>> &required_tiles,
              std::vector<double> tenpai_probs, std::vector<double> win_probs,
              std::vector<double> exp_values, bool syanten_down)
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

inline void add_tile(Hand &hand, int tile)
{
    if (tile <= Tile::Manzu9)
        hand.manzu += Bit::tile1[tile];
    else if (tile <= Tile::Pinzu9)
        hand.pinzu += Bit::tile1[tile];
    else if (tile <= Tile::Sozu9)
        hand.sozu += Bit::tile1[tile];
    else if (tile <= Tile::Tyun)
        hand.zihai += Bit::tile1[tile];
    else if (tile == Tile::AkaManzu5) {
        hand.manzu += Bit::tile1[Tile::Manzu5];
        hand.aka_manzu5 = true;
    }
    else if (tile == Tile::AkaPinzu5) {
        hand.pinzu += Bit::tile1[Tile::Pinzu5];
        hand.aka_pinzu5 = true;
    }
    else if (tile == Tile::AkaSozu5) {
        hand.sozu += Bit::tile1[Tile::Sozu5];
        hand.aka_sozu5 = true;
    }
}

inline void remove_tile(Hand &hand, int tile)
{
    if (tile <= Tile::Manzu9)
        hand.manzu -= Bit::tile1[tile];
    else if (tile <= Tile::Pinzu9)
        hand.pinzu -= Bit::tile1[tile];
    else if (tile <= Tile::Sozu9)
        hand.sozu -= Bit::tile1[tile];
    else if (tile <= Tile::Tyun)
        hand.zihai -= Bit::tile1[tile];
    else if (tile == Tile::AkaManzu5) {
        hand.manzu -= Bit::tile1[Tile::Manzu5];
        hand.aka_manzu5 = false;
    }
    else if (tile == Tile::AkaPinzu5) {
        hand.pinzu -= Bit::tile1[Tile::Pinzu5];
        hand.aka_pinzu5 = false;
    }
    else if (tile == Tile::AkaSozu5) {
        hand.sozu -= Bit::tile1[Tile::Sozu5];
        hand.aka_sozu5 = false;
    }
}

inline void add_tile(Hand &hand, int tile, std::vector<int> &counts)
{
    counts[tile]--;
    if (tile <= Tile::Manzu9)
        hand.manzu += Bit::tile1[tile];
    else if (tile <= Tile::Pinzu9)
        hand.pinzu += Bit::tile1[tile];
    else if (tile <= Tile::Sozu9)
        hand.sozu += Bit::tile1[tile];
    else if (tile <= Tile::Tyun)
        hand.zihai += Bit::tile1[tile];
    else if (tile == Tile::AkaManzu5) {
        hand.manzu += Bit::tile1[Tile::Manzu5];
        hand.aka_manzu5 = true;
        counts[Tile::Manzu5] -= 1;
    }
    else if (tile == Tile::AkaPinzu5) {
        hand.pinzu += Bit::tile1[Tile::Pinzu5];
        hand.aka_pinzu5 = true;
        counts[Tile::Pinzu5] -= 1;
    }
    else if (tile == Tile::AkaSozu5) {
        hand.sozu += Bit::tile1[Tile::Sozu5];
        hand.aka_sozu5 = true;
        counts[Tile::Sozu5] -= 1;
    }
}

inline void remove_tile(Hand &hand, int tile, std::vector<int> &counts)
{
    counts[tile]++;
    if (tile <= Tile::Manzu9)
        hand.manzu -= Bit::tile1[tile];
    else if (tile <= Tile::Pinzu9)
        hand.pinzu -= Bit::tile1[tile];
    else if (tile <= Tile::Sozu9)
        hand.sozu -= Bit::tile1[tile];
    else if (tile <= Tile::Tyun)
        hand.zihai -= Bit::tile1[tile];
    else if (tile == Tile::AkaManzu5) {
        hand.manzu -= Bit::tile1[Tile::Manzu5];
        hand.aka_manzu5 = false;
        counts[Tile::Manzu5] += 1;
    }
    else if (tile == Tile::AkaPinzu5) {
        hand.pinzu -= Bit::tile1[Tile::Pinzu5];
        hand.aka_pinzu5 = false;
        counts[Tile::Pinzu5] += 1;
    }
    else if (tile == Tile::AkaSozu5) {
        hand.sozu -= Bit::tile1[Tile::Sozu5];
        hand.aka_sozu5 = false;
        counts[Tile::Sozu5] += 1;
    }
}

struct CacheKey
{
    CacheKey(const Hand &hand, const std::vector<int> &counts, int n_extra_tumo)
        : hand(hand), manzu(0), pinzu(0), sozu(0), zihai(0)
    {
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
        zihai |= counts[Tile::AkaManzu5] << 22;
        zihai |= counts[Tile::AkaPinzu5] << 23;
        zihai |= counts[Tile::AkaSozu5] << 24;
        zihai |= hand.aka_manzu5 << 25;
        zihai |= hand.aka_pinzu5 << 26;
        zihai |= hand.aka_sozu5 << 27;
    }

    Hand hand;
    int manzu;
    int pinzu;
    int sozu;
    int zihai;
};

inline bool operator<(const CacheKey &lhs, const CacheKey &rhs)
{
    return std::make_tuple(lhs.hand.manzu, lhs.hand.pinzu, lhs.hand.sozu, lhs.hand.zihai, lhs.manzu,
                           lhs.pinzu, lhs.sozu, lhs.zihai) <
           std::make_tuple(rhs.hand.manzu, rhs.hand.pinzu, rhs.hand.sozu, rhs.hand.zihai, rhs.manzu,
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
        MaximaizeWinProb = 1 << 7, /* 和了確率を最大化 (指定されていない場合は期待値を最大化) */
    };

    ExpectedValueCalculator();

    std::tuple<bool, std::vector<Candidate>> calc(const Hand &hand,
                                                  const ScoreCalculator &score_calculator,
                                                  const std::vector<int> &dora_indicators,
                                                  int syanten_type, int flag = 0);
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

    std::vector<Candidate> analyze(int n_extra_tumo, int syanten, const Hand &hand);
    std::vector<Candidate> analyze(int syanten, const Hand &hand);

    std::tuple<std::vector<double>, std::vector<double>, std::vector<double>>
    discard(int n_extra_tumo, int syanten, Hand &hand, std::vector<int> &counts);
    std::tuple<std::vector<double>, std::vector<double>, std::vector<double>>
    draw(int n_extra_tumo, int syanten, Hand &hand, std::vector<int> &counts);
    std::tuple<std::vector<double>, std::vector<double>, std::vector<double>>
    draw_without_tegawari(int n_extra_tumo, int syanten, Hand &hand, std::vector<int> &counts);
    std::tuple<std::vector<double>, std::vector<double>, std::vector<double>>
    draw_with_tegawari(int n_extra_tumo, int syanten, Hand &hand, std::vector<int> &counts);

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

    /* この巡目で有効牌を引ける確率のテーブル */
    std::vector<std::vector<double>> tumo_prob_table_;

    /* これまでの巡目で有効牌が引けなかった確率のテーブル */
    std::vector<std::vector<double>> not_tumo_prob_table_;

    /* 裏ドラの乗る確率のテーブル */
    static std::vector<std::vector<double>> uradora_prob_table_;

    /* キャッシュ */
    using CacheValue = std::tuple<std::vector<double>, std::vector<double>, std::vector<double>>;
    std::vector<std::map<CacheKey, CacheValue>> discard_cache_;
    std::vector<std::map<CacheKey, CacheValue>> draw_cache_;
};

} // namespace mahjong

#endif // EXPECTEDVALUECALCULATOR
