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
    using Input = HandSeparator::Input;

  public:
    static Input create_input(const Player &player, int win_tile, int win_flag);

    static Result calc(const Round &round, const Player &player, int win_tile,
                       int win_flag);
    static Result calc_fast(const Round &round, const Player &player, int win_tile,
                            int win_flag, int shanten_type);
    static std::vector<int> get_scores_for_exp(const Result &result,
                                               const Round &round);

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

    static YakuList check_yakuman(const Input &input, const int shanten_type);
    static YakuList check_not_pattern_yaku(const Round &round, const Input &input,
                                           const int shanten_type);
    static std::tuple<YakuList, int, std::vector<Block>, int>
    check_pattern_yaku(const Input &input, int shanten_type, const Round &round);
    static std::vector<int> calc_score(const bool is_dealer, const bool is_tsumo,
                                       const int honba, const int kyotaku,
                                       const int score_title, const int han = 0,
                                       const int fu = 0);
    static int count_dora(const Hand &hand, const std::vector<Meld> &melds,
                          const std::vector<int> &dora_list);
    static int count_reddora(const bool rule_reddora, const Hand &hand,
                             const std::vector<Meld> &melds);
    static int get_score_title(const int fu, const int han);
    static int get_score_title(const int n);
    static int round_fu(const int fu);

    // Functions to check yaku
    static bool check_tanyao(const bool rule_open_tanyao, const Input &input);
    static bool check_pinfu(const std::vector<Block> &blocks, const int wait_type,
                            const int round_wind, const int seat_wind);
    static int check_pure_double_sequence(const std::vector<Block> &blocks);
    static bool check_all_triplets(const std::vector<Block> &blocks);
    static bool check_three_concealed_triplets(const std::vector<Block> &blocks);
    static bool check_triple_triplets(const std::vector<Block> &blocks);
    static bool check_mixed_triple_sequence(const std::vector<Block> &blocks);
    static bool check_all_terminals_and_honors(const Input &input);
    static bool check_pure_straight(const std::vector<Block> &blocks);
    static int check_outside_hand(const std::vector<Block> &blocks);
    static bool check_little_three_dragons(const Input &input);
    static bool check_three_kongs(const Input &input);
    static bool check_half_flush(const Input &input);
    static bool check_full_flush(const Input &input);
    static bool check_all_green(const Input &input);
    static bool check_big_three_dragons(const Input &input);
    static bool check_little_four_winds(const Input &input);
    static bool check_all_honors(const Input &input);
    static bool check_nine_gates(const Input &input);
    static int check_four_concealed_triplets(const Input &input);
    static bool check_all_terminals(const Input &input);
    static bool check_four_kongs(const Input &input);
    static bool check_big_four_winds(const Input &input);
    static bool check_true_nine_gates(const Input &input);
    static bool check_thirteen_wait_thirteen_orphans(const Input &input);
};

} // namespace mahjong

#endif /* MAHJONG_CPP_SCORE_CALCULATOR */
