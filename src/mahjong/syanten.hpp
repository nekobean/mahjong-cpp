#ifndef MAHJONG_CPP_SYANTEN
#define MAHJONG_CPP_SYANTEN

#include "types/types.hpp"

//#define USE_UNORDERED_MAP // テーブルに std::unordered_map を使う場合

namespace mahjong
{

/**
 * @brief 向聴数の種類
 */
namespace SyantenType
{

enum
{
    Null = 0,
    Normal = 1, /* 通常手 */
    Tiitoi = 2, /* 七対子手 */
    Kokusi = 4, /* 国士無双手 */
};

static inline std::map<int, std::string> Name = {
    {SyantenType::Normal, "通常手"},
    {SyantenType::Tiitoi, "七対子手"},
    {SyantenType::Kokusi, "国士無双手"},
};

}; // namespace SyantenType

class SyantenCalculator
{
    /**
     * @brief テーブルの情報
     */
    struct Pattern
    {
        /*! 面子の数 */
        signed char n_mentu;
        /*! 面子候補の数 */
        signed char n_kouho;
        /*! 頭あり */
        signed char head;
        /*! 頭ありの場合の面子の数の変化数 */
        signed char n_mentu_diff;
        /*! 頭ありの場合の面子候補の数の変化数 */
        signed char n_kouho_diff;
        /*! 1枚以上の数 */
        signed char n_ge1;
        /*! 2枚以上の数 */
        signed char n_ge2;
        /*! 3枚以上の数 */
        signed char n_ge3;
        /*! 4枚以上の数 */
        signed char n_ge4;
        /*! 合計数 */
        signed char n;
    };

  public:
    SyantenCalculator();
    static std::tuple<int, int> calc(const Hand &hand, int type = SyantenType::Normal |
                                                                  SyantenType::Tiitoi |
                                                                  SyantenType::Kokusi);
    static bool initialize();
    static int calc_normal(const Hand &hand);
    static int calc_tiitoi(const Hand &hand);
    static int calc_kokusi(const Hand &hand);

#ifdef USE_UNORDERED_MAP
    /*! 数牌のテーブル */
    static std::unordered_map<key_type, Pattern> s_tbl_;
    /*! 字牌のテーブル */
    static std::unordered_map<key_type, Pattern> z_tbl_;
#else
    /*! 数牌のテーブル */
    static std::vector<Pattern> s_tbl_;
    /*! 字牌のテーブル */
    static std::vector<Pattern> z_tbl_;
#endif

  private:
    /*! 数牌のテーブルサイズ */
    static const size_t ShuupaiTableSize = 76611584 + 1; // ハッシュ値の最大値 + 1
    /*! 字牌のテーブルサイズ */
    static const size_t ZihaiTableSize = 1197056 + 1; // ハッシュ値の最大値 + 1
    /*! 数牌のパターン数 */
    static const size_t ShuupaiPatternSize = 405350;
    /*! 字牌のパターン数 */
    static const size_t ZihaiPatternSize = 43130;
};

} // namespace mahjong

#endif /* MAHJONG_CPP_SYANTEN */
