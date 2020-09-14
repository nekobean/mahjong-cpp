#ifndef MAHJONG_CPP_SYANTEN
#define MAHJONG_CPP_SYANTEN

#include "tehai.hpp"

namespace mahjong
{

/**
 * @brief 向聴数の種類
 */
struct SyantenType {
    enum Type {
        Normal = 1,  /* 通常手 */
        Tiitoi = 2,  /* 七対子手 */
        Kokushi = 4, /* 国士無双手 */
    };
};

class SyantenCalculator
{
    /**
     * @brief テーブルの情報
     */
    struct Pattern {
        Pattern() : n_mentsu(-1), n_kouho(-1)
        {
        }

        Pattern(char n_mentsu, char n_kouho) : n_mentsu(n_mentsu), n_kouho(n_kouho)
        {
        }

        /*! 面子の数 */
        char n_mentsu;
        /*! 面子候補の数 */
        char n_kouho;
    };

public:
    static int calc(Tehai &tehai, int n_fuuro = 0,
                    int type = SyantenType::Normal | SyantenType::Tiitoi | SyantenType::Kokushi);
    static bool initialize();
    static int calc_normal(Tehai &tehai, int n_fuuro = 0);
    static int calc_tiitoi(const Tehai &tehai);
    static int calc_kokushi(const Tehai &tehai);

private:
    static bool make_table(const std::string &path, std::vector<Pattern> &table);

    /*! 数牌のテーブル */
    static std::vector<Pattern> s_tbl_;
    /*! 字牌のテーブル */
    static std::vector<Pattern> z_tbl_;
    /*! 数牌のテーブルサイズ */
    static const int ShuupaiTableSize = 76611584 + 1; // ハッシュ値の最大値 + 1
    /*! 字牌のテーブルサイズ */
    static const int ZihaiTableSize = 1197056 + 1; // ハッシュ値の最大値 + 1
};

} // namespace mahjong

#endif /* MAHJONG_CPP_SYANTEN */
