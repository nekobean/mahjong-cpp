#ifndef SCORE_CALCULATOR_H
#define SCORE_CALCULATOR_H

#include <string>
#include <tuple>
#include <vector>

#include "mahjong/core/hand_separator.hpp"
#include "mahjong/types/types.hpp"

namespace mahjong
{

/**
 * @brief Han values for a normal yaku.
 */
struct YakuHanEntry
{
    /*! Yaku flag. */
    YakuFlags yaku;

    /*! Han value for a closed hand. */
    int closed_han;

    /*! Han value for an open hand. */
    int open_han;
};

/**
 * @brief Yakuman multiplier for a yakuman yaku.
 */
struct YakumanMultiplierEntry
{
    /*! Yaku flag. */
    YakuFlags yaku;

    /*! Yakuman multiplier. */
    int multiplier;
};

/**
 * @brief Yaku value table used by score calculation.
 */
struct YakuValueTable
{
    /*! Han values for normal yaku. */
    std::vector<YakuHanEntry> yaku_han;

    /*! Multipliers for yakuman yaku. */
    std::vector<YakumanMultiplierEntry> yakuman_multipliers;
};

/**
 * @brief Score calculator
 */
class ScoreCalculator
{
  public:
    static const YakuValueTable &default_yaku_value_table();

    static ScoreResult calc(const TableConfig &table_config,
                            const RoundState &round_state,
                            const TableState &table_state,
                            const PlayerState &player, int win_tile, int win_flag);
    static ScoreResult calc(const TableConfig &table_config,
                            const RoundState &round_state,
                            const TableState &table_state,
                            const PlayerState &player, int win_tile, int win_flag,
                            const YakuValueTable &yaku_value_table);
    static ScoreResult calc_fast(const TableConfig &table_config,
                                 const RoundState &round_state,
                                 const TableState &table_state,
                                 const PlayerState &player, int win_tile, int win_flag,
                                 int shanten_type);
    static ScoreResult calc_fast(const TableConfig &table_config,
                                 const RoundState &round_state,
                                 const TableState &table_state,
                                 const PlayerState &player, int win_tile, int win_flag,
                                 int shanten_type,
                                 const YakuValueTable &yaku_value_table);
    static std::vector<int> get_up_scores(const TableConfig &table_config,
                                          const RoundState &round_state,
                                          const TableState &table_state,
                                          const PlayerState &player,
                                          const ScoreResult &result, const int win_flag,
                                          const int n);
};

namespace score_calculator_detail
{

using MergedHand = std::tuple<Hand, int32_t, int32_t, int32_t, int32_t>;

std::tuple<bool, std::string> check_arguments(const PlayerState &player, int win_tile,
                                              int yaku_list);
int calc_fu(const std::vector<Block> &blocks, const int wait_type, const bool is_closed,
            const bool is_tsumo, const bool is_pinfu, const int round_wind,
            const int wind);
ScoreResult aggregate(const TableConfig &table_config, const RoundState &round_state,
                      const TableState &table_state, const PlayerState &player,
                      const int win_tile, const int win_flag, YakuFlags yaku_list,
                      const YakuValueTable &yaku_value_table);
ScoreResult aggregate(const TableConfig &table_config, const RoundState &round_state,
                      const TableState &table_state, const PlayerState &player,
                      const int win_tile, const int win_flag, YakuFlags yaku_list, int fu,
                      const std::vector<Block> &blocks, int wait_type,
                      const YakuValueTable &yaku_value_table);
YakuFlags check_not_pattern_yaku(const TableConfig &table_config,
                                 const RoundState &round_state,
                                 const PlayerState &player,
                                 const int win_tile, const int win_flag,
                                 const int shanten_type);
std::tuple<YakuFlags, int, std::vector<Block>, int>
check_pattern_yaku(const RoundState &round_state, const PlayerState &player,
                   const int win_tile, const int win_flag, const int shanten_type,
                   const YakuValueTable &yaku_value_table);
std::vector<int> calc_score(const bool is_dealer, const bool is_tsumo, const int honba,
                            const int kyotaku, int game_mode, const int score_title,
                            const int han = 0, const int fu = 0);
int count_dora(const Hand &hand, const std::vector<Meld> &melds,
               const std::vector<int> &indicators, int game_mode, int nuki_count = 0);
int count_reddora(const bool rule_reddora, const Hand &hand,
                  const std::vector<Meld> &melds);
int get_score_title(const int fu, const int han);
int get_score_title(const int n);
MergedHand merge_hand(const PlayerState &player);
YakuFlags check_all_green(const MergedHand &merged_hand);
YakuFlags check_three_dragons(const MergedHand &merged_hand);
YakuFlags check_four_winds(const MergedHand &merged_hand);
YakuFlags check_all_honors(const MergedHand &merged_hand);
YakuFlags check_four_concealed_triplets(const PlayerState &player,
                                        const MergedHand &merged_hand, int win_tile,
                                        int win_flag);
YakuFlags check_all_terminals(const MergedHand &merged_hand);
YakuFlags check_kongs(const PlayerState &player);
YakuFlags check_nine_gates(const PlayerState &player, const MergedHand &merged_hand,
                           int win_tile);
bool check_thirteen_wait_thirteen_orphans(const MergedHand &merged_hand, int win_tile);
YakuFlags check_tanyao(const PlayerState &player, const MergedHand &merged_hand,
                       const bool rule_open_tanyao);
YakuFlags check_flush(const MergedHand &merged_hand);
YakuFlags check_value_tile(const RoundState &round_state, const PlayerState &player,
                           const MergedHand &merged_hand);
bool check_pinfu(const std::vector<Block> &blocks, const int wait_type,
                 const int round_wind, const int wind);
YakuFlags check_pure_double_sequence(const std::vector<Block> &blocks);
YakuFlags check_all_triplets(const std::vector<Block> &blocks);
YakuFlags check_three_concealed_triplets(const std::vector<Block> &blocks);
bool check_triple_triplets(const std::vector<Block> &blocks);
bool check_mixed_triple_sequence(const std::vector<Block> &blocks);
bool check_pure_straight(const std::vector<Block> &blocks);
YakuFlags check_outside_hand(const std::vector<Block> &blocks);

} // namespace score_calculator_detail

} // namespace mahjong

#endif // SCORE_CALCULATOR_H
