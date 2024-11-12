#include "score_calculator.hpp"

#include <spdlog/spdlog.h>

#include "mahjong/core/hand_separator.hpp"
#include "mahjong/core/shanten_calculator.hpp"
#include "mahjong/utils.hpp"

#undef NDEBUG
#include <cassert>

namespace mahjong
{

/**
 * @brief 点数計算機を作成する。
 */
ScoreCalculator::ScoreCalculator()
    : rules_(RuleFlag::RedDora | RuleFlag::OpenTanyao)
    , round_wind_(Tile::East)
    , self_wind_(Tile::East)
    , num_bonus_sticks_(0)
    , num_deposit_sticks_(0)
{
}

/**
 * @brief 点数を計算する。
 *
 * @param[in] hand 手牌
 * @param[in] win_tile 和了牌
 * @param[in] flag フラグ
 * @return Result 結果
 */
Result ScoreCalculator::calc(const Hand &hand, int win_tile, int flag) const
{
    if (auto [ok, err_msg] = check_arguments(hand, win_tile, flag); !ok)
        return {hand, win_tile, flag, err_msg}; // 異常終了

    if (flag & WinFlag::NagashiMangan)
        return aggregate(hand, win_tile, flag, Yaku::NagasiMangan);

    // 向聴数を計算する。
    auto [shanten_type, syanten] = ShantenCalculator::calc(hand);
    if (syanten != -1) {
        return {hand, win_tile, flag, "和了形ではありません。"};
    }

    // 副露牌を手牌に統合する。
    Hand norm_hand = merge_hand(hand);
    // 和了牌が赤牌の場合、赤なしの牌に変換する。
    int norm_win_tile = red2normal(win_tile);

    YakuList yaku_list = Yaku::Null;

    // 役満をチェックする。
    yaku_list |= check_yakuman(norm_hand, norm_win_tile, flag, shanten_type);
    if (yaku_list) {
        return aggregate(hand, win_tile, flag, yaku_list);
    }

    // 面子構成に関係ない役を調べる。
    yaku_list |= check_not_pattern_yaku(norm_hand, norm_win_tile, flag, shanten_type);

    // 面子構成に関係ある役を調べる。
    auto [pattern_yaku_list, fu, blocks, wait_type] =
        check_pattern_yaku(hand, norm_win_tile, flag, shanten_type);
    yaku_list |= pattern_yaku_list;

    if (!yaku_list)
        return {hand, win_tile, flag, "役がありません。"};

    return aggregate(hand, win_tile, flag, yaku_list, fu, blocks, wait_type);
}

////////////////////////////////////////////////////////////////////////////////////////
/// パラメータを設定する関数
////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief ルールを設定する。
 *
 * @param[in] flag ルールのフラグ
 */
void ScoreCalculator::set_rules(int flag)
{
    rules_ = flag;
}

/**
 * @brief 特定のルールを設定する。
 *
 * @param[in] flag ルールのフラグ
 * @param[in] enabled 有効にするかどうか
 */
void ScoreCalculator::set_rule(int flag, bool enabled)
{
    rules_ = enabled ? rules_ | flag : rules_ & ~flag;
}

/**
 * @brief ルールを取得する。
 *
 * @return int ルール
 */
int ScoreCalculator::rules() const
{
    return rules_;
}

/**
 * @brief 場風牌
 *
 * @param[in] tile 牌
 */
void ScoreCalculator::set_round_wind(int tile)
{
    round_wind_ = tile;
}

/**
 * @brief 場風牌を取得する。
 *
 * @return int 牌
 */
int ScoreCalculator::round_wind() const
{
    return round_wind_;
}

/**
 * @brief 自風牌を設定する
 *
 * @param[in] tile 牌
 */
void ScoreCalculator::set_self_wind(int tile)
{
    self_wind_ = tile;
}

/**
 * @brief 自風牌を取得する。
 *
 * @return int 牌
 */
int ScoreCalculator::self_wind() const
{
    return self_wind_;
}

/**
 * @brief 積み棒の数を設定する。
 *
 * @param[in] n 積み棒の数
 */
void ScoreCalculator::set_bonus_sticks(int n)
{
    num_bonus_sticks_ = n;
}

/**
 * @brief 積み棒の数を取得する。
 *
 * @return int 積み棒の数
 */
int ScoreCalculator::bonus_sticks() const
{
    return num_bonus_sticks_;
}

/**
 * @brief 供託棒の数を設定する。
 *
 * @param[in] n 供託棒の数
 */
void ScoreCalculator::set_deposit_sticks(int n)
{
    num_deposit_sticks_ = n;
}

/**
 * @brief 供託棒の数を取得する。
 *
 * @return int 供託棒の数
 */
int ScoreCalculator::deposit_sticks() const
{
    return num_deposit_sticks_;
}

/**
 * @brief 表ドラの一覧を設定する。
 *
 * @param[in] tiles 表ドラの一覧
 */
void ScoreCalculator::set_dora_tiles(const std::vector<int> &tiles)
{
    dora_tiles_ = tiles;
}

/**
 * @brief 表ドラの一覧を設定する。
 *
 * @param[in] tiles 表ドラの一覧
 */
void ScoreCalculator::set_dora_indicators(const std::vector<int> &tiles)
{
    dora_tiles_.clear();
    for (auto tile : tiles)
        dora_tiles_.push_back(Indicator2Dora.at(tile));
}

/**
 * @brief 表ドラの一覧を取得する。
 *
 * @return std::vector<int> 表ドラの一覧
 */
const std::vector<int> &ScoreCalculator::dora_tiles() const
{
    return dora_tiles_;
}

/**
 * @brief 裏ドラを設定する。
 *
 * @param[in] tiles 裏ドラの一覧
 */
void ScoreCalculator::set_uradora_tiles(const std::vector<int> &tiles)
{
    uradora_tiles_ = tiles;
}

/**
 * @brief 裏ドラの一覧を取得する。
 *
 * @return std::vector<int> 裏ドラの一覧
 */
const std::vector<int> &ScoreCalculator::uradora_tiles() const
{
    return uradora_tiles_;
}

////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief 集計する。
 *
 * @param[in] hand 手牌
 * @param[in] win_tile 和了牌
 * @param[in] yaku_list 成功した役一覧
 * @param[in] n_yakuman 何倍役満か
 * @return Result 結果
 */
Result ScoreCalculator::aggregate(const Hand &hand, int win_tile, int flag,
                                  YakuList yaku_list) const
{
    int score_title;
    std::vector<std::tuple<YakuList, int>> yaku_han_list;
    std::vector<int> score;

    if (yaku_list & Yaku::NagasiMangan) {
        // Nagashi Mangan
        yaku_han_list.emplace_back(Yaku::NagasiMangan, 0);
        score_title = ScoreTitle::Mangan;

        // Nagashi Mangan is treated as Tsumo.
        score = calc_score(true, score_title);
    }
    else {
        // 役満
        int n = 0;
        for (auto yaku : Yaku::Yakuman) {
            if (yaku_list & yaku) {
                yaku_han_list.emplace_back(yaku, Yaku::Han[yaku][0]);
                n += Yaku::Han[yaku][0];
            }
        }

        // 点数のタイトルを計算する。
        score_title = get_score_title(n);

        // 点数を計算する。
        score = calc_score(flag & WinFlag::Tsumo, score_title);
    }

    return {hand, win_tile, flag, yaku_han_list, score_title, score};
}

/**
 * @brief 集計する。
 *
 * @param[in] hand 手牌
 * @param[in] win_tile 和了牌
 * @param[in] yaku_list 成功した役一覧
 * @param[in] hu 符
 * @param[in] blocks 面子構成
 * @return Result 結果
 */
Result ScoreCalculator::aggregate(const Hand &hand, int win_tile, int flag,
                                  YakuList yaku_list, int fu,
                                  const std::vector<Block> &blocks, int wait_type) const
{
    // 飜を計算する。
    int han = 0;
    std::vector<std::tuple<YakuList, int>> yaku_han_list;
    for (auto &yaku : Yaku::NormalYaku) {
        if (yaku_list & yaku) {
            int yaku_han = hand.is_closed() ? Yaku::Han[yaku][0] : Yaku::Han[yaku][1];
            yaku_han_list.emplace_back(yaku, yaku_han);
            han += yaku_han;
        }
    }

    // ドラ集計
    int n_dora = count_dora(hand, dora_tiles_);
    if (n_dora) {
        yaku_han_list.emplace_back(Yaku::Dora, n_dora);
        han += n_dora;
    }

    int n_uradora = count_dora(hand, uradora_tiles_);
    if (n_uradora) {
        yaku_han_list.emplace_back(Yaku::UraDora, n_uradora);
        han += n_uradora;
    }

    if (rules_ & RuleFlag::RedDora) {
        int num_reddora = count_reddora(hand);
        if (num_reddora) {
            yaku_han_list.emplace_back(Yaku::RedDora, num_reddora);
            han += num_reddora;
        }
    }

    // 点数のタイトルを計算する。
    int score_title = get_score_title(fu, han);

    // 点数を計算する。
    auto score = calc_score(flag & WinFlag::Tsumo, score_title, han, fu);

    std::sort(
        yaku_han_list.begin(), yaku_han_list.end(),
        [](const auto &a, const auto &b) { return std::get<0>(a) < std::get<0>(b); });

    return {hand, win_tile,    flag,  yaku_han_list, han,
            fu,   score_title, score, blocks,        wait_type};
}

/**
 * @brief 引数をチェックする。
 *
 * @param[in] hand 手牌
 * @param[in] win_tile 和了牌
 * @param[in] int フラグ
 * @return (エラーかどうか, エラーメッセージ)
 */
std::tuple<bool, std::string>
ScoreCalculator::check_arguments(const Hand &hand, int win_tile, int flag) const
{
    // 和了牌をチェックする。
    if (!hand.counts[red2normal(win_tile)]) {
        std::string err_msg = fmt::format("和了牌 {} が手牌 {} に含まれていません。",
                                          Tile::Name.at(win_tile), hand.to_string());
        return {false, err_msg};
    }

    // 同時に指定できないフラグをチェックする。
    if (!check_exclusive(flag & (WinFlag::Riichi | WinFlag::DoubleRiichi))) {
        std::string err_msg =
            fmt::format("{}、{}はいずれか1つのみ指定できます。",
                        Yaku::Name[Yaku::Riichi], Yaku::Name[Yaku::DoubleRiichi]);
        return {false, err_msg};
    }

    if (!check_exclusive(flag & (WinFlag::RobbingAKong | WinFlag::AfterAKong |
                                 WinFlag::UnderTheSea | WinFlag::UnderTheRiver))) {
        std::string err_msg =
            fmt::format("{}、{}、{}、{}はいずれか1つのみ指定できます。",
                        Yaku::Name[Yaku::RobbingAKong], Yaku::Name[Yaku::AfterAKong],
                        Yaku::Name[Yaku::UnderTheSea], Yaku::Name[Yaku::UnderTheRiver]);
        return {false, err_msg};
    }

    if (!check_exclusive(flag & (WinFlag::BlessingOfHeaven | WinFlag::BlessingOfEarth |
                                 WinFlag::HandOfMan))) {
        std::string err_msg =
            fmt::format("{}、{}、{}はいずれか1つのみ指定できます。",
                        Yaku::Name[Yaku::BlessingOfHeaven],
                        Yaku::Name[Yaku::BlessingOfEarth], Yaku::Name[Yaku::HandOfMan]);
        return {false, err_msg};
    }

    // 条件が必要なフラグをチェックする。
    if ((flag & (WinFlag::Riichi | WinFlag::DoubleRiichi)) && !hand.is_closed()) {
        std::string err_msg =
            fmt::format("{}、{}は門前の場合のみ指定できます。",
                        Yaku::Name[Yaku::Riichi], Yaku::Name[Yaku::DoubleRiichi]);
        return {false, err_msg};
    }

    if ((flag & WinFlag::Ippatsu) &&
        !(flag & (WinFlag::Riichi | WinFlag::DoubleRiichi))) {
        std::string err_msg =
            fmt::format("{}は立直の場合のみ指定できます。", Yaku::Name[Yaku::Ippatsu]);
        return {false, err_msg};
    }

    if ((flag & (WinFlag::UnderTheSea | WinFlag::AfterAKong)) &&
        !(flag & WinFlag::Tsumo)) {
        std::string err_msg =
            fmt::format("{}、{}は自摸和了の場合のみ指定できます。",
                        Yaku::Name[Yaku::UnderTheSea], Yaku::Name[Yaku::AfterAKong]);
        return {false, err_msg};
    }

    return {true, ""};
}

/**
 * @brief Check if yakuman is established.
 *
 * @param[in] hand hand (normalized)
 * @param[in] win_tile win tile (normalized)
 * @param[in] flag flag
 * @param[in] shanten_type type of winning hand
 * @return YakuList list of established yaku
 */
YakuList ScoreCalculator::check_yakuman(const Hand &hand, const int win_tile,
                                        const int flag, const int shanten_type) const
{
    YakuList yaku_list = Yaku::Null;

    if (flag & WinFlag::BlessingOfHeaven) {
        yaku_list |= Yaku::BlessingOfHeaven; // 天和
    }
    else if (flag & WinFlag::BlessingOfEarth) {
        yaku_list |= Yaku::BlessingOfEarth; // 地和
    }
    else if (flag & WinFlag::HandOfMan) {
        yaku_list |= Yaku::HandOfMan; // 人和
    }

    // If both regular hand and seven pairs are established, prioritize the regular hand.
    if (shanten_type & ShantenFlag::Regular) {
        if (check_all_green(hand)) {
            yaku_list |= Yaku::AllGreen; // 緑一色
        }

        if (check_big_three_dragons(hand)) {
            yaku_list |= Yaku::BigThreeDragons; // 大三元
        }

        if (check_big_four_winds(hand)) {
            yaku_list |= Yaku::BigFourWinds; // 大四喜
        }
        else if (check_little_four_winds(hand)) {
            yaku_list |= Yaku::LittleFourWinds; // 小四喜
        }

        if (check_all_honors(hand)) {
            yaku_list |= Yaku::AllHonors; // 字一色
        }

        if (check_true_nine_gates(hand, win_tile)) {
            yaku_list |= Yaku::TrueNineGates; // 純正九蓮宝燈
        }
        else if (check_nine_gates(hand, win_tile)) {
            yaku_list |= Yaku::NineGates; // 九蓮宝燈
        }

        int suuankou = check_four_concealed_triplets(hand, flag, win_tile);
        if (suuankou == 2) {
            yaku_list |= Yaku::SingleWaitFourConcealedTriplets; // 四暗刻単騎
        }
        else if (suuankou == 1) {
            yaku_list |= Yaku::FourConcealedTriplets; // 四暗刻
        }

        if (check_all_terminals(hand)) {
            yaku_list |= Yaku::AllTerminals; // 字一色
        }

        if (check_four_kongs(hand)) {
            yaku_list |= Yaku::FourKongs; // 四槓子
        }
    }
    else if (shanten_type & ShantenFlag::SevenPairs) {
        if (check_all_honors(hand)) {
            yaku_list |= Yaku::AllHonors; // 字一色
        }
    }
    else {
        if (check_thirteen_wait_thirteen_orphans(hand, win_tile)) {
            yaku_list |= Yaku::ThirteenWaitThirteenOrphans; // 国士無双13面待ち
        }
        else {
            yaku_list |= Yaku::ThirteenOrphans; // 国士無双
        }
    }

    return yaku_list;
}

/**
 * @brief 面子構成に関係ない役を判定する。
 *
 * @param[in] hand 手牌 (normalized)
 * @param[in] win_tile 和了牌
 * @param[in] flag フラグ
 * @param[in] shanten_type 和了形の種類
 * @return YakuList 成立した役一覧
 */
YakuList ScoreCalculator::check_not_pattern_yaku(const Hand &hand, int win_tile,
                                                 int flag, int shanten_type) const
{
    YakuList yaku_list = Yaku::Null;

    if (flag & WinFlag::DoubleRiichi) {
        yaku_list |= Yaku::DoubleRiichi; // ダブル立直
    }
    else if (flag & WinFlag::Riichi) {
        yaku_list |= Yaku::Riichi; // 立直
    }

    if (flag & WinFlag::Ippatsu) {
        yaku_list |= Yaku::Ippatsu; // 一発
    }

    if (flag & WinFlag::RobbingAKong) {
        yaku_list |= Yaku::RobbingAKong; // 搶槓
    }
    else if (flag & WinFlag::AfterAKong) {
        yaku_list |= Yaku::AfterAKong; // 嶺上開花
    }
    else if (flag & WinFlag::UnderTheSea) {
        yaku_list |= Yaku::UnderTheSea; // 海底摸月
    }
    else if (flag & WinFlag::UnderTheRiver) {
        yaku_list |= Yaku::UnderTheRiver; // 河底撈魚
    }

    if ((flag & WinFlag::Tsumo) && hand.is_closed()) {
        yaku_list |= Yaku::Tsumo; // 門前清自摸和
    }

    if (check_tanyao(hand)) {
        yaku_list |= Yaku::Tanyao; // 断幺九
    }

    if (check_full_flush(hand)) {
        yaku_list |= Yaku::FullFlush; // 清一色
    }
    else if (check_half_flush(hand)) {
        yaku_list |= Yaku::HalfFlush; // 混一色
    }

    if (check_all_terminals_and_honors(hand)) {
        yaku_list |= Yaku::AllTerminalsAndHonors; // 清老頭
    }

    if (shanten_type & ShantenFlag::Regular) {
        if (check_little_three_dragons(hand)) {
            yaku_list |= Yaku::LittleThreeDragons; // 小三元
        }

        if (check_three_kongs(hand)) {
            yaku_list |= Yaku::ThreeKongs; // 三槓子
        }

        if (hand.counts[Tile::White] == 3) {
            yaku_list |= Yaku::WhiteDragon; // 三元牌 白
        }

        if (hand.counts[Tile::Green] == 3) {
            yaku_list |= Yaku::GreenDragon; // 三元牌 發
        }

        if (hand.counts[Tile::Red] == 3) {
            yaku_list |= Yaku::RedDragon; // 三元牌 中
        }

        if (hand.counts[self_wind_] == 3) {
            if (self_wind_ == Tile::East) {
                yaku_list |= Yaku::SelfWindEast; // 自風 東
            }
            else if (self_wind_ == Tile::South) {
                yaku_list |= Yaku::SelfWindSouth; // 自風 南
            }
            else if (self_wind_ == Tile::West) {
                yaku_list |= Yaku::SelfWindWest; // 自風 西
            }
            else if (self_wind_ == Tile::North) {
                yaku_list |= Yaku::SelfWindNorth; // 自風 北
            }
        }

        if (hand.counts[round_wind_] == 3) {
            if (round_wind_ == Tile::East) {
                yaku_list |= Yaku::RoundWindEast; // 場風 東
            }
            else if (round_wind_ == Tile::South) {
                yaku_list |= Yaku::RoundWindSouth; // 場風 南
            }
            else if (round_wind_ == Tile::West) {
                yaku_list |= Yaku::RoundWindWest; // 場風 西
            }
            else if (round_wind_ == Tile::North) {
                yaku_list |= Yaku::RoundWindNorth; // 場風 北
            }
        }
    }
    else if (shanten_type & ShantenFlag::SevenPairs) {
        yaku_list |= Yaku::SevenPairs; // 七対子
    }

    return yaku_list;
}

/**
 * @brief Check yaku related to the composition of blocks.
 *
 * @param[in] hand hand
 * @param[in] win_tile win tile
 * @param[in] flag flag
 * @param[in] shanten_type type of shanten
 * @return list of (yaku, fu, blocks)
 */
std::tuple<YakuList, int, std::vector<Block>, int>
ScoreCalculator::check_pattern_yaku(const Hand &_hand, int win_tile, int flag,
                                    int shanten_type) const
{
    Hand hand = _hand;
    hand.manzu = std::accumulate(hand.counts.begin(), hand.counts.begin() + 9, 0,
                                 [](int x, int y) { return x * 8 + y; });
    hand.pinzu = std::accumulate(hand.counts.begin() + 9, hand.counts.begin() + 18, 0,
                                 [](int x, int y) { return x * 8 + y; });
    hand.souzu = std::accumulate(hand.counts.begin() + 18, hand.counts.begin() + 27, 0,
                                 [](int x, int y) { return x * 8 + y; });
    hand.honors = std::accumulate(hand.counts.begin() + 27, hand.counts.end(), 0,
                                  [](int x, int y) { return x * 8 + y; });

    if (shanten_type == ShantenFlag::SevenPairs) {
        return {Yaku::Null, Hu::Hu25, {}, WaitType::PairWait};
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
    auto pattern = HandSeparator::separate(hand, win_tile, flag & WinFlag::Tsumo);

    // Find the block composition with the highest score.
    int max_han = 0;
    int max_fu = Hu::Null;
    size_t max_idx;
    YakuList max_yaku_list;
    for (size_t i = 0; i < pattern.size(); ++i) {
        int han, fu;
        const std::vector<Block> &blocks = std::get<0>(pattern[i]);
        int wait_type = std::get<1>(pattern[i]);
        YakuList yaku_list = Yaku::Null;

        // Check if pinfu is established.
        bool is_pinfu = check_pinfu(blocks, wait_type);

        // Check if yaku is established.
        if (hand.is_closed()) {
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
                han += hand.is_closed() ? Yaku::Han[yaku][0] : Yaku::Han[yaku][1];
            }
        }

        // Calculate fu.
        fu = calc_fu(blocks, wait_type, hand.is_closed(), flag & WinFlag::Tsumo,
                     is_pinfu);

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
 * @brief 符を計算する。
 *
 * @param[in] blocks 面子構成
 * @param[in] win_tile 和了牌
 * @param[in] menzen 門前かどうか
 * @param[in] tumo 自摸和了かどうか
 * @return int 符
 */
int ScoreCalculator::calc_fu(const std::vector<Block> &blocks, int wait_type,
                             bool is_closed, bool is_tsumo, bool is_pinfu) const
{
    // Exceptions
    //////////////////////////
    if (is_pinfu && is_tsumo && is_closed) {
        return Hu::Hu20; // Pinfu + Tsumo
    }
    else if (is_pinfu && !is_tsumo && !is_closed) {
        return Hu::Hu30; // Pinfu + Ron
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
        if (block.type & (BlockType::Triplet | BlockType::Kong)) { // 刻子、槓子の場合
            int block_fu = 0;
            if (block.type == (BlockType::Triplet | BlockType::Open)) {
                block_fu = 2; // 明刻子
            }
            else if (block.type == BlockType::Triplet) {
                block_fu = 4; // 暗刻子
            }
            else if (block.type == (BlockType::Kong | BlockType::Open)) {
                block_fu = 8; // 明槓子
            }
            else if (block.type == BlockType::Kong) {
                block_fu = 16; // 暗槓子
            }

            bool yaotyu =
                block.min_tile == Tile::Manzu1 || block.min_tile == Tile::Manzu9 ||
                block.min_tile == Tile::Pinzu1 || block.min_tile == Tile::Pinzu9 ||
                block.min_tile == Tile::Souzu1 || block.min_tile == Tile::Souzu9 ||
                block.min_tile >= Tile::East;
            fu += yaotyu ? block_fu * 2 : block_fu;
        }
        else if (block.type & BlockType::Pair) { // 対子の場合
            if (block.min_tile == self_wind_ && block.min_tile == round_wind_)
                fu += 4; // 連風牌
            else if (block.min_tile == self_wind_ || block.min_tile == round_wind_ ||
                     block.min_tile >= Tile::White)
                fu += 2; // 役牌
        }
    }

    return round_up_fu(fu);
}

/**
 * @brief 符を計算する (内訳)。
 */
std::vector<std::tuple<std::string, int>>
ScoreCalculator::calc_fu_detail(const std::vector<Block> &blocks, int wait_type,
                                bool is_menzen, bool is_tumo) const
{
    bool is_pinfu = check_pinfu(blocks, wait_type);

    // 符計算の例外
    //////////////////////////
    if (is_pinfu && is_tumo && is_menzen) { // 平和、自摸、門前
        return {{"平和・自摸", 20}};
    }
    else if (is_pinfu && !is_tumo && !is_menzen) { // 平和、ロン、非門前
        return {{"喰い平和・ロン", 30}};
    }

    // 通常の符計算
    //////////////////////////

    std::vector<std::tuple<std::string, int>> fu_detail;
    fu_detail.emplace_back("副底", 20);

    if (is_menzen && !is_tumo)
        fu_detail.emplace_back("門前加符", 10);
    else if (is_tumo)
        fu_detail.emplace_back("自摸加符", 2);

    if (wait_type == WaitType::ClosedWait || wait_type == WaitType::EdgeWait ||
        wait_type == WaitType::PairWait)
        fu_detail.emplace_back(fmt::format("待ち: {}", WaitType::Name.at(wait_type)),
                               2);

    for (const auto &block : blocks) {
        if (block.type & (BlockType::Triplet | BlockType::Kong)) {
            int block_fu = 0;
            if (block.type == (BlockType::Triplet | BlockType::Open))
                block_fu = 2; // 明刻子
            else if (block.type == BlockType::Triplet)
                block_fu = 4; // 暗刻子
            else if (block.type == (BlockType::Kong | BlockType::Open))
                block_fu = 8; // 明槓子
            else if (block.type == BlockType::Kong)
                block_fu = 16; // 暗槓子

            bool yaotyu =
                block.min_tile == Tile::Manzu1 || block.min_tile == Tile::Manzu9 ||
                block.min_tile == Tile::Pinzu1 || block.min_tile == Tile::Pinzu9 ||
                block.min_tile == Tile::Souzu1 || block.min_tile == Tile::Souzu9 ||
                block.min_tile >= Tile::East;

            fu_detail.emplace_back(fmt::format("面子構成: {} {}", block.to_string(),
                                               yaotyu ? "幺九牌" : "断幺牌"),
                                   yaotyu ? block_fu * 2 : block_fu);
        }
        else if (block.type & BlockType::Pair) {
            // 対子
            if (block.min_tile == self_wind_ && block.min_tile == round_wind_)
                fu_detail.emplace_back(
                    fmt::format("雀頭: {} 連風牌", block.to_string()), 4);
            else if (block.min_tile == self_wind_ || block.min_tile == round_wind_ ||
                     block.min_tile >= Tile::White)
                fu_detail.emplace_back(fmt::format("雀頭: {} 役牌", block.to_string()),
                                       2);
        }
    }

    return fu_detail;
}

/**
 * @brief Count number of dora tiles.
 *
 * @param[in] hand hand
 * @param[in] dora_tiles list of dora tiles (normalized)
 * @return int number of dora tiles
 */
int ScoreCalculator::count_dora(const Hand &hand, std::vector<int> dora_tiles) const
{
    int num_doras = 0;
    for (auto dora : dora_tiles) {
        num_doras += hand.counts[dora];

        for (const auto &block : hand.melds) {
            for (auto tile : block.tiles) {
                if (red2normal(tile) == dora) {
                    ++num_doras;
                }
            }
        }
    }

    return num_doras;
}

/**
 * @brief Count number of red dora tiles.
 *
 * @param[in] hand hand
 * @param[in] dora_tiles list of dora tiles (normalized)
 * @return int number of dora tiles
 */
int ScoreCalculator::count_reddora(const Hand &hand) const
{
    int num_reddora = 0;

    // 手牌に含まれる赤牌を集計する。
    if (hand.counts[Tile::Manzu5] && hand.aka_manzu5) {
        num_reddora += 1;
    }
    if (hand.counts[Tile::Pinzu5] && hand.aka_pinzu5) {
        num_reddora += 1;
    }
    if (hand.counts[Tile::Souzu5] && hand.aka_souzu5) {
        num_reddora += 1;
    }

    // 副露ブロックに含まれる赤牌を集計する。
    for (const auto &block : hand.melds) {
        for (auto tile : block.tiles) {
            if (is_reddora(tile)) {
                num_reddora++;
                break;
            }
        }
    }

    return num_reddora;
}

/**
 * @brief 副露ブロックを統合した手牌を作成する。
 *        槓子は刻子と同じ扱いで3つの牌だけ手牌に加え、統合後の手牌の枚数が14枚となるようにする。
 *
 * @param[in] hand 手牌
 * @return Hand 副露ブロックを統合した手牌
 */
Hand ScoreCalculator::merge_hand(const Hand &hand) const
{
    Hand norm_hand = hand;
    for (const auto &block : norm_hand.melds) {
        int min_tile = red2normal(block.tiles.front()); // 赤ドラは通常の牌として扱う

        if (block.type == MeldType::Chow) {
            ++norm_hand.counts[min_tile];
            ++norm_hand.counts[min_tile + 1];
            ++norm_hand.counts[min_tile + 2];
        }
        else {
            norm_hand.counts[min_tile] += 3;
        }
    }

    norm_hand.manzu =
        std::accumulate(norm_hand.counts.begin(), norm_hand.counts.begin() + 9, 0,
                        [](int x, int y) { return x * 8 + y; });
    norm_hand.pinzu =
        std::accumulate(norm_hand.counts.begin() + 9, norm_hand.counts.begin() + 18, 0,
                        [](int x, int y) { return x * 8 + y; });
    norm_hand.souzu =
        std::accumulate(norm_hand.counts.begin() + 18, norm_hand.counts.begin() + 27, 0,
                        [](int x, int y) { return x * 8 + y; });
    norm_hand.honors =
        std::accumulate(norm_hand.counts.begin() + 27, norm_hand.counts.end(), 0,
                        [](int x, int y) { return x * 8 + y; });

    return norm_hand;
}

/**
 * @brief プレイヤーの収支を計算する。
 *
 * @param[in] is_tumo 自摸和了かどうか
 * @param[in] score_title タイトル
 * @param[in] han 飜
 * @param[in] fu 符
 * @return std::vector<int>
 *         親の自摸和了の場合 (和了者の収入, 子の支出)
 *         子の自摸和了の場合 (和了者の収入, 親の支出, 子の支出)
 *         ロン和了の場合     (和了者の収入, 放銃者の支出)
 */
std::vector<int> ScoreCalculator::calc_score(bool is_tumo, int score_title, int han,
                                             int fu) const
{
    bool is_parent = self_wind_ == Tile::East;

    if (is_tumo && is_parent) {
        // 親の自摸和了
        int child_payment =
            (score_title == ScoreTitle::Null
                 ? ScoringTable::ParentTumoChild[fu][han - 1]
                 : ScoringTable::ParentTumoChildOverMangan[score_title]) +
            100 * num_bonus_sticks_;
        int score = 1000 * num_deposit_sticks_ + child_payment * 3;

        return {score, child_payment};
    }
    else if (is_tumo && !is_parent) {
        // 子の自摸和了
        int parent_payment =
            (score_title == ScoreTitle::Null
                 ? ScoringTable::ChildTumoParent[fu][han - 1]
                 : ScoringTable::ChildTumoParentOverMangan[score_title]) +
            100 * num_bonus_sticks_;
        int child_payment =
            (score_title == ScoreTitle::Null
                 ? ScoringTable::ChildTumoChild[fu][han - 1]
                 : ScoringTable::ChildTumoChildOverMangan[score_title]) +
            100 * num_bonus_sticks_;
        int score = 1000 * num_deposit_sticks_ + parent_payment + child_payment * 2;

        return {score, parent_payment, child_payment};
    }
    else if (!is_tumo && is_parent) {
        // 親のロン和了
        int payment = (score_title == ScoreTitle::Null
                           ? ScoringTable::ParentRon[fu][han - 1]
                           : ScoringTable::ParentRonOverMangan[score_title]) +
                      300 * num_bonus_sticks_;
        int score = 1000 * num_deposit_sticks_ + payment;

        return {score, payment};
    }
    else {
        // 子のロン和了
        int payment = (score_title == ScoreTitle::Null
                           ? ScoringTable::ChildRon[fu][han - 1]
                           : ScoringTable::ChildRonOverMangan[score_title]) +
                      300 * num_bonus_sticks_;
        int score = 1000 * num_deposit_sticks_ + payment;

        return {score, payment};
    }
}

std::vector<int> ScoreCalculator::get_scores_for_exp(const Result &result)
{
    if (result.score_title >= ScoreTitle::CountedYakuman)
        return {result.score.front()};

    int fu = Hu::Keys.at(result.fu);

    std::vector<int> scores;
    for (int han = result.han; han <= 13; ++han) {
        // 点数のタイトルを計算する。
        int score_title = get_score_title(fu, han);

        // 点数を計算する。
        auto score = calc_score(true, score_title, han, fu);

        scores.push_back(score.front());
    }

    return scores;
}

/**
 * @brief Get score title of non-yakuman.
 *
 * @param[in] hu Hu
 * @param[in] han Han
 * @return score title
 */
int ScoreCalculator::get_score_title(int fu, int han)
{
    if (han < 5) {
        return ScoringTable::IsMangan[fu][han - 1] ? ScoreTitle::Mangan
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
 * @brief Get score title of yakuman.
 *
 * @param[in] n yakuman multiplier
 * @return score title
 */
int ScoreCalculator::get_score_title(int n)
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
 * @brief 符を切り上げる。
 *
 * @param[in] hu 符
 * @return int 切り上げた符
 */
int ScoreCalculator::round_up_fu(int hu)
{
    hu = int(std::ceil(hu / 10.)) * 10;

    switch (hu) {
    case 20:
        return Hu::Hu20;
    case 25:
        return Hu::Hu25;
    case 30:
        return Hu::Hu30;
    case 40:
        return Hu::Hu40;
    case 50:
        return Hu::Hu50;
    case 60:
        return Hu::Hu60;
    case 70:
        return Hu::Hu70;
    case 80:
        return Hu::Hu80;
    case 90:
        return Hu::Hu90;
    case 100:
        return Hu::Hu100;
    case 110:
        return Hu::Hu110;
    }

    return Hu::Null;
}

////////////////////////////////////////////////////////////////////////////////////////
/// Functions to check yaku
////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Check if All Simples (断幺九) is established.
 */
bool ScoreCalculator::check_tanyao(const Hand &hand) const
{
    if (!(rules_ & RuleFlag::OpenTanyao) && !hand.is_closed()) {
        return false; // If Open Tanyao is not allowed, closed hand only
    }

    // Check if there are no terminal or honor tiles.
    const int32_t terminals_mask = 0b111'000'000'000'000'000'000'000'111;
    return !((hand.manzu & terminals_mask) || (hand.pinzu & terminals_mask) ||
             (hand.souzu & terminals_mask) || hand.honors);
}

/**
 * @brief Check if Pinfu (平和) is established.
 */
bool ScoreCalculator::check_pinfu(const std::vector<Block> &blocks,
                                  const int wait_type) const
{
    // Check if the hand is closed before calling this function.

    if (wait_type != WaitType::DoubleEdgeWait) {
        return false; // not double closed wait
    }

    // Check if all blocks are sequences or pairs that are not yakuhai.
    for (const auto &block : blocks) {
        if (block.type & (BlockType::Triplet | BlockType::Kong)) {
            return false; // triplet, kong
        }

        if ((block.type & BlockType::Pair) &&
            (block.min_tile == self_wind_ || block.min_tile == round_wind_ ||
             block.min_tile >= Tile::White)) {
            return false; // yakuhai pair
        }
    }

    return true;
}

/**
 * @brief Check if Pure Double Sequence (一盃口) or
 *        Twice Pure Double Sequence (二盃口) is established.
 */
int ScoreCalculator::check_pure_double_sequence(const std::vector<Block> &blocks) const
{
    // Check if the hand is closed before calling this function.

    std::vector<int> count(34, 0);
    for (const auto &block : blocks) {
        if (block.type & BlockType::Sequence) {
            count[block.min_tile]++; // sequence
        }
    }

    // Count number of iipeikou.
    int num_iipeikou = 0;
    for (const auto &x : count) {
        if (x == 4) {
            num_iipeikou += 2;
        }
        else if (x >= 2) {
            num_iipeikou++;
        }
    }

    return num_iipeikou;
}

/**
 * @brief Check if All Triplets (対々和) is established.
 */
bool ScoreCalculator::check_all_triplets(const std::vector<Block> &blocks) const
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
bool ScoreCalculator::check_three_concealed_triplets(
    const std::vector<Block> &blocks) const
{
    int num_triplets = 0;
    for (const auto &block : blocks) {
        if (block.type == BlockType::Triplet || block.type == BlockType::Kong) {
            num_triplets++; // closed triplet or closed kong
        }
    }

    return num_triplets == 3;
}

/**
 * @brief Check if Triple Triplets (三色同刻) is established.
 */
bool ScoreCalculator::check_triple_triplets(const std::vector<Block> &blocks) const
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
bool ScoreCalculator::check_mixed_triple_sequence(
    const std::vector<Block> &blocks) const
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
bool ScoreCalculator::check_all_terminals_and_honors(const Hand &hand) const
{
    // Check if there are no tiles other than terminal tiles and if there are honor tiles.
    const int32_t tanyao_mask = 0b000'111'111'111'111'111'111'111'000;
    return !((hand.manzu & tanyao_mask) || (hand.pinzu & tanyao_mask) ||
             (hand.souzu & tanyao_mask)) &&
           hand.honors;
}

/**
 * @brief Check if Pure Straight (一気通貫) is established.
 */
bool ScoreCalculator::check_pure_straight(const std::vector<Block> &blocks) const
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
int ScoreCalculator::check_outside_hand(const std::vector<Block> &blocks) const
{
    // | Yaku               | Terminal | Honor | Sequence |
    // | Honchan taiyaochuu | o        | ○     | ○        |
    // | Junchan taiyaochuu | ○        | x     | ○        |
    // | Honroutou          | ○        | ○     | x        |
    // | Chinroutou         | ○        | x     | x        |
    bool honor_block = false; // いずれかのブロックに字牌が含まれるかどうか
    bool sequence_block = false; // いずれかのブロックに順子が含まれるかどうか
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
        return 1; // Honchan taiyaochuu
    }
    if (!honor_block && sequence_block) {
        return 2; // Junchan taiyaochuu
    }

    return 0;
}

/**
 * @brief Check if Little Three Dragons (小三元) is established.
 */
bool ScoreCalculator::check_little_three_dragons(const Hand &hand) const
{
    // Check if the total number of dragon tiles is 8.
    return hand.counts[Tile::White] + hand.counts[Tile::Green] +
               hand.counts[Tile::Red] ==
           8;
}

/**
 * @brief Check if Three Kongs (三槓子) is established.
 */
bool ScoreCalculator::check_three_kongs(const Hand &hand) const
{
    // Check if there are 3 kongs.
    int num_kongs = 0;
    for (const auto &block : hand.melds) {
        // enum values of 2 or more are kongs
        num_kongs += MeldType::ClosedKong <= block.type;
    }

    return num_kongs == 3;
}

/**
 * @brief Check if HalfFlush (Half Flush) is established.
 */
bool ScoreCalculator::check_half_flush(const Hand &hand) const
{
    // Check if there is one type of number tile and if there are honor tiles
    return (hand.manzu && !hand.pinzu && !hand.souzu && hand.honors) ||
           (!hand.manzu && hand.pinzu && !hand.souzu && hand.honors) ||
           (!hand.manzu && !hand.pinzu && hand.souzu && hand.honors);
}

/**
 * @brief Check if Chinitsu (Full Flush) is established.
 */
bool ScoreCalculator::check_full_flush(const Hand &hand) const
{
    // Check if there is only one type of number tile.
    return (hand.manzu && !hand.pinzu && !hand.souzu && !hand.honors) ||
           (!hand.manzu && hand.pinzu && !hand.souzu && !hand.honors) ||
           (!hand.manzu && !hand.pinzu && hand.souzu && !hand.honors);
}

/**
 * @brief Check if All Green (緑一色) is established.
 */
bool ScoreCalculator::check_all_green(const Hand &hand) const
{
    // Check if there are no tiles other than 2, 3, 4, 6, 8 of souzu and green dragon.
    //                         | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |
    const int32_t souzu_mask = 0b111'000'000'000'111'000'111'000'111;
    //                          | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
    const int32_t honor_mask = 0b111'111'111'111'111'000'111;

    return !(hand.manzu || hand.pinzu || hand.souzu & souzu_mask ||
             hand.honors & honor_mask);
}

/**
 * @brief Check if Big Three Dragons (大三元) is established.
 */
bool ScoreCalculator::check_big_three_dragons(const Hand &hand) const
{
    // Check if number of each of the three dragon tiles is 3.
    //                            | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
    const int32_t dragons_mask = 0b000'000'000'000'111'111'111;
    //                    | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
    const int32_t mask = 0b000'000'000'000'011'011'011;

    return (hand.honors & dragons_mask) == mask;
}

/**
 * @brief Check if Little Four Winds (小四喜) is established.
 */
bool ScoreCalculator::check_little_four_winds(const Hand &hand) const
{
    // Check if the total number of wind tiles is 11.
    int sum = 0;
    for (int i = Tile::East; i <= Tile::North; ++i) {
        sum += hand.counts[i];
    }

    return sum == 11;
}

/**
 * @brief Check if All Honors (字一色) is established.
 */
bool ScoreCalculator::check_all_honors(const Hand &hand) const
{
    // Check if there are no tiles other than honor tiles.
    return !(hand.manzu || hand.pinzu || hand.souzu);
}

/**
 * @brief Check if Nine Gates (九連宝燈) is established.
 */
bool ScoreCalculator::check_nine_gates(const Hand &hand, const int win_tile) const
{
    if (!hand.is_closed()) {
        return 0; // Closed hand only
    }

    // Check if number of each terminal tile is 3 or more and number of each chunchan tile is 1 or more.
    const auto &c = hand.counts;
    if (win_tile <= Tile::Manzu9) {
        return c[Tile::Manzu1] >= 3 && c[Tile::Manzu2] && c[Tile::Manzu3] &&
               c[Tile::Manzu4] && c[Tile::Manzu5] && c[Tile::Manzu6] &&
               c[Tile::Manzu7] && c[Tile::Manzu8] && c[Tile::Manzu9] >= 3;
    }
    else if (win_tile <= Tile::Pinzu9) {
        return c[Tile::Pinzu1] >= 3 && c[Tile::Pinzu2] && c[Tile::Pinzu3] &&
               c[Tile::Pinzu4] && c[Tile::Pinzu5] && c[Tile::Pinzu6] &&
               c[Tile::Pinzu7] && c[Tile::Pinzu8] && c[Tile::Pinzu9] >= 3;
    }
    else if (win_tile <= Tile::Souzu9) {
        return c[Tile::Souzu1] >= 3 && c[Tile::Souzu2] && c[Tile::Souzu3] &&
               c[Tile::Souzu4] && c[Tile::Souzu5] && c[Tile::Souzu6] &&
               c[Tile::Souzu7] && c[Tile::Souzu8] && c[Tile::Souzu9] >= 3;
    }

    return false;
}

/**
 * @brief Check if Four Concealed Triplets (四暗刻) is established.
 */
int ScoreCalculator::check_four_concealed_triplets(const Hand &hand, const int win_flag,
                                                   const int win_tile) const
{
    if (!(win_flag & WinFlag::Tsumo)) {
        return 0; // Tsumo win only
    }

    if (!hand.is_closed()) {
        return 0; // Closed hand only
    }

    int num_triplets = 0;
    bool has_head = 0;
    bool single_wait = 0;
    for (int i = 0; i < 34; ++i) {
        if (hand.counts[i] == 3) {
            ++num_triplets;
        }
        else if (hand.counts[i] == 2) {
            has_head = true;
            single_wait = i == win_tile;
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
bool ScoreCalculator::check_all_terminals(const Hand &hand) const
{
    // Check if there are no tiles other than terminal tiles.
    //                              | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |
    const int32_t terminals_mask = 0b000'111'111'111'111'111'111'111'000;

    return !((hand.manzu & terminals_mask) || (hand.pinzu & terminals_mask) ||
             (hand.souzu & terminals_mask) || hand.honors);
}

/**
 * @brief Check if Four Kongs (四槓子) is established.
 */
bool ScoreCalculator::check_four_kongs(const Hand &hand) const
{
    // Check if there are 4 kongs.
    int num_kongs = 0;
    for (const auto &block : hand.melds) {
        // enum values of 2 or more are kongs
        num_kongs += MeldType::ClosedKong <= block.type;
    }

    return num_kongs == 4;
}

/**
 * @brief Check if Big Four Winds (大四喜) is established.
 */
bool ScoreCalculator::check_big_four_winds(const Hand &hand) const
{
    // Check if the total number of wind tiles is 12.
    int sum = 0;
    for (int i = Tile::East; i <= Tile::North; ++i) {
        sum += hand.counts[i];
    }

    return sum == 12;
}

/**
 * @brief Check if True Nine Gates (純正九蓮宝燈) is established.
 */
bool ScoreCalculator::check_true_nine_gates(const Hand &hand, const int win_tile) const
{
    if (!hand.is_closed()) {
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
    int mask = 0b011'001'001'001'001'001'001'001'011;
    if (win_tile <= Tile::Manzu9)
        return hand.manzu - tile1[win_tile] == mask;
    else if (win_tile <= Tile::Pinzu9)
        return hand.pinzu - tile1[win_tile] == mask;
    else if (win_tile <= Tile::Souzu9)
        return hand.souzu - tile1[win_tile] == mask;

    return false;
}

/**
 * @brief Check if Thirteen-wait Thirteen Orphans (国士無双13面待ち) is established.
 */
bool ScoreCalculator::check_thirteen_wait_thirteen_orphans(const Hand &hand,
                                                           const int win_tile) const
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

    int32_t manzu = hand.manzu;
    int32_t pinzu = hand.pinzu;
    int32_t sozu = hand.souzu;
    int32_t zihai = hand.honors;
    if (win_tile <= Tile::Manzu9) {
        manzu -= tile1[win_tile];
    }
    else if (win_tile <= Tile::Pinzu9) {
        pinzu -= tile1[win_tile];
    }
    else if (win_tile <= Tile::Souzu9) {
        sozu -= tile1[win_tile];
    }
    else {
        zihai -= tile1[win_tile];
    }

    return (manzu == terminals_mask) && (pinzu == terminals_mask) &&
           (sozu == terminals_mask) && (zihai == honors_mask);
}
} // namespace mahjong
