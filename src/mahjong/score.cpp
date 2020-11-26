#include "score.hpp"

#include "bitutils.hpp"
#include "handSeparator.hpp"
#include "syanten.hpp"

namespace mahjong {

/**
 * @brief 点数計算機を作成する。
 */
ScoreCalculator::ScoreCalculator()
    : rules_(RuleFlag::AkaDora | RuleFlag::OpenTanyao)
    , bakaze_(Tile::Ton)
    , zikaze_(Tile::Ton)
    , n_tumibo_(0)
    , n_kyotakubo_(0)
{
    HandSeparator::initialize();
}

/**
 * @brief 点数を計算する。
 * 
 * @param[in] hand 手牌
 * @param[in] win_tile 和了牌
 * @param[in] flag フラグ
 * @return Result 結果
 */
Result ScoreCalculator::calc(const Hand &hand, int win_tile, int flag)
{
    if (auto [ok, err_msg] = check_arguments(hand, win_tile, flag); !ok)
        return {hand, win_tile, flag, err_msg}; // 異常終了

    if (flag & HandFlag::NagasiMangan)
        return aggregate(hand, win_tile, flag, Yaku::NagasiMangan);

    // 向聴数を計算する。
    auto [syanten_type, syanten] = SyantenCalculator::calc(hand);
    if (syanten != -1)
        return {hand, win_tile, flag, "和了形ではありません。"};

    // 副露牌を手牌に統合する。
    Hand norm_hand = merge_hand(hand);
    // 和了牌が赤牌の場合、赤なしの牌に変換する。
    int norm_win_tile = aka2normal(win_tile);

    YakuList yaku_list = Yaku::Null;

    // 役満をチェックする。
    yaku_list |= check_yakuman(norm_hand, norm_win_tile, flag, syanten_type);
    if (yaku_list)
        return aggregate(hand, win_tile, flag, yaku_list);

    // 面子構成に関係ない役を調べる。
    yaku_list |= check_not_pattern_yaku(norm_hand, norm_win_tile, flag, syanten_type);

    // 面子構成に関係ある役を調べる。
    auto [pattern_yaku_list, fu, blocks, wait_type] =
        check_pattern_yaku(hand, norm_win_tile, flag, syanten_type);
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
void ScoreCalculator::set_bakaze(int tile)
{
    bakaze_ = tile;
}

/**
 * @brief 場風牌を取得する。
 * 
 * @return int 牌
 */
int ScoreCalculator::bakaze() const
{
    return bakaze_;
}

/**
 * @brief 自風牌を設定する
 * 
 * @param[in] tile 牌
 */
void ScoreCalculator::set_zikaze(int tile)
{
    zikaze_ = tile;
}

/**
 * @brief 自風牌を取得する。
 * 
 * @return int 牌
 */
int ScoreCalculator::zikaze() const
{
    return zikaze_;
}

/**
 * @brief 積み棒の数を設定する。
 * 
 * @param[in] n 積み棒の数
 */
void ScoreCalculator::set_num_tumibo(int n)
{
    n_tumibo_ = n;
}

/**
 * @brief 積み棒の数を取得する。
 * 
 * @return int 積み棒の数
 */
int ScoreCalculator::num_tumibo() const
{
    return n_tumibo_;
}

/**
 * @brief 供託棒の数を設定する。
 * 
 * @param[in] n 供託棒の数
 */
void ScoreCalculator::set_num_kyotakubo(int n)
{
    n_kyotakubo_ = n;
}

/**
 * @brief 供託棒の数を取得する。
 * 
 * @return int 供託棒の数
 */
int ScoreCalculator::num_kyotakubo() const
{
    return n_kyotakubo_;
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
                                  YakuList yaku_list)
{
    int score_title;
    std::vector<std::tuple<YakuList, int>> yaku_han_list;
    std::vector<int> score;

    if (yaku_list & Yaku::NagasiMangan) {
        // 流し満貫
        yaku_han_list.emplace_back(Yaku::NagasiMangan, 0);
        score_title = ScoreTitle::Mangan;

        // 流し満貫は自摸扱い
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
Result ScoreCalculator::aggregate(const Hand &hand, int win_tile, int flag,
                                  YakuList yaku_list, int fu,
                                  const std::vector<Block> &blocks, int wait_type)
{
    // 飜を計算する。
    int han = 0;
    std::vector<std::tuple<YakuList, int>> yaku_han_list;
    for (auto &yaku : Yaku::NormalYaku) {
        if (yaku_list & yaku) {
            int yaku_han =
                hand.is_menzen() ? Yaku::Info[yaku].han[0] : Yaku::Info[yaku].han[1];
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
        yaku_han_list.emplace_back(Yaku::UraDora, n_dora);
        han += n_uradora;
    }

    if (rules_ & RuleFlag::AkaDora) {
        int n_akadora = count_akadora(hand);
        if (n_akadora) {
            yaku_han_list.emplace_back(Yaku::AkaDora, n_akadora);
            han += n_akadora;
        }
    }

    // 点数のタイトルを計算する。
    int score_title = ScoreTitle::get_score_title(fu, han);

    // 点数を計算する。
    auto score = calc_score(flag & HandFlag::Tumo, score_title, han, fu);

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
    if (!hand.contains(win_tile)) {
        std::string err_msg = fmt::format("和了牌 {} が手牌 {} に含まれていません。",
                                          Tile::Name.at(win_tile), hand.to_string());
        return {false, err_msg};
    }

    // 同時に指定できないフラグをチェックする。
    if (!Bit::check_exclusive(flag & (HandFlag::Reach | HandFlag::DoubleReach))) {
        std::string err_msg = fmt::format("{}、{}はいずれか1つのみ指定できます。",
                                          Yaku::Info[Yaku::Reach].name,
                                          Yaku::Info[Yaku::DoubleReach].name);
        return {false, err_msg};
    }

    if (!Bit::check_exclusive(flag & (HandFlag::Tyankan | HandFlag::Rinsyankaiho |
                                      HandFlag::Haiteitumo | HandFlag::Hoteiron))) {
        std::string err_msg = fmt::format(
            "{}、{}、{}、{}はいずれか1つのみ指定できます。",
            Yaku::Info[Yaku::Tyankan].name, Yaku::Info[Yaku::Rinsyankaiho].name,
            Yaku::Info[Yaku::Haiteitumo].name, Yaku::Info[Yaku::Hoteiron].name);
        return {false, err_msg};
    }

    if (!Bit::check_exclusive(flag &
                              (HandFlag::Tenho | HandFlag::Tiho | HandFlag::Renho))) {
        std::string err_msg = fmt::format(
            "{}、{}、{}はいずれか1つのみ指定できます。", Yaku::Info[Yaku::Tenho].name,
            Yaku::Info[Yaku::Tiho].name, Yaku::Info[Yaku::Renho].name);
        return {false, err_msg};
    }

    // 条件が必要なフラグをチェックする。
    if ((flag & (HandFlag::Reach | HandFlag::DoubleReach)) && !hand.is_menzen()) {
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
 * @brief 役満を判定する。
 * 
 * @param[in] hand 手牌 (正規化)
 * @param[in] win_tile 和了牌 (正規化)
 * @param[in] flag フラグ
 * @param[in] syanten_type 和了形の種類
 * @return YakuList 成立した役一覧
 */
YakuList ScoreCalculator::check_yakuman(const Hand &hand, int win_tile, int flag,
                                        int syanten_type) const
{
    YakuList yaku_list = Yaku::Null;

    if (flag & HandFlag::Tenho)
        yaku_list |= Yaku::Tenho; // 天和
    else if (flag & HandFlag::Tiho)
        yaku_list |= Yaku::Tiho; // 地和
    else if (flag & HandFlag::Renho)
        yaku_list |= Yaku::Renho; // 人和

    if (syanten_type == SyantenType::Normal) { // 通常手
        if (check_ryuiso(hand))
            yaku_list |= Yaku::Ryuiso; // 緑一色

        if (check_daisangen(hand))
            yaku_list |= Yaku::Daisangen; // 大三元

        if (check_daisusi(hand))
            yaku_list |= Yaku::Daisusi; // 大四喜
        else if (check_syosusi(hand))
            yaku_list |= Yaku::Syosusi; // 小四喜

        if (check_tuiso(hand))
            yaku_list |= Yaku::Tuiso; // 字一色

        if (check_tyurenpoto9(hand, win_tile))
            yaku_list |= Yaku::Tyurenpoto9; // 純正九連宝灯
        else if (check_tyurenpoto(hand, win_tile))
            yaku_list |= Yaku::Tyurenpoto; // 九連宝灯

        if (check_suanko_tanki(hand, win_tile))
            yaku_list |= Yaku::SuankoTanki; // 四暗刻単騎
        else if (check_suanko(hand, flag))
            yaku_list |= Yaku::Suanko; // 四暗刻

        if (check_tinroto(hand))
            yaku_list |= Yaku::Tinroto; // 清老頭

        if (check_sukantu(hand))
            yaku_list |= Yaku::Sukantu; // 四槓子
    }
    else if (syanten_type == SyantenType::Tiitoi) { // 七対子手
        if (check_tuiso(hand))
            yaku_list |= Yaku::Tuiso; // 字一色
    }
    else { // 国士無双手
        if (check_kokusi13(hand, win_tile))
            yaku_list |= Yaku::Kokusimuso13; // 国士無双13面待ち
        else
            yaku_list |= Yaku::Kokusimuso; // 国士無双
    }

    return yaku_list;
}

/**
 * @brief 面子構成に関係ない役を判定する。
 * 
 * @param[in] hand 手牌
 * @param[in] win_tile 和了牌
 * @param[in] flag フラグ
 * @param[in] syanten_type 和了形の種類
 * @return YakuList 成立した役一覧
 */
YakuList ScoreCalculator::check_not_pattern_yaku(const Hand &hand, int win_tile,
                                                 int flag, int syanten_type) const
{
    YakuList yaku_list = Yaku::Null;

    if (flag & HandFlag::DoubleReach)
        yaku_list |= Yaku::DoubleReach; // ダブル立直
    else if (flag & HandFlag::Reach)
        yaku_list |= Yaku::Reach; // 立直

    if (flag & HandFlag::Ippatu)
        yaku_list |= Yaku::Ippatu; // 一発

    if (flag & HandFlag::Tyankan)
        yaku_list |= Yaku::Tyankan; // 搶槓
    else if (flag & HandFlag::Rinsyankaiho)
        yaku_list |= Yaku::Rinsyankaiho; // 嶺上開花
    else if (flag & HandFlag::Haiteitumo)
        yaku_list |= Yaku::Haiteitumo; // 海底摸月
    else if (flag & HandFlag::Hoteiron)
        yaku_list |= Yaku::Hoteiron; // 河底撈魚

    if ((flag & HandFlag::Tumo) && hand.is_menzen())
        yaku_list |= Yaku::Tumo; // 門前清自摸和

    if (check_tanyao(hand))
        yaku_list |= Yaku::Tanyao; // 断幺九

    if (check_tiniso(hand))
        yaku_list |= Yaku::Tiniso; // 清一色
    else if (check_honiso(hand))
        yaku_list |= Yaku::Honiso; // 混一色

    if (check_honroto(hand))
        yaku_list |= Yaku::Honroto; // 清老頭

    if (syanten_type == SyantenType::Normal) {
        if (check_syosangen(hand))
            yaku_list |= Yaku::Syosangen; // 小三元

        if (check_sankantu(hand))
            yaku_list |= Yaku::Sankantu; // 三槓子

        if ((hand.zihai & Bit::mask[Tile::Haku]) == Bit::tile3[Tile::Haku])
            yaku_list |= Yaku::SangenhaiHaku; // 三元牌 (白)

        if ((hand.zihai & Bit::mask[Tile::Hatu]) == Bit::tile3[Tile::Hatu])
            yaku_list |= Yaku::SangenhaiHatu; // 三元牌 (發)

        if ((hand.zihai & Bit::mask[Tile::Tyun]) == Bit::tile3[Tile::Tyun])
            yaku_list |= Yaku::SangenhaiTyun; // 三元牌 (中)

        if ((hand.zihai & Bit::mask[zikaze_]) == Bit::tile3[zikaze_]) {
            if (zikaze_ == Tile::Ton)
                yaku_list |= Yaku::ZikazeTon; // 自風牌 (東)
            else if (zikaze_ == Tile::Nan)
                yaku_list |= Yaku::ZikazeNan; // 自風牌 (南)
            else if (zikaze_ == Tile::Sya)
                yaku_list |= Yaku::ZikazeSya; // 自風牌 (西)
            else if (zikaze_ == Tile::Pe)
                yaku_list |= Yaku::ZikazePe; // 自風牌 (北)
        }

        if ((hand.zihai & Bit::mask[bakaze_]) == Bit::tile3[bakaze_]) {
            if (bakaze_ == Tile::Ton)
                yaku_list |= Yaku::BakazeTon; // 場風牌 (東)
            else if (bakaze_ == Tile::Nan)
                yaku_list |= Yaku::BakazeNan; // 場風牌 (南)
            else if (bakaze_ == Tile::Sya)
                yaku_list |= Yaku::BakazeSya; // 場風牌 (西)
            else if (bakaze_ == Tile::Pe)
                yaku_list |= Yaku::BakazePe; // 場風牌 (北)
        }
    }
    else if (syanten_type == SyantenType::Tiitoi) {
        yaku_list |= Yaku::Tiitoitu; // 七対子
    }

    return yaku_list;
}

/**
 * @brief 面子構成が関係ある役を判定する。
 * 
 * @param[in] hand 手牌
 * @param[in] win_tile 和了牌
 * @param[in] flag フラグ
 * @return std::tuple<YakuList, int, std::vector<Block>> (成立した役一覧, 符, 面子構成)
 */
std::tuple<YakuList, int, std::vector<Block>, int>
ScoreCalculator::check_pattern_yaku(const Hand &hand, int win_tile, int flag,
                                    int syanten_type)
{
    if (syanten_type == SyantenType::Tiitoi)
        return {Yaku::Null, Hu::Hu25, {}, WaitType::Tanki};

    static const std::vector<YakuList> pattern_yaku = {
        Yaku::Pinhu,        Yaku::Ipeko,          Yaku::Toitoiho,  Yaku::Sananko,
        Yaku::SansyokuDoko, Yaku::SansyokuDozyun, Yaku::IkkiTukan, Yaku::Tyanta,
        Yaku::Sankantu,     Yaku::Zyuntyanta,     Yaku::Ryanpeko,
    };

    // 面子構成一覧を取得する。
    auto pattern = HandSeparator::separate(hand, win_tile, flag & HandFlag::Tumo);

    // 点数が最大となる面子構成を探す。
    int max_han = 0;
    int max_fu  = Hu::Null;
    size_t max_idx;
    YakuList max_yaku_list;
    for (size_t i = 0; i < pattern.size(); ++i) {
        int han, fu;
        const std::vector<Block> &blocks = std::get<0>(pattern[i]);
        int wait_type                    = std::get<1>(pattern[i]);
        YakuList yaku_list               = Yaku::Null;

        // 平和形かどうかを判定する。
        bool is_pinhu = check_pinhu(blocks, wait_type);

        // 役を判定する。
        if (hand.is_menzen()) {
            if (is_pinhu)
                yaku_list |= Yaku::Pinhu; // 平和

            int ipeko_type = check_ipeko(blocks);
            if (ipeko_type == 1)
                yaku_list |= Yaku::Ipeko; // 一盃口
            else if (ipeko_type == 2)
                yaku_list |= Yaku::Ryanpeko; // 二盃口
        }

        if (check_ikkitukan(blocks))
            yaku_list |= Yaku::IkkiTukan; // 一気通貫
        else if (check_sansyokudoko(blocks))
            yaku_list |= Yaku::SansyokuDoko; // 三色同刻
        else if (check_sansyokudozyun(blocks))
            yaku_list |= Yaku::SansyokuDozyun; // 三色同順

        int tyanta_type = check_tyanta(blocks);
        if (tyanta_type == 1)
            yaku_list |= Yaku::Tyanta; // 混全帯幺九
        else if (tyanta_type == 2)
            yaku_list |= Yaku::Zyuntyanta; // 純全帯幺九

        if (check_toitoiho(blocks))
            yaku_list |= Yaku::Toitoiho; // 対々和

        if (check_sananko(blocks))
            yaku_list |= Yaku::Sananko; // 三暗刻

        // 飜を計算する。
        han = 0;
        for (const auto &yaku : pattern_yaku) {
            if (yaku_list & yaku)
                han += hand.is_menzen() ? Yaku::Info[yaku].han[0]
                                        : Yaku::Info[yaku].han[1];
        }

        // 符を計算する。
        fu = calc_fu(blocks, wait_type, hand.is_menzen(), flag & HandFlag::Tumo,
                     is_pinhu);

        if (max_han < han || (max_han == han && max_fu < fu)) {
            max_han       = han;
            max_fu        = fu;
            max_idx       = i;
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
                             bool is_menzen, bool is_tumo, bool is_pinhu) const
{
    // 符計算の例外
    //////////////////////////
    if (is_pinhu && is_tumo && is_menzen)
        return Hu::Hu20; // 平和形 + 自摸
    else if (is_pinhu && !is_tumo && !is_menzen)
        return Hu::Hu30; // 平和形 + ロン

    // 通常の符計算
    //////////////////////////

    // 副底
    int fu = 20;

    // 門前加符
    if (is_menzen && !is_tumo)
        fu += 10; // 門前ロン
    // 自摸符
    else if (is_tumo)
        fu += 2; // 門前自摸、非門前自摸

    // 待ちによる符
    if (wait_type == WaitType::Kantyan || wait_type == WaitType::Pentyan ||
        wait_type == WaitType::Tanki)
        fu += 2; // 嵌張待ち、辺張待ち、単騎待ち

    // 面子構成による符
    for (const auto &block : blocks) {
        if (block.type & (BlockType::Kotu | BlockType::Kantu)) { // 刻子、槓子の場合
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
ScoreCalculator::calc_fu_detail(const std::vector<Block> &blocks, int wait_type,
                                bool is_menzen, bool is_tumo) const
{
    bool is_pinhu = check_pinhu(blocks, wait_type);

    // 符計算の例外
    //////////////////////////
    if (is_pinhu && is_tumo && is_menzen) { // 平和、自摸、門前
        return {{"平和・自摸", 20}};
    }
    else if (is_pinhu && !is_tumo && !is_menzen) { // 平和、ロン、非門前
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
/// 役を判定する関数
////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief 緑一色かどうかを判定する。
 */
bool ScoreCalculator::check_ryuiso(const Hand &hand) const
{
    // 「2, 3, 4, 6, 8, 發以外の牌がない」かどうかを調べる
    //               | 9 | 8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 |
    int sozu_mask = 0b111'000'111'000'111'000'000'000'111;
    //                |中 |發 |白 |北 |西 |南 |東 |
    int zihai_mask = 0b111'000'111'111'111'111'111;

    return !(hand.manzu || hand.pinzu || (hand.sozu & sozu_mask) ||
             (hand.zihai & zihai_mask));
}

/**
 * @brief 大三元かどうかを判定する。
 */
bool ScoreCalculator::check_daisangen(const Hand &hand) const
{
    // 「三元牌が各3枚以上」かどうかを調べる。
    //          |中 |發 |白 |北 |西 |南 |東 |
    int mask = 0b011'011'011'000'000'000'000;

    return (hand.zihai & Bit::SangenhaiMask) == mask;
}

/**
 * @brief 小四喜かどうかを判定する。
 */
bool ScoreCalculator::check_syosusi(const Hand &hand) const
{
    // 「風牌の合計が11枚」かどうかを調べる。
    int kazehai = hand.zihai & Bit::KazehaiMask;
    return Bit::sum(kazehai) == 11;
}

/**
 * @brief 字一色かどうかを判定する。
 */
bool ScoreCalculator::check_tuiso(const Hand &hand) const
{
    // 「字牌以外の牌がない」かどうかを調べる。
    return !(hand.manzu || hand.pinzu || hand.sozu);
}

/**
 * @brief 九連宝灯かどうかを判定する。
 */
bool ScoreCalculator::check_tyurenpoto(const Hand &hand, int win_tile) const
{
    const auto &s_tbl = SyantenCalculator::s_tbl_;

    if (hand.is_melded())
        return false; // 副露している場合

    int key;
    if (win_tile <= Tile::Manzu9)
        key = hand.manzu;
    else if (win_tile <= Tile::Pinzu9)
        key = hand.pinzu;
    else if (win_tile <= Tile::Sozu9)
        key = hand.sozu;
    else
        return false; // 字牌

    // 「老頭牌が各3枚以上」かつ「中張牌が各1枚以上」かどうかを調べる。
    int rotohai     = key & Bit::RotohaiMask; // 老頭牌
    int tyuntyanhai = key & Bit::TanyaoMask;  // 中張牌

    return s_tbl[rotohai].n_ge3 == 2 && s_tbl[tyuntyanhai].n_ge1 == 7;
}

/**
 * @brief 四暗刻かどうかを判定する。
 */
bool ScoreCalculator::check_suanko(const Hand &hand, int flag) const
{
    const auto &s_tbl = SyantenCalculator::s_tbl_;
    const auto &z_tbl = SyantenCalculator::z_tbl_;

    if (!(flag & HandFlag::Tumo))
        return false; // ロンした場合、暗刻4つにならないので自摸和了限定

    if (!hand.is_menzen())
        return false; // 門前でない場合

    // 「刻子が4つある」かどうかを調べる。
    int n_ge4 = s_tbl[hand.manzu].n_ge4 + s_tbl[hand.pinzu].n_ge4 +
                s_tbl[hand.sozu].n_ge4 + z_tbl[hand.zihai].n_ge4;
    int n_ge3 = s_tbl[hand.manzu].n_ge3 + s_tbl[hand.pinzu].n_ge3 +
                s_tbl[hand.sozu].n_ge3 + z_tbl[hand.zihai].n_ge3;

    // 牌: 888p 3444r5999s [7777m, 暗槓] のような3枚以上が4つあっても6種類牌があるとだめ
    int n_types = s_tbl[hand.manzu].n_ge1 + s_tbl[hand.pinzu].n_ge1 +
                  s_tbl[hand.sozu].n_ge1 + z_tbl[hand.zihai].n_ge1;

    return n_ge3 - n_ge4 == 4 && n_types == 5;
}

/**
 * @brief 清老頭かどうかをチェックする。
 */
bool ScoreCalculator::check_tinroto(const Hand &hand) const
{
    // 「老頭牌以外の牌がない」かどうかを調べる
    return !((hand.manzu & Bit::TanyaoMask) || (hand.pinzu & Bit::TanyaoMask) ||
             (hand.sozu & Bit::TanyaoMask) || hand.zihai);
}

/**
 * @brief 四槓子かどうかを判定する。
 */
bool ScoreCalculator::check_sukantu(const Hand &hand) const
{
    // 「副露ブロックに4つの槓子がある」かどうかを調べる。
    int cnt = 0;
    for (const auto &block : hand.melded_blocks)
        cnt += MeldType::Ankan <= block.type; // enum の値で2以上が槓子であるため

    return cnt == 4;
}

/**
 * @brief 四暗刻単騎かどうかを判定する。
 */
bool ScoreCalculator::check_suanko_tanki(const Hand &hand, int win_tile) const
{
    const auto &s_tbl = SyantenCalculator::s_tbl_;
    const auto &z_tbl = SyantenCalculator::z_tbl_;

    if (!hand.is_menzen())
        return false; // 門前でない場合

    // 和了牌の種類で雀頭が構成されているかどうかを調べる。
    int key;
    if (win_tile <= Tile::Manzu9)
        key = hand.manzu;
    else if (win_tile <= Tile::Pinzu9)
        key = hand.pinzu;
    else if (win_tile <= Tile::Sozu9)
        key = hand.sozu;
    else
        key = hand.zihai;

    if ((key & Bit::mask[win_tile]) != Bit::tile2[win_tile])
        return false;

    // 刻子が4つかどうかを調べる。
    int n_ge4 = s_tbl[hand.manzu].n_ge4 + s_tbl[hand.pinzu].n_ge4 +
                s_tbl[hand.sozu].n_ge4 + z_tbl[hand.zihai].n_ge4;
    int n_ge3 = s_tbl[hand.manzu].n_ge3 + s_tbl[hand.pinzu].n_ge3 +
                s_tbl[hand.sozu].n_ge3 + z_tbl[hand.zihai].n_ge3;

    return n_ge3 - n_ge4 == 4;
}

/**
 * @brief 大四喜かどうかを判定する。
 */
bool ScoreCalculator::check_daisusi(const Hand &hand) const
{
    // 「風牌の合計が12枚」かどうかを調べる。
    int kazehai = hand.zihai & Bit::KazehaiMask;
    return Bit::sum(kazehai) == 12;
}

/**
 * @brief 純正九連宝灯かどうかを判定する。
 */
bool ScoreCalculator::check_tyurenpoto9(const Hand &hand, int win_tile) const
{
    if (hand.is_melded())
        return false; // 副露している場合

    // 「和了牌を除いた場合に 1112345678999 となっている」かどうかを調べる。
    //          | 9 | 8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 |
    int mask = 0b011'001'001'001'001'001'001'001'011;

    if (win_tile <= Tile::Manzu9)
        return hand.manzu - Bit::tile1[win_tile] == mask;
    else if (win_tile <= Tile::Pinzu9)
        return hand.pinzu - Bit::tile1[win_tile] == mask;
    else if (win_tile <= Tile::Sozu9)
        return hand.sozu - Bit::tile1[win_tile] == mask;

    return false;
}

/**
 * @brief 国士無双13面待ちかどうかを判定する。
 */
bool ScoreCalculator::check_kokusi13(const Hand &hand, int win_tile) const
{
    // 「和了牌を除いた場合に幺九牌がすべて1個ずつある」かどうかを調べる。
    int manzu = hand.manzu;
    int pinzu = hand.pinzu;
    int sozu  = hand.sozu;
    int zihai = hand.zihai;

    if (win_tile <= Tile::Manzu9)
        manzu -= Bit::tile1[win_tile];
    else if (win_tile <= Tile::Pinzu9)
        pinzu -= Bit::tile1[win_tile];
    else if (win_tile <= Tile::Sozu9)
        sozu -= Bit::tile1[win_tile];
    else
        zihai -= Bit::tile1[win_tile];

    //               | 9 | 8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 |
    int roto_mask = 0b001'000'000'000'000'000'000'000'001;
    //                |中 |發 |白 |北 |西 |南 |東 |
    int zihai_mask = 0b001'001'001'001'001'001'001;

    return (manzu == roto_mask) && (pinzu == roto_mask) && (sozu == roto_mask) &&
           (zihai == zihai_mask);
}

/**
 * @brief 混全帯幺九かどうかを判定する。
 */
bool ScoreCalculator::check_tanyao(const Hand &hand) const
{
    if (!(rules_ & RuleFlag::OpenTanyao) && !hand.is_menzen())
        return false; // 喰い断なしで副露している場合

    // 「幺九牌がない」かどうかを調べる。
    return !((hand.manzu & Bit::RotohaiMask) || (hand.pinzu & Bit::RotohaiMask) ||
             (hand.sozu & Bit::RotohaiMask) || hand.zihai);
}

/**
 * @brief 混老頭かどうかを判定する。
 */
bool ScoreCalculator::check_honroto(const Hand &hand) const
{
    // 「幺九牌以外の牌がない」かつ「字牌がある」かどうかを調べる。
    return !((hand.manzu & Bit::TanyaoMask) || (hand.pinzu & Bit::TanyaoMask) ||
             (hand.sozu & Bit::TanyaoMask)) &&
           hand.zihai;
}

/**
 * @brief 混一色かどうかを判定する。
 */
bool ScoreCalculator::check_honiso(const Hand &hand) const
{
    // 「1種類の数牌がある」かつ「字牌がある」かどうかを調べる
    if (hand.manzu && !hand.pinzu && !hand.sozu && hand.zihai)
        return true;
    else if (!hand.manzu && hand.pinzu && !hand.sozu && hand.zihai)
        return true;
    else if (!hand.manzu && !hand.pinzu && hand.sozu && hand.zihai)
        return true;

    return false;
}

/**
 * @brief 清一色かどうかを判定する。
 */
bool ScoreCalculator::check_tiniso(const Hand &hand) const
{
    // 「1種類の数牌がある」かつ「字牌がない」かどうかを調べる
    if (hand.manzu && !hand.pinzu && !hand.sozu && !hand.zihai)
        return true;
    else if (!hand.manzu && hand.pinzu && !hand.sozu && !hand.zihai)
        return true;
    else if (!hand.manzu && !hand.pinzu && hand.sozu && !hand.zihai)
        return true;

    return false;
}

/**
 * @brief 小三元かどうかを判定する。
 */
bool ScoreCalculator::check_syosangen(const Hand &hand) const
{
    // 「役牌の合計が8枚」かどうかを調べる。
    int yakuhai = hand.zihai & Bit::SangenhaiMask;
    return Bit::sum(yakuhai) == 8;
}

/**
 * @brief 三槓子かどうかを判定する。
 */
bool ScoreCalculator::check_sankantu(const Hand &hand) const
{
    // 「副露ブロックに3つの槓子がある」かどうかを調べる。
    int cnt = 0;
    for (const auto &block : hand.melded_blocks)
        cnt += MeldType::Ankan <= block.type; // enum の値で2以上が槓子であるため

    return cnt == 3;
}

/**
 * @brief 平和形かどうかを判定する。(門前かどうかは呼び出し側でチェックすること)
 */
bool ScoreCalculator::check_pinhu(const std::vector<Block> blocks, int wait_type) const
{
    if (wait_type != WaitType::Ryanmen)
        return false; // 両面待ちでない場合

    // すべてのブロックが順子または役牌でない対子かどうか
    for (const auto &block : blocks) {
        if (block.type & (BlockType::Kotu | BlockType::Kantu))
            return false; // 刻子、槓子の場合

        if ((block.type & BlockType::Toitu) &&
            (block.min_tile == zikaze_ || block.min_tile == bakaze_ ||
             block.min_tile >= Tile::Haku))
            return false; // 対子の役牌の場合
    }

    return true;
}

/**
 * @brief 一盃口の数を数える。(門前かどうかは呼び出し側でチェックすること)
 */
int ScoreCalculator::check_ipeko(const std::vector<Block> blocks) const
{
    std::vector<int> count(34, 0);
    for (const auto &block : blocks) {
        if (block.type & BlockType::Syuntu)
            count[block.min_tile]++; // 順子の場合
    }

    // 一盃口の数を数える。
    int n_ipeko = 0;
    for (const auto &x : count) {
        if (x == 4)
            n_ipeko += 2;
        else if (x >= 2)
            n_ipeko++;
    }

    return n_ipeko;
}

/**
 * @brief 一気通貫かどうかを判定する。
 */
bool ScoreCalculator::check_ikkitukan(const std::vector<Block> blocks) const
{
    std::vector<int> count(34, 0);
    for (const auto &block : blocks) {
        if (block.type & BlockType::Syuntu)
            count[block.min_tile]++; // 順子の場合
    }

    // 123, 456, 789 があるかどうかを調べる。
    return (count[Tile::Manzu1] && count[Tile::Manzu4] && count[Tile::Manzu7]) ||
           (count[Tile::Pinzu1] && count[Tile::Pinzu4] && count[Tile::Pinzu7]) ||
           (count[Tile::Sozu1] && count[Tile::Sozu4] && count[Tile::Sozu7]);
}

/**
 * @brief 三色同刻かどうかを判定する。
 */
bool ScoreCalculator::check_sansyokudoko(const std::vector<Block> blocks) const
{
    std::vector<int> count(34, 0);
    for (const auto &block : blocks) {
        if (block.type & (BlockType::Kotu | BlockType::Kantu))
            count[block.min_tile]++; // 刻子または槓子の場合
    }

    for (size_t i = 0; i < 9; ++i) {
        if (count[i] && count[i + 9] && count[i + 18])
            return true;
    }

    return false;
}

/**
 * @brief 三色同順かどうかを判定する。
 */
bool ScoreCalculator::check_sansyokudozyun(const std::vector<Block> blocks) const
{
    std::vector<int> count(34, 0);
    for (const auto &block : blocks) {
        if (block.type & BlockType::Syuntu)
            count[block.min_tile]++; // 順子の場合
    }

    for (size_t i = 0; i < 9; ++i) {
        if (count[i] && count[i + 9] && count[i + 18])
            return true;
    }

    return false;
}

/**
 * @brief 対々和かどうかを判定する。
 */
bool ScoreCalculator::check_toitoiho(const std::vector<Block> blocks) const
{
    for (const auto &block : blocks) {
        if (block.type & BlockType::Syuntu)
            return false; // 順子の場合
    }

    return true;
}

/**
 * @brief 混全帯幺九、純全帯幺九かどうかを判定する。
 */
int ScoreCalculator::check_tyanta(const std::vector<Block> blocks) const
{
    // 幺九牌 字牌 順子  役
    // ○      ○    ○    混全帯幺九
    // ○      ○    x    混老頭
    // ○      x    ○    純全帯幺九
    // ○      x    x    清老頭
    bool zihai = false; // いずれかのブロックに字牌が含まれるかどうか
    bool syuntu = false; // いずれかのブロックに順子が含まれるかどうか

    for (const auto &block : blocks) {
        if (block.type & BlockType::Syuntu) { // 順子の場合
            if (!(block.min_tile == Tile::Manzu1 || block.min_tile == Tile::Manzu7 ||
                  block.min_tile == Tile::Pinzu1 || block.min_tile == Tile::Pinzu7 ||
                  block.min_tile == Tile::Sozu1 || block.min_tile == Tile::Sozu7))
                return 0; // 幺九牌を含まない場合

            syuntu = true;
        }
        else { // 刻子、槓子、対子の場合
            if (!(block.min_tile == Tile::Manzu1 || block.min_tile == Tile::Manzu9 ||
                  block.min_tile == Tile::Pinzu1 || block.min_tile == Tile::Pinzu9 ||
                  block.min_tile == Tile::Sozu1 || block.min_tile == Tile::Sozu9 ||
                  block.min_tile >= Tile::Ton))
                return 0; // 幺九牌を含まない場合

            zihai |= block.min_tile >= Tile::Ton;
        }
    }

    if (!zihai && syuntu)
        return 2; // 純全帯幺九
    else if (zihai && syuntu)
        return 1; // 混全帯幺九

    return 0;
}

/**
 * @brief 三暗刻かどうかを判定する。
 */
bool ScoreCalculator::check_sananko(const std::vector<Block> blocks) const
{
    int cnt = 0;
    for (const auto &block : blocks) {
        if (block.type == BlockType::Kotu || block.type == BlockType::Kantu)
            cnt++; // 暗刻子、暗槓子の場合
    }

    return cnt == 3;
}

/**
 * @brief ドラの数を数える。
 * 
 * @param[in] hand 手牌
 * @param[in] dora_tiles ドラ牌の一覧
 * @return int ドラの数
 */
int ScoreCalculator::count_dora(const Hand &hand, std::vector<int> dora_tiles) const
{
    int n_dora = 0;
    for (const auto &dora : dora_tiles) {
        n_dora += hand.num_tiles(dora);

        for (const auto &block : hand.melded_blocks) {
            for (auto tile : block.tiles) {
                if (aka2normal(tile) == dora)
                    n_dora++;
            }
        }
    }

    return n_dora;
}

/**
 * @brief 赤ドラの数を数える。
 * 
 * @param[in] hand 手牌
 * @param[in] dora_tiles ドラ牌の一覧
 * @return int ドラの数
 */
int ScoreCalculator::count_akadora(const Hand &hand) const
{
    int n_akadora = hand.aka_manzu5 + hand.aka_pinzu5 + hand.aka_sozu5;

    for (const auto &block : hand.melded_blocks) {
        for (auto tile : block.tiles) {
            if (tile == Tile::AkaManzu5 || tile == Tile::AkaPinzu5 ||
                tile == Tile::AkaSozu5) {
                n_akadora++;
                break;
            }
        }
    }

    return n_akadora;
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

    for (const auto &block : norm_hand.melded_blocks) {
        int min_tile = aka2normal(block.tiles.front()); // 赤ドラは通常の牌として扱う

        int *key;
        if (min_tile <= Tile::Manzu9)
            key = &norm_hand.manzu;
        else if (min_tile <= Tile::Pinzu9)
            key = &norm_hand.pinzu;
        else if (min_tile <= Tile::Sozu9)
            key = &norm_hand.sozu;
        else
            key = &norm_hand.zihai;

        if (block.type == MeldType::Ti) // チー
            *key += Bit::tile1[min_tile] | Bit::tile1[min_tile + 1] |
                    Bit::tile1[min_tile + 2];
        else // ポン、暗槓、明槓、加槓
            *key += Bit::tile3[min_tile];
    }

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

} // namespace mahjong
