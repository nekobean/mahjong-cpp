#undef NDEBUG

#include "score_calculator.hpp"

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

HandSeparator::Input ScoreCalculator::create_input(const Player &player, int win_tile,
                                                   int win_flag)
{
    Input input;
    input.hand = player.hand;
    input.win_tile = to_no_reddora(win_tile);
    input.win_flag = win_flag;
    input.melds = player.melds;

    input.merged_hand = player.hand;
    for (const auto &block : player.melds) {
        int min_tile = to_no_reddora(block.tiles.front()); // 赤ドラは通常の牌として扱う

        if (block.type == MeldType::Chow) {
            ++input.merged_hand[min_tile];
            ++input.merged_hand[min_tile + 1];
            ++input.merged_hand[min_tile + 2];
        }
        else {
            input.merged_hand[min_tile] += 3;
        }
    }

    input.manzu = std::accumulate(input.hand.begin(), input.hand.begin() + 9, 0,
                                  [](int x, int y) { return x * 8 + y; });
    input.pinzu = std::accumulate(input.hand.begin() + 9, input.hand.begin() + 18, 0,
                                  [](int x, int y) { return x * 8 + y; });
    input.souzu = std::accumulate(input.hand.begin() + 18, input.hand.begin() + 27, 0,
                                  [](int x, int y) { return x * 8 + y; });
    input.honors = std::accumulate(input.hand.begin() + 27, input.hand.begin() + 34, 0,
                                   [](int x, int y) { return x * 8 + y; });

    input.merged_manzu =
        std::accumulate(input.merged_hand.begin(), input.merged_hand.begin() + 9, 0,
                        [](int x, int y) { return x * 8 + y; });
    input.merged_pinzu =
        std::accumulate(input.merged_hand.begin() + 9, input.merged_hand.begin() + 18,
                        0, [](int x, int y) { return x * 8 + y; });
    input.merged_souzu =
        std::accumulate(input.merged_hand.begin() + 18, input.merged_hand.begin() + 27,
                        0, [](int x, int y) { return x * 8 + y; });
    input.merged_honors =
        std::accumulate(input.merged_hand.begin() + 27, input.merged_hand.begin() + 34,
                        0, [](int x, int y) { return x * 8 + y; });

    return input;
}

/**
 * @brief Calculate score.
 *
 * @param[in] round round
 * @param[in] player player
 * @param[in] win_tile win tile
 * @param[in] win_flag win flag
 * @return result
 */
Result ScoreCalculator::calc(const Round &round, const Player &player, int win_tile,
                             int win_flag)
{
    if (auto [ok, err_msg] = check_arguments(player, win_tile, win_flag); !ok) {
        return Result(player, win_tile, win_flag, err_msg);
    }

    // 向聴数を計算する。
    const auto [shanten_type, syanten] = ShantenCalculator::calc(
        player.hand, int(player.melds.size()), ShantenFlag::All);
    if (syanten != -1) {
        return Result(player, win_tile, win_flag, u8"The hand is not winning form.");
    }

    return calc_fast(round, player, win_tile, win_flag, shanten_type);
}

/**
 * @brief Calculate score.
 *
 * @param[in] round round
 * @param[in] player player
 * @param[in] win_tile win tile
 * @param[in] win_flag win flag
 * @param[in] shanten_type shanten number type
 * @param[in] shanten shanten number
 * @return result
 */
Result ScoreCalculator::calc_fast(const Round &round, const Player &player,
                                  int win_tile, int win_flag, int shanten_type)
{
    Input input = create_input(player, win_tile, win_flag);

    if (win_flag & WinFlag::NagashiMangan) {
        return aggregate(round, player, win_tile, win_flag, Yaku::NagasiMangan);
    }

    YakuList yaku_list = Yaku::Null;

    // 役満をチェックする。
    yaku_list |= check_yakuman(input, shanten_type);
    if (yaku_list) {
        return aggregate(round, player, win_tile, win_flag, yaku_list);
    }

    // 面子構成に関係ない役を調べる。
    yaku_list |= check_not_pattern_yaku(round, player, input, shanten_type);

    // 面子構成に関係ある役を調べる。
    const auto [pattern_yaku_list, fu, blocks, wait_type] =
        check_pattern_yaku(round, player, input, shanten_type);
    yaku_list |= pattern_yaku_list;

    if (!yaku_list) {
        return Result(player, win_tile, win_flag, u8"No yaku is established.");
    }

    return aggregate(round, player, win_tile, win_flag, yaku_list, fu, blocks,
                     wait_type);
}

/**
 * @brief Aggregate scores for Nagashi Mangan or yakuman.
 *
 * @param[in] round round
 * @param[in] player player
 * @param[in] win_tile win tile
 * @param[in] win_flag win flag
 * @param[in] yaku_list list of yaku
 * @return result
 */
