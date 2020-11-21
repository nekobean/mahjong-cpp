#include "score.hpp"

#include <cstdio>
#include <fstream>

#include <boost/dll.hpp>
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/writer.h>
#include <spdlog/spdlog.h>

#include "bitutils.hpp"
#include "syanten.hpp"

namespace mahjong {

/**
 * @brief 点数計算機を作成する。
 */
ScoreCalculator::ScoreCalculator()
    : akadora_enabled_(true)
    , open_tanyao_enabled_(true)
    , bakaze_(Tile::Ton)
    , zikaze_(Tile::Ton)
    , n_tumibo_(0)
    , n_kyotakubo_(0)
{
}

/**
 * @brief 初期化する。
 * 
 * @return 初期化に成功した場合は true、そうでない場合は false を返す。
 */
bool ScoreCalculator::initialize()
{
    boost::filesystem::path s_tbl_path =
        boost::dll::program_location().parent_path() / "syupai_pattern.json";
    boost::filesystem::path z_tbl_path =
        boost::dll::program_location().parent_path() / "zihai_pattern.json";

    return make_table(s_tbl_path.string(), s_tbl_) &&
           make_table(z_tbl_path.string(), z_tbl_);
}

/**
 * @brief 点数を計算する。
 * 
 * @param[in] tehai 手牌
 * @param[in] winning_tile 和了牌
 * @param[in] flag フラグ
 * @return Result 結果
 * 
 * flag には手牌に関係ない成立役及び自摸和了りかどうかのフラグを指定します。
 *   Yaku::Tumo: 自摸和了りかどうか
 *   Yaku::Tenho: 天和成立
 *   Yaku::Tiho: 地和成立
 *   Yaku::Renho: 人和成立
 *   Yaku::Reach: 立直成立
 *   Yaku::DoubleReach: ダブル立直成立
 *   Yaku::Ippatu: 一発成立
 *   Yaku::Tyankan: 搶槓成立
 *   Yaku::Rinsyankaiho: 嶺上開花成立
 *   Yaku::Haiteitumo: 海底撈月成立
 *   Yaku::Hoteiron: 河底撈魚成立
 *   Yaku::NagasiMangan: 流し満貫成立
 *
 *   1. Yaku::Tumo は自摸和了りの場合は門前かどうかに関わらず指定します。
 *   2. 天和、地和、人和はどれか1つのみ指定できます。
 *   3. 立直またはダブル立直は門前の場合のみ指定できます。
 *   4. 一発は立直またはダブル立直している場合のみ指定できます。
 *   5. 搶槓、嶺上開花、河底撈魚はどれか1つのみ指定できます。
 */
Result ScoreCalculator::calc(const Hand &tehai, int winning_tile, YakuList flag)
{
    // 引数をチェックする。
    std::string err_msg;
    if (!check_arguments(tehai, winning_tile, flag, err_msg))
        return {tehai, winning_tile, err_msg}; // 異常終了

    if (flag & Yaku::NagasiMangan) {
        return aggregate(tehai, winning_tile, Yaku::NagasiMangan, flag & Yaku::Tumo);
    }

    // 向聴数を計算する。
    auto [syanten_type, syanten] = SyantenCalculator::calc(tehai);
    if (syanten != -1) {
        std::string err_msg =
            fmt::format("手牌 {} は和了り形ではありません。", tehai.to_string());
        return {tehai, winning_tile, err_msg};
    }

    YakuList yaku_list = Yaku::Null;
    int hu;
    std::vector<Block> blocks;

    // 副露牌を手牌に統合する。
    Hand merged_tehai = merge_hand(tehai);

    // 役満をチェックする。
    yaku_list |= check_yakuman(merged_tehai, winning_tile, flag, syanten_type);
    if (yaku_list)
        return aggregate(tehai, winning_tile, yaku_list, flag & Yaku::Tumo);

    // 面子構成に関係ない役を調べる。
    yaku_list |= check_not_pattern_yaku(merged_tehai, winning_tile, flag, syanten_type);

    // 面子構成に関係ある役を調べる。
    if (syanten_type == SyantenType::Normal) {
        // 一般手
        YakuList pattern_yaku_list;
        std::tie(pattern_yaku_list, hu, blocks) =
            check_pattern_yaku(tehai, winning_tile, flag);
        yaku_list |= pattern_yaku_list;
    }
    else {
        // 七対子手
        hu = 25;
    }

    if (!yaku_list) {
        // 役なしの場合
        std::string err_msg = fmt::format("役がありません。");
        return {tehai, winning_tile, err_msg};
    }

    return aggregate(tehai, winning_tile, yaku_list, blocks, flag & Yaku::Tumo);
}

/**
 * @brief 集計する。
 * 
 * @param[in] tehai 手牌
 * @param[in] winning_tile 和了牌
 * @param[in] yaku_list 成功した役一覧
 * @param[in] n_yakuman 何倍役満か
 * @return Result 結果
 */
Result ScoreCalculator::aggregate(const Hand &tehai, int winning_tile,
                                  YakuList yaku_list, bool tumo)
{
    // 何倍役満か数える。
    int cnt = 0;
    std::vector<std::tuple<YakuList, int>> yaku_han_list;
    for (auto &yaku : Yaku::Yakuman) {
        if (yaku_list & yaku) {
            yaku_han_list.emplace_back(yaku, Yaku::Info[yaku].han[0]);
            cnt += Yaku::Info[yaku].han[0];
        }
    }
    int score_title = ScoreTitle::get_yakuman_title(cnt);

    auto [ko2oya_ron, ko2oya_tumo, ko2ko_tumo, oya2ko_ron, oya2ko_tumo] =
        calc_score(-1, -1, score_title);

    return {tehai,      winning_tile, tumo,       yaku_han_list, score_title,
            ko2oya_ron, ko2oya_tumo,  ko2ko_tumo, oya2ko_ron,    oya2ko_tumo};
}

/**
 * @brief 集計する。
 * 
 * @param[in] tehai 手牌
 * @param[in] winning_tile 和了牌
 * @param[in] yaku_list 成功した役一覧
 * @param[in] hu 符
 * @param[in] blocks 面子構成
 * @return Result 結果
 */
Result ScoreCalculator::aggregate(const Hand &tehai, int winning_tile,
                                  YakuList yaku_list, const std::vector<Block> &blocks,
                                  bool tumo)
{
    // 何倍役満か数える。
    int han = 0;
    std::vector<std::tuple<YakuList, int>> yaku_han_list;
    for (auto &yaku : Yaku::NormalYaku) {
        if (yaku_list & yaku) {
            int yaku_han =
                tehai.is_menzen() ? Yaku::Info[yaku].han[0] : Yaku::Info[yaku].han[1];
            yaku_han_list.emplace_back(yaku, yaku_han);
            han += yaku_han;
        }
    }

    // ドラ集計
    int n_dora = count_dora(tehai, dora_tiles_);
    if (n_dora) {
        yaku_han_list.emplace_back(Yaku::Dora, n_dora);
        han += n_dora;
    }

    int n_uradora = count_dora(tehai, uradora_tiles_);
    if (n_uradora) {
        yaku_han_list.emplace_back(Yaku::UraDora, n_dora);
        han += n_uradora;
    }

    int n_akadora = count_akadora(tehai);
    if (akadora_enabled_ && n_akadora) {
        yaku_han_list.emplace_back(Yaku::AkaDora, n_akadora);
        han += n_akadora;
    }

    // 符集計
    int hu;
    std::vector<std::tuple<std::string, int>> hu_list;
    if (yaku_list & Yaku::Tiitoitu) {
        hu_list.emplace_back("七対子", 25);
        hu = 25;
    }
    else {
        std::tie(hu, hu_list) = calc_hu(blocks, winning_tile, tehai.is_menzen(), tumo);
    }

    int score_title = ScoreTitle::get_score_title(hu, han);

    auto [ko2oya_ron, ko2oya_tumo, ko2ko_tumo, oya2ko_ron, oya2ko_tumo] =
        calc_score(han, hu, score_title);

    return {tehai,       winning_tile, tumo,       yaku_han_list, hu_list,
            score_title, han,          hu,         blocks,        ko2oya_ron,
            ko2oya_tumo, ko2ko_tumo,   oya2ko_ron, oya2ko_tumo};
}

/**
 * @brief 引数をチェックする。
 * 
 * @param[in] tehai 手牌
 * @param[in] winning_tile 和了牌
 * @param[in] yaku_list フラグ
 * @param[out] err_msg エラーメッセージ
 * @return エラーがない場合は true、そうでない場合は false を返す。
 */
bool ScoreCalculator::check_arguments(const Hand &tehai, int winning_tile,
                                      YakuList yaku_list, std::string &err_msg) const
{
    // 和了牌をチェックする。
    if (!tehai.contains(winning_tile)) {
        err_msg = fmt::format("和了牌 {} が手牌 {} に含まれていません。",
                              Tile::Names.at(winning_tile), tehai.to_string());
        return false;
    }

    // フラグをチェックする。
    if (Bit::check_exclusive(yaku_list & (Yaku::Tenho | Yaku::Tiho | Yaku::Renho))) {
        err_msg = "天和、地和、人和はどれか1つのみ指定できます。";
        return false;
    }

    if ((yaku_list & (Yaku::Reach | Yaku::DoubleReach)) && !tehai.is_menzen()) {
        err_msg = "立直は門前の場合のみ指定できます。";
        return false;
    }

    if ((yaku_list & (Yaku::Ippatu)) &&
        !(yaku_list & (Yaku::Reach | Yaku::DoubleReach))) {
        err_msg = "一発は立直している場合のみ指定できます。";
        return false;
    }

    if (Bit::check_exclusive(yaku_list & (Yaku::Tyankan | Yaku::Rinsyankaiho |
                                          Yaku::Haiteitumo | Yaku::Hoteiron))) {
        err_msg = "搶槓、嶺上開花、海底撈月、河底撈魚はどれか1つのみ指定できます。";
        return false;
    }

    if ((yaku_list & (Yaku::Haiteitumo | Yaku::Rinsyankaiho)) &&
        !(yaku_list & Yaku::Tumo)) {
        err_msg =
            "ツモ和了りが指定されていないのに、嶺上開花、海底撈月が指定されています。";
        return false;
    }

    return true;
}

/**
 * @brief 役満を判定する。
 * 
 * @param[in] tehai 手牌
 * @param[in] winning_tile 和了牌
 * @param[in] flag フラグ
 * @param[in] syanten_type 和了り形の種類
 * @return YakuList 成立した役一覧
 */
YakuList ScoreCalculator::check_yakuman(const Hand &tehai, int winning_tile,
                                        YakuList flag, int syanten_type) const
{
    YakuList yaku_list = Yaku::Null;

    if (flag & Yaku::Tenho)
        yaku_list |= Yaku::Tenho; // 天和
    else if (flag & Yaku::Tiho)
        yaku_list |= Yaku::Tiho; // 地和
    else if (flag & Yaku::Renho)
        yaku_list |= Yaku::Renho; // 人和

    if (syanten_type == SyantenType::Normal) {
        // 通常手
        if (check_ryuiso(tehai))
            yaku_list |= Yaku::Ryuiso; // 緑一色

        if (check_daisangen(tehai))
            yaku_list |= Yaku::Daisangen; // 大三元

        if (check_daisusi(tehai))
            yaku_list |= Yaku::Daisusi; // 大四喜
        else if (check_syosusi(tehai))
            yaku_list |= Yaku::Syosusi; // 小四喜

        if (check_tuiso(tehai))
            yaku_list |= Yaku::Tuiso; // 字一色

        if (check_tyurenpoto9(tehai, winning_tile))
            yaku_list |= Yaku::Tyurenpoto9; // 純正九連宝灯
        else if (check_tyurenpoto(tehai, winning_tile))
            yaku_list |= Yaku::Tyurenpoto; // 九連宝灯

        if (check_suanko_tanki(tehai, winning_tile))
            yaku_list |= Yaku::SuankoTanki; // 四暗刻単騎
        else if (check_suanko(tehai, flag))
            yaku_list |= Yaku::Suanko; // 四暗刻

        if (check_tinroto(tehai))
            yaku_list |= Yaku::Tinroto; // 清老頭

        if (check_sukantu(tehai))
            yaku_list |= Yaku::Sukantu; // 四槓子
    }
    else if (syanten_type == SyantenType::Tiitoi) {
        // 七対子手
        if (check_tuiso(tehai))
            yaku_list |= Yaku::Tuiso; // 字一色
    }
    else {
        // 国士無双手
        if (check_kokusi13(tehai, winning_tile))
            yaku_list |= Yaku::Kokusimuso13; // 国士無双13面待ち
        else
            yaku_list |= Yaku::Kokusimuso;
    }

    return yaku_list;
}

/**
 * @brief 面子構成に関係ない役を判定する。
 * 
 * @param[in] tehai 手牌
 * @param[in] winning_tile 和了牌
 * @param[in] flag フラグ
 * @param[in] syanten_type 和了り形の種類
 * @return YakuList 成立した役一覧
 */
YakuList ScoreCalculator::check_not_pattern_yaku(const Hand &tehai, int winning_tile,
                                                 YakuList flag, int syanten_type) const
{
    YakuList yaku_list = Yaku::Null;

    if (flag & Yaku::DoubleReach)
        yaku_list |= Yaku::DoubleReach; // ダブル立直
    else if (flag & Yaku::Reach)
        yaku_list |= Yaku::Reach; // 立直

    if (flag & Yaku::Ippatu)
        yaku_list |= Yaku::Ippatu; // 一発

    if (flag & Yaku::Tyankan)
        yaku_list |= Yaku::Tyankan; // 搶槓
    else if (flag & Yaku::Rinsyankaiho)
        yaku_list |= Yaku::Rinsyankaiho; // 嶺上開花
    else if (flag & Yaku::Haiteitumo)
        yaku_list |= Yaku::Haiteitumo; // 海底摸月
    else if (flag & Yaku::Hoteiron)
        yaku_list |= Yaku::Hoteiron; // 河底撈魚

    if ((flag & Yaku::Tumo) && tehai.is_menzen())
        yaku_list |= Yaku::Tumo; // 門前清自摸和

    if (check_tanyao(tehai))
        yaku_list |= Yaku::Tanyao; // 断幺九
    else if (check_honroto(tehai))
        yaku_list |= Yaku::Honroto; // 混老頭
    else if (check_honiso(tehai))
        yaku_list |= Yaku::Honiso; // 混一色
    else if (check_tiniso(tehai))
        yaku_list |= Yaku::Tiniso; // 清一色

    if (syanten_type == SyantenType::Normal) {
        if (check_syosangen(tehai))
            yaku_list |= Yaku::Syosangen; // 小三元

        if (check_sankantu(tehai))
            yaku_list |= Yaku::Sankantu; // 三槓子

        if ((tehai.zihai & Bit::mask[Tile::Haku]) >= Bit::hai3[Tile::Haku])
            yaku_list |= Yaku::SangenhaiHaku; // 三元牌 (白)
        if ((tehai.zihai & Bit::mask[Tile::Hatu]) >= Bit::hai3[Tile::Hatu])
            yaku_list |= Yaku::SangenhaiHatu; // 三元牌 (發)
        if ((tehai.zihai & Bit::mask[Tile::Tyun]) >= Bit::hai3[Tile::Tyun])
            yaku_list |= Yaku::SangenhaiTyun; // 三元牌 (中)

        if ((tehai.zihai & Bit::mask[zikaze_]) >= Bit::hai3[zikaze_]) {
            if (zikaze_ == Tile::Ton)
                yaku_list |= Yaku::ZikazeTon; // 自風牌 (東)
            else if (zikaze_ == Tile::Nan)
                yaku_list |= Yaku::ZikazeNan; // 自風牌 (南)
            else if (zikaze_ == Tile::Sya)
                yaku_list |= Yaku::ZikazeSya; // 自風牌 (西)
            else if (zikaze_ == Tile::Pe)
                yaku_list |= Yaku::ZikazePe; // 自風牌 (北)
        }

        if ((tehai.zihai & Bit::mask[bakaze_]) >= Bit::hai3[bakaze_]) {
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
 * @param[in] tehai 手牌
 * @param[in] winning_tile 和了牌
 * @return std::tuple<YakuList, int, std::vector<Block>> (成立した役一覧, 符, 面子構成)
 */
std::tuple<YakuList, int, std::vector<Block>>
ScoreCalculator::check_pattern_yaku(const Hand &tehai, int winning_tile, YakuList flag)
{
    static const std::vector<YakuList> pattern_yaku = {
        Yaku::Pinhu,        Yaku::Ipeko,          Yaku::Toitoiho,  Yaku::Sananko,
        Yaku::SansyokuDoko, Yaku::SansyokuDozyun, Yaku::IkkiTukan, Yaku::Tyanta,
        Yaku::Sankantu,     Yaku::Zyuntyanta,     Yaku::Ryanpeko,
    };

    // 面子構成一覧を取得する。
    std::vector<std::vector<Block>> pattern =
        create_block_patterns(tehai, winning_tile, flag & Yaku::Tumo);

    int max_han = 0;
    int max_hu  = 0;
    size_t max_idx;
    YakuList max_yaku_list;

    for (size_t i = 0; i < pattern.size(); ++i) {
        const auto &blocks = pattern[i];
        YakuList yaku_list = Yaku::Null;

        bool pinhu_flag = false; // 平和形かどうか (符計算で利用)
        if (tehai.is_menzen()) {
            if (check_pinhu(blocks, winning_tile)) {
                yaku_list |= Yaku::Pinhu; // 平和
                pinhu_flag = true;
            }

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
        int han = 0;
        for (const auto &yaku : pattern_yaku) {
            if (yaku_list & yaku)
                han += tehai.is_menzen() ? Yaku::Info[yaku].han[0]
                                         : Yaku::Info[yaku].han[1];
        }

        // 符を計算する。
        int hu = calc_hu(blocks, winning_tile, tehai.is_menzen(), flag & Yaku::Tumo,
                         pinhu_flag);

        // 符を計算する。
        if (max_han < han || (max_han == han && max_hu < hu)) {
            max_han       = han;
            max_hu        = hu;
            max_idx       = i;
            max_yaku_list = yaku_list;
        }
    }

    return {max_yaku_list, max_hu, pattern[max_idx]};
}

/**
 * @brief 符を計算する。
 * 
 * @param[in] blocks 面子構成
 * @param[in] winning_tile 和了牌
 * @param[in] menzen 門前かどうか
 * @param[in] tumo 自摸和了りかどうか
 * @return int 符
 */
int ScoreCalculator::calc_hu(const std::vector<Block> &blocks, int winning_tile,
                             bool menzen, bool tumo, bool pinhu) const
{
    // 符計算の例外
    //////////////////////////
    if (pinhu && tumo)
        return 20; // 平和形 + ツモ
    if (pinhu && !tumo)
        return 30; // 平和形 + ロン

    // 通常の符計算
    //////////////////////////

    // 副底
    int hu = 20;

    // 門前加符
    if (menzen && !tumo)
        hu += 10; // 門前ロン
    // ツモ符
    else if (tumo)
        hu += 2; // 門前ツモ、非門前ツモ

    // 待ちによる符
    int mati = get_wait_type(blocks, winning_tile);
    if (mati == WaitType::Kantyan || mati == WaitType::Pentyan ||
        mati == WaitType::Tanki)
        hu += 2; // 嵌張待ち、辺張待ち、単騎待ち

    // 面子構成による符
    for (const auto &block : blocks) {
        int block_hu = 0;
        if (block.type & (Block::Kotu | Block::Kantu)) {
            // 刻子、対子の場合
            if (block.type == (Block::Kotu | Block::Huro))
                block_hu = 2; // 明刻子
            else if (block.type == Block::Kotu)
                block_hu = 4; // 暗刻子
            else if (block.type == (Block::Kantu | Block::Huro))
                block_hu = 8; // 明槓子
            else if (block.type == Block::Kantu)
                block_hu = 16; // 暗槓子

            bool yaotyu =
                block.min_tile == Tile::Manzu1 || block.min_tile == Tile::Manzu9 ||
                block.min_tile == Tile::Pinzu1 || block.min_tile == Tile::Pinzu9 ||
                block.min_tile == Tile::Sozu1 || block.min_tile == Tile::Sozu9 ||
                block.min_tile >= Tile::Ton;

            if (yaotyu)
                hu += block_hu * 2; // 幺九牌を含む場合
            else
                hu += block_hu; // 幺九牌を含まない場合
        }
        else if (block.type & Block::Toitu) {
            // 対子の場合
            if (block.min_tile >= Tile::Haku)
                hu += 2; // 役牌
            if (block.min_tile == zikaze_)
                hu += 2; // 自風牌
            if (block.min_tile == bakaze_)
                hu += 2; // 場風牌
        }
    }

    return Hu::round_up_hu(hu);
}

/**
 * @brief 符を計算する (内訳)。
 */
std::tuple<int, std::vector<std::tuple<std::string, int>>>
ScoreCalculator::calc_hu(const std::vector<Block> &blocks, int winning_tile,
                         bool menzen, bool tumo) const
{
    std::vector<std::tuple<std::string, int>> hu;

    bool pinhu = check_pinhu(blocks, winning_tile);

    // 符計算の例外
    //////////////////////////
    if (pinhu && tumo) {
        hu.emplace_back("平和・ツモ", 20);
        return {20, hu};
    }
    else if (pinhu && !tumo && !menzen) {
        hu.emplace_back("喰い平和・ロン", 30);
        return {30, hu};
    }

    // 通常の符計算
    //////////////////////////

    // 副底
    hu.emplace_back("副底", 20);

    // 門前加符
    if (menzen && !tumo)
        hu.emplace_back("門前加符", 10);
    else
        hu.emplace_back("門前加符", 0);

    // 自摸加符
    if (tumo)
        hu.emplace_back("自摸加符", 2);
    else
        hu.emplace_back("自摸加符", 0);

    // 待ちによる符
    int mati = get_wait_type(blocks, winning_tile);
    if (mati == WaitType::Kantyan)
        hu.emplace_back("待ちによる符: 嵌張待ち", 2);
    else if (mati == WaitType::Pentyan)
        hu.emplace_back("待ちによる符: 辺張待ち", 2);
    else if (mati == WaitType::Tanki)
        hu.emplace_back("待ちによる符: 単騎待ち", 2);
    else if (mati == WaitType::Syanpon)
        hu.emplace_back("待ちによる符: 双ポン待ち", 0);
    else if (mati == WaitType::Ryanmen)
        hu.emplace_back("待ちによる符: 両面待ち", 0);

    // 面子構成による符
    for (const auto &block : blocks) {
        if (block.type & (Block::Kotu | Block::Kantu)) {
            bool yaotyu =
                block.min_tile == Tile::Manzu1 || block.min_tile == Tile::Manzu9 ||
                block.min_tile == Tile::Pinzu1 || block.min_tile == Tile::Pinzu9 ||
                block.min_tile == Tile::Sozu1 || block.min_tile == Tile::Sozu9 ||
                block.min_tile >= Tile::Ton;

            // 刻子、対子の場合
            if (block.type == (Block::Kotu | Block::Huro) && !yaotyu)
                hu.emplace_back(
                    fmt::format("面子構成による符: {} (断幺牌)", block.to_string()), 2);
            else if (block.type == Block::Kotu && !yaotyu)
                hu.emplace_back(
                    fmt::format("面子構成による符: {} (断幺牌)", block.to_string()), 4);
            else if (block.type == (Block::Kantu | Block::Huro) && !yaotyu)
                hu.emplace_back(
                    fmt::format("面子構成による符: {} (断幺牌)", block.to_string()), 8);
            else if (block.type == Block::Kantu && !yaotyu)
                hu.emplace_back(
                    fmt::format("面子構成による符: {} (断幺牌)", block.to_string()),
                    16);
            else if (block.type == (Block::Kantu | Block::Huro) && yaotyu)
                hu.emplace_back(
                    fmt::format("面子構成による符: {} (幺九牌)", block.to_string()), 4);
            else if (block.type == Block::Kotu && yaotyu)
                hu.emplace_back(
                    fmt::format("面子構成による符: {} (幺九牌)", block.to_string()), 8);
            else if (block.type == (Block::Kantu | Block::Huro) && yaotyu)
                hu.emplace_back(
                    fmt::format("面子構成による符: {} (幺九牌)", block.to_string()),
                    16);
            else if (block.type == Block::Kantu && yaotyu)
                hu.emplace_back(
                    fmt::format("面子構成による符: {} (幺九牌)", block.to_string()),
                    32);
        }
        else if (block.type & Block::Toitu) {
            // 対子の場合
            if (is_yakuhai(block.min_tile))
                hu.emplace_back(fmt::format("雀頭による符: {} 役牌", block.to_string()),
                                2);
            else
                hu.emplace_back(fmt::format("雀頭による符: {}", block.to_string()), 0);
        }
        else {
            hu.emplace_back(fmt::format("面子構成による符: {}", block.to_string()), 0);
        }
    }

    int total_hu = 0;
    for (auto &[type, n] : hu)
        total_hu += n;

    // 下一桁は切り上げる。
    return {Hu::round_up_hu(total_hu), hu};
}

////////////////////////////////////////////////////////////////////////////////////////
/// パラメータを設定する関数
////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief 赤ドラ有りかどうかを設定する。
 * 
 * @param[in] enabled 有効にするかどうか
 */
void ScoreCalculator::enable_akadora(bool enabled)
{
    akadora_enabled_ = enabled;
}

/**
 * @brief 喰い断有りかどうかを設定する。
 * 
 * @param[in] enabled 有効にするかどうか
 */
void ScoreCalculator::enable_open_tanyao(bool enabled)
{
    open_tanyao_enabled_ = enabled;
}

/**
 * @brief 場風牌
 * 
 * @param[in] hai 牌
 */
void ScoreCalculator::set_bakaze(int bakaze)
{
    bakaze_ = bakaze;
}

/**
 * @brief 自風を設定する
 * 
 * @param[in] hai 牌
 */
void ScoreCalculator::set_zikaze(int zikaze)
{
    zikaze_ = zikaze;
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
 * @brief 供託棒の数を設定する。
 * 
 * @param[in] n 供託棒の数
 */
void ScoreCalculator::set_num_kyotakubo(int n)
{
    n_kyotakubo_ = n;
}

/**
 * @brief 表ドラを設定する。
 * 
 * @param[in] dora_list 表ドラの一覧
 */
void ScoreCalculator::set_dora_tiles(const std::vector<int> &dora_list)
{
    dora_tiles_ = dora_list;
}

/**
 * @brief 裏ドラを設定する。
 * 
 * @param[in] uradora_list 裏ドラの一覧
 */
void ScoreCalculator::set_uradora_tiles(const std::vector<int> &uradora_list)
{
    uradora_tiles_ = uradora_list;
}

////////////////////////////////////////////////////////////////////////////////////////
/// 役を判定する関数
////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief 緑一色かどうかを判定する。
 */
bool ScoreCalculator::check_ryuiso(const Hand &tehai) const
{
    // 「2, 3, 4, 6, 8, 發以外の牌がない」かどうかを調べる
    //               | 9 | 8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 |
    int sozu_mask = 0b111'000'111'000'111'000'000'000'111;
    //                |中 |發 |白 |北 |西 |南 |東 |
    int zihai_mask = 0b111'000'111'111'111'111'111;

    return !(tehai.manzu || tehai.pinzu || (tehai.sozu & sozu_mask) ||
             (tehai.zihai & zihai_mask));
}

/**
 * @brief 大三元かどうかを判定する。
 */
bool ScoreCalculator::check_daisangen(const Hand &tehai) const
{
    // 「三元牌が各3枚以上」かどうかを調べる。
    //          |中 |發 |白 |北 |西 |南 |東 |
    int mask = 0b011'011'011'000'000'000'000;

    return (tehai.zihai & Bit::SangenhaiMask) == mask;
}

/**
 * @brief 小四喜かどうかを判定する。
 */
bool ScoreCalculator::check_syosusi(const Hand &tehai) const
{
    // 「風牌の合計が11枚」かどうかを調べる。
    int kazehai = tehai.zihai & Bit::KazehaiMask;
    return Bit::sum(kazehai) == 11;
}

/**
 * @brief 字一色かどうかを判定する。
 */
bool ScoreCalculator::check_tuiso(const Hand &tehai) const
{
    // 「字牌以外の牌がない」かどうかを調べる。
    return !(tehai.manzu || tehai.pinzu || tehai.sozu);
}

/**
 * @brief 九連宝灯かどうかを判定する。
 */
bool ScoreCalculator::check_tyurenpoto(const Hand &tehai, int winning_tile) const
{
    const auto &s_tbl = SyantenCalculator::s_tbl_;

    if (tehai.is_melded())
        return false; // 副露している場合

    int key;
    if (winning_tile <= Tile::Manzu9)
        key = tehai.manzu;
    else if (winning_tile <= Tile::Pinzu9)
        key = tehai.pinzu;
    else if (winning_tile <= Tile::Sozu9)
        key = tehai.sozu;
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
bool ScoreCalculator::check_suanko(const Hand &tehai, YakuList flag) const
{
    const auto &s_tbl = SyantenCalculator::s_tbl_;
    const auto &z_tbl = SyantenCalculator::z_tbl_;

    if (!(flag & Yaku::Tumo))
        return false; // ロンした場合、暗刻4つにならないので自摸和了り限定

    if (!tehai.is_menzen())
        return false; // 門前でない場合

    // 「刻子が4つある」かどうかを調べる。
    int n_ge4 = s_tbl[tehai.manzu].n_ge4 + s_tbl[tehai.pinzu].n_ge4 +
                s_tbl[tehai.sozu].n_ge4 + z_tbl[tehai.zihai].n_ge4;
    int n_ge3 = s_tbl[tehai.manzu].n_ge3 + s_tbl[tehai.pinzu].n_ge3 +
                s_tbl[tehai.sozu].n_ge3 + z_tbl[tehai.zihai].n_ge3;

    return n_ge3 - n_ge4 == 4;
}

/**
 * @brief 清老頭かどうかをチェックする。
 */
bool ScoreCalculator::check_tinroto(const Hand &tehai) const
{
    // 「老頭牌以外の牌がない」かどうかを調べる
    return !((tehai.manzu & Bit::TanyaoMask) || (tehai.pinzu & Bit::TanyaoMask) ||
             (tehai.sozu & Bit::TanyaoMask) || tehai.zihai);
}

/**
 * @brief 四槓子かどうかを判定する。
 */
bool ScoreCalculator::check_sukantu(const Hand &tehai) const
{
    // 「副露ブロックに4つの槓子がある」かどうかを調べる。
    int cnt = 0;
    for (const auto &block : tehai.melded_blocks)
        cnt += MeldType::Ankan <= block.type; // enum の値で2以上が槓子であるため

    return cnt == 4;
}

/**
 * @brief 四暗刻単騎かどうかを判定する。
 */
bool ScoreCalculator::check_suanko_tanki(const Hand &tehai, int winning_tile) const
{
    const auto &s_tbl = SyantenCalculator::s_tbl_;
    const auto &z_tbl = SyantenCalculator::z_tbl_;

    if (!tehai.is_menzen())
        return false; // 門前でない場合

    // 和了牌の種類で雀頭が構成されているかどうかを調べる。
    int key;
    if (winning_tile <= Tile::Manzu9)
        key = tehai.manzu;
    else if (winning_tile <= Tile::Pinzu9)
        key = tehai.pinzu;
    else if (winning_tile <= Tile::Sozu9)
        key = tehai.sozu;
    else
        key = tehai.zihai;

    if ((key & Bit::mask[winning_tile]) != Bit::hai2[winning_tile])
        return false;

    // 刻子が4つかどうかを調べる。
    int n_ge4 = s_tbl[tehai.manzu].n_ge4 + s_tbl[tehai.pinzu].n_ge4 +
                s_tbl[tehai.sozu].n_ge4 + z_tbl[tehai.zihai].n_ge4;
    int n_ge3 = s_tbl[tehai.manzu].n_ge3 + s_tbl[tehai.pinzu].n_ge3 +
                s_tbl[tehai.sozu].n_ge3 + z_tbl[tehai.zihai].n_ge3;

    return n_ge3 - n_ge4 == 4;
}

/**
 * @brief 大四喜かどうかを判定する。
 */
bool ScoreCalculator::check_daisusi(const Hand &tehai) const
{
    // 「風牌の合計が12枚」かどうかを調べる。
    int kazehai = tehai.zihai & Bit::KazehaiMask;
    return Bit::sum(kazehai) == 12;
}

/**
 * @brief 純正九連宝灯かどうかを判定する。
 */
bool ScoreCalculator::check_tyurenpoto9(const Hand &tehai, int winning_tile) const
{
    if (tehai.is_melded())
        return false; // 副露している場合

    // 「和了牌を除いた場合に 1112345678999 となっている」かどうかを調べる。
    //          | 9 | 8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 |
    int mask = 0b011'001'001'001'001'001'001'001'011;

    if (winning_tile <= Tile::Manzu9)
        return tehai.manzu - Bit::hai1[winning_tile] == mask;
    else if (winning_tile <= Tile::Pinzu9)
        return tehai.pinzu - Bit::hai1[winning_tile] == mask;
    else if (winning_tile <= Tile::Sozu9)
        return tehai.sozu - Bit::hai1[winning_tile] == mask;

    return false;
}

/**
 * @brief 国士無双13面待ちかどうかを判定する。
 */
bool ScoreCalculator::check_kokusi13(const Hand &tehai, int winning_tile) const
{
    // 「和了牌を除いた場合に幺九牌がすべて1個ずつある」かどうかを調べる。
    int manzu = tehai.manzu;
    int pinzu = tehai.pinzu;
    int sozu  = tehai.sozu;
    int zihai = tehai.zihai;

    if (winning_tile <= Tile::Manzu9)
        manzu -= Bit::hai1[winning_tile];
    else if (winning_tile <= Tile::Pinzu9)
        pinzu -= Bit::hai1[winning_tile];
    else if (winning_tile <= Tile::Sozu9)
        sozu -= Bit::hai1[winning_tile];
    else
        zihai -= Bit::hai1[winning_tile];

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
bool ScoreCalculator::check_tanyao(const Hand &tehai) const
{
    if (!open_tanyao_enabled_ && !tehai.is_menzen())
        return false; // 喰い断なしで副露している場合

    // 「幺九牌がない」かどうかを調べる。
    return !((tehai.manzu & Bit::RotohaiMask) || (tehai.pinzu & Bit::RotohaiMask) ||
             (tehai.sozu & Bit::RotohaiMask) || tehai.zihai);
}

/**
 * @brief 混老頭かどうかを判定する。
 */
bool ScoreCalculator::check_honroto(const Hand &tehai) const
{
    // 「幺九牌以外の牌がない」かつ「字牌がある」かどうかを調べる。
    return !((tehai.manzu & Bit::TanyaoMask) || (tehai.pinzu & Bit::TanyaoMask) ||
             (tehai.sozu & Bit::TanyaoMask)) &&
           tehai.zihai;
}

/**
 * @brief 混一色かどうかを判定する。
 */
bool ScoreCalculator::check_honiso(const Hand &tehai) const
{
    // 「1種類の数牌がある」かつ「字牌がある」かどうかを調べる
    if (tehai.manzu && !tehai.pinzu && !tehai.sozu && tehai.zihai)
        return true;
    else if (!tehai.manzu && tehai.pinzu && !tehai.sozu && tehai.zihai)
        return true;
    else if (!tehai.manzu && !tehai.pinzu && tehai.sozu && tehai.zihai)
        return true;

    return false;
}

/**
 * @brief 清一色かどうかを判定する。
 */
bool ScoreCalculator::check_tiniso(const Hand &tehai) const
{
    // 「1種類の数牌がある」かつ「字牌がない」かどうかを調べる
    if (tehai.manzu && !tehai.pinzu && !tehai.sozu && !tehai.zihai)
        return true;
    else if (!tehai.manzu && tehai.pinzu && !tehai.sozu && !tehai.zihai)
        return true;
    else if (!tehai.manzu && !tehai.pinzu && tehai.sozu && !tehai.zihai)
        return true;

    return false;
}

/**
 * @brief 小三元かどうかを判定する。
 */
bool ScoreCalculator::check_syosangen(const Hand &tehai) const
{
    // 「役牌の合計が8枚」かどうかを調べる。
    int yakuhai = tehai.zihai & Bit::SangenhaiMask;
    return Bit::sum(yakuhai) == 8;
}

/**
 * @brief 三槓子かどうかを判定する。
 */
bool ScoreCalculator::check_sankantu(const Hand &tehai) const
{
    // 「副露ブロックに3つの槓子がある」かどうかを調べる。
    int cnt = 0;
    for (const auto &block : tehai.melded_blocks)
        cnt += MeldType::Ankan <= block.type; // enum の値で2以上が槓子であるため

    return cnt == 3;
}

/**
 * @brief 平和形かどうかを判定する。(門前かどうかは呼び出し側でチェックすること)
 */
bool ScoreCalculator::check_pinhu(const std::vector<Block> blocks,
                                  int winning_tile) const
{
    // すべてのブロックが順子または役牌でない対子かどうか
    for (const auto &block : blocks) {
        if (block.type & (Block::Kotu | Block::Kantu))
            return false; // 刻子、槓子の場合

        if ((block.type & Block::Toitu) && is_yakuhai(block.min_tile))
            return false; // 対子の役牌の場合
    }

    // 両面待ちかどうかを判定する。
    return get_wait_type(blocks, winning_tile) == WaitType::Ryanmen;
}

/**
 * @brief 一盃口の数を数える。(門前かどうかは呼び出し側でチェックすること)
 */
int ScoreCalculator::check_ipeko(const std::vector<Block> blocks) const
{
    std::vector<int> count(34, 0);
    for (const auto &block : blocks) {
        if (block.type & Block::Syuntu)
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
        if (block.type & Block::Syuntu)
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
        if (block.type & (Block::Kotu | Block::Kantu))
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
        if (block.type & Block::Syuntu)
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
        if (block.type & Block::Syuntu)
            return false; // 順子の場合
    }

    return true;
}

/**
 * @brief 混全帯幺九、純全帯幺九かどうかを判定する。
 */
int ScoreCalculator::check_tyanta(const std::vector<Block> blocks) const
{
    // 条件「順子がない」
    bool yaotyu = true; // すべてのブロックに幺九牌が含まれるかどうか
    bool zihai = false; // 字牌が含まれるかどうか
    for (const auto &block : blocks) {
        if (block.type & Block::Syuntu) {
            // 順子の場合

            // ブロックが 123 または 789 かどうか
            yaotyu &=
                block.min_tile == Tile::Manzu1 || block.min_tile == Tile::Manzu7 ||
                block.min_tile == Tile::Pinzu1 || block.min_tile == Tile::Pinzu7 ||
                block.min_tile == Tile::Sozu1 || block.min_tile == Tile::Sozu7;
        }
        else {
            // 刻子、槓子、対子の場合、

            // ブロックが 111 または 999 かどうか
            yaotyu &= block.min_tile == Tile::Manzu1 ||
                      block.min_tile == Tile::Manzu9 ||
                      block.min_tile == Tile::Pinzu1 ||
                      block.min_tile == Tile::Pinzu9 || block.min_tile == Tile::Sozu1 ||
                      block.min_tile == Tile::Sozu9 || block.min_tile >= Tile::Ton;
            // ブロックが字牌かどうか
            zihai |= block.min_tile >= Tile::Ton;
        }
    }

    if (yaotyu && zihai)
        return 1; // 混全帯幺九
    else if (yaotyu && !zihai)
        return 2; // 純全帯幺九

    return 0;
}

/**
 * @brief 三暗刻かどうかを判定する。
 */
bool ScoreCalculator::check_sananko(const std::vector<Block> blocks) const
{
    int cnt = 0;
    for (const auto &block : blocks) {
        if (block.type == Block::Kotu || block.type == Block::Kantu)
            cnt++; // 暗刻子、暗槓子の場合
    }

    return cnt == 3;
}

/**
 * @brief ドラの数を数える。
 * 
 * @param[in] tehai 手牌
 * @param[in] dora_list ドラ牌の一覧
 * @return int ドラの数
 */
int ScoreCalculator::count_dora(const Hand &tehai, std::vector<int> dora_list) const
{
    int n_dora = 0;
    for (const auto &dora : dora_list) {
        n_dora += tehai.num_tiles(dora);

        for (const auto &block : tehai.melded_blocks) {
            for (auto tile : block.tiles) {
                if (tile == dora)
                    n_dora++;
            }
        }

        return n_dora;
    }

    return n_dora;
}

/**
 * @brief 赤ドラの数を数える。
 * 
 * @param[in] tehai 手牌
 * @param[in] dora_list ドラ牌の一覧
 * @return int ドラの数
 */
int ScoreCalculator::count_akadora(const Hand &tehai) const
{
    int n_akadora = tehai.aka_manzu5 + tehai.aka_pinzu5 + tehai.aka_sozu5;

    for (const auto &block : tehai.melded_blocks) {
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

////////////////////////////////////////////////////////////////////////////////////////
/// helper functions
////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief 副露ブロックを統合した手牌を作成する。
 *        槓子は刻子と同じ扱いで3つの牌だけ手牌に加え、統合後の手牌の枚数が14枚となるようにする。
 * 
 * @param[in]  tehai 手牌
 * @return Hand 副露ブロックを統合した手牌
 */
Hand ScoreCalculator::merge_hand(const Hand &tehai) const
{
    Hand merged_tehai = tehai;

    for (const auto &block : tehai.melded_blocks) {
        int min_tile = aka2normal(block.tiles.front());

        int *key;
        if (min_tile <= Tile::Manzu9)
            key = &merged_tehai.manzu;
        else if (min_tile <= Tile::Pinzu9)
            key = &merged_tehai.pinzu;
        else if (min_tile <= Tile::Sozu9)
            key = &merged_tehai.sozu;
        else
            key = &merged_tehai.zihai;

        if (block.type == MeldType::Ti) // チー
            *key +=
                Bit::hai1[min_tile] | Bit::hai1[min_tile + 1] | Bit::hai1[min_tile + 2];
        else // ポン、暗槓、明槓、加槓
            *key += Bit::hai3[min_tile];
    }

    return merged_tehai;
}

/**
 * @brief 供託棒、積み棒による点数を計算する。
 * 
 * @return int 供託棒、積み棒による点数
 */
int ScoreCalculator::calc_extra_score() const
{
    return 1000 * n_kyotakubo_ + 300 * n_tumibo_;
}

/**
 * @brief 設定を文字列にして返す。
 * 
 * @return std::string 文字列
 */
std::string ScoreCalculator::to_string() const
{
    std::string s;

    s += fmt::format("[ルール] 赤ドラ: {}, 喰い断: {}\n",
                     akadora_enabled_ ? "あり" : "なし",
                     open_tanyao_enabled_ ? "あり" : "なし");
    s += fmt::format("[場] 場風: {}, 自風: {}, 積み棒の数: {}, 供託棒の数: {}\n",
                     Tile::Names.at(bakaze_), Tile::Names.at(zikaze_), n_tumibo_,
                     n_kyotakubo_);

    s += "[表ドラ] ";
    for (const auto &hai : dora_tiles_)
        s += fmt::format("{}{}", Tile::Names.at(hai),
                         &hai == &dora_tiles_.back() ? "\n" : ", ");

    s += "[裏ドラ] ";
    for (const auto &hai : uradora_tiles_)
        s += fmt::format("{}{}", Tile::Names.at(hai),
                         &hai == &uradora_tiles_.back() ? "\n" : ", ");

    return s;
}

/**
 * @brief 初期化する。
 * 
 * @param[in] path パス
 * @param[out] table テーブル
 * @return 初期化に成功した場合は true、そうでない場合は false を返す。
 */
bool ScoreCalculator::make_table(const std::string &path,
                                 std::map<int, std::vector<std::vector<Block>>> &table)
{
    std::FILE *fp = std::fopen(path.c_str(), "rb");
    if (!fp) {
        spdlog::error("Failed to open {}.", path);
        return false;
    }

    char *buffer = new char[1000000];
    rapidjson::FileReadStream is(fp, buffer, sizeof(buffer));
    rapidjson::Document doc;
    doc.ParseStream(is);
    if (doc.HasParseError()) {
        spdlog::error("Failed to parse {}.", path);
        return false;
    }

    for (auto &v : doc.GetArray()) {
        int key = v["key"].GetInt();

        std::vector<std::vector<Block>> pattern;
        for (auto &v2 : v["pattern"].GetArray())
            pattern.push_back(get_blocks(v2.GetString()));

        table[key] = pattern;
    }

    fclose(fp);
    delete buffer;

    return true;
}

std::vector<Block> ScoreCalculator::get_blocks(const std::string &s)
{
    std::vector<Block> blocks;

    size_t len = s.size();
    for (size_t i = 0; i < len; i += 2) {
        Block block;
        block.min_tile = s[i] - '0';
        if (s[i + 1] == 'k')
            block.type = Block::Kotu;
        else if (s[i + 1] == 's')
            block.type = Block::Syuntu;
        else if (s[i + 1] == 't')
            block.type = Block::Toitu;

        blocks.emplace_back(block);
    }

    return blocks;
}

/**
 * @brief 役牌かどうか判定する。
 * 
 * @param[in] hai 牌
 * @return 役牌の場合は true、そうでない場合は false を返す。
 */
bool ScoreCalculator::is_yakuhai(int hai) const
{
    return hai == zikaze_ || hai == bakaze_ || hai >= Tile::Haku;
}

/**
 * @brief 手牌の可能なブロック構成パターンを生成する。
 * 
 * @param[in] tehai 手牌
 * @param[in] winning_tile 和了牌
 * @param[in] tumo ツモかどうか
 * @return std::vector<std::vector<Block>> 面子構成の一覧
 */
std::vector<std::vector<Block>>
ScoreCalculator::create_block_patterns(const Hand &tehai, int winning_tile,
                                       bool tumo) const
{
    std::vector<std::vector<Block>> pattern;
    std::vector<Block> blocks(5);
    int i = 0;

    // 副露ブロックをブロック一覧に追加する。
    for (const auto &huro_block : tehai.melded_blocks) {
        if (huro_block.type == MeldType::Pon)
            blocks[i].type = Block::Kotu | Block::Huro;
        else if (huro_block.type == MeldType::Ti)
            blocks[i].type = Block::Syuntu | Block::Huro;
        else if (huro_block.type == MeldType::Ankan)
            blocks[i].type = Block::Kantu;
        else
            blocks[i].type = Block::Kantu | Block::Huro;
        blocks[i].min_tile = aka2normal(huro_block.tiles.front());
        i++;
    }

    // 手牌の切り分けパターンを列挙する。
    create_block_patterns(tehai, winning_tile, tumo, pattern, blocks, i);

    // for (const auto &p : pattern) {
    //     for (const auto &b : p)
    //         std::cout << b.to_string() << " ";
    //     std::cout << std::endl;
    // }

    // 和了牌と同じ牌が手牌に3枚しかない刻子は明刻子になる。
    // int n_tiles = tehai.num_tiles(winning_tile);
    // for (auto &blocks : pattern) {
    //     for (auto &block : blocks) {
    //         if (!(block.type & Block::Huro) && block.type == Block::Kotu &&
    //             block.min_tile == winning_tile && n_tiles < 4) {
    //             block.type |= Block::Huro;
    //             break;
    //         }
    //     }
    // }

    return pattern;
}

/**
 * @brief 役満かどうかを判定する。
 * 
 * @param[in] tehai 手牌
 * @param[in] winning_tile 和了牌
 * @param[in] flag 成立フラグ
 * @param[in] syanten_type 和了り形の種類
 * @return YakuList 成立した役一覧
 */
void ScoreCalculator::create_block_patterns(const Hand &tehai, int winning_tile,
                                            bool tumo,
                                            std::vector<std::vector<Block>> &pattern,
                                            std::vector<Block> &blocks, size_t i,
                                            int d) const
{
    if (d == 4) {
        //pattern.push_back(blocks);

        if (tumo) {
            // ツモ和了りの場合
            pattern.push_back(blocks);
        }
        else {
            // ロン和了りの場合、ロンした牌を含むブロックを副露にする。
            bool syuntu = false; // 123123のような場合はどれか1つだけ明順子にする
            for (auto &block : blocks) {
                if (block.type & Block::Huro)
                    continue;

                if (!syuntu && block.type == Block::Syuntu &&
                    block.min_tile <= winning_tile &&
                    winning_tile <= block.min_tile + 2) {
                    block.type |= Block::Huro;
                    pattern.push_back(blocks);
                    block.type &= ~Block::Huro;
                    syuntu = true;
                }
                else if (block.type != Block::Syuntu &&
                         block.min_tile == winning_tile) {
                    block.type |= Block::Huro;
                    pattern.push_back(blocks);
                    block.type &= ~Block::Huro;
                }
            }
        }

        return;
    }

    if (d == 0) {
        // 萬子の面子構成
        if (s_tbl_[tehai.manzu].empty())
            create_block_patterns(tehai, winning_tile, tumo, pattern, blocks, i, d + 1);

        for (const auto &manzu_pattern : s_tbl_[tehai.manzu]) {
            for (const auto &block : manzu_pattern)
                blocks[i++] = block;
            create_block_patterns(tehai, winning_tile, tumo, pattern, blocks, i, d + 1);
            i -= manzu_pattern.size();
        }
    }
    else if (d == 1) {
        // 筒子の面子構成
        if (s_tbl_[tehai.pinzu].empty())
            create_block_patterns(tehai, winning_tile, tumo, pattern, blocks, i, d + 1);

        for (const auto &pinzu_pattern : s_tbl_[tehai.pinzu]) {
            for (const auto &block : pinzu_pattern)
                blocks[i++] = {block.type, block.min_tile + 9};

            create_block_patterns(tehai, winning_tile, tumo, pattern, blocks, i, d + 1);
            i -= pinzu_pattern.size();
        }
    }
    else if (d == 2) {
        // 索子の面子構成
        if (s_tbl_[tehai.sozu].empty())
            create_block_patterns(tehai, winning_tile, tumo, pattern, blocks, i, d + 1);

        for (const auto &sozu_pattern : s_tbl_[tehai.sozu]) {
            for (const auto &block : sozu_pattern)
                blocks[i++] = {block.type, block.min_tile + 18};
            create_block_patterns(tehai, winning_tile, tumo, pattern, blocks, i, d + 1);
            i -= sozu_pattern.size();
        }
    }
    else if (d == 3) {
        // 字牌の面子構成
        if (z_tbl_[tehai.zihai].empty())
            create_block_patterns(tehai, winning_tile, tumo, pattern, blocks, i, d + 1);

        for (const auto &zihai_pattern : z_tbl_[tehai.zihai]) {
            for (const auto &block : zihai_pattern)
                blocks[i++] = {block.type, block.min_tile + 27};
            create_block_patterns(tehai, winning_tile, tumo, pattern, blocks, i, d + 1);
            i -= zihai_pattern.size();
        }
    }
}

std::tuple<int, int, int, int, int> ScoreCalculator::calc_score(int han, int hu,
                                                                int score_title) const
{
    int ko2oya_ron, ko2oya_tumo, ko2ko_tumo, oya2ko_ron, oya2ko_tumo;

    if (score_title == ScoreTitle::Null) {
        // 満貫未満
        ko2oya_ron  = ScoreBoard::Ko2OyaRon[hu][han - 1];  // 子 → 親ロン
        ko2oya_tumo = ScoreBoard::Ko2OyaTumo[hu][han - 1]; // 子 → 親ツモ
        ko2ko_tumo  = ScoreBoard::Ko2KoTumo[hu][han - 1];  // 子 → 子ツモ
        oya2ko_ron  = ScoreBoard::Oya2KoRon[hu][han - 1];  // 親 → 子ロン
        oya2ko_tumo = ScoreBoard::Oya2KoTumo[hu][han - 1]; // 親 → 子ツモ
    }
    else {
        // 満貫以上
        ko2oya_ron  = ScoreBoard::Ko2OyaRonOverMangan[score_title]; // 子 → 親ロン
        ko2oya_tumo = ScoreBoard::Ko2OyaTumoOverMangan[score_title]; // 子 → 親ツモ
        ko2ko_tumo  = ScoreBoard::Ko2KoTumoOverMangan[score_title]; // 子 → 子ツモ
        oya2ko_ron  = ScoreBoard::Oya2KoRonOverMangan[score_title]; // 親 → 子ロン
        oya2ko_tumo = ScoreBoard::Oya2KoTumoOverMangan[score_title]; // 親 → 子ツモ
    }

    return {ko2oya_ron, ko2oya_tumo, ko2ko_tumo, oya2ko_ron, oya2ko_tumo};
}

////////////////////////////////////////////////////////////////////////////////////////

std::map<int, std::vector<std::vector<Block>>> ScoreCalculator::s_tbl_;
std::map<int, std::vector<std::vector<Block>>> ScoreCalculator::z_tbl_;

} // namespace mahjong
