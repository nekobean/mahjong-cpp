#ifndef MAHJONG_CPP_SCORE_CALCULATOR
#define MAHJONG_CPP_SCORE_CALCULATOR

#include <map>
#include <vector>

#include "mahjong/types/types.hpp"

namespace mahjong
{
/**
 * @brief Score calculator
 */
class ScoreCalculator
{
  public:
    static Result calc(const Hand &hand, int win_tile, int win_flag,
                       const Round &params);
    static std::vector<int> get_scores_for_exp(const Result &result,
                                               const Round &params);

  public:
    static std::tuple<bool, std::string> check_arguments(const Hand &hand, int win_tile,
                                                         int yaku_list);
    static std::tuple<YakuList, int, std::vector<Block>, int>
    check_pattern_yaku(const Hand &hand, int win_tile, int flag, int shanten_type,
                       const Round &params);
    static Hand merge_hand(const Hand &hand);

    static int calc_fu(const std::vector<Block> &blocks, int wait_type, bool is_menzen,
                       bool is_tumo, bool is_pinhu, const Round &params);
    static std::vector<std::tuple<std::string, int>>
    calc_fu_detail(const std::vector<Block> &blocks, int wait_type, bool is_menzen,
                   bool is_tumo, const Round &params);
    static Result aggregate(const Hand &hand, int win_tile, int flag,
                            YakuList yaku_list, const Round &params);
    static Result aggregate(const Hand &hand, int win_tile, int flag,
                            YakuList yaku_list, int fu,
                            const std::vector<Block> &blocks, int wait_type,
                            const Round &params);

    static YakuList check_yakuman(const Hand &hand, const int win_tile, const int flag,
                                  const int shanten_type);
    static YakuList check_not_pattern_yaku(const Hand &hand, const int win_tile,
                                           const int flag, const int shanten_type,
                                           const Round &params);
    static std::vector<int> calc_score(const bool is_tumo, const int score_type,
                                       const Round &params, const int han = 0,
                                       const int fu = 0);
    static int count_dora(const Hand &hand, const std::vector<int> &dora_list);
    static int count_reddora(const Hand &hand);
    static int get_score_title(const int fu, const int han);
    static int get_score_title(const int n);
    static int round_fu(const int fu);

    // Functions to check yaku
    static bool check_tanyao(const Hand &hand, const Round &params);
    static bool check_pinfu(const std::vector<Block> &blocks, const int wait_type,
                            const Round &params);
    static int check_pure_double_sequence(const std::vector<Block> &blocks);
    static bool check_all_triplets(const std::vector<Block> &blocks);
    static bool check_three_concealed_triplets(const std::vector<Block> &blocks);
    static bool check_triple_triplets(const std::vector<Block> &blocks);
    static bool check_mixed_triple_sequence(const std::vector<Block> &blocks);
    static bool check_all_terminals_and_honors(const Hand &hand);
    static bool check_pure_straight(const std::vector<Block> &blocks);
    static int check_outside_hand(const std::vector<Block> &blocks);
    static bool check_little_three_dragons(const Hand &hand);
    static bool check_three_kongs(const Hand &hand);
    static bool check_half_flush(const Hand &hand);
    static bool check_full_flush(const Hand &hand);
    static bool check_all_green(const Hand &hand);
    static bool check_big_three_dragons(const Hand &hand);
    static bool check_little_four_winds(const Hand &hand);
    static bool check_all_honors(const Hand &hand);
    static bool check_nine_gates(const Hand &hand, const int win_tile);
    static int check_four_concealed_triplets(const Hand &hand, const int win_flag,
                                             const int win_tile);
    static bool check_all_terminals(const Hand &hand);
    static bool check_four_kongs(const Hand &hand);
    static bool check_big_four_winds(const Hand &hand);
    static bool check_true_nine_gates(const Hand &hand, const int win_tile);
    static bool check_thirteen_wait_thirteen_orphans(const Hand &hand,
                                                     const int win_tile);
};
;
} // namespace mahjong

#endif /* MAHJONG_CPP_SCORE_CALCULATOR */
