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
ScoreCalculator2::ScoreCalculator2()
    : rules_(RuleFlag2::AkaDora | RuleFlag2::OpenTanyao)
    , bakaze_(Tile::Ton)
    , zikaze_(Tile::Ton)
    , n_tumibo_(0)
    , n_kyotakubo_(0)
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
Result ScoreCalculator2::calc(const Hand &hand, int win_tile, int flag) const
{
    if (auto [ok, err_msg] = check_arguments(hand, win_tile, flag); !ok)
        return {hand, win_tile, flag, err_msg}; // 異常終了

    if (flag & HandFlag::NagasiMangan)
        return aggregate(hand, win_tile, flag, Yaku::NagasiMangan);

    // 向聴数を計算する。
    auto [shanten_type, syanten] = ShantenCalculator::calc(hand);
    if (syanten != -1) {
        return {hand, win_tile, flag, "和了形ではありません。"};
    }

    // 副露牌を手牌に統合する。
    Hand norm_hand = merge_hand(hand);
    // 和了牌が赤牌の場合、赤なしの牌に変換する。
    int norm_win_tile = aka2normal(win_tile);

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
void ScoreCalculator2::set_rules(int flag)
{
    rules_ = flag;
}

/**
 * @brief 特定のルールを設定する。
 *
 * @param[in] flag ルールのフラグ
 * @param[in] enabled 有効にするかどうか
 */
void ScoreCalculator2::set_rule(int flag, bool enabled)
{
    rules_ = enabled ? rules_ | flag : rules_ & ~flag;
}

/**
 * @brief ルールを取得する。
 *
 * @return int ルール
 */
int ScoreCalculator2::rules() const
{
    return rules_;
}

/**
 * @brief 場風牌
 *
 * @param[in] tile 牌
 */
void ScoreCalculator2::set_bakaze(int tile)
{
    bakaze_ = tile;
}

/**
 * @brief 場風牌を取得する。
 *
 * @return int 牌
 */
int ScoreCalculator2::bakaze() const
{
    return bakaze_;
}

/**
 * @brief 自風牌を設定する
 *
 * @param[in] tile 牌
 */
void ScoreCalculator2::set_zikaze(int tile)
{
    zikaze_ = tile;
}

/**
 * @brief 自風牌を取得する。
 *
 * @return int 牌
 */
int ScoreCalculator2::zikaze() const
{
    return zikaze_;
}

/**
 * @brief 積み棒の数を設定する。
 *
 * @param[in] n 積み棒の数
 */
void ScoreCalculator2::set_num_tumibo(int n)
{
    n_tumibo_ = n;
}

/**
 * @brief 積み棒の数を取得する。
 *
 * @return int 積み棒の数
 */
int ScoreCalculator2::num_tumibo() const
{
    return n_tumibo_;
}

/**
 * @brief 供託棒の数を設定する。
 *
 * @param[in] n 供託棒の数
 */
void ScoreCalculator2::set_num_kyotakubo(int n)
{
    n_kyotakubo_ = n;
}

/**
 * @brief 供託棒の数を取得する。
 *
 * @return int 供託棒の数
 */
int ScoreCalculator2::num_kyotakubo() const
{
    return n_kyotakubo_;
}

/**
 * @brief 表ドラの一覧を設定する。
 *
 * @param[in] tiles 表ドラの一覧
 */
void ScoreCalculator2::set_dora_tiles(const std::vector<int> &tiles)
{
    dora_tiles_ = tiles;
}

/**
 * @brief 表ドラの一覧を設定する。
 *
 * @param[in] tiles 表ドラの一覧
 */
void ScoreCalculator2::set_dora_indicators(const std::vector<int> &tiles)
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
const std::vector<int> &ScoreCalculator2::dora_tiles() const
{
    return dora_tiles_;
}

/**
 * @brief 裏ドラを設定する。
 *
 * @param[in] tiles 裏ドラの一覧
 */
void ScoreCalculator2::set_uradora_tiles(const std::vector<int> &tiles)
{
    uradora_tiles_ = tiles;
}

/**
 * @brief 裏ドラの一覧を取得する。
 *
 * @return std::vector<int> 裏ドラの一覧
 */
const std::vector<int> &ScoreCalculator2::uradora_tiles() const
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
Result ScoreCalculator2::aggregate(const Hand &hand, int win_tile, int flag,
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
                yaku_han_list.emplace_back(yaku, Yaku::Info[yaku].han[0]);
                n += Yaku::Info[yaku].han[0];
            }
        }

        // 点数のタイトルを計算する。
        score_title = ScoreTitle::get_score_title(n);

        // 点数を計算する。
        score = calc_score(flag & HandFlag::Tumo, score_title);
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
Result ScoreCalculator2::aggregate(const Hand &hand, int win_tile, int flag,
                                   YakuList yaku_list, int fu,
                                   const std::vector<Block> &blocks,
                                   int wait_type) const
{
    // 飜を計算する。
    int han = 0;
    std::vector<std::tuple<YakuList, int>> yaku_han_list;
    for (auto &yaku : Yaku::NormalYaku) {
        if (yaku_list & yaku) {
            int yaku_han =
                hand.is_closed() ? Yaku::Info[yaku].han[0] : Yaku::Info[yaku].han[1];
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

    if (rules_ & RuleFlag2::AkaDora) {
        int num_reddora = count_reddora(hand);
        if (num_reddora) {
            yaku_han_list.emplace_back(Yaku::AkaDora, num_reddora);
            han += num_reddora;
        }
    }

    // 点数のタイトルを計算する。
    int score_title = ScoreTitle::get_score_title(fu, han);

    // 点数を計算する。
    auto score = calc_score(flag & HandFlag::Tumo, score_title, han, fu);

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
ScoreCalculator2::check_arguments(const Hand &hand, int win_tile, int flag) const
{
    // 和了牌をチェックする。
    if (!hand.counts[aka2normal(win_tile)]) {
        std::string err_msg = fmt::format("和了牌 {} が手牌 {} に含まれていません。",
                                          Tile::Name.at(win_tile), hand.to_string());
        return {false, err_msg};
    }

    // 同時に指定できないフラグをチェックする。
    if (!check_exclusive(flag & (HandFlag::Reach | HandFlag::DoubleReach))) {
        std::string err_msg = fmt::format("{}、{}はいずれか1つのみ指定できます。",
                                          Yaku::Info[Yaku::Reach].name,
                                          Yaku::Info[Yaku::DoubleReach].name);
        return {false, err_msg};
    }

    if (!check_exclusive(flag & (HandFlag::Tyankan | HandFlag::Rinsyankaiho |
                                 HandFlag::Haiteitumo | HandFlag::Hoteiron))) {
        std::string err_msg = fmt::format(
            "{}、{}、{}、{}はいずれか1つのみ指定できます。",
            Yaku::Info[Yaku::Tyankan].name, Yaku::Info[Yaku::Rinsyankaiho].name,
            Yaku::Info[Yaku::Haiteitumo].name, Yaku::Info[Yaku::Hoteiron].name);
        return {false, err_msg};
    }

    if (!check_exclusive(flag & (HandFlag::Tenho | HandFlag::Tiho | HandFlag::Renho))) {
        std::string err_msg = fmt::format(
            "{}、{}、{}はいずれか1つのみ指定できます。", Yaku::Info[Yaku::Tenho].name,
            Yaku::Info[Yaku::Tiho].name, Yaku::Info[Yaku::Renho].name);
        return {false, err_msg};
    }

    // 条件が必要なフラグをチェックする。
    if ((flag & (HandFlag::Reach | HandFlag::DoubleReach)) && !hand.is_closed()) {
        std::string err_msg = fmt::format("{}、{}は門前の場合のみ指定できます。",
                                          Yaku::Info[Yaku::Reach].name,
                                          Yaku::Info[Yaku::DoubleReach].name);
        return {false, err_msg};
    }

    if ((flag & HandFlag::Ippatu) &&
        !(flag & (HandFlag::Reach | HandFlag::DoubleReach))) {
        std::string err_msg = fmt::format("{}は立直の場合のみ指定できます。",
                                          Yaku::Info[Yaku::Ippatu].name);
        return {false, err_msg};
    }

    if ((flag & (HandFlag::Haiteitumo | HandFlag::Rinsyankaiho)) &&
        !(flag & HandFlag::Tumo)) {
        std::string err_msg = fmt::format("{}、{}は自摸和了の場合のみ指定できます。",
                                          Yaku::Info[Yaku::Haiteitumo].name,
                                          Yaku::Info[Yaku::Rinsyankaiho].name);
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
YakuList ScoreCalculator2::check_yakuman(const Hand &hand, const int win_tile,
                                         const int flag, const int shanten_type) const
{
    YakuList yaku_list = Yaku::Null;

    if (flag & HandFlag::Tenho) {
        yaku_list |= Yaku::Tenho; // Tenhou (Heavenly Hand)
    }
    else if (flag & HandFlag::Tiho) {
        yaku_list |= Yaku::Tiho; // Chiihou (Earthly Hand)
    }
    else if (flag & HandFlag::Renho) {
        yaku_list |= Yaku::Renho; // Renhou (Hand of Man)
    }

    if (shanten_type & ShantenType::Regular) {
        // If both regular hand and seven pairs are established, prioritize the regular hand
        if (check_ryuuiisou(hand)) {
            yaku_list |= Yaku::Ryuiso; // Ryuuiisou (All Green)
        }

        if (check_daisangen(hand)) {
            yaku_list |= Yaku::Daisangen; // Daisangen (Big Three Dragons)
        }

        if (check_daisuushii(hand)) {
            yaku_list |= Yaku::Daisusi; // Daisuushii (Big Four Winds)
        }
        else if (check_shousuushii(hand)) {
            yaku_list |= Yaku::Syosusi; // Shousuushii (Little Four Winds)
        }

        if (check_tsuuiisou(hand)) {
            yaku_list |= Yaku::Tuiso; // Tsuuiisou (All Honors)
        }

        if (check_chuuren_poutou9(hand, win_tile)) {
            yaku_list |= Yaku::Tyurenpoto9; // Junsei Chuuren Poutou (Pure Nine Gates)
        }
        else if (check_chuuren_poutou(hand, win_tile)) {
            yaku_list |= Yaku::Tyurenpoto; // Chuuren Poutou (Nine Gates)
        }

        int suuankou = check_suuankou(hand, flag, win_tile);
        if (suuankou == 2) {
            yaku_list |=
                Yaku::SuankoTanki; // Suuankou (Four Closed Triplets Single Wait)
        }
        else if (suuankou == 1) {
            yaku_list |= Yaku::Suanko; // Suuankou (Four Closed Triplets)
        }

        if (check_chinroutou(hand)) {
            yaku_list |= Yaku::Tinroto; // Chinroutou (All Terminals)
        }

        if (check_suukantsu(hand)) {
            yaku_list |= Yaku::Sukantu; // Suukantsu (Four Kongs)
        }
    }
    else if (shanten_type & ShantenType::Chiitoitsu) {
        if (check_tsuuiisou(hand)) {
            yaku_list |= Yaku::Tuiso; // 字一色
        }
    }
    else {
        if (check_kokushimusou13(hand, win_tile)) {
            yaku_list |=
                Yaku::Kokusimuso13; // Kokushimusou13 (Thirteen Orphans 13-sided wait)
        }
        else {
            yaku_list |= Yaku::Kokusimuso; // Kokushimusou13 (Thirteen Orphans)
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
YakuList ScoreCalculator2::check_not_pattern_yaku(const Hand &hand, int win_tile,
                                                  int flag, int shanten_type) const
{
    YakuList yaku_list = Yaku::Null;

    if (flag & HandFlag::DoubleReach) {
        yaku_list |= Yaku::DoubleReach; // ダブル立直
    }
    else if (flag & HandFlag::Reach) {
        yaku_list |= Yaku::Reach; // 立直
    }

    if (flag & HandFlag::Ippatu) {
        yaku_list |= Yaku::Ippatu; // 一発
    }

    if (flag & HandFlag::Tyankan) {
        yaku_list |= Yaku::Tyankan; // 搶槓
    }
    else if (flag & HandFlag::Rinsyankaiho) {
        yaku_list |= Yaku::Rinsyankaiho; // 嶺上開花
    }
    else if (flag & HandFlag::Haiteitumo) {
        yaku_list |= Yaku::Haiteitumo; // 海底摸月
    }
    else if (flag & HandFlag::Hoteiron) {
        yaku_list |= Yaku::Hoteiron; // 河底撈魚
    }

    if ((flag & HandFlag::Tumo) && hand.is_closed()) {
        yaku_list |= Yaku::Tumo; // 門前清自摸和
    }

    if (check_tanyao(hand))
        yaku_list |= Yaku::Tanyao; // 断幺九

    if (check_chinitsu(hand))
        yaku_list |= Yaku::Tiniso; // 清一色
    else if (check_honitsu(hand))
        yaku_list |= Yaku::Honiso; // 混一色

    if (check_honroutou(hand))
        yaku_list |= Yaku::Honroto; // 清老頭

    if (shanten_type & ShantenType::Regular) {
        if (check_shousangen(hand))
            yaku_list |= Yaku::Syosangen; // 小三元

        if (check_sankantsu(hand))
            yaku_list |= Yaku::Sankantu; // 三槓子

        if (hand.counts[Tile::Haku] == 3) {
            yaku_list |= Yaku::SangenhaiHaku; // 三元牌 (白)
        }

        if (hand.counts[Tile::Hatu] == 3) {
            yaku_list |= Yaku::SangenhaiHatu; // 三元牌 (發)
        }

        if (hand.counts[Tile::Tyun] == 3) {
            yaku_list |= Yaku::SangenhaiTyun; // 三元牌 (中)
        }

        if (hand.counts[zikaze_] == 3) {
            if (zikaze_ == Tile::Ton) {
                yaku_list |= Yaku::ZikazeTon; // 自風牌 (東)
            }
            else if (zikaze_ == Tile::Nan) {
                yaku_list |= Yaku::ZikazeNan; // 自風牌 (南)
            }
            else if (zikaze_ == Tile::Sya) {
                yaku_list |= Yaku::ZikazeSya; // 自風牌 (西)
            }
            else if (zikaze_ == Tile::Pe) {
                yaku_list |= Yaku::ZikazePe; // 自風牌 (北)
            }
        }

        if (hand.counts[bakaze_] == 3) {
            if (bakaze_ == Tile::Ton) {
                yaku_list |= Yaku::BakazeTon; // 場風牌 (東)
            }
            else if (bakaze_ == Tile::Nan) {
                yaku_list |= Yaku::BakazeNan; // 場風牌 (南)
            }
            else if (bakaze_ == Tile::Sya) {
                yaku_list |= Yaku::BakazeSya; // 場風牌 (西)
            }
            else if (bakaze_ == Tile::Pe) {
                yaku_list |= Yaku::BakazePe; // 場風牌 (北)
            }
        }
    }
    else if (shanten_type & ShantenType::Chiitoitsu) {
        yaku_list |= Yaku::Tiitoitu; // 七対子
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
ScoreCalculator2::check_pattern_yaku(const Hand &_hand, int win_tile, int flag,
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

    if (shanten_type == ShantenType::Chiitoitsu) {
        return {Yaku::Null, Hu::Hu25, {}, WaitType::Tanki};
    }

    static const std::vector<YakuList> pattern_yaku = {
        Yaku::Pinhu,        Yaku::Ipeko,          Yaku::Toitoiho,  Yaku::Sananko,
        Yaku::SansyokuDoko, Yaku::SansyokuDozyun, Yaku::IkkiTukan, Yaku::Tyanta,
        Yaku::Sankantu,     Yaku::Zyuntyanta,     Yaku::Ryanpeko,
    };

    // Get list of block compositions.
    auto pattern = HandSeparator::separate(hand, win_tile, flag & HandFlag::Tumo);

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
                yaku_list |= Yaku::Pinhu; // Pinfu
            }

            int ipeko_type = check_iipeikou(blocks);
            if (ipeko_type == 1) {
                yaku_list |= Yaku::Ipeko; // Iipeikou
            }
            else if (ipeko_type == 2) {
                yaku_list |= Yaku::Ryanpeko; // Ryanpeikou
            }
        }

        if (check_ikkitsuukan(blocks)) {
            yaku_list |= Yaku::IkkiTukan; // Ikkitsuukan
        }
        else if (check_sanshoku_doukou(blocks)) {
            yaku_list |= Yaku::SansyokuDoko; // Sanshoku Doukou
        }
        else if (check_sanshoku_doujun(blocks)) {
            yaku_list |= Yaku::SansyokuDozyun; // Sanshoku Doujun
        }

        int chanta = check_chanta(blocks);
        if (chanta == 1) {
            yaku_list |= Yaku::Tyanta; // Honchan Taiyaochuu
        }
        else if (chanta == 2) {
            yaku_list |= Yaku::Zyuntyanta; // Junchan Taiyaochuu
        }

        if (check_toitoihou(blocks)) {
            yaku_list |= Yaku::Toitoiho; // Toitoihou
        }

        if (check_sanankou(blocks)) {
            yaku_list |= Yaku::Sananko; // Sanankou
        }

        // Calculate han.
        han = 0;
        for (const auto &yaku : pattern_yaku) {
            if (yaku_list & yaku) {
                han += hand.is_closed() ? Yaku::Info[yaku].han[0]
                                        : Yaku::Info[yaku].han[1];
            }
        }

        // Calculate fu.
        fu = calc_fu(blocks, wait_type, hand.is_closed(), flag & HandFlag::Tumo,
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
int ScoreCalculator2::calc_fu(const std::vector<Block> &blocks, int wait_type,
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
    if (wait_type == WaitType::Kantyan || wait_type == WaitType::Pentyan ||
        wait_type == WaitType::Tanki) {
        fu += 2; // 嵌張待ち、辺張待ち、単騎待ち
    }

    // 面子構成による符
    for (const auto &block : blocks) {
        if (block.type & (BlockType::Kotu | BlockType::Kantu)) { // 刻子、槓子の場合
            int block_fu = 0;
            if (block.type == (BlockType::Kotu | BlockType::Open)) {
                block_fu = 2; // 明刻子
            }
            else if (block.type == BlockType::Kotu) {
                block_fu = 4; // 暗刻子
            }
            else if (block.type == (BlockType::Kantu | BlockType::Open)) {
                block_fu = 8; // 明槓子
            }
            else if (block.type == BlockType::Kantu) {
                block_fu = 16; // 暗槓子
            }

            bool yaotyu =
                block.min_tile == Tile::Manzu1 || block.min_tile == Tile::Manzu9 ||
                block.min_tile == Tile::Pinzu1 || block.min_tile == Tile::Pinzu9 ||
                block.min_tile == Tile::Sozu1 || block.min_tile == Tile::Sozu9 ||
                block.min_tile >= Tile::Ton;
            fu += yaotyu ? block_fu * 2 : block_fu;
        }
        else if (block.type & BlockType::Toitu) { // 対子の場合
            if (block.min_tile == zikaze_ && block.min_tile == bakaze_)
                fu += 4; // 連風牌
            else if (block.min_tile == zikaze_ || block.min_tile == bakaze_ ||
                     block.min_tile >= Tile::Haku)
                fu += 2; // 役牌
        }
    }

    return Hu::round_up_fu(fu);
}

/**
 * @brief 符を計算する (内訳)。
 */
std::vector<std::tuple<std::string, int>>
ScoreCalculator2::calc_fu_detail(const std::vector<Block> &blocks, int wait_type,
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

    if (wait_type == WaitType::Kantyan || wait_type == WaitType::Pentyan ||
        wait_type == WaitType::Tanki)
        fu_detail.emplace_back(fmt::format("待ち: {}", WaitType::Name.at(wait_type)),
                               2);

    for (const auto &block : blocks) {
        if (block.type & (BlockType::Kotu | BlockType::Kantu)) {
            int block_fu = 0;
            if (block.type == (BlockType::Kotu | BlockType::Open))
                block_fu = 2; // 明刻子
            else if (block.type == BlockType::Kotu)
                block_fu = 4; // 暗刻子
            else if (block.type == (BlockType::Kantu | BlockType::Open))
                block_fu = 8; // 明槓子
            else if (block.type == BlockType::Kantu)
                block_fu = 16; // 暗槓子

            bool yaotyu =
                block.min_tile == Tile::Manzu1 || block.min_tile == Tile::Manzu9 ||
                block.min_tile == Tile::Pinzu1 || block.min_tile == Tile::Pinzu9 ||
                block.min_tile == Tile::Sozu1 || block.min_tile == Tile::Sozu9 ||
                block.min_tile >= Tile::Ton;

            fu_detail.emplace_back(fmt::format("面子構成: {} {}", block.to_string(),
                                               yaotyu ? "幺九牌" : "断幺牌"),
                                   yaotyu ? block_fu * 2 : block_fu);
        }
        else if (block.type & BlockType::Toitu) {
            // 対子
            if (block.min_tile == zikaze_ && block.min_tile == bakaze_)
                fu_detail.emplace_back(
                    fmt::format("雀頭: {} 連風牌", block.to_string()), 4);
            else if (block.min_tile == zikaze_ || block.min_tile == bakaze_ ||
                     block.min_tile >= Tile::Haku)
                fu_detail.emplace_back(fmt::format("雀頭: {} 役牌", block.to_string()),
                                       2);
        }
    }

    return fu_detail;
}

////////////////////////////////////////////////////////////////////////////////////////
/// Functions to check yaku
////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Check if Ryuuiisou (All Green) is established.
 */
bool ScoreCalculator2::check_ryuuiisou(const Hand &hand) const
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
 * @brief Check if Daisangen (Big Three Dragons) is established.
 */
bool ScoreCalculator2::check_daisangen(const Hand &hand) const
{
    // Check if number of each of the three dragon tiles is 3.
    //                            | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
    const int32_t dragons_mask = 0b000'000'000'000'111'111'111;
    //                    | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
    const int32_t mask = 0b000'000'000'000'011'011'011;

    return (hand.honors & dragons_mask) == mask;
}

/**
 * @brief Check if Shousuushii (Little Four Winds) is established.
 */
bool ScoreCalculator2::check_shousuushii(const Hand &hand) const
{
    // Check if the total number of wind tiles is 11.
    int sum = 0;
    for (int i = Tile::Ton; i <= Tile::Pe; ++i) {
        sum += hand.counts[i];
    }

    return sum == 11;
}

/**
 * @brief Check if Tsuuiisou (All Honors) is established.
 */
bool ScoreCalculator2::check_tsuuiisou(const Hand &hand) const
{
    // Check if there are no tiles other than honor tiles.
    return !(hand.manzu || hand.pinzu || hand.souzu);
}

/**
 * @brief Check if Chuuren Poutou (Nine Gates) is established.
 */
bool ScoreCalculator2::check_chuuren_poutou(const Hand &hand, int win_tile) const
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
    else if (win_tile <= Tile::Sozu9) {
        return c[Tile::Sozu1] >= 3 && c[Tile::Sozu2] && c[Tile::Sozu3] &&
               c[Tile::Sozu4] && c[Tile::Sozu5] && c[Tile::Sozu6] && c[Tile::Sozu7] &&
               c[Tile::Sozu8] && c[Tile::Sozu9] >= 3;
    }

    return false;
}

/**
 * @brief Check if Junsei Chuuren Poutou (Pure Nine Gates) is established.
 */
bool ScoreCalculator2::check_chuuren_poutou9(const Hand &hand, int win_tile) const
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
    else if (win_tile <= Tile::Sozu9)
        return hand.souzu - tile1[win_tile] == mask;

    return false;
}

/**
 * @brief Check if Suuankou (Four Closed Triplets) is established.
 */
int ScoreCalculator2::check_suuankou(const Hand &hand, int flag, int win_tile) const
{
    if (!(flag & HandFlag::Tumo)) {
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
 * @brief Check if Chinroutou (All Terminals) is established.
 */
bool ScoreCalculator2::check_chinroutou(const Hand &hand) const
{
    // Check if there are no tiles other than terminal tiles.
    //                              | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |
    const int32_t terminals_mask = 0b000'111'111'111'111'111'111'111'000;

    return !((hand.manzu & terminals_mask) || (hand.pinzu & terminals_mask) ||
             (hand.souzu & terminals_mask) || hand.honors);
}

/**
 * @brief Check if Suukantsu (Four Kongs) is established.
 */
bool ScoreCalculator2::check_suukantsu(const Hand &hand) const
{
    // Check if there are 4 kongs.
    int num_kongs = 0;
    for (const auto &block : hand.melds) {
        // enum values of 2 or more are kongs
        num_kongs += MeldType::Ankan <= block.type;
    }

    return num_kongs == 4;
}

/**
 * @brief Check if Daisuushii (Big Four Winds) is established.
 */
bool ScoreCalculator2::check_daisuushii(const Hand &hand) const
{
    // Check if the total number of wind tiles is 12.
    int sum = 0;
    for (int i = Tile::Ton; i <= Tile::Pe; ++i) {
        sum += hand.counts[i];
    }

    return sum == 12;
}

/**
 * @brief Check if Kokushimusou13 (Thirteen Orphans 13-sided wait) is established.
 */
bool ScoreCalculator2::check_kokushimusou13(const Hand &hand, int win_tile) const
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
    else if (win_tile <= Tile::Sozu9) {
        sozu -= tile1[win_tile];
    }
    else {
        zihai -= tile1[win_tile];
    }

    return (manzu == terminals_mask) && (pinzu == terminals_mask) &&
           (sozu == terminals_mask) && (zihai == honors_mask);
}

/**
 * @brief Check if Tanyao (All Simples) is established.
 */
bool ScoreCalculator2::check_tanyao(const Hand &hand) const
{
    if (!(rules_ & RuleFlag2::OpenTanyao) && !hand.is_closed()) {
        return false; // If Open Tanyao is not allowed, closed hand only
    }

    // Check if there are no terminal or honor tiles.
    const int32_t terminals_mask = 0b111'000'000'000'000'000'000'000'111;
    return !((hand.manzu & terminals_mask) || (hand.pinzu & terminals_mask) ||
             (hand.souzu & terminals_mask) || hand.honors);
}

/**
 * @brief Check if Honroutou (Terminals and Honors) is established.
 */
bool ScoreCalculator2::check_honroutou(const Hand &hand) const
{
    // Check if there are no tiles other than terminal tiles and if there are honor tiles.
    const int32_t tanyao_mask = 0b000'111'111'111'111'111'111'111'000;
    return !((hand.manzu & tanyao_mask) || (hand.pinzu & tanyao_mask) ||
             (hand.souzu & tanyao_mask)) &&
           hand.honors;
}

/**
 * @brief Check if Honitsu (Half Flush) is established.
 */
bool ScoreCalculator2::check_honitsu(const Hand &hand) const
{
    // Check if there is one type of number tile and if there are honor tiles
    return (hand.manzu && !hand.pinzu && !hand.souzu && hand.honors) ||
           (!hand.manzu && hand.pinzu && !hand.souzu && hand.honors) ||
           (!hand.manzu && !hand.pinzu && hand.souzu && hand.honors);
}

/**
 * @brief Check if Chinitsu (Full Flush) is established.
 */
bool ScoreCalculator2::check_chinitsu(const Hand &hand) const
{
    // Check if there is only one type of number tile.
    return (hand.manzu && !hand.pinzu && !hand.souzu && !hand.honors) ||
           (!hand.manzu && hand.pinzu && !hand.souzu && !hand.honors) ||
           (!hand.manzu && !hand.pinzu && hand.souzu && !hand.honors);
}

/**
 * @brief Check if Shousangen (Little Three Dragons) is established.
 */
bool ScoreCalculator2::check_shousangen(const Hand &hand) const
{
    // Check if the total number of dragon tiles is 8.
    return hand.counts[Tile::Haku] + hand.counts[Tile::Hatu] +
               hand.counts[Tile::Tyun] ==
           8;
}

/**
 * @brief Check if Sankantsu (Three Kongs) is established.
 */
bool ScoreCalculator2::check_sankantsu(const Hand &hand) const
{
    // Check if there are 3 kongs.
    int num_kongs = 0;
    for (const auto &block : hand.melds) {
        // enum values of 2 or more are kongs
        num_kongs += MeldType::Ankan <= block.type;
    }

    return num_kongs == 3;
}

/**
 * @brief Check if Pinfu (All Sequences) is established.
 */
bool ScoreCalculator2::check_pinfu(const std::vector<Block> blocks,
                                   const int wait_type) const
{
    // Check if the hand is closed before calling this function.

    if (wait_type != WaitType::Ryanmen) {
        return false; // not double closed wait
    }

    // Check if all blocks are sequences or pairs that are not yakuhai.
    for (const auto &block : blocks) {
        if (block.type & (BlockType::Kotu | BlockType::Kantu)) {
            return false; // triplet, kong
        }

        if ((block.type & BlockType::Toitu) &&
            (block.min_tile == zikaze_ || block.min_tile == bakaze_ ||
             block.min_tile >= Tile::Haku)) {
            return false; // yakuhai pair
        }
    }

    return true;
}

/**
 * @brief Check if Iipeikou (Two Identical Sequences) or
 *        Ryanpeikou (Two Sets of Identical Sequences) is established.
 */
int ScoreCalculator2::check_iipeikou(const std::vector<Block> blocks) const
{
    // Check if the hand is closed before calling this function.

    std::vector<int> count(34, 0);
    for (const auto &block : blocks) {
        if (block.type & BlockType::Syuntu) {
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
 * @brief Check if Ikkitsuukan (Straight) is established.
 */
bool ScoreCalculator2::check_ikkitsuukan(const std::vector<Block> blocks) const
{
    std::vector<int> count(34, 0);
    for (const auto &block : blocks) {
        if (block.type & BlockType::Syuntu) {
            count[block.min_tile]++; // sequence
        }
    }

    // Check if there is 123, 456, 789 in any type of number tiles.
    return (count[Tile::Manzu1] && count[Tile::Manzu4] && count[Tile::Manzu7]) ||
           (count[Tile::Pinzu1] && count[Tile::Pinzu4] && count[Tile::Pinzu7]) ||
           (count[Tile::Sozu1] && count[Tile::Sozu4] && count[Tile::Sozu7]);
}

/**
 * @brief Check if Sanshoku Doukou (Three Colored Triplets) is established.
 */
bool ScoreCalculator2::check_sanshoku_doukou(const std::vector<Block> blocks) const
{
    std::vector<int> count(34, 0);
    for (const auto &block : blocks) {
        if (block.type & (BlockType::Kotu | BlockType::Kantu)) {
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
 * @brief Check if Sanshoku Doujun (Three Colored Sequences) is established.
 */
bool ScoreCalculator2::check_sanshoku_doujun(const std::vector<Block> blocks) const
{
    std::vector<int> count(34, 0);
    for (const auto &block : blocks) {
        if (block.type & BlockType::Syuntu) {
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
 * @brief Check if Toitoihou (All Triplets) is established.
 */
bool ScoreCalculator2::check_toitoihou(const std::vector<Block> blocks) const
{
    for (const auto &block : blocks) {
        if (block.type & BlockType::Syuntu) {
            return false; // sequence
        }
    }

    return true;
}

/**
 * @brief Check if Honchan taiyaochuu (Terminal or honor in each group) or
 *        Junchan taiyaochuu (Terminal in each meld) is established.
 */
int ScoreCalculator2::check_chanta(const std::vector<Block> blocks) const
{
    // | Yaku               | Terminal | Honor | Sequence |
    // | Honchan taiyaochuu | o        | ○     | ○        |
    // | Junchan taiyaochuu | ○        | x     | ○        |
    // | Honroutou          | ○        | ○     | x        |
    // | Chinroutou         | ○        | x     | x        |
    bool honor_block = false; // いずれかのブロックに字牌が含まれるかどうか
    bool sequence_block = false; // いずれかのブロックに順子が含まれるかどうか
    for (const auto &block : blocks) {
        if (block.type & BlockType::Syuntu) {
            if (!(block.min_tile == Tile::Manzu1 || block.min_tile == Tile::Manzu7 ||
                  block.min_tile == Tile::Pinzu1 || block.min_tile == Tile::Pinzu7 ||
                  block.min_tile == Tile::Sozu1 || block.min_tile == Tile::Sozu7)) {
                return 0; // sequence that does not contain terminal or honor tiles
            }

            sequence_block = true;
        }
        else {
            if (!(block.min_tile == Tile::Manzu1 || block.min_tile == Tile::Manzu9 ||
                  block.min_tile == Tile::Pinzu1 || block.min_tile == Tile::Pinzu9 ||
                  block.min_tile == Tile::Sozu1 || block.min_tile == Tile::Sozu9 ||
                  block.min_tile >= Tile::Ton)) {
                return 0; // triplet, kong, pair that does not contain terminal or honor tiles
            }

            honor_block |= block.min_tile >= Tile::Ton;
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
 * @brief Check if Sanankou (Three Closed Triplets) is established.
 */
bool ScoreCalculator2::check_sanankou(const std::vector<Block> blocks) const
{
    int num_triplets = 0;
    for (const auto &block : blocks) {
        if (block.type == BlockType::Kotu || block.type == BlockType::Kantu) {
            num_triplets++; // closed triplet or closed kong
        }
    }

    return num_triplets == 3;
}

/**
 * @brief Count number of dora tiles.
 *
 * @param[in] hand hand
 * @param[in] dora_tiles list of dora tiles (normalized)
 * @return int number of dora tiles
 */
int ScoreCalculator2::count_dora(const Hand &hand, std::vector<int> dora_tiles) const
{
    int num_doras = 0;
    for (auto dora : dora_tiles) {
        num_doras += hand.counts[dora];

        for (const auto &block : hand.melds) {
            for (auto tile : block.tiles) {
                if (aka2normal(tile) == dora) {
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
int ScoreCalculator2::count_reddora(const Hand &hand) const
{
    int num_reddora = 0;

    // 手牌に含まれる赤牌を集計する。
    if (hand.counts[Tile::Manzu5] && hand.aka_manzu5) {
        num_reddora += 1;
    }
    if (hand.counts[Tile::Pinzu5] && hand.aka_pinzu5) {
        num_reddora += 1;
    }
    if (hand.counts[Tile::Sozu5] && hand.aka_souzu5) {
        num_reddora += 1;
    }

    // 副露ブロックに含まれる赤牌を集計する。
    for (const auto &block : hand.melds) {
        for (auto tile : block.tiles) {
            if (is_akahai(tile)) {
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
Hand ScoreCalculator2::merge_hand(const Hand &hand) const
{
    Hand norm_hand = hand;
    for (const auto &block : norm_hand.melds) {
        int min_tile = aka2normal(block.tiles.front()); // 赤ドラは通常の牌として扱う

        if (block.type == MeldType::Ti) {
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
std::vector<int> ScoreCalculator2::calc_score(bool is_tumo, int score_title, int han,
                                              int fu) const
{
    bool is_parent = zikaze_ == Tile::Ton;

    if (is_tumo && is_parent) {
        // 親の自摸和了
        int child_payment =
            (score_title == ScoreTitle::Null
                 ? ScoringTable::ParentTumoChild[fu][han - 1]
                 : ScoringTable::ParentTumoChildOverMangan[score_title]) +
            100 * n_tumibo_;
        int score = 1000 * n_kyotakubo_ + child_payment * 3;

        return {score, child_payment};
    }
    else if (is_tumo && !is_parent) {
        // 子の自摸和了
        int parent_payment =
            (score_title == ScoreTitle::Null
                 ? ScoringTable::ChildTumoParent[fu][han - 1]
                 : ScoringTable::ChildTumoParentOverMangan[score_title]) +
            100 * n_tumibo_;
        int child_payment =
            (score_title == ScoreTitle::Null
                 ? ScoringTable::ChildTumoChild[fu][han - 1]
                 : ScoringTable::ChildTumoChildOverMangan[score_title]) +
            100 * n_tumibo_;
        int score = 1000 * n_kyotakubo_ + parent_payment + child_payment * 2;

        return {score, parent_payment, child_payment};
    }
    else if (!is_tumo && is_parent) {
        // 親のロン和了
        int payment = (score_title == ScoreTitle::Null
                           ? ScoringTable::ParentRon[fu][han - 1]
                           : ScoringTable::ParentRonOverMangan[score_title]) +
                      300 * n_tumibo_;
        int score = 1000 * n_kyotakubo_ + payment;

        return {score, payment};
    }
    else {
        // 子のロン和了
        int payment = (score_title == ScoreTitle::Null
                           ? ScoringTable::ChildRon[fu][han - 1]
                           : ScoringTable::ChildRonOverMangan[score_title]) +
                      300 * n_tumibo_;
        int score = 1000 * n_kyotakubo_ + payment;

        return {score, payment};
    }
}

std::vector<int> ScoreCalculator2::get_scores_for_exp(const Result &result)
{
    if (result.score_title >= ScoreTitle::KazoeYakuman)
        return {result.score.front()};

    int fu = Hu::Keys.at(result.fu);

    std::vector<int> scores;
    for (int han = result.han; han <= 13; ++han) {
        // 点数のタイトルを計算する。
        int score_title = ScoreTitle::get_score_title(fu, han);

        // 点数を計算する。
        auto score = calc_score(true, score_title, han, fu);

        scores.push_back(score.front());
    }

    return scores;
}

} // namespace mahjong
