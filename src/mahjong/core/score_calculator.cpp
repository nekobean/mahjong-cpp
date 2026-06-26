#undef NDEBUG

#include "score_calculator.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <numeric>

#include <spdlog/spdlog.h>

#include "mahjong/core/hand_separator.hpp"
#include "mahjong/core/score_table.hpp"
#include "mahjong/core/shanten_calculator.hpp"
#include "mahjong/core/string.hpp"
#include "mahjong/core/utils.hpp"

namespace mahjong
{

namespace
{

const YakuValueTable DefaultYakuValueTable{
    {
        {Yaku::Tsumo, 1, 1},
        {Yaku::Riichi, 1, 1},
        {Yaku::Ippatsu, 1, 1},
        {Yaku::Tanyao, 1, 1},
        {Yaku::Pinfu, 1, 1},
        {Yaku::PureDoubleSequence, 1, 1},
        {Yaku::RobbingAKan, 1, 1},
        {Yaku::AfterAKan, 1, 1},
        {Yaku::UnderTheSea, 1, 1},
        {Yaku::UnderTheRiver, 1, 1},
        {Yaku::Dora, 0, 0},
        {Yaku::UraDora, 0, 0},
        {Yaku::RedDora, 0, 0},
        {Yaku::WhiteDragon, 1, 1},
        {Yaku::GreenDragon, 1, 1},
        {Yaku::RedDragon, 1, 1},
        {Yaku::SelfWindEast, 1, 1},
        {Yaku::SelfWindSouth, 1, 1},
        {Yaku::SelfWindWest, 1, 1},
        {Yaku::SelfWindNorth, 1, 1},
        {Yaku::RoundWindEast, 1, 1},
        {Yaku::RoundWindSouth, 1, 1},
        {Yaku::RoundWindWest, 1, 1},
        {Yaku::RoundWindNorth, 1, 1},
        {Yaku::DoubleRiichi, 2, 0},
        {Yaku::SevenPairs, 2, 0},
        {Yaku::AllTriplets, 2, 2},
        {Yaku::ThreeConcealedTriplets, 2, 2},
        {Yaku::MixedTripleTriplets, 2, 2},
        {Yaku::MixedTripleSequence, 2, 1},
        {Yaku::AllTerminalsAndHonors, 2, 2},
        {Yaku::PureStraight, 2, 1},
        {Yaku::HalfOutsideHand, 2, 1},
        {Yaku::LittleThreeDragons, 2, 2},
        {Yaku::ThreeKans, 2, 2},
        {Yaku::HalfFlush, 3, 2},
        {Yaku::FullyOutsideHand, 3, 2},
        {Yaku::TwicePureDoubleSequence, 3, 0},
        {Yaku::NagashiMangan, 0, 0},
        {Yaku::FullFlush, 6, 5},
        {Yaku::NukiDora, 0, 0},
    },
    {
        {Yaku::HeavenlyHand, 1},
        {Yaku::EarthlyHand, 1},
        {Yaku::HandOfMan, 1},
        {Yaku::AllGreen, 1},
        {Yaku::BigThreeDragons, 1},
        {Yaku::LittleFourWinds, 1},
        {Yaku::AllHonors, 1},
        {Yaku::ThirteenOrphans, 1},
        {Yaku::NineGates, 1},
        {Yaku::FourConcealedTriplets, 1},
        {Yaku::AllTerminals, 1},
        {Yaku::FourKans, 1},
        {Yaku::SingleWaitFourConcealedTriplets, 2},
        {Yaku::BigFourWinds, 2},
        {Yaku::TrueNineGates, 2},
        {Yaku::ThirteenWaitThirteenOrphans, 2},
    },
};

YakuHanEntry get_yaku_han(const YakuValueTable &table, YakuFlags yaku) noexcept
{
    for (const auto &entry : table.yaku_han) {
        if (entry.yaku == yaku) {
            return entry;
        }
    }
    return {yaku, 0, 0};
}

int get_yakuman_multiplier(const YakuValueTable &table, YakuFlags yaku) noexcept
{
    for (const auto &entry : table.yakuman_multipliers) {
        if (entry.yaku == yaku) {
            return entry.multiplier;
        }
    }
    return 0;
}

} // namespace

const YakuValueTable &ScoreCalculator::default_yaku_value_table()
{
    return DefaultYakuValueTable;
}

/**
 * @brief Calculate score.
 *
 * @param[in] table_config table configuration
 * @param[in] round_state round state
 * @param[in] table_state table state
 * @param[in] PlayerState player
 * @param[in] win_tile win tile
 * @param[in] win_flag win flag
 * @return ScoreResult
 */
ScoreResult ScoreCalculator::calc(const TableConfig &table_config,
                                  const RoundState &round_state,
                                  const TableState &table_state,
                                  const PlayerState &player, int win_tile,
                                  int win_flag)
{
    return calc(table_config, round_state, table_state, player, win_tile, win_flag,
                default_yaku_value_table());
}

/**
 * @brief Calculate score.
 *
 * @param[in] table_config table configuration
 * @param[in] round_state round state
 * @param[in] table_state table state
 * @param[in] PlayerState player
 * @param[in] win_tile win tile
 * @param[in] win_flag win flag
 * @param[in] yaku_value_table yaku value table
 * @return ScoreResult
 */
ScoreResult ScoreCalculator::calc(const TableConfig &table_config,
                                  const RoundState &round_state,
                                  const TableState &table_state,
                                  const PlayerState &player, int win_tile,
                                  int win_flag,
                                  const YakuValueTable &yaku_value_table)
{
    if (auto [ok, err_msg] =
            score_calculator_detail::check_arguments(player, win_tile, win_flag);
        !ok) {
        ScoreResult result;
        result.err_msg = err_msg;
        return result;
    }

    // 向聴数を計算する。
    const auto [shanten_type, shanten] = ShantenCalculator::calc(
        player.hand, int(player.melds.size()), ShantenFlag::All,
        table_config.game_mode);
    if (shanten != -1) {
        ScoreResult result;
        result.err_msg = u8"The hand is not winning form.";
        return result;
    }

    return calc_fast(table_config, round_state, table_state, player, win_tile, win_flag,
                     shanten_type, yaku_value_table);
}

/**
 * @brief Calculate score.
 *
 * @param[in] table_config table configuration
 * @param[in] round_state round state
 * @param[in] table_state table state
 * @param[in] PlayerState player
 * @param[in] win_tile win tile
 * @param[in] win_flag win flag
 * @param[in] shanten_type shanten number type
 * @param[in] shanten shanten number
 * @return ScoreResult
 */
ScoreResult ScoreCalculator::calc_fast(const TableConfig &table_config,
                                       const RoundState &round_state,
                                       const TableState &table_state,
                                       const PlayerState &player, int win_tile,
                                       int win_flag, int shanten_type)
{
    return calc_fast(table_config, round_state, table_state, player, win_tile, win_flag,
                     shanten_type, default_yaku_value_table());
}

/**
 * @brief Calculate score.
 *
 * @param[in] table_config table configuration
 * @param[in] round_state round state
 * @param[in] table_state table state
 * @param[in] PlayerState player
 * @param[in] win_tile win tile
 * @param[in] win_flag win flag
 * @param[in] shanten_type shanten number type
 * @param[in] yaku_value_table yaku value table
 * @return ScoreResult
 */
ScoreResult ScoreCalculator::calc_fast(const TableConfig &table_config,
                                       const RoundState &round_state,
                                       const TableState &table_state,
                                       const PlayerState &player, int win_tile,
                                       int win_flag, int shanten_type,
                                       const YakuValueTable &yaku_value_table)
{
    YakuFlags yaku_list = Yaku::None;
    yaku_list |= score_calculator_detail::check_not_pattern_yaku(
        table_config, round_state, player, win_tile, win_flag, shanten_type);

    if (yaku_list & Yaku::NagashiMangan) {
        return score_calculator_detail::aggregate(
            table_config, round_state, table_state, player, win_tile, win_flag,
            Yaku::NagashiMangan, yaku_value_table);
    }
    else if (yaku_list & Yaku::YakumanMask) {
        return score_calculator_detail::aggregate(
            table_config, round_state, table_state, player, win_tile, win_flag,
            yaku_list & Yaku::YakumanMask, yaku_value_table);
    }

    // 面子構成に関係ある役を調べる。
    const auto [pattern_yaku_list, fu, blocks, wait_type] =
        score_calculator_detail::check_pattern_yaku(round_state, player, win_tile,
                                                    win_flag, shanten_type,
                                                    yaku_value_table);
    yaku_list |= pattern_yaku_list;

    if (!yaku_list) {
        ScoreResult result;
        result.err_msg = u8"No yaku is established.";
        return result;
    }

    return score_calculator_detail::aggregate(table_config, round_state, table_state,
                                              player, win_tile, win_flag, yaku_list, fu,
                                              blocks, wait_type, yaku_value_table);
}

/**
 * @brief Aggregate scores for Nagashi Mangan or yakuman.
 *
 * @param[in] table_config table configuration
 * @param[in] round_state round state
 * @param[in] table_state table state
 * @param[in] PlayerState player
 * @param[in] win_tile win tile
 * @param[in] win_flag win flag
 * @param[in] yaku_list list of yaku
 * @param[in] yaku_value_table yaku value table
 * @return ScoreResult
 */
ScoreResult score_calculator_detail::aggregate(const TableConfig &table_config,
                                               const RoundState &round_state,
                                               const TableState &table_state,
                                               const PlayerState &player,
                                               const int win_tile, const int win_flag,
                                               YakuFlags yaku_list,
                                               const YakuValueTable &yaku_value_table)
{
    int score_title;
    std::vector<YakuEntry> yaku_han_list;
    std::vector<int> score;

    const bool is_dealer = player.seat_wind == Tile::East;

    if (yaku_list & Yaku::NagashiMangan) {
        // Nagashi Mangan
        yaku_han_list.push_back({Yaku::NagashiMangan, 0});
        score_title = ScoreLimit::Mangan;

        // Nagashi Mangan is treated as Tsumo.
        score = calc_score(is_dealer, true, round_state.honba, table_state.kyotaku,
                           table_config.game_mode, score_title);
    }
    else {
        // Count yakuman multiplier.
        int n = 0;

        for (YakuFlags yaku = 1LL << 40; yaku <= 1LL << 55; yaku <<= 1) {
            // yakuman: 1LL << 40 ~ 1LL << 55
            if ((yaku & Yaku::YakumanMask) && (yaku_list & yaku)) {
                const int multiplier = get_yakuman_multiplier(yaku_value_table, yaku);
                n += multiplier;
                yaku_han_list.push_back({yaku, multiplier});
            }
        }

        score_title = get_score_title(n);
        score = calc_score(is_dealer, win_flag & WinFlag::Tsumo, round_state.honba,
                           table_state.kyotaku, table_config.game_mode, score_title);
    }

    ScoreResult result;
    result.success = true;
    result.yaku_list = yaku_han_list;
    result.score_limit = score_title;
    result.payments = score;
    return result;
}

/**
 * @brief Aggregate scores for normal yaku.
 *
 * @param[in] table_config table configuration
 * @param[in] round_state round state
 * @param[in] table_state table state
 * @param[in] PlayerState player
 * @param[in] win_tile win tile
 * @param[in] win_flag win flag
 * @param[in] yaku_list list of yaku
 * @param[in] fu fu
 * @param[in] blocks list of blocks
 * @param[in] wait_type wait type
 * @param[in] yaku_value_table yaku value table
 * @return ScoreResult
 */
ScoreResult score_calculator_detail::aggregate(const TableConfig &table_config,
                                               const RoundState &round_state,
                                               const TableState &table_state,
                                               const PlayerState &player,
                                               const int win_tile, const int win_flag,
                                               YakuFlags yaku_list, int fu,
                                               const std::vector<Block> &blocks,
                                               int wait_type,
                                               const YakuValueTable &yaku_value_table)
{
    // Count total number of han.
    int han = 0;
    std::vector<YakuEntry> yaku_han_list;
    for (YakuFlags yaku = 1LL; yaku <= 1LL << 39; yaku <<= 1) {
        // normal: 1LL << 0 ~ 1LL << 39
        if ((yaku & Yaku::NormalMask) && (yaku_list & yaku)) {
            const auto yaku_value = get_yaku_han(yaku_value_table, yaku);
            const int yaku_han =
                player.is_closed() ? yaku_value.closed_han : yaku_value.open_han;
            yaku_han_list.push_back({yaku, yaku_han});
            han += yaku_han;
        }
    }

    // Count number of doras, uradoras and red doras.
    // Extracted north tiles (nuki dora) are also counted as dora/uradora when
    // north is indicated as dora.
    const int nuki_count =
        table_config.game_mode == GameMode::Sanma ? player.nuki_count : 0;
    const int num_doras =
        count_dora(player.hand, player.melds, table_state.dora_indicators,
                   table_config.game_mode, nuki_count);
    if (num_doras) {
        yaku_han_list.push_back({Yaku::Dora, num_doras});
        han += num_doras;
    }

    const int num_uradoras = count_dora(
        player.hand, player.melds, table_state.uradora_indicators,
        table_config.game_mode, nuki_count);
    if (num_uradoras) {
        yaku_han_list.push_back({Yaku::UraDora, num_uradoras});
        han += num_uradoras;
    }

    const bool rule_reddora = table_config.rule_flags & RuleFlag::RedDora;
    const int num_reddoras = count_reddora(rule_reddora, player.hand, player.melds);
    if (num_reddoras) {
        yaku_han_list.push_back({Yaku::RedDora, num_reddoras});
        han += num_reddoras;
    }

    // Count extracted north tiles (nuki dora). Each extracted north always counts
    // as one dora regardless of dora indicators.
    if (nuki_count) {
        yaku_han_list.push_back({Yaku::NukiDora, nuki_count});
        han += nuki_count;
    }

    const int score_title = get_score_title(fu, han);
    const bool is_dealer = player.seat_wind == Tile::East;
    const std::vector<int> score =
        calc_score(is_dealer, win_flag & WinFlag::Tsumo, round_state.honba,
                   table_state.kyotaku, table_config.game_mode, score_title, han, fu);

    std::sort(
        yaku_han_list.begin(), yaku_han_list.end(),
        [](const auto &a, const auto &b) { return a.yaku < b.yaku; });

    ScoreResult result;
    result.success = true;
    result.yaku_list = yaku_han_list;
    result.han = han;
    result.fu = fu;
    result.score_limit = score_title;
    result.payments = score;
    result.blocks = blocks;
    result.wait_type = wait_type;
    return result;
}

/**
 * @brief 引数をチェックする。
 *
 * @param[in] PlayerState player
 * @param[in] win_tile win tile
 * @param[in] int flag
 * @return (is valid, error message)
 */
std::tuple<bool, std::string>
score_calculator_detail::check_arguments(const PlayerState &player, int win_tile,
                                         int win_flag)
{
    // 和了牌をチェックする。
    if (!player.hand[Tile::to_normal(win_tile)]) {
        std::string err_msg = fmt::format(u8"和了牌 {} が手牌 {} に含まれていません。",
                                          Tile::name(win_tile), to_mpsz(player.hand));
        return {false, err_msg};
    }

    // 同時に指定できないフラグをチェックする。
    if (!check_exclusive(win_flag & (WinFlag::Riichi | WinFlag::DoubleRiichi))) {
        std::string err_msg =
            fmt::format(u8"{}、{}はいずれか1つのみ指定できます。",
                        Yaku::name(Yaku::Riichi), Yaku::name(Yaku::DoubleRiichi));
        return {false, err_msg};
    }

    if (!check_exclusive(win_flag & (WinFlag::RobbingAKan | WinFlag::AfterAKan |
                                     WinFlag::UnderTheSea | WinFlag::UnderTheRiver))) {
        std::string err_msg =
            fmt::format(u8"Only one of {}, {}, {}, or {} may be specified.",
                        Yaku::name(Yaku::RobbingAKan), Yaku::name(Yaku::AfterAKan),
                        Yaku::name(Yaku::UnderTheSea), Yaku::name(Yaku::UnderTheRiver));
        return {false, err_msg};
    }

    if (!check_exclusive(win_flag & (WinFlag::HeavenlyHand | WinFlag::EarthlyHand |
                                     WinFlag::HandOfMan))) {
        std::string err_msg =
            fmt::format(u8"Only one of {}, {}, or {} may be specified.",
                        Yaku::name(Yaku::HeavenlyHand), Yaku::name(Yaku::EarthlyHand),
                        Yaku::name(Yaku::HandOfMan));
        return {false, err_msg};
    }

    // 条件が必要なフラグをチェックする。
    if ((win_flag & (WinFlag::Riichi | WinFlag::DoubleRiichi)) && !player.is_closed()) {
        std::string err_msg =
            fmt::format(u8"{}、{}は門前の場合のみ指定できます。",
                        Yaku::name(Yaku::Riichi), Yaku::name(Yaku::DoubleRiichi));
        return {false, err_msg};
    }

    if ((win_flag & WinFlag::Ippatsu) &&
        !(win_flag & (WinFlag::Riichi | WinFlag::DoubleRiichi))) {
        std::string err_msg = fmt::format(u8"{}は立直の場合のみ指定できます。",
                                          Yaku::name(Yaku::Ippatsu));
        return {false, err_msg};
    }

    if ((win_flag & (WinFlag::UnderTheSea | WinFlag::AfterAKan)) &&
        !(win_flag & WinFlag::Tsumo)) {
        std::string err_msg =
            fmt::format(u8"{}、{}は自摸和了の場合のみ指定できます。",
                        Yaku::name(Yaku::UnderTheSea), Yaku::name(Yaku::AfterAKan));
        return {false, err_msg};
    }

    return {true, ""};
}

/**
 * @brief Calculate fu.
 *
 * @param[in] blocks list of blocks
 * @param[in] wait_type wait type
 * @param[in] is_closed is hand closed
 * @param[in] is_tsumo is tsumo win
 * @param[in] is_pinfu is Pinfu established
 * @param[in] round_wind round wind
 * @param[in] wind seat wind
 * @return int 符
 */
int score_calculator_detail::calc_fu(const std::vector<Block> &blocks,
                                     const int wait_type, const bool is_closed,
                                     const bool is_tsumo, const bool is_pinfu,
                                     const int round_wind, const int wind)
{
    // Exceptions
    //////////////////////////
    if (is_pinfu && is_tsumo && is_closed) {
        return 20; // Pinfu + Tsumo
    }
    else if (is_pinfu && !is_tsumo && !is_closed) {
        return 30; // Pinfu + Ron
    }

    // Normal calculation
    //////////////////////////

    // 副底 (base fu)
    int fu = 20;

    if (is_closed && !is_tsumo) {
        fu += 10; // 門前ロンの場合、門前加符がつく
    }
    else if (is_tsumo) {
        fu += 2; // 門前自摸、非門前自摸の場合、自摸符がつく
    }

    // 待ちによる符
    if (wait_type == WaitType::MiddleWait || wait_type == WaitType::EdgeWait ||
        wait_type == WaitType::SingleTileWait) {
        fu += 2; // 嵌張待ち、辺張待ち、単騎待ち
    }

    // 面子構成による符
    for (const auto &block : blocks) {
        if (block.type & (BlockType::Triplet | BlockType::Kan)) {
            int block_fu = 0;
            if (block.type == (BlockType::Triplet | BlockType::Open)) {
                block_fu = 2; // open triplet
            }
            else if (block.type == BlockType::Triplet) {
                block_fu = 4; // closed triplet
            }
            else if (block.type == (BlockType::Kan | BlockType::Open)) {
                block_fu = 8; // open kong
            }
            else if (block.type == BlockType::Kan) {
                block_fu = 16; // closed kong
            }

            fu += Tile::is_terminal_or_honor(block.min_tile) ? block_fu * 2 : block_fu;
        }
        else if (block.type & BlockType::Pair) {
            if (block.min_tile == wind && block.min_tile == round_wind) {
                fu += 4; // round wind and seat wind (連風牌)
            }
            else if (block.min_tile == wind || block.min_tile == round_wind ||
                     block.min_tile >= Tile::WhiteDragon) {
                fu += 2; // value tile (役牌)
            }
        }
    }

    return fu;
}

std::vector<int> ScoreCalculator::get_up_scores(const TableConfig &table_config,
                                                const RoundState &round_state,
                                                const TableState &table_state,
                                                const PlayerState &player,
                                                const ScoreResult &result,
                                                const int win_flag, const int n)
{
    if (!result.success) {
        return {};
    }

    if (result.score_limit >= ScoreLimit::CountedYakuman) {
        return {result.payments[0]}; // Over yakuman
    }

    // Get scores from current han to han + n.
    std::vector<int> scores(n + 1);
    for (int i = 0; i <= n; ++i) {
        int han = result.han + i;
        int score_title = score_calculator_detail::get_score_title(result.fu, han);
        const bool is_dealer = player.seat_wind == Tile::East;
        const bool tsumo = win_flag & WinFlag::Tsumo;
        const int score = score_calculator_detail::calc_score(
            is_dealer, tsumo, round_state.honba, table_state.kyotaku,
            table_config.game_mode, score_title, han, result.fu)[0];
        scores[i] = score;
    }

    return scores;
}

////////////////////////////////////////////////////////////////////////////////////////
/// Helper functions for calculating score
////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Check if yaku is established. (not dependent on how the melds are formed)
 *
 * @param[in] input input data
 * @param[in] shanten_type type of winning hand
 * @return list of established yaku
 */
YakuFlags score_calculator_detail::check_not_pattern_yaku(const TableConfig &table_config,
                                                          const RoundState &round_state,
                                                          const PlayerState &player,
                                                          const int win_tile,
                                                          const int win_flag,
                                                          const int shanten_type)
{
    YakuFlags yaku_list = Yaku::None;

    yaku_list |=
        (win_flag & WinFlag::Tsumo) && player.is_closed() ? Yaku::Tsumo : Yaku::None;
    yaku_list |= win_flag & WinFlag::Riichi ? Yaku::Riichi : Yaku::None;
    yaku_list |= win_flag & WinFlag::Ippatsu ? Yaku::Ippatsu : Yaku::None;
    yaku_list |= win_flag & WinFlag::RobbingAKan ? Yaku::RobbingAKan : Yaku::None;
    yaku_list |= win_flag & WinFlag::AfterAKan ? Yaku::AfterAKan : Yaku::None;
    yaku_list |= win_flag & WinFlag::UnderTheSea ? Yaku::UnderTheSea : Yaku::None;
    yaku_list |= win_flag & WinFlag::UnderTheRiver ? Yaku::UnderTheRiver : Yaku::None;
    yaku_list |= win_flag & WinFlag::DoubleRiichi ? Yaku::DoubleRiichi : Yaku::None;
    yaku_list |= win_flag & WinFlag::NagashiMangan ? Yaku::NagashiMangan : Yaku::None;
    yaku_list |= win_flag & WinFlag::HeavenlyHand ? Yaku::HeavenlyHand : Yaku::None;
    yaku_list |= win_flag & WinFlag::EarthlyHand ? Yaku::EarthlyHand : Yaku::None;
    yaku_list |= win_flag & WinFlag::HandOfMan ? Yaku::HandOfMan : Yaku::None;

    const auto marged_hand = merge_hand(player);
    const int nored_win_tile = Tile::to_normal(win_tile);

    // If both regular hand and seven pairs are established, prioritize the regular hand.
    if (shanten_type & ShantenFlag::StandardHand) {
        yaku_list |= check_all_green(marged_hand);
        yaku_list |= check_three_dragons(marged_hand);
        yaku_list |= check_four_winds(marged_hand);
        yaku_list |= check_all_honors(marged_hand);
        yaku_list |= check_four_concealed_triplets(player, marged_hand, nored_win_tile,
                                                   win_flag);
        yaku_list |= check_all_terminals(marged_hand);
        yaku_list |= check_kongs(player);
        yaku_list |= check_nine_gates(player, marged_hand, nored_win_tile);
        yaku_list |= check_tanyao(player, marged_hand,
                                  table_config.rule_flags & RuleFlag::OpenTanyao);
        yaku_list |= check_flush(marged_hand);
        yaku_list |= check_value_tile(round_state, player, marged_hand);
    }
    else if (shanten_type & ShantenFlag::SevenPairs) {
        yaku_list |= Yaku::SevenPairs;
        yaku_list |= check_all_honors(marged_hand);
        yaku_list |= check_all_terminals(marged_hand);
        yaku_list |= check_tanyao(player, marged_hand,
                                  table_config.rule_flags & RuleFlag::OpenTanyao);
        yaku_list |= check_flush(marged_hand);
    }
    else {
        yaku_list |= check_thirteen_wait_thirteen_orphans(marged_hand, nored_win_tile)
                         ? Yaku::ThirteenWaitThirteenOrphans
                         : Yaku::ThirteenOrphans;
    }

    return yaku_list;
}

/**
 * @brief Check yaku related to the composition of blocks.
 *
 * @param[in] round_state round state
 * @param[in] player player state
 * @param[in] win_tile winning tile
 * @param[in] win_flag win flags
 * @param[in] shanten_type shanten type
 * @return (yaku, fu, list of blocks, wait type)
 */
std::tuple<YakuFlags, int, std::vector<Block>, int>
score_calculator_detail::check_pattern_yaku(const RoundState &round_state,
                                            const PlayerState &player,
                                            const int win_tile, const int win_flag,
                                            const int shanten_type,
                                            const YakuValueTable &yaku_value_table)
{
    if (shanten_type == ShantenFlag::SevenPairs) {
        return {Yaku::None, 25, {}, WaitType::SingleTileWait};
    }

    static const std::vector<YakuFlags> pattern_yaku = {
        Yaku::Pinfu,
        Yaku::PureDoubleSequence,
        Yaku::AllTriplets,
        Yaku::ThreeConcealedTriplets,
        Yaku::MixedTripleTriplets,
        Yaku::MixedTripleSequence,
        Yaku::PureStraight,
        Yaku::HalfOutsideHand,
        Yaku::ThreeKans,
        Yaku::FullyOutsideHand,
        Yaku::TwicePureDoubleSequence,
    };

    // Get list of block compositions.
    const auto pattern = HandSeparator::separate(player, win_tile, win_flag);

    // Find the block composition with the highest score.
    int max_han = 0;
    int max_fu = 0;
    size_t max_idx;
    YakuFlags max_yaku_list;
    for (size_t i = 0; i < pattern.size(); ++i) {
        YakuFlags yaku_list = Yaku::None;
        int han, fu;
        const std::vector<Block> &blocks = std::get<0>(pattern[i]);
        int wait_type = std::get<1>(pattern[i]);

        // Check if Pinfu is established.
        const bool is_pinfu =
            check_pinfu(blocks, wait_type, round_state.round_wind, player.seat_wind);

        if (player.is_closed()) {
            if (is_pinfu) {
                yaku_list |= Yaku::Pinfu; // 平和
            }

            yaku_list |= check_pure_double_sequence(blocks);
        }

        if (check_pure_straight(blocks)) {
            yaku_list |= Yaku::PureStraight; // 一気通貫
        }
        else if (check_triple_triplets(blocks)) {
            yaku_list |= Yaku::MixedTripleTriplets; // 三色同刻
        }
        else if (check_mixed_triple_sequence(blocks)) {
            yaku_list |= Yaku::MixedTripleSequence; // 三色同順
        }

        yaku_list |= check_outside_hand(blocks);
        yaku_list |= check_all_triplets(blocks);             // 対々和
        yaku_list |= check_three_concealed_triplets(blocks); // 三暗刻

        // Calculate han.
        han = 0;
        for (const auto &yaku : pattern_yaku) {
            if (yaku_list & yaku) {
                const auto yaku_value = get_yaku_han(yaku_value_table, yaku);
                han += player.is_closed() ? yaku_value.closed_han
                                          : yaku_value.open_han;
            }
        }

        // Calculate fu.
        fu = calc_fu(blocks, wait_type, player.is_closed(), win_flag & WinFlag::Tsumo,
                     is_pinfu, round_state.round_wind, player.seat_wind);

        if (max_han < han || (max_han == han && max_fu < fu)) {
            max_han = han;
            max_fu = fu;
            max_idx = i;
            max_yaku_list = yaku_list;
        }
    }

    max_fu = int(std::ceil(max_fu / 10.)) * 10;
    ;

    return {max_yaku_list, max_fu, std::get<0>(pattern[max_idx]),
            std::get<1>(pattern[max_idx])};
}

/**
 * @brief Calculate score and payment.
 *
 * @param[in] is_dealer  Whether the PlayerState is dealer
 * @param[in] is_tsumo Whether the PlayerState wins by Tsumo
 * @param[in] honba honba
 * @param[in] kyotaku kyotaku
 * @param[in] game_mode Mahjong game mode
 * @param[in] score_title score title
 * @param[in] han han
 * @param[in] fu fu
 * @return (winner score, Non-dealer payment) if dealer wins by Tsumo.
 *         (winner score, dealer payment, Non-dealer payment) if PlayerState wins by Tsumo.
 *         (winner score, discarder payment) if dealer or PlayerState wins by Ron.
 */
std::vector<int>
score_calculator_detail::calc_score(const bool is_dealer, const bool is_tsumo,
                                    const int honba, const int kyotaku,
                                    const int game_mode, const int score_title,
                                    const int han, const int fu)
{
    using namespace ScoreTable;

    const int fu_idx = ScoreTable::fu_to_index(fu);
    const int han_idx = han - 1;
    const int num_tsumo_payers = game_mode == GameMode::Sanma ? 2 : 3;

    if (is_tsumo && is_dealer) {
        // dealer tsumo
        const int player_payment =
            (score_title == ScoreLimit::Null
                 ? BelowMangan[TsumoPlayerToDealer][fu_idx][han_idx]
                 : AboveMangan[TsumoPlayerToDealer][score_title]) +
            100 * honba;
        const int score = 1000 * kyotaku + player_payment * num_tsumo_payers;

        return {score, player_payment};
    }
    else if (is_tsumo && !is_dealer) {
        // PlayerState tsumo
        const int dealer_payment =
            (score_title == ScoreLimit::Null
                 ? BelowMangan[TsumoDealerToPlayer][fu_idx][han_idx]
                 : AboveMangan[TsumoDealerToPlayer][score_title]) +
            100 * honba;
        const int player_payment =
            (score_title == ScoreLimit::Null
                 ? BelowMangan[TsumoPlayerToPlayer][fu_idx][han_idx]
                 : AboveMangan[TsumoPlayerToPlayer][score_title]) +
            100 * honba;
        const int score =
            1000 * kyotaku + dealer_payment + player_payment * (num_tsumo_payers - 1);

        return {score, dealer_payment, player_payment};
    }
    else if (!is_tsumo && is_dealer) {
        // dealer ron
        const int honba_payment = (game_mode == GameMode::Sanma ? 200 : 300) * honba;
        const int payment = (score_title == ScoreLimit::Null
                                 ? BelowMangan[RonDiscarderToDealer][fu_idx][han_idx]
                                 : AboveMangan[RonDiscarderToDealer][score_title]) +
                            honba_payment;
        const int score = 1000 * kyotaku + payment;

        return {score, payment};
    }
    else {
        // PlayerState ron
        const int honba_payment = (game_mode == GameMode::Sanma ? 200 : 300) * honba;
        const int payment = (score_title == ScoreLimit::Null
                                 ? BelowMangan[RonDiscarderToPlayer][fu_idx][han_idx]
                                 : AboveMangan[RonDiscarderToPlayer][score_title]) +
                            honba_payment;
        const int score = 1000 * kyotaku + payment;

        return {score, payment};
    }
}

/**
 * @brief Count number of dora tiles.
 *
 * @param hand hand
 * @param melds list of melds
 * @param indicators list of dora indicators
 * @param game_mode Mahjong game mode
 * @param nuki_count number of extracted north tiles
 * @return number of dora tiles
 */
int score_calculator_detail::count_dora(const Hand &hand,
                                        const std::vector<Meld> &melds,
                                        const std::vector<int> &indicators,
                                        const int game_mode, const int nuki_count)
{
    int num_doras = 0;
    for (const auto tile : indicators) {
        const int dora = Tile::to_dora(tile, game_mode);
        // Count doras in the hand.
        num_doras += hand[dora];

        // Count doras in the melds.
        for (const auto &meld : melds) {
            for (const auto tile : meld.tiles) {
                num_doras += Tile::to_normal(tile) == dora;
            }
        }

        // Count extracted north tiles as dora when north is indicated as dora.
        if (dora == Tile::North) {
            num_doras += nuki_count;
        }
    }

    return num_doras;
}

/**
 * @brief Count number of red dora tiles.
 *
 * @param hand hand
 * @param melds list of melds
 * @return number of red dora tiles
 */
int score_calculator_detail::count_reddora(const bool rule_reddora, const Hand &hand,
                                           const std::vector<Meld> &melds)
{
    if (!rule_reddora) {
        return 0;
    }

    int num_doras = 0;

    // Count red doras in the hand.
    num_doras += hand[Tile::RedManzu5] + hand[Tile::RedPinzu5] + hand[Tile::RedSouzu5];

    // Count red doras in the melds.
    for (const auto &meld : melds) {
        for (const auto tile : meld.tiles) {
            if (Tile::is_red(tile)) {
                ++num_doras;
                break;
            }
        }
    }

    return num_doras;
}

/**
 * @brief Get score title for not yakuman.
 *
 * @param[in] fu fu
 * @param[in] han han
 * @return score title
 */
int score_calculator_detail::get_score_title(const int fu, const int han)
{
    const int fu_idx = ScoreTable::fu_to_index(fu);
    const int han_idx = han - 1;

    if (han < 5) {
        return ScoreTable::IsMangan[fu_idx][han_idx] ? ScoreLimit::Mangan
                                                     : ScoreLimit::Null;
    }

    if (han == 5) {
        return ScoreLimit::Mangan;
    }
    else if (han <= 7) {
        return ScoreLimit::Haneman;
    }
    else if (han <= 10) {
        return ScoreLimit::Baiman;
    }
    else if (han <= 12) {
        return ScoreLimit::Sanbaiman;
    }

    return ScoreLimit::CountedYakuman;
};

/**
 * @brief Get score title for yakuman.
 *
 * @param[in] n yakuman multiplier
 * @return score title
 */
int score_calculator_detail::get_score_title(const int n)
{
    if (n == 1) {
        return ScoreLimit::Yakuman;
    }
    else if (n == 2) {
        return ScoreLimit::DoubleYakuman;
    }
    else if (n == 3) {
        return ScoreLimit::TripleYakuman;
    }
    else if (n == 4) {
        return ScoreLimit::QuadrupleYakuman;
    }
    else if (n == 5) {
        return ScoreLimit::QuintupleYakuman;
    }
    else if (n == 6) {
        return ScoreLimit::SextupleYakuman;
    }

    return ScoreLimit::Null;
};

////////////////////////////////////////////////////////////////////////////////////////
/// Functions to check yaku
////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Check if Pinfu (平和) is established.
 */
bool score_calculator_detail::check_pinfu(const std::vector<Block> &blocks,
                                          const int wait_type, const int round_wind,
                                          const int wind)
{
    // Before calling this function, Check if the hand is closed.

    if (wait_type != WaitType::TwoSidedWait) {
        return false; // not double closed wait
    }

    // Check if all blocks are sequences or pairs that are not yakuhai.
    for (const auto &block : blocks) {
        if (block.type & (BlockType::Triplet | BlockType::Kan)) {
            return false; // triplet, kong
        }

        if ((block.type & BlockType::Pair) &&
            (block.min_tile == round_wind || block.min_tile == wind ||
             block.min_tile >= Tile::WhiteDragon)) {
            return false; // value tile pair
        }
    }

    return true;
}

/**
 * @brief Check if Pure Double Sequence (一盃口) or
 *        Twice Pure Double Sequence (二盃口) is established.
 */
YakuFlags
score_calculator_detail::check_pure_double_sequence(const std::vector<Block> &blocks)
{
    // Before calling this function, Check if the hand is closed.

    std::vector<int> count(34, 0);
    for (const auto &block : blocks) {
        if (block.type & BlockType::Sequence) {
            count[block.min_tile]++; // sequence
        }
    }

    // Count number of double sequences.
    int num_double_sequences = 0;
    for (const auto &x : count) {
        if (x == 4) {
            num_double_sequences += 2;
        }
        else if (x >= 2) {
            ++num_double_sequences;
        }
    }

    if (num_double_sequences == 1) {
        return Yaku::PureDoubleSequence;
    }
    else if (num_double_sequences == 2) {
        return Yaku::TwicePureDoubleSequence;
    }

    return Yaku::None;
}

/**
 * @brief Check if All Triplets (対々和) is established.
 */
YakuFlags score_calculator_detail::check_all_triplets(const std::vector<Block> &blocks)
{
    for (const auto &block : blocks) {
        if (block.type & BlockType::Sequence) {
            return Yaku::None; // sequence
        }
    }

    return Yaku::AllTriplets;
}

/**
 * @brief Check if Three Concealed Triplets (三暗刻) is established.
 */
YakuFlags score_calculator_detail::check_three_concealed_triplets(
    const std::vector<Block> &blocks)
{
    int num_triplets = 0;
    for (const auto &block : blocks) {
        if (block.type == BlockType::Triplet || block.type == BlockType::Kan) {
            ++num_triplets; // closed triplet or closed kong
        }
    }

    return num_triplets == 3 ? Yaku::ThreeConcealedTriplets : Yaku::None;
}

/**
 * @brief Check if Triple Triplets (三色同刻) is established.
 */
bool score_calculator_detail::check_triple_triplets(const std::vector<Block> &blocks)
{
    std::vector<int> count(34, 0);
    for (const auto &block : blocks) {
        if (block.type & (BlockType::Triplet | BlockType::Kan)) {
            count[block.min_tile]++; // triplet, kong
        }
    }

    for (int i = 0; i < 9; ++i) {
        if (count[i] && count[i + 9] && count[i + 18]) {
            return true;
        }
    }

    return false;
}

/**
 * @brief Check if Mixed Triple Sequence (三色同順) is established.
 */
bool score_calculator_detail::check_mixed_triple_sequence(
    const std::vector<Block> &blocks)
{
    std::vector<int> count(34, 0);
    for (const auto &block : blocks) {
        if (block.type & BlockType::Sequence) {
            count[block.min_tile]++; // sequence
        }
    }

    for (size_t i = 0; i < 9; ++i) {
        if (count[i] && count[i + 9] && count[i + 18]) {
            return true;
        }
    }

    return false;
}

/**
 * @brief Check if Pure Straight (一気通貫) is established.
 */
bool score_calculator_detail::check_pure_straight(const std::vector<Block> &blocks)
{
    std::vector<int> count(34, 0);
    for (const auto &block : blocks) {
        if (block.type & BlockType::Sequence) {
            count[block.min_tile]++; // sequence
        }
    }

    // Check if there is 123, 456, 789 in any type of number tiles.
    return (count[Tile::Manzu1] && count[Tile::Manzu4] && count[Tile::Manzu7]) ||
           (count[Tile::Pinzu1] && count[Tile::Pinzu4] && count[Tile::Pinzu7]) ||
           (count[Tile::Souzu1] && count[Tile::Souzu4] && count[Tile::Souzu7]);
}

/**
 * @brief Check if Half Outside Hand (混全帯幺九) or
 *        Fully Outside Hand (純全帯幺九) is established.
 */
YakuFlags score_calculator_detail::check_outside_hand(const std::vector<Block> &blocks)
{
    // | Yaku                     | Terminal | Honor | Sequence |
    // | Half Outside Hand        | o        | ○     | ○        |
    // | Fully Outside Hand       | ○        | x     | ○        |
    // | All Terminals and Honors | ○        | ○     | x        |
    // | All Terminals            | ○        | x     | x        |
    bool honor_block = false;
    bool sequence_block = false;
    for (const auto &block : blocks) {
        if (block.type & BlockType::Sequence) {
            if (!(block.min_tile == Tile::Manzu1 || block.min_tile == Tile::Manzu7 ||
                  block.min_tile == Tile::Pinzu1 || block.min_tile == Tile::Pinzu7 ||
                  block.min_tile == Tile::Souzu1 || block.min_tile == Tile::Souzu7)) {
                return 0; // sequence that does not contain terminal or honor tiles
            }

            sequence_block = true;
        }
        else {
            if (!(block.min_tile == Tile::Manzu1 || block.min_tile == Tile::Manzu9 ||
                  block.min_tile == Tile::Pinzu1 || block.min_tile == Tile::Pinzu9 ||
                  block.min_tile == Tile::Souzu1 || block.min_tile == Tile::Souzu9 ||
                  block.min_tile >= Tile::East)) {
                return 0; // triplet, kong, pair that does not contain terminal or honor tiles
            }

            honor_block |= block.min_tile >= Tile::East;
        }
    }

    if (honor_block && sequence_block) {
        return Yaku::HalfOutsideHand; // Half Outside Hand
    }
    if (!honor_block && sequence_block) {
        return Yaku::FullyOutsideHand; // Fully Outside Hand
    }

    return 0;
}

/**
 * @brief Check if All Green (緑一色) is established.
 */
YakuFlags score_calculator_detail::check_all_green(const MergedHand &merged_hand)
{
    const auto &[hand, manzu, pinzu, souzu, honors] = merged_hand;

    // Check if there are no tiles other than 2, 3, 4, 6, 8 of souzu and green dragon.
    //                         | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |
    const int32_t souzu_mask = 0b111'000'000'000'111'000'111'000'111;
    //                          | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
    const int32_t honors_mask = 0b111'111'111'111'111'000'111;

    return manzu || pinzu || (souzu & souzu_mask) || (honors & honors_mask)
               ? Yaku::None
               : Yaku::AllGreen;
}

/**
 * @brief Check if Little Three Dragons (小三元) or Big Three Dragons (大三元) is established.
 */
YakuFlags score_calculator_detail::check_three_dragons(const MergedHand &merged_hand)
{
    const auto &[hand, manzu, pinzu, souzu, honors] = merged_hand;

    int sum = hand[Tile::WhiteDragon] + hand[Tile::GreenDragon] + hand[Tile::RedDragon];
    if (sum == 8) {
        return Yaku::LittleThreeDragons;
    }
    else if (sum == 9) {
        return Yaku::BigThreeDragons;
    }

    return Yaku::None;
}

/**
 * @brief Check if Little Four Winds (小四喜) or Big Four Winds (大四喜) is established.
 */
YakuFlags score_calculator_detail::check_four_winds(const MergedHand &merged_hand)
{
    const auto &[hand, manzu, pinzu, souzu, honors] = merged_hand;
    int sum =
        hand[Tile::East] + hand[Tile::South] + hand[Tile::West] + hand[Tile::North];
    if (sum == 11) {
        return Yaku::LittleFourWinds;
    }
    else if (sum == 12) {
        return Yaku::BigFourWinds;
    }

    return Yaku::None;
}

/**
 * @brief Check if All Honors (字一色) is established.
 */
YakuFlags score_calculator_detail::check_all_honors(const MergedHand &merged_hand)
{
    const auto &[hand, manzu, pinzu, souzu, honors] = merged_hand;
    return manzu || pinzu || souzu ? Yaku::None : Yaku::AllHonors;
}

/**
 * @brief Check if Four Concealed Triplets (四暗刻) is established.
 */
YakuFlags
score_calculator_detail::check_four_concealed_triplets(const PlayerState &player,
                                                       const MergedHand &merged_hand,
                                                       int win_tile, int win_flag)
{
    if (!player.is_closed()) {
        return Yaku::None; // closed hand only
    }

    const auto &[hand, manzu, pinzu, souzu, honors] = merged_hand;

    int num_triplets = 0;
    int num_pairs = 0;
    bool single_wait = 0;
    for (int i = 0; i < 34; ++i) {
        if (hand[i] == 3) {
            ++num_triplets;
        }
        else if (hand[i] == 2) {
            ++num_pairs;
            single_wait = i == win_tile;
        }
    }

    if (num_triplets == 4 && num_pairs == 1) {
        if (!(win_flag & WinFlag::Tsumo) && !single_wait) {
            return Yaku::None;
        }
        return single_wait ? Yaku::SingleWaitFourConcealedTriplets
                           : Yaku::FourConcealedTriplets;
    }

    return Yaku::None;
}

/**
 * @brief Check if All Terminals (清老頭) is established.
 */
YakuFlags score_calculator_detail::check_all_terminals(const MergedHand &merged_hand)
{
    const auto &[hand, manzu, pinzu, souzu, honors] = merged_hand;

    // Check if there are no tiles other than terminal tiles.
    //                              | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |
    const int32_t terminals_mask = 0b000'111'111'111'111'111'111'111'000;
    if ((manzu | pinzu | souzu) & terminals_mask) {
        return Yaku::None;
    }

    return honors ? Yaku::AllTerminalsAndHonors : Yaku::AllTerminals;
}

/**
 * @brief Check if Three Kongs (三槓子) or Four Kongs (四槓子) is established.
 */
YakuFlags score_calculator_detail::check_kongs(const PlayerState &player)
{
    // Check if there are 4 kongs.
    int num_kongs = 0;
    for (const auto &meld : player.melds) {
        // enum values of 2 or more are kongs
        num_kongs += MeldType::Ankan <= meld.type;
    }

    if (num_kongs == 4) {
        return Yaku::FourKans;
    }
    else if (num_kongs == 3) {
        return Yaku::ThreeKans;
    }

    return Yaku::None;
}

/**
 * @brief Check if Nine Gates (九連宝燈) or True Nine Gates (純正九蓮宝燈) is established.
 */
YakuFlags score_calculator_detail::check_nine_gates(const PlayerState &player,
                                                    const MergedHand &merged_hand,
                                                    int win_tile)
{
    if (!player.melds.empty()) {
        return Yaku::None; // no meld hand only
    }

    const auto &[hand, manzu, pinzu, souzu, honors] = merged_hand;

    static const std::array<int32_t, 34> tile1 = {
        1 << 24, 1 << 21, 1 << 18, 1 << 15, 1 << 12, 1 << 9, 1 << 6, 1 << 3, 1,
        1 << 24, 1 << 21, 1 << 18, 1 << 15, 1 << 12, 1 << 9, 1 << 6, 1 << 3, 1,
        1 << 24, 1 << 21, 1 << 18, 1 << 15, 1 << 12, 1 << 9, 1 << 6, 1 << 3, 1,
    };

    // Check if number of each terminal tile is 3 or more and number of each chunchan tile is 1 or more.
    // Exclude the winning tile and
    // check if the number of each terminal tile is 3 and the number of each chunchan tile is 1.
    //          | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |
    const int mask = 0b011'001'001'001'001'001'001'001'011;
    bool is_valid = false, is_true = false;
    if (win_tile <= Tile::Manzu9) {
        is_valid = hand[Tile::Manzu1] >= 3 && hand[Tile::Manzu2] &&
                   hand[Tile::Manzu3] && hand[Tile::Manzu4] && hand[Tile::Manzu5] &&
                   hand[Tile::Manzu6] && hand[Tile::Manzu7] && hand[Tile::Manzu8] &&
                   hand[Tile::Manzu9] >= 3;
        is_true = manzu - tile1[win_tile] == mask;
    }
    else if (win_tile <= Tile::Pinzu9) {
        is_valid = hand[Tile::Pinzu1] >= 3 && hand[Tile::Pinzu2] &&
                   hand[Tile::Pinzu3] && hand[Tile::Pinzu4] && hand[Tile::Pinzu5] &&
                   hand[Tile::Pinzu6] && hand[Tile::Pinzu7] && hand[Tile::Pinzu8] &&
                   hand[Tile::Pinzu9] >= 3;
        is_true = pinzu - tile1[win_tile] == mask;
    }
    else if (win_tile <= Tile::Souzu9) {
        is_valid = hand[Tile::Souzu1] >= 3 && hand[Tile::Souzu2] &&
                   hand[Tile::Souzu3] && hand[Tile::Souzu4] && hand[Tile::Souzu5] &&
                   hand[Tile::Souzu6] && hand[Tile::Souzu7] && hand[Tile::Souzu8] &&
                   hand[Tile::Souzu9] >= 3;
        is_true = souzu - tile1[win_tile] == mask;
    }

    if (is_valid) {
        return is_true ? Yaku::TrueNineGates : Yaku::NineGates;
    }

    return Yaku::None;
}

/**
 * @brief Check if Thirteen-wait Thirteen Orphans (国士無双13面待ち) is established.
 */
bool score_calculator_detail::check_thirteen_wait_thirteen_orphans(
    const MergedHand &merged_hand, int win_tile)
{
    static const std::array<int32_t, 34> tile1 = {
        1 << 24, 1 << 21, 1 << 18, 1 << 15, 1 << 12, 1 << 9, 1 << 6, 1 << 3, 1,
        1 << 24, 1 << 21, 1 << 18, 1 << 15, 1 << 12, 1 << 9, 1 << 6, 1 << 3, 1,
        1 << 24, 1 << 21, 1 << 18, 1 << 15, 1 << 12, 1 << 9, 1 << 6, 1 << 3, 1,
        1 << 18, 1 << 15, 1 << 12, 1 << 9,  1 << 6,  1 << 3, 1,
    };

    // 「和了牌を除いた場合に幺九牌がすべて1個ずつある」かどうかを調べる。
    //                              | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |
    const int32_t terminals_mask = 0b001'000'000'000'000'000'000'000'001;
    //                           | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
    const int32_t honors_mask = 0b001'001'001'001'001'001'001;

    const auto &[hand, manzu, pinzu, souzu, honors] = merged_hand;

    if (win_tile <= Tile::Manzu9) {
        return (manzu - tile1[win_tile] == terminals_mask) &&
               (pinzu == terminals_mask) && (souzu == terminals_mask) &&
               (honors == honors_mask);
    }
    else if (win_tile <= Tile::Pinzu9) {
        return (manzu == terminals_mask) &&
               (pinzu - tile1[win_tile] == terminals_mask) &&
               (souzu == terminals_mask) && (honors == honors_mask);
    }
    else if (win_tile <= Tile::Souzu9) {
        return (manzu == terminals_mask) && (pinzu == terminals_mask) &&
               (souzu - tile1[win_tile] == terminals_mask) && (honors == honors_mask);
    }
    else {
        return (manzu == terminals_mask) && (pinzu == terminals_mask) &&
               (souzu == terminals_mask) && (honors - tile1[win_tile] == honors_mask);
    }
}

/**
 * @brief Check if All Simples (断幺九) is established.
 */
YakuFlags score_calculator_detail::check_tanyao(const PlayerState &player,
                                                const MergedHand &merged_hand,
                                                const bool rule_open_tanyao)
{
    const auto &[hand, manzu, pinzu, souzu, honors] = merged_hand;

    if (!rule_open_tanyao && !player.is_closed()) {
        return Yaku::None; // If Open Tanyao is not allowed, closed hand only
    }

    // Check if there are no terminal or honor tiles.
    const int32_t terminals_mask = 0b111'000'000'000'000'000'000'000'111;
    return (manzu & terminals_mask) || (pinzu & terminals_mask) ||
                   (souzu & terminals_mask) || honors
               ? Yaku::None
               : Yaku::Tanyao;
}

/**
 * @brief Check if Half Flush (混一色) or Full Flush (清一色) is established.
 */
YakuFlags score_calculator_detail::check_flush(const MergedHand &merged_hand)
{
    const auto &[hand, manzu, pinzu, souzu, honors] = merged_hand;

    // Check if there is only one type of number tile.
    bool is_flash = (manzu != 0) + (pinzu != 0) + (souzu != 0) == 1;

    if (is_flash) {
        return honors ? Yaku::HalfFlush : Yaku::FullFlush;
    }

    return Yaku::None;
}

/**
 * @brief Check if value tile (役牌) is established.
 *
 * @param[in] input input data
 * @param[in] shanten_type type of winning hand
 * @return list of established yaku
 */
YakuFlags score_calculator_detail::check_value_tile(const RoundState &round_state,
                                                    const PlayerState &player,
                                                    const MergedHand &merged_hand)
{
    const auto &[hand, manzu, pinzu, souzu, honors] = merged_hand;

    YakuFlags yaku_list = Yaku::None;

    // doragons
    if (hand[Tile::WhiteDragon] == 3) {
        yaku_list |= Yaku::WhiteDragon;
    }

    if (hand[Tile::GreenDragon] == 3) {
        yaku_list |= Yaku::GreenDragon;
    }

    if (hand[Tile::RedDragon] == 3) {
        yaku_list |= Yaku::RedDragon;
    }

    // round wind
    if (hand[round_state.round_wind] == 3) {
        if (round_state.round_wind == Tile::East) {
            yaku_list |= Yaku::RoundWindEast;
        }
        else if (round_state.round_wind == Tile::South) {
            yaku_list |= Yaku::RoundWindSouth;
        }
        else if (round_state.round_wind == Tile::West) {
            yaku_list |= Yaku::RoundWindWest;
        }
        else if (round_state.round_wind == Tile::North) {
            yaku_list |= Yaku::RoundWindNorth;
        }
    }

    // seat wind
    if (hand[player.seat_wind] == 3) {
        if (player.seat_wind == Tile::East) {
            yaku_list |= Yaku::SelfWindEast;
        }
        else if (player.seat_wind == Tile::South) {
            yaku_list |= Yaku::SelfWindSouth;
        }
        else if (player.seat_wind == Tile::West) {
            yaku_list |= Yaku::SelfWindWest;
        }
        else if (player.seat_wind == Tile::North) {
            yaku_list |= Yaku::SelfWindNorth;
        }
    }

    return yaku_list;
}

score_calculator_detail::MergedHand
score_calculator_detail::merge_hand(const PlayerState &player)
{
    Hand hand = player.hand;
    for (const auto &block : player.melds) {
        int min_tile = Tile::to_normal(block.tiles.front());
        if (block.type == MeldType::Chi) {
            ++hand[min_tile];
            ++hand[min_tile + 1];
            ++hand[min_tile + 2];
        }
        else {
            hand[min_tile] += 3;
        }
    }

    int32_t manzu = std::accumulate(hand.begin(), hand.begin() + 9, 0,
                                    [](int x, int y) { return x * 8 + y; });
    int32_t pinzu = std::accumulate(hand.begin() + 9, hand.begin() + 18, 0,
                                    [](int x, int y) { return x * 8 + y; });
    int32_t souzu = std::accumulate(hand.begin() + 18, hand.begin() + 27, 0,
                                    [](int x, int y) { return x * 8 + y; });
    int32_t honors = std::accumulate(hand.begin() + 27, hand.begin() + 34, 0,
                                     [](int x, int y) { return x * 8 + y; });

    return {hand, manzu, pinzu, souzu, honors};
}

} // namespace mahjong
