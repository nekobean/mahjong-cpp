#ifndef MAHJONG_CPP_SCORE_CALCULATOR
#define MAHJONG_CPP_SCORE_CALCULATOR

#include <utility>

#include "mahjong/types/types.hpp"

namespace mahjong
{

/**
 * @brief ルールに関するフラグ
 */
namespace RuleFlag2
{
enum
{
    Null = 0,
    AkaDora = 1 << 1,    /* 赤ドラ有り */
    OpenTanyao = 1 << 2, /* 喰い断有り */
};

static inline const std::map<int, std::string> Name = {
    {Null, "Null"}, {AkaDora, "赤ドラ有り"}, {OpenTanyao, "喰い断有り"}};
} // namespace RuleFlag2

/**
 * @brief 点数計算機
 */
class ScoreCalculator2
{
  public:
    ScoreCalculator2();

    Result2 calc(const Hand2 &hand, int win_tile, int flag = HandFlag::Null) const;
    std::vector<std::tuple<std::string, int>>
    calc_fu_detail(const std::vector<Block> &blocks, int wait_type, bool is_menzen,
                   bool is_tumo) const;
    std::vector<int> get_scores_for_exp(const Result2 &result);

    /* パラメータを設定・取得する関数 */
    void set_rules(int rule = RuleFlag2::Null);
    void set_rule(int rule, bool enabled);
    int rules() const;

    void set_dora_tiles(const std::vector<int> &tiles);
    void set_dora_indicators(const std::vector<int> &tiles);
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
    std::tuple<bool, std::string> check_arguments(const Hand2 &hand, int win_tile,
                                                  int yaku_list) const;
    YakuList check_yakuman(const Hand2 &hand, const int win_tile, const int flag,
                           const int shanten_type) const;
    YakuList check_not_pattern_yaku(const Hand2 &hand, int win_tile, int flag,
                                    int shanten_type) const;
    std::tuple<YakuList, int, std::vector<Block>, int>
    check_pattern_yaku(const Hand2 &hand, int win_tile, int flag,
                       int shanten_type) const;
    Hand2 merge_hand(const Hand2 &hand) const;

    int calc_fu(const std::vector<Block> &blocks, int wait_type, bool is_menzen,
                bool is_tumo, bool is_pinhu) const;
    std::vector<int> calc_score(bool is_tumo, int score_type, int han = 0,
                                int fu = 0) const;
    Result2 aggregate(const Hand2 &hand, int win_tile, int flag,
                      YakuList yaku_list) const;
    Result2 aggregate(const Hand2 &hand, int win_tile, int flag, YakuList yaku_list,
                      int fu, const std::vector<Block> &blocks, int wait_type) const;

    // 役満をチェックする関数
    bool check_ryuuiisou(const Hand2 &hand) const;
    bool check_daisangen(const Hand2 &hand) const;
    bool check_shousuushii(const Hand2 &hand) const;
    bool check_tsuuiisou(const Hand2 &hand) const;
    bool check_chuuren_poutou(const Hand2 &hand, int win_tile) const;
    bool check_chuuren_poutou9(const Hand2 &hand, int win_tile) const;
    int check_suuankou(const Hand2 &hand, int flag, int win_tile) const;
    bool check_chinroutou(const Hand2 &hand) const;
    bool check_suukantsu(const Hand2 &hand) const;
    bool check_daisuushii(const Hand2 &hand) const;
    bool check_kokushimusou13(const Hand2 &hand, int win_tile) const;

    // 一般役をチェックする関数
    bool check_tanyao(const Hand2 &hand) const;
    bool check_honroutou(const Hand2 &hand) const;
    bool check_honitsu(const Hand2 &hand) const;
    bool check_chinitsu(const Hand2 &hand) const;
    bool check_shousangen(const Hand2 &hand) const;
    bool check_sankantsu(const Hand2 &hand) const;
    bool check_pinfu(const std::vector<Block> blocks, const int wait_type) const;
    int check_iipeikou(const std::vector<Block> blocks) const;
    bool check_ikkitsuukan(const std::vector<Block> blocks) const;
    bool check_sanshoku_doukou(const std::vector<Block> blocks) const;
    bool check_sanshoku_doujun(const std::vector<Block> blocks) const;
    bool check_toitoihou(const std::vector<Block> blocks) const;
    int check_chanta(const std::vector<Block> blocks) const;
    bool check_sanankou(const std::vector<Block> blocks) const;
    int count_dora(const Hand2 &hand, std::vector<int> dora_list) const;
    int count_reddora(const Hand2 &hand) const;

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

#endif /* MAHJONG_CPP_SCORE_CALCULATOR */
