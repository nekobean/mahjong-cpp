#ifndef MAHJONG_CPP_SCORE_CALCULATOR
#define MAHJONG_CPP_SCORE_CALCULATOR

#include <map>
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
    using MergedHand = std::tuple<Hand, int32_t, int32_t, int32_t, int32_t>;

  public:
    struct Config
    {
        int win_tile = 0;
        int win_flag = 0;
    };

    static Result calc(const Round &round, const Player &player, int win_tile,
                       int win_flag);
    static Result calc_fast(const Round &round, const Player &player, int win_tile,
                            int win_flag, int shanten_type);
    static std::vector<int> get_scores_for_exp(const Result &result, const Round &round,
                                               int wind);

  public:
    static std::tuple<bool, std::string> check_arguments(const Player &player,
                                                         int win_tile, int yaku_list);
    static int calc_fu(const std::vector<Block> &blocks, const int wait_type,
                       const bool is_closed, const bool is_tsumo, const bool is_pinfu,
                       const int round_wind, const int seat_wind);
    static Result aggregate(const Round &round, const Player &player,
                            const int win_tile, const int win_flag, YakuList yaku_list);
    static Result aggregate(const Round &round, const Player &player,
                            const int win_tile, const int win_flag, YakuList yaku_list,
                            int fu, const std::vector<Block> &blocks, int wait_type);

    static YakuList check_not_pattern_yaku(const Round &round, const Player &player,
                                           const int win_tile, const int win_flag,
                                           const int shanten_type);
    static std::tuple<YakuList, int, std::vector<Block>, int>
    check_pattern_yaku(const Round &round, const Player &player, const int win_tile,
                       const int win_flag, const int shanten_type);
    static std::vector<int> calc_score(const bool is_dealer, const bool is_tsumo,
                                       const int honba, const int kyotaku,
                                       const int score_title, const int han = 0,
                                       const int fu = 0);
    static int count_dora(const Hand &hand, const std::vector<Meld> &melds,
                          const std::vector<int> &indicators);
    static int count_reddora(const bool rule_reddora, const Hand &hand,
                             const std::vector<Meld> &melds);
    static int get_score_title(const int fu, const int han);
    static int get_score_title(const int n);
    static int round_fu(const int fu);

    // Functions to check yaku
    static MergedHand merge_hand(const Player &player);
    static YakuList check_all_green(const MergedHand &merged_hand);
    static YakuList check_three_dragons(const MergedHand &merged_hand);
    static YakuList check_four_winds(const MergedHand &merged_hand);
    static YakuList check_all_honors(const MergedHand &merged_hand);
    static YakuList check_four_concealed_triplets(const Player &player,
                                                  const MergedHand &merged_hand,
                                                  int win_tile, int win_flag);
    static YakuList check_all_terminals(const MergedHand &merged_hand);
    static YakuList check_kongs(const Player &player);
    static YakuList check_nine_gates(const Player &player,
                                     const MergedHand &merged_hand, int win_tile);
    static bool check_thirteen_wait_thirteen_orphans(const MergedHand &merged_hand,
                                                     int win_tile);
    static YakuList check_tanyao(const Player &player, const MergedHand &merged_hand,
                                 const bool rule_open_tanyao);
    static YakuList check_flush(const MergedHand &merged_hand);
    static YakuList check_value_tile(const Round &round, const Player &player,
                                     const MergedHand &merged_hand);

    static bool check_pinfu(const std::vector<Block> &blocks, const int wait_type,
                            const int round_wind, const int seat_wind);
    static int check_pure_double_sequence(const std::vector<Block> &blocks);
    static bool check_all_triplets(const std::vector<Block> &blocks);
    static bool check_three_concealed_triplets(const std::vector<Block> &blocks);
    static bool check_triple_triplets(const std::vector<Block> &blocks);
    static bool check_mixed_triple_sequence(const std::vector<Block> &blocks);
    static bool check_pure_straight(const std::vector<Block> &blocks);
    static int check_outside_hand(const std::vector<Block> &blocks);
};

} // namespace mahjong

#endif /* MAHJONG_CPP_SCORE_CALCULATOR */