Result ScoreCalculator::aggregate(const Round &round, const Player &player,
                                  const int win_tile, const int win_flag,
                                  YakuList yaku_list)
{
    static const std::vector<YakuList> yakuman = {
        Yaku::BlessingOfHeaven,
        Yaku::BlessingOfEarth,
        Yaku::HandOfMan,
        Yaku::AllGreen,
        Yaku::BigThreeDragons,
        Yaku::LittleFourWinds,
        Yaku::AllHonors,
        Yaku::ThirteenOrphans,
        Yaku::NineGates,
        Yaku::FourConcealedTriplets,
        Yaku::AllTerminals,
        Yaku::FourKongs,
        Yaku::SingleWaitFourConcealedTriplets,
        Yaku::BigFourWinds,
        Yaku::TrueNineGates,
        Yaku::ThirteenWaitThirteenOrphans,
    };

    int score_title;
    std::vector<std::tuple<YakuList, int>> yaku_han_list;
    std::vector<int> score;

    const bool is_dealer = player.wind == Tile::East;

    if (yaku_list & Yaku::NagasiMangan) {
        // Nagashi Mangan
        yaku_han_list.emplace_back(Yaku::NagasiMangan, 0);
        score_title = ScoreTitle::Mangan;

        // Nagashi Mangan is treated as Tsumo.
        score = calc_score(is_dealer, true, round.honba, round.kyotaku, score_title);
    }
    else {
        // Count yakuman multiplier.
        int n = 0;
        for (auto yaku : yakuman) {
            if (yaku_list & yaku) {
                yaku_han_list.emplace_back(yaku, Yaku::Han[yaku][0]);
                n += Yaku::Han[yaku][0];
            }
        }

        score_title = get_score_title(n);
        score = calc_score(is_dealer, win_flag & WinFlag::Tsumo, round.honba,
                           round.kyotaku, score_title);
    }

    return {player, win_tile, win_flag, yaku_han_list, score_title, score};
}

/**
 * @brief Aggregate scores for normal yaku.
 *
 * @param[in] round round
 * @param[in] player player
 * @param[in] win_tile win tile
 * @param[in] win_flag win flag
 * @param[in] yaku_list list of yaku
 * @param[in] fu fu
 * @param[in] blocks list of blocks
 * @param[in] wait_type wait type
 * @return result
 */
Result ScoreCalculator::aggregate(const Round &round, const Player &player,
                                  const int win_tile, const int win_flag,
                                  YakuList yaku_list, int fu,
                                  const std::vector<Block> &blocks, int wait_type)
{
    /*! List of normal yaku */
    static const std::vector<YakuList> normal_yaku = {
        Yaku::Tsumo,
        Yaku::Riichi,
        Yaku::Ippatsu,
        Yaku::Tanyao,
        Yaku::Pinfu,
        Yaku::PureDoubleSequence,
        Yaku::RobbingAKong,
        Yaku::AfterAKong,
        Yaku::UnderTheSea,
        Yaku::UnderTheRiver,
        Yaku::WhiteDragon,
        Yaku::GreenDragon,
        Yaku::RedDragon,
        Yaku::SelfWindEast,
        Yaku::SelfWindSouth,
        Yaku::SelfWindWest,
        Yaku::SelfWindNorth,
        Yaku::RoundWindEast,
        Yaku::RoundWindSouth,
        Yaku::RoundWindWest,
        Yaku::RoundWindNorth,
        Yaku::DoubleRiichi,
        Yaku::SevenPairs,
        Yaku::AllTriplets,
        Yaku::ThreeConcealedTriplets,
        Yaku::TripleTriplets,
        Yaku::MixedTripleSequence,
        Yaku::AllTerminalsAndHonors,
        Yaku::PureStraight,
        Yaku::HalfOutsideHand,
        Yaku::LittleThreeDragons,
        Yaku::ThreeKongs,
        Yaku::HalfFlush,
        Yaku::FullyOutsideHand,
        Yaku::TwicePureDoubleSequence,
        Yaku::FullFlush,
    };

    // Count total number of han.
    int han = 0;
    std::vector<std::tuple<YakuList, int>> yaku_han_list;
    for (auto &yaku : normal_yaku) {
        if (yaku_list & yaku) {
            int yaku_han = player.is_closed() ? Yaku::Han[yaku][0] : Yaku::Han[yaku][1];
            yaku_han_list.emplace_back(yaku, yaku_han);
            han += yaku_han;
        }
    }

    // Count number of doras, uradoras and red doras.
    const int num_doras = count_dora(player.hand, player.melds, round.dora_indicators);
    if (num_doras) {
        yaku_han_list.emplace_back(Yaku::Dora, num_doras);
        han += num_doras;
    }

    const int num_uradoras =
        count_dora(player.hand, player.melds, round.uradora_indicators);
    if (num_uradoras) {
        yaku_han_list.emplace_back(Yaku::UraDora, num_uradoras);
        han += num_uradoras;
    }

    const bool rule_reddora = round.rules & RuleFlag::RedDora;
    const int num_reddoras = count_reddora(rule_reddora, player.hand, player.melds);
    if (num_reddoras) {
        yaku_han_list.emplace_back(Yaku::RedDora, num_reddoras);
        han += num_reddoras;
    }

    const int score_title = get_score_title(fu, han);
    const bool is_dealer = player.wind == Tile::East;
    const std::vector<int> score =
        calc_score(is_dealer, win_flag & WinFlag::Tsumo, round.honba, round.kyotaku,
                   score_title, han, fu);

    std::sort(
        yaku_han_list.begin(), yaku_han_list.end(),
        [](const auto &a, const auto &b) { return std::get<0>(a) < std::get<0>(b); });

    return {player,      win_tile, win_flag, yaku_han_list, han, Fu::Values.at(fu),
            score_title, score,    blocks,   wait_type};
}

/**
 * @brief 引数をチェックする。
 *
 * @param[in] player player
 * @param[in] win_tile win tile
 * @param[in] int flag
 * @return (is valid, error message)
 */
