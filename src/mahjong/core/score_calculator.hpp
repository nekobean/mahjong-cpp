#ifndef MAHJONG_CPP_SCORE_CALCULATOR
#define MAHJONG_CPP_SCORE_CALCULATOR

#include <map>
#include <vector>

#include "mahjong/types/types.hpp"

namespace mahjong
{

/**
 * @brief 点数計算機
 */
class ScoreCalculator
{
  public:
    ScoreCalculator();

    Result calc(const Hand &hand, int win_tile, int flag = WinFlag::Null) const;
    std::vector<std::tuple<std::string, int>>
    calc_fu_detail(const std::vector<Block> &blocks, int wait_type, bool is_menzen,
                   bool is_tumo) const;
    std::vector<int> get_scores_for_exp(const Result &result);

    /* setter and getter */
    void set_rules(int rule = RuleFlag::Null);
    void set_rule(int rule, bool enabled);
    int rules() const;
    void set_dora_tiles(const std::vector<int> &tiles);
    void set_dora_indicators(const std::vector<int> &tiles);
    const std::vector<int> &dora_tiles() const;
    void set_uradora_tiles(const std::vector<int> &tiles);
    const std::vector<int> &uradora_tiles() const;
    void set_round_wind(int tile);
    int round_wind() const;
    void set_self_wind(int tile);
    int self_wind() const;
    void set_bonus_sticks(int n);
    int bonus_sticks() const;
    void set_deposit_sticks(int n);
    int deposit_sticks() const;

  public:
    std::tuple<bool, std::string> check_arguments(const Hand &hand, int win_tile,
                                                  int yaku_list) const;
    YakuList check_yakuman(const Hand &hand, const int win_tile, const int flag,
                           const int shanten_type) const;
    YakuList check_not_pattern_yaku(const Hand &hand, int win_tile, int flag,
                                    int shanten_type) const;
    std::tuple<YakuList, int, std::vector<Block>, int>
    check_pattern_yaku(const Hand &hand, int win_tile, int flag,
                       int shanten_type) const;
    Hand merge_hand(const Hand &hand) const;

    int calc_fu(const std::vector<Block> &blocks, int wait_type, bool is_menzen,
                bool is_tumo, bool is_pinhu) const;
    std::vector<int> calc_score(bool is_tumo, int score_type, int han = 0,
                                int fu = 0) const;
    Result aggregate(const Hand &hand, int win_tile, int flag,
                     YakuList yaku_list) const;
    Result aggregate(const Hand &hand, int win_tile, int flag, YakuList yaku_list,
                     int fu, const std::vector<Block> &blocks, int wait_type) const;

    int count_dora(const Hand &hand, std::vector<int> dora_list) const;
    int count_reddora(const Hand &hand) const;
    static int get_score_title(int fu, int han);
    static int get_score_title(int n);
    static int round_up_fu(int hu);

    // Functions to check yaku
    bool check_tanyao(const Hand &hand) const;
    bool check_pinfu(const std::vector<Block> &blocks, const int wait_type) const;
    int check_pure_double_sequence(const std::vector<Block> &blocks) const;
    bool check_all_triplets(const std::vector<Block> &blocks) const;
    bool check_three_concealed_triplets(const std::vector<Block> &blocks) const;
    bool check_triple_triplets(const std::vector<Block> &blocks) const;
    bool check_mixed_triple_sequence(const std::vector<Block> &blocks) const;
    bool check_all_terminals_and_honors(const Hand &hand) const;
    bool check_pure_straight(const std::vector<Block> &blocks) const;
    int check_outside_hand(const std::vector<Block> &blocks) const;
    bool check_little_three_dragons(const Hand &hand) const;
    bool check_three_kongs(const Hand &hand) const;
    bool check_half_flush(const Hand &hand) const;
    bool check_full_flush(const Hand &hand) const;
    bool check_all_green(const Hand &hand) const;
    bool check_big_three_dragons(const Hand &hand) const;
    bool check_little_four_winds(const Hand &hand) const;
    bool check_all_honors(const Hand &hand) const;
    bool check_nine_gates(const Hand &hand, const int win_tile) const;
    int check_four_concealed_triplets(const Hand &hand, const int win_flag,
                                      const int win_tile) const;
    bool check_all_terminals(const Hand &hand) const;
    bool check_four_kongs(const Hand &hand) const;
    bool check_big_four_winds(const Hand &hand) const;
    bool check_true_nine_gates(const Hand &hand, const int win_tile) const;
    bool check_thirteen_wait_thirteen_orphans(const Hand &hand,
                                              const int win_tile) const;

  private:
    /* Game rule */
    int rules_;
    /* Round wind */
    int round_wind_;
    /* Self wind */
    int self_wind_;
    /* Number of bonus sticks */
    int num_bonus_sticks_;
    /* Number of deposit sticks */
    int num_deposit_sticks_;
    /* List of dora tiles */
    std::vector<int> dora_tiles_;
    /* List of ura dora tiles */
    std::vector<int> uradora_tiles_;
};
} // namespace mahjong

#endif /* MAHJONG_CPP_SCORE_CALCULATOR */
