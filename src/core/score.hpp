#ifndef MAHJONG_CPP_SCORE
#define MAHJONG_CPP_SCORE

#include <utility>

#include "types/types.hpp"
#include <spdlog/spdlog.h>

namespace mahjong {

/**
 * @brief 点数計算機
 */
class ScoreCalculator {
public:
    ScoreCalculator();

    Result calc(const Hand &tehai, int winning_tile, YakuList yaku_list = Yaku::Null);
    static bool initialize();

    void enable_akadora(bool enabled);
    void enable_open_tanyao(bool enabled);
    void set_dora_tiles(const std::vector<int> &dora_list);
    void set_uradora_tiles(const std::vector<int> &uradora_list);
    void set_bakaze(int bakaze);
    void set_zikaze(int zikaze);
    void set_num_tumibo(int n);
    void set_num_kyotakubo(int n);

public:
    bool check_arguments(const Hand &tehai, int winning_tile, YakuList yaku_list,
                         std::string &err_msg) const;
    YakuList check_yakuman(const Hand &tehai, int winning_tile, YakuList flag,
                           int syanten_type) const;
    YakuList check_not_pattern_yaku(const Hand &tehai, int winning_tile, YakuList flag,
                                    int syanten_type) const;
    std::tuple<YakuList, int, std::vector<Block>>
    check_pattern_yaku(const Hand &tehai, int winning_tile, YakuList flag);
    int calc_hu(const std::vector<Block> &blocks, int winning_tile, bool menzen,
                bool tumo, bool pinhu) const;
    Hand merge_tehai(const Hand &tehai) const;
    int calc_extra_score() const;
    std::string to_string() const;
    bool is_yakuhai(int hai) const;

    Result aggregate(const Hand &tehai, int winning_tile, YakuList yaku_list,
                     bool tumo);
    Result aggregate(const Hand &tehai, int winning_tile, YakuList yaku_list,
                     const std::vector<Block> &blocks, bool tumo);

    static bool make_table(const std::string &path,
                           std::map<int, std::vector<std::vector<Block>>> &table);
    std::vector<std::vector<Block>>
    create_block_patterns(const Hand &tehai, int winning_tile, bool tumo) const;
    static std::vector<Block> get_blocks(const std::string &s);
    void create_block_patterns(const Hand &tehai, int winning_tile, bool tumo,
                               std::vector<std::vector<Block>> &patterns,
                               std::vector<Block> &blocks, size_t i, int d = 0) const;
    std::tuple<int, std::vector<std::tuple<std::string, int>>>
    calc_hu(const std::vector<Block> &blocks, int winning_tile, bool menzen,
            bool tumo) const;
    std::tuple<int, int, int, int, int> calc_score(int han, int hu,
                                                   int score_type) const;

    // 役満をチェックする関数
    bool check_ryuiso(const Hand &tehai) const;
    bool check_daisangen(const Hand &tehai) const;
    bool check_syosusi(const Hand &tehai) const;
    bool check_tuiso(const Hand &tehai) const;
    bool check_tyurenpoto(const Hand &tehai, int winning_tile) const;
    bool check_suanko(const Hand &tehai, YakuList flag) const;
    bool check_tinroto(const Hand &tehai) const;
    bool check_sukantu(const Hand &tehai) const;
    bool check_suanko_tanki(const Hand &tehai, int winning_tile) const;
    bool check_daisusi(const Hand &tehai) const;
    bool check_tyurenpoto9(const Hand &tehai, int winning_tile) const;
    bool check_kokusi13(const Hand &tehai, int winning_tile) const;
    // 一般役をチェックする関数
    bool check_tanyao(const Hand &tehai) const;
    bool check_honroto(const Hand &tehai) const;
    bool check_honiso(const Hand &tehai) const;
    bool check_tiniso(const Hand &tehai) const;
    bool check_syosangen(const Hand &tehai) const;
    bool check_sankantu(const Hand &tehai) const;
    bool check_pinhu(const std::vector<Block> blocks, int winning_tile) const;
    int check_ipeko(const std::vector<Block> blocks) const;
    bool check_ikkitukan(const std::vector<Block> blocks) const;
    bool check_sansyokudoko(const std::vector<Block> blocks) const;
    bool check_sansyokudozyun(const std::vector<Block> blocks) const;
    bool check_toitoiho(const std::vector<Block> blocks) const;
    int check_tyanta(const std::vector<Block> blocks) const;
    bool check_sananko(const std::vector<Block> blocks) const;
    int count_dora(const Hand &tehai, std::vector<int> dora_list) const;
    int count_akadora(const Hand &tehai) const;

private:
    /* 赤ドラありかどうか */
    bool akadora_enabled_;
    /* 喰い断有りかどうか */
    bool open_tanyao_enabled_;
    /* 場風牌 */
    int bakaze_;
    /* 自風牌 */
    int zikaze_;
    /* 積み棒の数 */
    int n_tumibo_;
    /* 供託棒の数 */
    int n_kyotakubo_;
    /* ドラの一覧 */
    std::vector<int> dora_tiles_;
    /* 裏ドラの一覧 */
    std::vector<int> uradora_tiles_;

private:
    static std::map<int, std::vector<std::vector<Block>>> s_tbl_;
    static std::map<int, std::vector<std::vector<Block>>> z_tbl_;
};
} // namespace mahjong

#endif /* MAHJONG_CPP_SCORE */
