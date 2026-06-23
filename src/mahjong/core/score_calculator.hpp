#ifndef SCORE_CALCULATOR_H
#define SCORE_CALCULATOR_H

#include <map>
#include <string>
#include <tuple>
#include <vector>

#include "mahjong/core/hand_separator.hpp"
#include "mahjong/types/types.hpp"

namespace mahjong
{

/**
 * @brief Score calculator
 */
class ScoreCalculator
{
  public:
    static Result calc(const Round &round, const Player &player, int win_tile,
                       int win_flag);
    static Result calc_fast(const Round &round, const Player &player, int win_tile,
                            int win_flag, int shanten_type);
    static std::vector<int> get_up_scores(const Round &round, const Player &player,
                                          const Result &result, const int win_flag,
                                          const int n);
};

namespace score_calculator_detail
{

using MergedHand = std::tuple<Hand, int32_t, int32_t, int32_t, int32_t>;

std::tuple<bool, std::string> check_arguments(const Player &player, int win_tile,
                                              int yaku_list);
int calc_fu(const std::vector<Block> &blocks, const int wait_type, const bool is_closed,
            const bool is_tsumo, const bool is_pinfu, const int round_wind,
            const int seat_wind);
Result aggregate(const Round &round, const Player &player, const int win_tile,
                 const int win_flag, YakuList yaku_list);
Result aggregate(const Round &round, const Player &player, const int win_tile,
                 const int win_flag, YakuList yaku_list, int fu,
                 const std::vector<Block> &blocks, int wait_type);
YakuList check_not_pattern_yaku(const Round &round, const Player &player,
                                const int win_tile, const int win_flag,
                                const int shanten_type);
std::tuple<YakuList, int, std::vector<Block>, int>
check_pattern_yaku(const Round &round, const Player &player, const int win_tile,
                   const int win_flag, const int shanten_type);
std::vector<int> calc_score(const bool is_dealer, const bool is_tsumo, const int honba,
                            const int kyotaku, MahjongMode mode, const int score_title,
                            const int han = 0, const int fu = 0);
int count_dora(const Hand &hand, const std::vector<Meld> &melds,
               const std::vector<int> &indicators, MahjongMode mode, int num_nuki = 0);
int count_reddora(const bool rule_reddora, const Hand &hand,
                  const std::vector<Meld> &melds);
int get_score_title(const int fu, const int han);
int get_score_title(const int n);

// Functions to check yaku
MergedHand merge_hand(const Player &player);
YakuList check_all_green(const MergedHand &merged_hand);
YakuList check_three_dragons(const MergedHand &merged_hand);
YakuList check_four_winds(const MergedHand &merged_hand);
YakuList check_all_honors(const MergedHand &merged_hand);
YakuList check_four_concealed_triplets(const Player &player,
                                       const MergedHand &merged_hand, int win_tile,
                                       int win_flag);
YakuList check_all_terminals(const MergedHand &merged_hand);
YakuList check_kongs(const Player &player);
YakuList check_nine_gates(const Player &player, const MergedHand &merged_hand,
                          int win_tile);
bool check_thirteen_wait_thirteen_orphans(const MergedHand &merged_hand, int win_tile);
YakuList check_tanyao(const Player &player, const MergedHand &merged_hand,
                      const bool rule_open_tanyao);
YakuList check_flush(const MergedHand &merged_hand);
YakuList check_value_tile(const Round &round, const Player &player,
                          const MergedHand &merged_hand);
bool check_pinfu(const std::vector<Block> &blocks, const int wait_type,
                 const int round_wind, const int seat_wind);
YakuList check_pure_double_sequence(const std::vector<Block> &blocks);
YakuList check_all_triplets(const std::vector<Block> &blocks);
YakuList check_three_concealed_triplets(const std::vector<Block> &blocks);
bool check_triple_triplets(const std::vector<Block> &blocks);
bool check_mixed_triple_sequence(const std::vector<Block> &blocks);
bool check_pure_straight(const std::vector<Block> &blocks);
YakuList check_outside_hand(const std::vector<Block> &blocks);

} // namespace score_calculator_detail

} // namespace mahjong

#endif // SCORE_CALCULATOR_H