std::tuple<bool, std::string>
ScoreCalculator::check_arguments(const Player &player, int win_tile, int win_flag)
{
    // 和了牌をチェックする。
    if (!player.hand[to_no_reddora(win_tile)]) {
        std::string err_msg =
            fmt::format("和了牌 {} が手牌 {} に含まれていません。",
                        Tile::Name.at(win_tile), to_mpsz(player.hand));
        return {false, err_msg};
    }

    // 同時に指定できないフラグをチェックする。
    if (!check_exclusive(win_flag & (WinFlag::Riichi | WinFlag::DoubleRiichi))) {
        std::string err_msg =
            fmt::format("{}、{}はいずれか1つのみ指定できます。",
                        Yaku::Name[Yaku::Riichi], Yaku::Name[Yaku::DoubleRiichi]);
        return {false, err_msg};
    }

    if (!check_exclusive(win_flag & (WinFlag::RobbingAKong | WinFlag::AfterAKong |
                                     WinFlag::UnderTheSea | WinFlag::UnderTheRiver))) {
        std::string err_msg =
            fmt::format("{}、{}、{}、{}はいずれか1つのみ指定できます。",
                        Yaku::Name[Yaku::RobbingAKong], Yaku::Name[Yaku::AfterAKong],
                        Yaku::Name[Yaku::UnderTheSea], Yaku::Name[Yaku::UnderTheRiver]);
        return {false, err_msg};
    }

    if (!check_exclusive(win_flag & (WinFlag::BlessingOfHeaven |
                                     WinFlag::BlessingOfEarth | WinFlag::HandOfMan))) {
        std::string err_msg =
            fmt::format("{}、{}、{}はいずれか1つのみ指定できます。",
                        Yaku::Name[Yaku::BlessingOfHeaven],
                        Yaku::Name[Yaku::BlessingOfEarth], Yaku::Name[Yaku::HandOfMan]);
        return {false, err_msg};
    }

    // 条件が必要なフラグをチェックする。
    if ((win_flag & (WinFlag::Riichi | WinFlag::DoubleRiichi)) && !player.is_closed()) {
        std::string err_msg =
            fmt::format("{}、{}は門前の場合のみ指定できます。",
                        Yaku::Name[Yaku::Riichi], Yaku::Name[Yaku::DoubleRiichi]);
        return {false, err_msg};
    }

    if ((win_flag & WinFlag::Ippatsu) &&
        !(win_flag & (WinFlag::Riichi | WinFlag::DoubleRiichi))) {
        std::string err_msg =
            fmt::format("{}は立直の場合のみ指定できます。", Yaku::Name[Yaku::Ippatsu]);
        return {false, err_msg};
    }

    if ((win_flag & (WinFlag::UnderTheSea | WinFlag::AfterAKong)) &&
        !(win_flag & WinFlag::Tsumo)) {
        std::string err_msg =
            fmt::format("{}、{}は自摸和了の場合のみ指定できます。",
                        Yaku::Name[Yaku::UnderTheSea], Yaku::Name[Yaku::AfterAKong]);
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
 * @param[in] seat_wind seat wind
 * @return int 符
 */
int ScoreCalculator::calc_fu(const std::vector<Block> &blocks, const int wait_type,
                             const bool is_closed, const bool is_tsumo,
                             const bool is_pinfu, const int round_wind,
                             const int seat_wind)
{
    // Exceptions
    //////////////////////////
    if (is_pinfu && is_tsumo && is_closed) {
        return Fu::Hu20; // Pinfu + Tsumo
    }
    else if (is_pinfu && !is_tsumo && !is_closed) {
        return Fu::Hu30; // Pinfu + Ron
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
    if (wait_type == WaitType::ClosedWait || wait_type == WaitType::EdgeWait ||
        wait_type == WaitType::PairWait) {
        fu += 2; // 嵌張待ち、辺張待ち、単騎待ち
    }

    // 面子構成による符
    for (const auto &block : blocks) {
        if (block.type & (BlockType::Triplet | BlockType::Kong)) {
            int block_fu = 0;
            if (block.type == (BlockType::Triplet | BlockType::Open)) {
                block_fu = 2; // open triplet
            }
            else if (block.type == BlockType::Triplet) {
                block_fu = 4; // closed triplet
            }
            else if (block.type == (BlockType::Kong | BlockType::Open)) {
                block_fu = 8; // open kong
            }
            else if (block.type == BlockType::Kong) {
                block_fu = 16; // closed kong
            }

            fu += is_terminal_or_honor(block.min_tile) ? block_fu * 2 : block_fu;
        }
        else if (block.type & BlockType::Pair) {
            if (block.min_tile == seat_wind && block.min_tile == round_wind) {
                fu += 4; // round wind and seat wind (連風牌)
            }
            else if (block.min_tile == seat_wind || block.min_tile == round_wind ||
                     block.min_tile >= Tile::White) {
                fu += 2; // value tile (役牌)
            }
        }
    }

    return round_fu(fu);
}

std::vector<int> ScoreCalculator::get_scores_for_exp(const Result &result,
                                                     const Round &round, int seat_wind)
{
    if (result.score_title >= ScoreTitle::CountedYakuman)
        return {result.score.front()};

    int fu = Fu::Keys.at(result.fu);

    std::vector<int> scores;
    for (int han = result.han; han <= 13; ++han) {
        // 点数のタイトルを計算する。
        int score_title = get_score_title(fu, han);

        // 点数を計算する。
        const bool is_dealer = seat_wind == Tile::East;
        const auto score = calc_score(is_dealer, true, round.honba, round.kyotaku,
                                      score_title, han, fu);

        scores.push_back(score.front());
    }

    return scores;
}

////////////////////////////////////////////////////////////////////////////////////////
/// Helper functions for calculating score
////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Check if yakuman is established.
 *
 * @param[in] input input data
 * @param[in] shanten_type type of winning hand
 * @return list of established yaku
 */
YakuList ScoreCalculator::check_yakuman(const Input &input, const int shanten_type)
{
    YakuList yaku_list = Yaku::Null;

    if (input.win_flag & WinFlag::BlessingOfHeaven) {
        yaku_list |= Yaku::BlessingOfHeaven;
    }
    else if (input.win_flag & WinFlag::BlessingOfEarth) {
        yaku_list |= Yaku::BlessingOfEarth;
    }
    else if (input.win_flag & WinFlag::HandOfMan) {
        yaku_list |= Yaku::HandOfMan;
    }

    // If both regular hand and seven pairs are established, prioritize the regular hand.
    if (shanten_type & ShantenFlag::Regular) {
        if (check_all_green(input)) {
            yaku_list |= Yaku::AllGreen;
        }

        if (check_big_three_dragons(input)) {
            yaku_list |= Yaku::BigThreeDragons;
        }

        if (check_big_four_winds(input)) {
            yaku_list |= Yaku::BigFourWinds;
        }
        else if (check_little_four_winds(input)) {
            yaku_list |= Yaku::LittleFourWinds;
        }

        if (check_all_honors(input)) {
            yaku_list |= Yaku::AllHonors;
        }

        if (check_true_nine_gates(input)) {
            yaku_list |= Yaku::TrueNineGates;
        }
        else if (check_nine_gates(input)) {
            yaku_list |= Yaku::NineGates;
        }

        int suuankou = check_four_concealed_triplets(input);
        if (suuankou == 2) {
            yaku_list |= Yaku::SingleWaitFourConcealedTriplets;
        }
        else if (suuankou == 1) {
            yaku_list |= Yaku::FourConcealedTriplets;
        }

        if (check_all_terminals(input)) {
            yaku_list |= Yaku::AllTerminals;
        }

        if (check_four_kongs(input)) {
            yaku_list |= Yaku::FourKongs;
        }
    }
    else if (shanten_type & ShantenFlag::SevenPairs) {
        if (check_all_honors(input)) {
            yaku_list |= Yaku::AllHonors;
        }
    }
    else {
        if (check_thirteen_wait_thirteen_orphans(input)) {
            yaku_list |= Yaku::ThirteenWaitThirteenOrphans;
        }
        else {
            yaku_list |= Yaku::ThirteenOrphans;
        }
    }

    return yaku_list;
}

/**
 * @brief Check if yaku is established. (not dependent on how the melds are formed)
 *
 * @param[in] input input data
 * @param[in] shanten_type type of winning hand
 * @return list of established yaku
 */
YakuList ScoreCalculator::check_not_pattern_yaku(const Round &round,
                                                 const Player &player,
                                                 const Input &input,
                                                 const int shanten_type)
{
    YakuList yaku_list = Yaku::Null;

    if (input.win_flag & WinFlag::DoubleRiichi) {
        yaku_list |= Yaku::DoubleRiichi;
    }
    else if (input.win_flag & WinFlag::Riichi) {
        yaku_list |= Yaku::Riichi;
    }

    if (input.win_flag & WinFlag::Ippatsu) {
        yaku_list |= Yaku::Ippatsu;
    }

    if (input.win_flag & WinFlag::RobbingAKong) {
        yaku_list |= Yaku::RobbingAKong;
    }
    else if (input.win_flag & WinFlag::AfterAKong) {
        yaku_list |= Yaku::AfterAKong;
    }
    else if (input.win_flag & WinFlag::UnderTheSea) {
        yaku_list |= Yaku::UnderTheSea;
    }
    else if (input.win_flag & WinFlag::UnderTheRiver) {
        yaku_list |= Yaku::UnderTheRiver;
    }

    if ((input.win_flag & WinFlag::Tsumo) && input.is_closed()) {
        yaku_list |= Yaku::Tsumo;
    }

    const bool rule_open_tanyao = round.rules & RuleFlag::OpenTanyao;
    if (check_tanyao(rule_open_tanyao, input)) {
        yaku_list |= Yaku::Tanyao;
    }

    if (check_full_flush(input)) {
        yaku_list |= Yaku::FullFlush;
    }
    else if (check_half_flush(input)) {
        yaku_list |= Yaku::HalfFlush;
    }

    if (check_all_terminals_and_honors(input)) {
        yaku_list |= Yaku::AllTerminalsAndHonors;
    }

    if (shanten_type & ShantenFlag::Regular) {
        if (check_little_three_dragons(input)) {
            yaku_list |= Yaku::LittleThreeDragons;
        }

        if (check_three_kongs(input)) {
            yaku_list |= Yaku::ThreeKongs;
        }

        if (input.merged_hand[Tile::White] == 3) {
            yaku_list |= Yaku::WhiteDragon;
        }

        if (input.merged_hand[Tile::Green] == 3) {
            yaku_list |= Yaku::GreenDragon;
        }

        if (input.merged_hand[Tile::Red] == 3) {
            yaku_list |= Yaku::RedDragon;
        }

        if (input.merged_hand[player.wind] == 3) {
            if (player.wind == Tile::East) {
                yaku_list |= Yaku::SelfWindEast;
            }
            else if (player.wind == Tile::South) {
                yaku_list |= Yaku::SelfWindSouth;
            }
            else if (player.wind == Tile::West) {
                yaku_list |= Yaku::SelfWindWest;
            }
            else if (player.wind == Tile::North) {
                yaku_list |= Yaku::SelfWindNorth;
            }
        }

        if (input.merged_hand[round.wind] == 3) {
            if (round.wind == Tile::East) {
                yaku_list |= Yaku::RoundWindEast;
            }
            else if (round.wind == Tile::South) {
                yaku_list |= Yaku::RoundWindSouth;
            }
            else if (round.wind == Tile::West) {
                yaku_list |= Yaku::RoundWindWest;
            }
            else if (round.wind == Tile::North) {
                yaku_list |= Yaku::RoundWindNorth;
            }
        }
    }
    else if (shanten_type & ShantenFlag::SevenPairs) {
        yaku_list |= Yaku::SevenPairs;
    }

    return yaku_list;
}

/**
 * @brief Check yaku related to the composition of blocks.
 *
 * @param[in] input input data
 * @param[in] shanten_type shanten type
 * @return (yaku, fu, list of blocks, wait type)
 */
std::tuple<YakuList, int, std::vector<Block>, int>
ScoreCalculator::check_pattern_yaku(const Round &round, const Player &player,
                                    Input &input, int shanten_type)
{
    if (shanten_type == ShantenFlag::SevenPairs) {
        return {Yaku::Null, Fu::Hu25, {}, WaitType::PairWait};
    }

    static const std::vector<YakuList> pattern_yaku = {
        Yaku::Pinfu,
        Yaku::PureDoubleSequence,
        Yaku::AllTriplets,
        Yaku::ThreeConcealedTriplets,
        Yaku::TripleTriplets,
        Yaku::MixedTripleSequence,
        Yaku::PureStraight,
        Yaku::HalfOutsideHand,
        Yaku::ThreeKongs,
        Yaku::FullyOutsideHand,
        Yaku::TwicePureDoubleSequence,
    };

    // Get list of block compositions.
    const auto pattern = HandSeparator::separate(input);

    // Find the block composition with the highest score.
    int max_han = 0;
    int max_fu = Fu::Null;
    size_t max_idx;
    YakuList max_yaku_list;
    for (size_t i = 0; i < pattern.size(); ++i) {
        YakuList yaku_list = Yaku::Null;
        int han, fu;
        const std::vector<Block> &blocks = std::get<0>(pattern[i]);
        int wait_type = std::get<1>(pattern[i]);

        // Check if Pinfu is established.
        const bool is_pinfu = check_pinfu(blocks, wait_type, round.wind, player.wind);

        if (input.is_closed()) {
            if (is_pinfu) {
                yaku_list |= Yaku::Pinfu; // 平和
            }

            int ipeko_type = check_pure_double_sequence(blocks);
            if (ipeko_type == 1) {
                yaku_list |= Yaku::PureDoubleSequence; // 一盃口
            }
            else if (ipeko_type == 2) {
                yaku_list |= Yaku::TwicePureDoubleSequence; // 二盃口
            }
        }

        if (check_pure_straight(blocks)) {
            yaku_list |= Yaku::PureStraight; // 一気通貫
        }
        else if (check_triple_triplets(blocks)) {
            yaku_list |= Yaku::TripleTriplets; // 三色同刻
        }
        else if (check_mixed_triple_sequence(blocks)) {
            yaku_list |= Yaku::MixedTripleSequence; // 三色同順
        }

        int chanta = check_outside_hand(blocks);
        if (chanta == 1) {
            yaku_list |= Yaku::HalfOutsideHand; // 混全帯幺九
        }
        else if (chanta == 2) {
            yaku_list |= Yaku::FullyOutsideHand; // 純全帯幺九
        }

        if (check_all_triplets(blocks)) {
            yaku_list |= Yaku::AllTriplets; // 対々和
        }

        if (check_three_concealed_triplets(blocks)) {
            yaku_list |= Yaku::ThreeConcealedTriplets; // 三暗刻
        }

        // Calculate han.
        han = 0;
        for (const auto &yaku : pattern_yaku) {
            if (yaku_list & yaku) {
                han += input.is_closed() ? Yaku::Han[yaku][0] : Yaku::Han[yaku][1];
            }
        }

        // Calculate fu.
        fu =
            calc_fu(blocks, wait_type, input.is_closed(),
                    input.win_flag & WinFlag::Tsumo, is_pinfu, round.wind, player.wind);

        if (max_han < han || (max_han == han && max_fu < fu)) {
            max_han = han;
            max_fu = fu;
            max_idx = i;
            max_yaku_list = yaku_list;
        }
    }

    return {max_yaku_list, max_fu, std::get<0>(pattern[max_idx]),
            std::get<1>(pattern[max_idx])};
}

/**
 * @brief Calculate score and payment.
 *
 * @param[in] is_dealer  Whether the player is dealer
 * @param[in] is_tsumo Whether the player wins by Tsumo
 * @param[in] honba honba
 * @param[in] kyotaku kyotaku
 * @param[in] score_title score title
 * @param[in] han han
 * @param[in] fu fu
 * @return (winner score, player payment) if dealer wins by Tsumo.
 *         (winner score, dealer payment, player payment) if player wins by Tsumo.
 *         (winner score, discarder payment) if dealer or player wins by Ron.
 */
std::vector<int> ScoreCalculator::calc_score(const bool is_dealer, const bool is_tsumo,
                                             const int honba, const int kyotaku,
                                             const int score_title, const int han,
                                             const int fu)
{
    using namespace ScoreTable;

    if (is_tsumo && is_dealer) {
        // dealer tsumo
        const int player_payment =
            (score_title == ScoreTitle::Null
                 ? BelowMangan[TsumoPlayerToDealer][fu][han - 1]
                 : AboveMangan[TsumoPlayerToDealer][score_title]) +
            100 * honba;
        const int score = 1000 * kyotaku + player_payment * 3;

        return {score, player_payment};
    }
    else if (is_tsumo && !is_dealer) {
        // player tsumo
        const int dealer_payment =
            (score_title == ScoreTitle::Null
                 ? BelowMangan[TsumoDealerToPlayer][fu][han - 1]
                 : AboveMangan[TsumoDealerToPlayer][score_title]) +
            100 * honba;
        const int player_payment =
            (score_title == ScoreTitle::Null
                 ? BelowMangan[TsumoPlayerToPlayer][fu][han - 1]
                 : AboveMangan[TsumoPlayerToPlayer][score_title]) +
            100 * honba;
        const int score = 1000 * kyotaku + dealer_payment + player_payment * 2;

        return {score, dealer_payment, player_payment};
    }
    else if (!is_tsumo && is_dealer) {
        // dealer ron
        const int payment = (score_title == ScoreTitle::Null
                                 ? BelowMangan[RonDiscarderToDealer][fu][han - 1]
                                 : AboveMangan[RonDiscarderToDealer][score_title]) +
                            300 * honba;
        const int score = 1000 * kyotaku + payment;

        return {score, payment};
    }
    else {
        // player ron
        const int payment = (score_title == ScoreTitle::Null
                                 ? BelowMangan[RonDiscarderToPlayer][fu][han - 1]
                                 : AboveMangan[RonDiscarderToPlayer][score_title]) +
                            300 * honba;
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
 * @return number of dora tiles
 */
int ScoreCalculator::count_dora(const Hand &hand, const std::vector<Meld> &melds,
                                const std::vector<int> &indicators)
{
    int num_doras = 0;
    for (const auto tile : indicators) {
        const int dora = ToDora[tile];
        // Count doras in the hand.
        num_doras += hand[dora];

        // Count doras in the melds.
        for (const auto &meld : melds) {
            for (const auto tile : meld.tiles) {
                num_doras += to_no_reddora(tile) == dora;
            }
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
int ScoreCalculator::count_reddora(const bool rule_reddora, const Hand &hand,
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
            if (is_reddora(tile)) {
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
int ScoreCalculator::get_score_title(const int fu, const int han)
{
    if (han < 5) {
        return ScoreTable::IsMangan[fu][han - 1] ? ScoreTitle::Mangan
                                                 : ScoreTitle::Null;
    }

    if (han == 5) {
        return ScoreTitle::Mangan;
    }
    else if (han <= 7) {
        return ScoreTitle::Haneman;
    }
    else if (han <= 10) {
        return ScoreTitle::Baiman;
    }
    else if (han <= 12) {
        return ScoreTitle::Sanbaiman;
    }

    return ScoreTitle::CountedYakuman;
};

/**
 * @brief Get score title for yakuman.
 *
 * @param[in] n yakuman multiplier
 * @return score title
 */
int ScoreCalculator::get_score_title(const int n)
{
    if (n == 1) {
        return ScoreTitle::Yakuman;
    }
    else if (n == 2) {
        return ScoreTitle::DoubleYakuman;
    }
    else if (n == 3) {
        return ScoreTitle::TripleYakuman;
    }
    else if (n == 4) {
        return ScoreTitle::QuadrupleYakuman;
    }
    else if (n == 5) {
        return ScoreTitle::QuintupleYakuman;
    }
    else if (n == 6) {
        return ScoreTitle::SextupleYakuman;
    }

    return ScoreTitle::Null;
};

/**
 * @brief Round up fu.
 *
 * @param[in] fu fu
 * @return rounded fu
 */
int ScoreCalculator::round_fu(const int fu)
{
    const int rounded_fu = int(std::ceil(fu / 10.)) * 10;

    switch (rounded_fu) {
    case 20:
        return Fu::Hu20;
    case 25:
        return Fu::Hu25;
    case 30:
        return Fu::Hu30;
    case 40:
        return Fu::Hu40;
    case 50:
        return Fu::Hu50;
    case 60:
        return Fu::Hu60;
    case 70:
        return Fu::Hu70;
    case 80:
        return Fu::Hu80;
    case 90:
        return Fu::Hu90;
    case 100:
        return Fu::Hu100;
    case 110:
        return Fu::Hu110;
    }

    return Fu::Null;
}

////////////////////////////////////////////////////////////////////////////////////////
/// Functions to check yaku
////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Check if All Simples (断幺九) is established.
 */
bool ScoreCalculator::check_tanyao(const bool rule_open_tanyao, const Input &input)
{
    if (!rule_open_tanyao && !input.is_closed()) {
        return false; // If Open Tanyao is not allowed, closed hand only
    }

    // Check if there are no terminal or honor tiles.
    const int32_t terminals_mask = 0b111'000'000'000'000'000'000'000'111;
    return !((input.merged_manzu & terminals_mask) ||
             (input.merged_pinzu & terminals_mask) ||
             (input.merged_souzu & terminals_mask) || input.merged_honors);
}

/**
 * @brief Check if Pinfu (平和) is established.
 */
bool ScoreCalculator::check_pinfu(const std::vector<Block> &blocks, const int wait_type,
                                  const int round_wind, const int seat_wind)
{
    // Before calling this function, Check if the hand is closed.

    if (wait_type != WaitType::DoubleEdgeWait) {
        return false; // not double closed wait
    }

    // Check if all blocks are sequences or pairs that are not yakuhai.
    for (const auto &block : blocks) {
        if (block.type & (BlockType::Triplet | BlockType::Kong)) {
            return false; // triplet, kong
        }

        if ((block.type & BlockType::Pair) &&
            (block.min_tile == round_wind || block.min_tile == seat_wind ||
             block.min_tile >= Tile::White)) {
            return false; // value tile pair
        }
    }

    return true;
}

/**
 * @brief Check if Pure Double Sequence (一盃口) or
 *        Twice Pure Double Sequence (二盃口) is established.
 */
int ScoreCalculator::check_pure_double_sequence(const std::vector<Block> &blocks)
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

    return num_double_sequences;
}

/**
 * @brief Check if All Triplets (対々和) is established.
 */
bool ScoreCalculator::check_all_triplets(const std::vector<Block> &blocks)
{
    for (const auto &block : blocks) {
        if (block.type & BlockType::Sequence) {
            return false; // sequence
        }
    }

    return true;
}

/**
 * @brief Check if Three Concealed Triplets (三暗刻) is established.
 */
bool ScoreCalculator::check_three_concealed_triplets(const std::vector<Block> &blocks)
{
    int num_triplets = 0;
    for (const auto &block : blocks) {
        if (block.type == BlockType::Triplet || block.type == BlockType::Kong) {
            ++num_triplets; // closed triplet or closed kong
        }
    }

    return num_triplets == 3;
}

/**
 * @brief Check if Triple Triplets (三色同刻) is established.
 */
bool ScoreCalculator::check_triple_triplets(const std::vector<Block> &blocks)
{
    std::vector<int> count(34, 0);
    for (const auto &block : blocks) {
        if (block.type & (BlockType::Triplet | BlockType::Kong)) {
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
bool ScoreCalculator::check_mixed_triple_sequence(const std::vector<Block> &blocks)
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
 * @brief Check if All Terminals and Honors (混老頭) is established.
 */
bool ScoreCalculator::check_all_terminals_and_honors(const Input &input)
{
    // Check if there are no tiles other than terminal tiles and if there are honor tiles.
    const int32_t tanyao_mask = 0b000'111'111'111'111'111'111'111'000;
    return !((input.merged_manzu & tanyao_mask) || (input.merged_pinzu & tanyao_mask) ||
             (input.merged_souzu & tanyao_mask)) &&
           input.merged_honors;
}

/**
 * @brief Check if Pure Straight (一気通貫) is established.
 */
bool ScoreCalculator::check_pure_straight(const std::vector<Block> &blocks)
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
int ScoreCalculator::check_outside_hand(const std::vector<Block> &blocks)
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
        return 1; // Half Outside Hand
    }
    if (!honor_block && sequence_block) {
        return 2; // Fully Outside Hand
    }

    return 0;
}

/**
 * @brief Check if Little Three Dragons (小三元) is established.
 */
bool ScoreCalculator::check_little_three_dragons(const Input &input)
{
    // Check if the total number of dragon tiles is 8.
    return input.merged_hand[Tile::White] + input.merged_hand[Tile::Green] +
               input.merged_hand[Tile::Red] ==
           8;
}

/**
 * @brief Check if Three Kongs (三槓子) is established.
 */
bool ScoreCalculator::check_three_kongs(const Input &input)
{
    // Check if there are 3 kongs.
    int num_kongs = 0;
    for (const auto &block : input.melds) {
        // enum values of 2 or more are kongs
        num_kongs += MeldType::ClosedKong <= block.type;
    }

    return num_kongs == 3;
}

/**
 * @brief Check if HalfFlush (Half Flush) is established.
 */
bool ScoreCalculator::check_half_flush(const Input &input)
{
    // Check if there is one type of number tile and if there are honor tiles
    return (input.merged_manzu && !input.merged_pinzu && !input.merged_souzu &&
            input.merged_honors) ||
           (!input.merged_manzu && input.merged_pinzu && !input.merged_souzu &&
            input.merged_honors) ||
           (!input.merged_manzu && !input.merged_pinzu && input.merged_souzu &&
            input.merged_honors);
}

/**
 * @brief Check if Chinitsu (Full Flush) is established.
 */
bool ScoreCalculator::check_full_flush(const Input &input)
{
    // Check if there is only one type of number tile.
    return (input.merged_manzu && !input.merged_pinzu && !input.merged_souzu &&
            !input.merged_honors) ||
           (!input.merged_manzu && input.merged_pinzu && !input.merged_souzu &&
            !input.merged_honors) ||
           (!input.merged_manzu && !input.merged_pinzu && input.merged_souzu &&
            !input.merged_honors);
}

/**
 * @brief Check if All Green (緑一色) is established.
 */
bool ScoreCalculator::check_all_green(const Input &input)
{
    // Check if there are no tiles other than 2, 3, 4, 6, 8 of souzu and green dragon.
    //                         | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |
    const int32_t souzu_mask = 0b111'000'000'000'111'000'111'000'111;
    //                          | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
    const int32_t honor_mask = 0b111'111'111'111'111'000'111;

    return !(input.merged_manzu || input.merged_pinzu ||
             (input.merged_souzu & souzu_mask) || (input.merged_honors & honor_mask));
}

/**
 * @brief Check if Big Three Dragons (大三元) is established.
 */
bool ScoreCalculator::check_big_three_dragons(const Input &input)
{
    // Check if number of each of the three dragon tiles is 3.
    //                            | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
    const int32_t dragons_mask = 0b000'000'000'000'111'111'111;
    //                    | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
    const int32_t mask = 0b000'000'000'000'011'011'011;

    return (input.merged_honors & dragons_mask) == mask;
}

/**
 * @brief Check if Little Four Winds (小四喜) is established.
 */
bool ScoreCalculator::check_little_four_winds(const Input &input)
{
    // Check if the total number of wind tiles is 11.
    int sum = 0;
    for (int i = Tile::East; i <= Tile::North; ++i) {
        sum += input.merged_hand[i];
    }

    return sum == 11;
}

/**
 * @brief Check if All Honors (字一色) is established.
 */
bool ScoreCalculator::check_all_honors(const Input &input)
{
    // Check if there are no tiles other than honor tiles.
    return !(input.merged_manzu || input.merged_pinzu || input.merged_souzu);
}

/**
 * @brief Check if Nine Gates (九連宝燈) is established.
 */
bool ScoreCalculator::check_nine_gates(const Input &input)
{
    if (!input.melds.empty()) {
        return 0; // Closed hand only
    }

    // Check if number of each terminal tile is 3 or more and number of each chunchan tile is 1 or more.
    const Hand &h = input.merged_hand;
    if (input.win_tile <= Tile::Manzu9) {
        return h[Tile::Manzu1] >= 3 && h[Tile::Manzu2] && h[Tile::Manzu3] &&
               h[Tile::Manzu4] && h[Tile::Manzu5] && h[Tile::Manzu6] &&
               h[Tile::Manzu7] && h[Tile::Manzu8] && h[Tile::Manzu9] >= 3;
    }
    else if (input.win_tile <= Tile::Pinzu9) {
        return h[Tile::Pinzu1] >= 3 && h[Tile::Pinzu2] && h[Tile::Pinzu3] &&
               h[Tile::Pinzu4] && h[Tile::Pinzu5] && h[Tile::Pinzu6] &&
               h[Tile::Pinzu7] && h[Tile::Pinzu8] && h[Tile::Pinzu9] >= 3;
    }
    else if (input.win_tile <= Tile::Souzu9) {
        return h[Tile::Souzu1] >= 3 && h[Tile::Souzu2] && h[Tile::Souzu3] &&
               h[Tile::Souzu4] && h[Tile::Souzu5] && h[Tile::Souzu6] &&
               h[Tile::Souzu7] && h[Tile::Souzu8] && h[Tile::Souzu9] >= 3;
    }

    return false;
}

/**
 * @brief Check if Four Concealed Triplets (四暗刻) is established.
 */
int ScoreCalculator::check_four_concealed_triplets(const Input &input)
{
    if (!(input.win_flag & WinFlag::Tsumo)) {
        return 0; // Tsumo win only
    }

    if (!input.is_closed()) {
        return 0; // Closed hand only, kong is ok
    }

    int num_triplets = 0;
    bool has_head = 0;
    bool single_wait = 0;
    for (int i = 0; i < 34; ++i) {
        if (input.merged_hand[i] == 3) {
            ++num_triplets;
        }
        else if (input.merged_hand[i] == 2) {
            has_head = true;
            single_wait = i == input.win_tile;
        }
    }

    if (num_triplets == 4 && has_head) {
        return single_wait ? 2 : 1;
    }

    return 0;
}

/**
 * @brief Check if All Terminals (清老頭) is established.
 */
bool ScoreCalculator::check_all_terminals(const Input &input)
{
    // Check if there are no tiles other than terminal tiles.
    //                              | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |
    const int32_t terminals_mask = 0b000'111'111'111'111'111'111'111'000;

    return !((input.merged_manzu & terminals_mask) ||
             (input.merged_pinzu & terminals_mask) ||
             (input.merged_souzu & terminals_mask) || input.merged_honors);
}

/**
 * @brief Check if Four Kongs (四槓子) is established.
 */
bool ScoreCalculator::check_four_kongs(const Input &input)
{
    // Check if there are 4 kongs.
    int num_kongs = 0;
    for (const auto &block : input.melds) {
        // enum values of 2 or more are kongs
        num_kongs += MeldType::ClosedKong <= block.type;
    }

    return num_kongs == 4;
}

/**
 * @brief Check if Big Four Winds (大四喜) is established.
 */
bool ScoreCalculator::check_big_four_winds(const Input &input)
{
    // Check if the total number of wind tiles is 12.
    int sum = 0;
    for (int i = Tile::East; i <= Tile::North; ++i) {
        sum += input.hand[i];
    }

    return sum == 12;
}

/**
 * @brief Check if True Nine Gates (純正九蓮宝燈) is established.
 */
bool ScoreCalculator::check_true_nine_gates(const Input &input)
{
    if (!input.melds.empty()) {
        return 0; // Closed hand only
    }

    static const std::array<int32_t, 34> tile1 = {
        1 << 24, 1 << 21, 1 << 18, 1 << 15, 1 << 12, 1 << 9, 1 << 6, 1 << 3, 1,
        1 << 24, 1 << 21, 1 << 18, 1 << 15, 1 << 12, 1 << 9, 1 << 6, 1 << 3, 1,
        1 << 24, 1 << 21, 1 << 18, 1 << 15, 1 << 12, 1 << 9, 1 << 6, 1 << 3, 1,
        1 << 18, 1 << 15, 1 << 12, 1 << 9,  1 << 6,  1 << 3, 1,
    };

    // Exclude the winning tile and
    // check if the number of each terminal tile is 3 and the number of each chunchan tile is 1.
    //          | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |
    const int mask = 0b011'001'001'001'001'001'001'001'011;
    if (input.win_tile <= Tile::Manzu9)
        return input.merged_manzu - tile1[input.win_tile] == mask;
    else if (input.win_tile <= Tile::Pinzu9)
        return input.merged_pinzu - tile1[input.win_tile] == mask;
    else if (input.win_tile <= Tile::Souzu9)
        return input.merged_souzu - tile1[input.win_tile] == mask;

    return false;
}

/**
 * @brief Check if Thirteen-wait Thirteen Orphans (国士無双13面待ち) is established.
 */
bool ScoreCalculator::check_thirteen_wait_thirteen_orphans(const Input &input)
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

    int32_t manzu = input.merged_manzu;
    int32_t pinzu = input.merged_pinzu;
    int32_t sozu = input.merged_souzu;
    int32_t zihai = input.merged_honors;
    if (input.win_tile <= Tile::Manzu9) {
        manzu -= tile1[input.win_tile];
    }
    else if (input.win_tile <= Tile::Pinzu9) {
        pinzu -= tile1[input.win_tile];
    }
    else if (input.win_tile <= Tile::Souzu9) {
        sozu -= tile1[input.win_tile];
    }
    else {
        zihai -= tile1[input.win_tile];
    }

    return (manzu == terminals_mask) && (pinzu == terminals_mask) &&
           (sozu == terminals_mask) && (zihai == honors_mask);
}

} // namespace mahjong
