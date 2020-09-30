#ifndef MAHJONG_CPP_SCORE
#define MAHJONG_CPP_SCORE

#include <utility>

#include "types.hpp"
#include <spdlog/spdlog.h>

namespace mahjong
{

class ScoreCalculator
{
    /**
     * @brief 結果
     */
    struct Status {
        enum Type {
            Success,         /* 正常終了 */
            InvalidNumTiles, /* 牌が14枚でない場合 */
            InvalidAgarihai, /* 和了り牌が手牌にない場合 */
            InvalidSyanten,  /* 手牌が和了り形でない場合 */
            NakiReach,       /* 鳴き立直している場合 */
            Yakunasi,        /* 役なし */
            Null,
        };

        /* 名前 */
        static const std::vector<std::string> Names;
    };

    /**
     * @brief ドラの種類
     */
    enum class DoraType {
        Dora,
        Uradora,
        Akadora,
    };

    /**
     * @brief 結果
     */
    struct Result {
        Result() : status(Status::Null), score(-1), han(-1), fu(-1)
        {
        }

        /* 結果 */
        int status;

        /* 成立した役の一覧 */
        std::vector<std::pair<unsigned long long, int>> yaku_list;

        /* ドラの一覧 */
        std::vector<std::tuple<DoraType, int, int>> dora_list;

        /* 点数 */
        int score;

        /* 飜 */
        int han;

        /* 符 */
        int fu;

        /* 点数名 */
        int score_name;

        std::string to_string()
        {
            std::string str;

            str += "[Result]\n";
            str += fmt::format("Status: {}\n", Status::Names[status]);
            str += "役:\n";
            for (const auto &[yaku, yaku_han] : yaku_list) {
                if (yaku_han != -1)
                    str += fmt::format("  {}: {}飜\n", Yaku::Names.at(yaku), yaku_han);
                else
                    str += fmt::format("  {}\n", Yaku::Names.at(yaku));
            }

            std::cout << score_name << std::endl;

            if (fu != -1)
                str += fmt::format("{}符{}飜 {}", fu, han, ScoreName::Names[score_name]);
            else
                str += fmt::format("{}", ScoreName::Names[score_name]);

            return str;
        }
    };

public:
    ScoreCalculator();
    Result calc(const Tehai &tehai, const std::vector<HuroBlock> &huro_blocks, int agarihai,
                unsigned long long yaku_list = Yaku::Null);
    int calc_kyotaku_score();
    void debug_print(const Tehai &tehai, const std::vector<HuroBlock> &huro_blocks, int agarihai,
                     unsigned long long yaku_list);
    int check_error(const Tehai &tehai, const std::vector<HuroBlock> &huro_blocks, int agarihai,
                    unsigned long long yaku_list) const;
    Tehai merge_tehai(const Tehai &tehai, const std::vector<HuroBlock> &huro_blocks) const;

    void enable_akahai(bool enabled);
    void enable_kuitan(bool enabled);
    void set_dora(const std::vector<int> &dora_list);
    void set_bakaze(int hai);
    void set_zikaze(int hai);
    void set_honba(int n);
    void set_tumibo(int n);
    bool check_yakuman(const Tehai &tehai, const std::vector<HuroBlock> &huro_blocks, int agarihai,
                       unsigned long long yaku_list, int syanten_type, Result &result) const;

    // 役満チェック関数
    bool check_ryuiso(const Tehai &tehai, const std::vector<HuroBlock> &huro_blocks,
                      int agarihai) const;
    bool check_daisangen(const Tehai &tehai, const std::vector<HuroBlock> &huro_blocks,
                         int agarihai) const;
    bool check_syosusi(const Tehai &tehai, const std::vector<HuroBlock> &huro_blocks,
                       int agarihai) const;
    bool check_tuiso(const Tehai &tehai, const std::vector<HuroBlock> &huro_blocks,
                     int agarihai) const;
    bool check_tyurenpoto(const Tehai &tehai, const std::vector<HuroBlock> &huro_blocks,
                          int agarihai) const;
    bool check_suanko(const Tehai &tehai, const std::vector<HuroBlock> &huro_blocks,
                      int agarihai) const;
    bool check_tinroto(const Tehai &tehai, const std::vector<HuroBlock> &huro_blocks,
                       int agarihai) const;
    bool check_sukantu(const Tehai &tehai, const std::vector<HuroBlock> &huro_blocks,
                       int agarihai) const;
    bool check_suanko_tanki(const Tehai &tehai, const std::vector<HuroBlock> &huro_blocks,
                            int agarihai) const;
    bool check_daisusi(const Tehai &tehai, const std::vector<HuroBlock> &huro_blocks,
                       int agarihai) const;
    bool check_tyurenpoto9(const Tehai &tehai, const std::vector<HuroBlock> &huro_blocks,
                           int agarihai) const;
    bool check_kokusi13(const Tehai &tehai, const std::vector<HuroBlock> &huro_blocks,
                        int agarihai) const;

    // 一般役チェック関数

private:
    /* 赤牌ありかどうか */
    bool rule_akahai_;
    /* 喰い断有り */
    bool rule_kuitan_;
    /* ドラ牌の一覧 */
    std::vector<int> dora_list_;
    /* 場風 */
    int bakaze_;
    /* 自風 */
    int jikaze_;
    /* 本場 */
    int honba_;
    /* 積み棒の数 */
    int tumibo_;
    /* 手牌に関係ない役の一覧 */
    int yaku_list_;
};
} // namespace mahjong

#endif /* MAHJONG_CPP_SCORE */
