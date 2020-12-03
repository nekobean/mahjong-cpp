#ifndef MAHJONG_CPP_SCORE
#define MAHJONG_CPP_SCORE

#include <utility>

#include "types/types.hpp"
#include <spdlog/spdlog.h>

namespace mahjong {

/**
 * @brief ルールに関するフラグ
 */
namespace RuleFlag {
enum {
    Null       = 0,
    AkaDora    = 1 << 1, /* 赤ドラ有り */
    OpenTanyao = 1 << 2, /* 喰い断有り */
};

static inline const std::map<int, std::string> Name = {
    {Null, "Null"}, {AkaDora, "赤ドラ有り"}, {OpenTanyao, "喰い断有り"}};
} // namespace RuleFlag

/**
 * @brief 点数計算機
 */
class ScoreCalculator {
public:
    ScoreCalculator();

    Result calc(const Hand &hand, int win_tile, int flag = HandFlag::Null) const;
    std::vector<std::tuple<std::string, int>>
    calc_fu_detail(const std::vector<Block> &blocks, int wait_type, bool is_menzen,
                   bool is_tumo) const;

    /* パラメータを設定・取得する関数 */
    void set_rules(int rule = RuleFlag::Null);
    void set_rule(int rule, bool enabled);
    int rules() const;

    void set_dora_tiles(const std::vector<int> &tiles);
    const std::vector<int> &dora_tiles() const;

    void set_uradora_tiles(const std::vector<int> &tiles);
    const std::vector<int> &uradora_tiles() const;

    void set_bakaze(int tile);
    int bakaze() const;

    void set_zikaze(int tile);
    int zikaze() const;

    void set_num_tumibo(int n);
    int num_tumibo() const;

    void set_num_kyotakubo(int n);
    int num_kyotakubo() const;

public:
    std::tuple<bool, std::string> check_arguments(const Hand &hand, int win_tile,
                                                  int yaku_list) const;
    YakuList check_yakuman(const Hand &hand, int win_tile, int flag,
                           int syanten_type) const;
    YakuList check_not_pattern_yaku(const Hand &hand, int win_tile, int flag,
                                    int syanten_type) const;
    std::tuple<YakuList, int, std::vector<Block>, int>
    check_pattern_yaku(const Hand &hand, int win_tile, int flag,
                       int syanten_type) const;
    Hand merge_hand(const Hand &hand) const;
    int calc_fu(const std::vector<Block> &blocks, int wait_type, bool is_menzen,
                bool is_tumo, bool is_pinhu) const;
    std::vector<int> calc_score(bool is_tumo, int score_type, int han = 0,
                                int fu = 0) const;
    Result aggregate(const Hand &hand, int win_tile, int flag,
                     YakuList yaku_list) const;
    Result aggregate(const Hand &hand, int win_tile, int flag, YakuList yaku_list,
                     int fu, const std::vector<Block> &blocks, int wait_type) const;

    // 役満をチェックする関数
    bool check_ryuiso(const Hand &hand) const;
    bool check_daisangen(const Hand &hand) const;
    bool check_syosusi(const Hand &hand) const;
    bool check_tuiso(const Hand &hand) const;
    bool check_tyurenpoto(const Hand &hand, int win_tile) const;
    bool check_suanko(const Hand &hand, int flag) const;
    bool check_tinroto(const Hand &hand) const;
    bool check_sukantu(const Hand &hand) const;
    bool check_suanko_tanki(const Hand &hand, int win_tile) const;
    bool check_daisusi(const Hand &hand) const;
    bool check_tyurenpoto9(const Hand &hand, int win_tile) const;
    bool check_kokusi13(const Hand &hand, int win_tile) const;
    // 一般役をチェックする関数
    bool check_tanyao(const Hand &hand) const;
    bool check_honroto(const Hand &hand) const;
    bool check_honiso(const Hand &hand) const;
    bool check_tiniso(const Hand &hand) const;
    bool check_syosangen(const Hand &hand) const;
    bool check_sankantu(const Hand &hand) const;
    bool check_pinhu(const std::vector<Block> blocks, int wait_type) const;
    int check_ipeko(const std::vector<Block> blocks) const;
    bool check_ikkitukan(const std::vector<Block> blocks) const;
    bool check_sansyokudoko(const std::vector<Block> blocks) const;
    bool check_sansyokudozyun(const std::vector<Block> blocks) const;
    bool check_toitoiho(const std::vector<Block> blocks) const;
    int check_tyanta(const std::vector<Block> blocks) const;
    bool check_sananko(const std::vector<Block> blocks) const;
    int count_dora(const Hand &hand, std::vector<int> dora_list) const;
    int count_akadora(const Hand &hand) const;

private:
    /* ゲームルール */
    int rules_;
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
};
} // namespace mahjong

#endif /* MAHJONG_CPP_SCORE */
