#ifndef MAHJONG_CPP_SCORE
#define MAHJONG_CPP_SCORE

#include <utility>

#include "types/types.hpp"
#include <spdlog/spdlog.h>

namespace mahjong {

/**
 * @brief 手牌に関するフラグ
 * 
 *    自摸和了の場合は門前かどうかに関わらず、Tumo を指定します。
 *    立直、ダブル立直はどれか1つのみ指定できます。
 *    搶槓、嶺上開花、海底撈月、河底撈魚はどれか1つのみ指定できます。
 *    天和、地和、人和はどれか1つのみ指定できます。
 */
namespace HandFlag {
enum {
    Null         = 0,
    Tumo         = 1 << 1,  /* 自摸和了 */
    Reach        = 1 << 2,  /* 立直成立 */
    Ippatu       = 1 << 3,  /* 一発成立 */
    Tyankan      = 1 << 4,  /* 搶槓成立 */
    Rinsyankaiho = 1 << 5,  /* 嶺上開花成立 */
    Haiteitumo   = 1 << 6,  /* 海底撈月成立 */
    Hoteiron     = 1 << 7,  /* 河底撈魚成立 */
    DoubleReach  = 1 << 8,  /* ダブル立直成立 */
    NagasiMangan = 1 << 9,  /* 流し満貫成立 */
    Tenho        = 1 << 10, /* 天和成立 */
    Tiho         = 1 << 11, /* 地和成立 */
    Renho        = 1 << 12, /* 人和成立 */
};

static inline const std::map<int, std::string> Names = {{Null, "Null"},
                                                        {Tumo, "自摸和了"},
                                                        {Reach, "立直成立"},
                                                        {Ippatu, "一発成立"},
                                                        {Tyankan, "搶槓成立"},
                                                        {Rinsyankaiho, "嶺上開花成立"},
                                                        {Haiteitumo, "海底撈月成立"},
                                                        {Hoteiron, "河底撈魚成立"},
                                                        {DoubleReach, "ダブル立直成立"},
                                                        {NagasiMangan, "流し満貫成立"},
                                                        {Tenho, "天和成立"},
                                                        {Tiho, "地和成立"},
                                                        {Renho, "人和成立"}};
} // namespace HandFlag

/**
 * @brief ルールに関するフラグ
 */
namespace RuleFlag {
enum {
    Null       = 0,
    AkaDora    = 1 << 1, /* 赤ドラ有り */
    OpenTanyao = 1 << 2, /* 喰い断有り */
};

static inline const std::map<int, std::string> Names = {
    {Null, "Null"}, {AkaDora, "赤ドラ有り"}, {OpenTanyao, "喰い断有り"}};
} // namespace RuleFlag

/**
 * @brief 点数計算機
 */
class ScoreCalculator {
public:
    ScoreCalculator();

    static bool initialize();
    Result calc(const Hand &tehai, int win_tile, int flag = HandFlag::Null);

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
    std::tuple<bool, std::string> check_arguments(const Hand &tehai, int win_tile,
                                                  int yaku_list) const;
    YakuList check_yakuman(const Hand &tehai, int win_tile, int flag,
                           int syanten_type) const;
    YakuList check_not_pattern_yaku(const Hand &tehai, int win_tile, int flag,
                                    int syanten_type) const;
    std::tuple<YakuList, int, std::vector<Block>, int>
    check_pattern_yaku(const Hand &tehai, int win_tile, int flag, int syanten_type);

    Hand merge_hand(const Hand &tehai) const;
    int calc_hu(const std::vector<Block> &blocks, int wait_type, bool menzen, bool tumo,
                bool pinhu) const;
    bool is_yakuhai(int tile) const;

    Result aggregate(const Hand &tehai, int win_tile, YakuList yaku_list, bool tumo);
    Result aggregate(const Hand &tehai, int win_tile, YakuList yaku_list,
                     const std::vector<Block> &blocks, int wait_type, bool tumo);

    static bool make_table(const std::string &path,
                           std::map<int, std::vector<std::vector<Block>>> &table);
    std::vector<std::vector<Block>>
    create_block_patterns(const Hand &tehai, int win_tile, bool tumo) const;
    static std::vector<Block> get_blocks(const std::string &s);
    void create_block_patterns(const Hand &tehai, int win_tile, bool tumo,
                               std::vector<std::vector<Block>> &patterns,
                               std::vector<Block> &blocks, size_t i, int d = 0) const;
    std::tuple<int, std::vector<std::tuple<std::string, int>>>
    calc_hu(const std::vector<Block> &blocks, int wait_type, bool menzen,
            bool tumo) const;
    std::vector<int> calc_score(int han, int hu, int score_type, bool is_tumo) const;

    // 役満をチェックする関数
    bool check_ryuiso(const Hand &tehai) const;
    bool check_daisangen(const Hand &tehai) const;
    bool check_syosusi(const Hand &tehai) const;
    bool check_tuiso(const Hand &tehai) const;
    bool check_tyurenpoto(const Hand &tehai, int win_tile) const;
    bool check_suanko(const Hand &tehai, int flag) const;
    bool check_tinroto(const Hand &tehai) const;
    bool check_sukantu(const Hand &tehai) const;
    bool check_suanko_tanki(const Hand &tehai, int win_tile) const;
    bool check_daisusi(const Hand &tehai) const;
    bool check_tyurenpoto9(const Hand &tehai, int win_tile) const;
    bool check_kokusi13(const Hand &tehai, int win_tile) const;
    // 一般役をチェックする関数
    bool check_tanyao(const Hand &tehai) const;
    bool check_honroto(const Hand &tehai) const;
    bool check_honiso(const Hand &tehai) const;
    bool check_tiniso(const Hand &tehai) const;
    bool check_syosangen(const Hand &tehai) const;
    bool check_sankantu(const Hand &tehai) const;
    bool check_pinhu(const std::vector<Block> blocks) const;
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

private:
    static std::map<int, std::vector<std::vector<Block>>> s_tbl_;
    static std::map<int, std::vector<std::vector<Block>>> z_tbl_;
};
} // namespace mahjong

#endif /* MAHJONG_CPP_SCORE */
