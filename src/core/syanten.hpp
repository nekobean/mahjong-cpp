#ifndef MAHJONG_CPP_SYANTEN
#define MAHJONG_CPP_SYANTEN

#include "types/types.hpp"

namespace mahjong {

/**
 * @brief 向聴の種類
 */
struct SyantenType {
    enum Type {
        Null   = 0,
        Normal = 1, /* 通常手 */
        Tiitoi = 2, /* 七対子手 */
        Kokusi = 4, /* 国士無双手 */
    };

    static inline std::map<int, std::string> Names = {
        {SyantenType::Normal, "通常手"},
        {SyantenType::Tiitoi, "七対子手"},
        {SyantenType::Kokusi, "国士無双手"},
    };
};

class SyantenCalculator {
    /**
     * @brief テーブルの情報
     */
    struct Pattern {
        Pattern()
            : n_mentu(-1)
            , n_kouho(-1)
        {
        }

        Pattern(char n_mentu, char n_kouho)
            : n_mentu(n_mentu)
            , n_kouho(n_kouho)
        {
        }

        /*! 面子の数 */
        char n_mentu;
        /*! 面子候補の数 */
        char n_kouho;
        /*! 1枚以上の数 */
        char n_ge1;
        /*! 2枚以上の数 */
        char n_ge2;
        /*! 3枚以上の数 */
        char n_ge3;
        /*! 4枚以上の数 */
        char n_ge4;
    };

public:
    static std::tuple<int, int> calc(const Hand &tehai, int type = SyantenType::Normal |
                                                                   SyantenType::Tiitoi |
                                                                   SyantenType::Kokusi);
    static bool initialize();
    static int calc_normal(const Hand &tehai);
    static int calc_tiitoi(const Hand &tehai);
    static int calc_kokusi(const Hand &tehai);

    /*! 数牌のテーブル */
    static std::vector<Pattern> s_tbl_;
    /*! 字牌のテーブル */
    static std::vector<Pattern> z_tbl_;

private:
    static bool make_table(const std::string &path, std::vector<Pattern> &table);

    /*! 数牌のテーブルサイズ */
    static const int ShuupaiTableSize = 76611584 + 1; // ハッシュ値の最大値 + 1
    /*! 字牌のテーブルサイズ */
    static const int ZihaiTableSize = 1197056 + 1; // ハッシュ値の最大値 + 1
};

} // namespace mahjong

#endif /* MAHJONG_CPP_SYANTEN */
