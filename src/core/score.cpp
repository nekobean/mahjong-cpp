#include "score.hpp"

#include "bitutils.hpp"
#include "syanten.hpp"

namespace mahjong
{
const std::vector<std::string> ScoreCalculator::Status::Names = {
    "正常終了",
    "牌が14枚でない場合",
    "和了り牌が手牌にない場合",
    "手牌が和了り形でない場合",
    "鳴き立直している場合",
    "役なし",
};

ScoreCalculator::ScoreCalculator() : rule_akahai_(true), rule_kuitan_(true)
{
}

/**
 * @brief 赤牌有りかどうかを設定する。
 * 
 * @param enabled 有効にするかどうか
 */
void ScoreCalculator::enable_akahai(bool enabled)
{
    rule_akahai_ = enabled;
}

/**
 * @brief 喰い断有りかどうかを設定する。
 * 
 * @param enabled 有効にするかどうか
 */
void ScoreCalculator::enable_kuitan(bool enabled)
{
    rule_kuitan_ = enabled;
}

/**
 * @brief ドラ牌を設定する。
 * 
 * @param[in] dorahais ドラ牌の一覧
 */
void ScoreCalculator::set_dora(const std::vector<int> &dora_list)
{
    dora_list_ = dora_list;
}

/**
 * @brief 場風牌
 * 
 * @param[in] hai 牌
 */
void ScoreCalculator::set_bakaze(int hai)
{
    bakaze_ = hai;
}

/**
 * @brief 自風を設定する
 * 
 * @param[in] hai 牌
 */
void ScoreCalculator::set_zikaze(int hai)
{
    jikaze_ = hai;
}

/**
 * @brief 本場を設定する。
 * 
 * @param[in] n 本場
 */
void ScoreCalculator::set_honba(int n)
{
    honba_ = n;
}

/**
 * @brief 積み棒を設定する。
 * 
 * @param[in] n 積み棒の数
 */
void ScoreCalculator::set_tumibo(int n)
{
    tumibo_ = n;
}

/**
 * @brief 供託棒による点数を計算する。
 * 
 * @return int 供託棒による点数
 */
int ScoreCalculator::calc_kyotaku_score()
{
    return 1000 * tumibo_ + 300 * honba_;
}

/**
 * @brief 副露を統合した手牌を作成する。
 * 
 * @param[in]  tehai 手牌
 * @param[in] huro_blocks 副露牌 
 * @return Tehai 副露を統合した手牌
 */
Tehai ScoreCalculator::merge_tehai(const Tehai &tehai,
                                   const std::vector<HuroBlock> &huro_blocks) const
{
    Tehai merged_tehai = tehai;

    for (const auto &block : huro_blocks) {
        if (block.type == Huro::Pon || block.type == Huro::Ti) {
            merged_tehai.manzu += block.tiles.manzu;
            merged_tehai.pinzu += block.tiles.pinzu;
            merged_tehai.sozu += block.tiles.sozu;
            merged_tehai.zihai += block.tiles.zihai;
        } else {
            merged_tehai.manzu += block.tiles.manzu / 4 * 3;
            merged_tehai.pinzu += block.tiles.pinzu / 4 * 3;
            merged_tehai.sozu += block.tiles.sozu / 4 * 3;
            merged_tehai.zihai += block.tiles.zihai / 4 * 3;
        }
    }

    return merged_tehai;
}

void ScoreCalculator::debug_print(const Tehai &tehai, const std::vector<HuroBlock> &huro_blocks,
                                  int agarihai, unsigned long long yaku_list)
{
    std::cout << std::boolalpha;
    std::cout << "手牌: " << tehai << std::endl;
    std::cout << "副露牌: ";
    for (const auto &block : huro_blocks)
        std::cout << block.tiles << " ";
    std::cout << std::endl;
    std::cout << "和了り牌: " << Tile::Names[agarihai] << std::endl;
    std::cout << "手牌に関係ない成立役: ";
    for (int i = 0; i < Yaku::Length; ++i) {
        unsigned long long yaku = (1ull << i) & yaku_list;
        if (yaku)
            std::cout << Yaku::Names.at(yaku) << " ";
    }
    std::cout << std::endl;
    std::cout << "自摸: " << bool(yaku_list & Yaku::Tumo) << std::endl;

    std::cout << "場の情報:" << std::endl;
    std::cout << "  場風牌: " << Tile::Names[bakaze_] << std::endl;
    std::cout << "  自風牌: " << Tile::Names[jikaze_] << std::endl;
    std::cout << "  本場: " << honba_ << "本場" << std::endl;
    std::cout << "  積み棒の数: " << tumibo_ << std::endl;
}

/**
 * @brief エラーをチェックする。
 */
int ScoreCalculator::check_error(const Tehai &tehai, const std::vector<HuroBlock> &huro_blocks,
                                 int agarihai, unsigned long long yaku_list) const
{
    if (tehai.count() != 14 - huro_blocks.size() * 3)
        return Status::InvalidNumTiles;
    if (!tehai.contains(agarihai))
        return Status::InvalidAgarihai;
    else if (!huro_blocks.empty() && (yaku_list & Yaku::Reach))
        return Status::NakiReach;
    else if (SyantenCalculator::calc(tehai, int(huro_blocks.size())) != -1)
        return Status::InvalidSyanten;

    return Status::Success;
}

/**
 * @brief 点数を計算する。
 * 
 * @param[in] tehai 手牌
 * @param[in] huro_blocks 副露ブロック
 * @param[in] agarihai 和了り牌
 * @param[in] yaku_list 手牌に関係ない役の一覧
 * @param[in] tumo 自摸和了りかどうか
 */
ScoreCalculator::Result ScoreCalculator::calc(const Tehai &tehai,
                                              const std::vector<HuroBlock> &huro_blocks,
                                              int agarihai, unsigned long long yaku_list)
{
    Result result;
    debug_print(tehai, huro_blocks, agarihai, yaku_list);

    // 副露牌を手牌に統合する。
    Tehai merged_tehai = merge_tehai(tehai, huro_blocks);

    // エラーを調べる。
    int status = check_error(tehai, huro_blocks, agarihai, yaku_list);
    if (status != Status::Success) {
        result.status = status;

        return result;
    }

    if (yaku_list & Yaku::NagasiMangan) {
        // 流し満貫
        result.status = Status::Success;
        result.yaku_list.emplace_back(Yaku::NagasiMangan, -1);
        result.score_name = ScoreName::Mangan;

        return result;
    }

    if (SyantenCalculator::calc_normal(tehai, int(huro_blocks.size())) == -1 &&
        !check_yakuman(tehai, huro_blocks, agarihai, yaku_list, SyantenType::Normal, result)) {
    } else if (SyantenCalculator::calc_tiitoi(tehai) == -1 &&
               !check_yakuman(tehai, huro_blocks, agarihai, yaku_list, SyantenType::Tiitoi,
                              result)) {
    } else {
        // 国士無双手
        check_yakuman(tehai, huro_blocks, agarihai, yaku_list, SyantenType::Kokushi, result);
    }

    result.status = Status::Success;

    return result;
}

/**
 * @brief 緑一色かどうかを判定する。
 */
bool ScoreCalculator::check_ryuiso(const Tehai &tehai, const std::vector<HuroBlock> &huro_blocks,
                                   int agarihai) const
{
    // 条件: 2, 3, 4, 6, 8, 發 以外の牌がないかどうか調べる
    //               | 9 | 8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 |
    int sozu_mask = 0b111'000'111'000'111'000'000'000'111;
    //                |Tyu|Hat|Hak|Pe |Sya|Nan|Ton|
    int zihai_mask = 0b111'000'111'111'111'111'111;

    return !(tehai.manzu || tehai.pinzu || tehai.sozu & sozu_mask || tehai.zihai & zihai_mask);
}

/**
 * @brief 大三元かどうかを判定する。
 */
bool ScoreCalculator::check_daisangen(const Tehai &tehai, const std::vector<HuroBlock> &huro_blocks,
                                      int agarihai) const
{
    // 白、發、中が3枚ずつあるかどうかを調べる。
    //          |Tyu|Hat|Hak|Pe |Sya|Nan|Ton|
    int mask = 0b011'011'011'000'000'000'000;

    return (tehai.zihai & Bit::SangenhaiMask) == mask;
}

/**
 * @brief 小四喜かどうかを判定する。
 */
bool ScoreCalculator::check_syosusi(const Tehai &tehai, const std::vector<HuroBlock> &huro_blocks,
                                    int agarihai) const
{
    // 風牌の合計が11枚
    int kazehai = tehai.zihai & Bit::KazehaiMask;
    return Bit::sum(kazehai) == 11;
}

/**
 * @brief 字一色かどうかを判定する。
 */
bool ScoreCalculator::check_tuiso(const Tehai &tehai, const std::vector<HuroBlock> &huro_blocks,
                                  int agarihai) const
{
    // 字牌以外の牌がない
    return !(tehai.manzu || tehai.pinzu || tehai.sozu);
}

/**
 * @brief 九連宝灯かどうかを判定する。
 */
bool ScoreCalculator::check_tyurenpoto(const Tehai &tehai,
                                       const std::vector<HuroBlock> &huro_blocks,
                                       int agarihai) const
{
    const auto &s_tbl = SyantenCalculator::s_tbl_;

    if (!huro_blocks.empty())
        return false; // 門前役

    int key;
    if (agarihai <= Tile::Manzu9)
        key = tehai.manzu;
    else if (agarihai <= Tile::Pinzu9)
        key = tehai.pinzu;
    else if (agarihai <= Tile::Sozu9)
        key = tehai.sozu;
    else
        return false; // 字牌

    int key19 = key & Bit::RotohaiMask;
    int key2345678 = key & Bit::TanyaoMask;

    return s_tbl[key19].n_ge3 == 2 && s_tbl[key2345678].n_ge1 == 7;
}

/**
 * @brief 四暗刻かどうかを判定する。
 */
bool ScoreCalculator::check_suanko(const Tehai &tehai, const std::vector<HuroBlock> &huro_blocks,
                                   int agarihai) const
{
    const auto &s_tbl = SyantenCalculator::s_tbl_;
    const auto &z_tbl = SyantenCalculator::z_tbl_;

    // 暗槓以外の副露ブロックがないかどうか
    for (const auto &block : huro_blocks)
        if (block.type != Huro::Ankan)
            return false;

    // 刻子が4つかどうか調べる。
    int n_ge4 = s_tbl[tehai.manzu].n_ge4 + s_tbl[tehai.pinzu].n_ge4 + s_tbl[tehai.sozu].n_ge4 +
                z_tbl[tehai.zihai].n_ge4;
    int n_ge3 = s_tbl[tehai.manzu].n_ge3 + s_tbl[tehai.pinzu].n_ge3 + s_tbl[tehai.sozu].n_ge3 +
                z_tbl[tehai.zihai].n_ge3;

    return n_ge3 - n_ge4 == 4;
}

/**
 * @brief 清老頭かどうかをチェックする。
 */
bool ScoreCalculator::check_tinroto(const Tehai &tehai, const std::vector<HuroBlock> &huro_blocks,
                                    int agarihai) const
{
    // 条件: 1, 9以外の牌がないかどうか調べる
    return !((tehai.manzu & Bit::TanyaoMask) || (tehai.pinzu & Bit::TanyaoMask) ||
             (tehai.sozu & Bit::TanyaoMask) || tehai.zihai);
}

/**
 * @brief 四槓子かどうかを判定する。
 */
bool ScoreCalculator::check_sukantu(const Tehai &tehai, const std::vector<HuroBlock> &huro_blocks,
                                    int agarihai) const
{
    // 条件: 副露ブロックに4つの槓子があるかどうかを調べる。
    int cnt = 0;
    for (const auto &block : huro_blocks)
        cnt += Huro::Ankan <= block.type; // enum の値で2以上が槓子であるため

    return cnt == 4;
}

/**
 * @brief 四暗刻単騎かどうかを判定する。
 */
bool ScoreCalculator::check_suanko_tanki(const Tehai &tehai,
                                         const std::vector<HuroBlock> &huro_blocks,
                                         int agarihai) const
{
    const auto &s_tbl = SyantenCalculator::s_tbl_;
    const auto &z_tbl = SyantenCalculator::z_tbl_;

    // 暗槓以外の副露ブロックがないかどうか
    for (const auto &block : huro_blocks)
        if (block.type != Huro::Ankan)
            return false;

    // 和了り牌の種類で雀頭が構成されているかどうかを調べる。
    int key;
    if (agarihai <= Tile::Manzu9)
        key = tehai.manzu;
    else if (agarihai <= Tile::Pinzu9)
        key = tehai.pinzu;
    else if (agarihai <= Tile::Sozu9)
        key = tehai.sozu;
    else
        key = tehai.zihai;

    if ((key & Bit::mask[agarihai]) != Bit::hai2[agarihai])
        return false;

    // 刻子が4つかどうか調べる。
    int n_ge4 = s_tbl[tehai.manzu].n_ge4 + s_tbl[tehai.pinzu].n_ge4 + s_tbl[tehai.sozu].n_ge4 +
                z_tbl[tehai.zihai].n_ge4;
    int n_ge3 = s_tbl[tehai.manzu].n_ge3 + s_tbl[tehai.pinzu].n_ge3 + s_tbl[tehai.sozu].n_ge3 +
                z_tbl[tehai.zihai].n_ge3;

    return n_ge3 - n_ge4 == 4;
}

/**
 * @brief 大四喜かどうかを判定する。
 */
bool ScoreCalculator::check_daisusi(const Tehai &tehai, const std::vector<HuroBlock> &huro_blocks,
                                    int agarihai) const
{
    // 風牌の合計が12枚
    int kazehai = tehai.zihai & Bit::KazehaiMask;
    return Bit::sum(kazehai) == 12;
}

/**
 * @brief 純正九連宝灯かどうかを判定する。
 */
bool ScoreCalculator::check_tyurenpoto9(const Tehai &tehai,
                                        const std::vector<HuroBlock> &huro_blocks,
                                        int agarihai) const
{
    if (!huro_blocks.empty())
        return false; // 門前役

    // 条件: 和了り牌を除いた場合に 1112345678999 となっているかどうかを調べる。
    //          | 9 | 8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 |
    int mask = 0b011'001'001'001'001'001'001'001'011;

    if (agarihai <= Tile::Manzu9)
        return tehai.manzu - Bit::hai1[agarihai] == mask;
    else if (agarihai <= Tile::Pinzu9)
        return tehai.pinzu - Bit::hai1[agarihai] == mask;
    else if (agarihai <= Tile::Sozu9)
        return tehai.sozu - Bit::hai1[agarihai] == mask;

    return false;
}

/**
 * @brief 国士無双13面待ちかどうかを判定する。
 */
bool ScoreCalculator::check_kokusi13(const Tehai &tehai, const std::vector<HuroBlock> &huro_blocks,
                                     int agarihai) const
{
    // 条件: 和了り牌を除いた場合に幺九牌がすべて1個ずつあるかどうかを調べる。
    int manzu = tehai.manzu;
    int pinzu = tehai.pinzu;
    int sozu = tehai.sozu;
    int zihai = tehai.zihai;

    if (agarihai <= Tile::Manzu9)
        manzu -= Bit::hai1[agarihai];
    else if (agarihai <= Tile::Pinzu9)
        pinzu -= Bit::hai1[agarihai];
    else if (agarihai <= Tile::Sozu9)
        sozu -= Bit::hai1[agarihai];
    else
        zihai -= Bit::hai1[agarihai];

    //             | 9 | 8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 |
    int mask_19 = 0b001'000'000'000'000'000'000'000'001;
    //                |Tyu|Hat|Hak|Pe |Sya|Nan|Ton|
    int mask_zihai = 0b001'001'001'001'001'001'001;

    return (manzu == mask_19) && (pinzu == mask_19) && (sozu == mask_19) && (zihai == mask_zihai);
}

/**
 * @brief 役満かどうかをチェックする。
 */
bool ScoreCalculator::check_yakuman(const Tehai &tehai, const std::vector<HuroBlock> &huro_blocks,
                                    int agarihai, unsigned long long yaku_list, int syanten_type,
                                    Result &result) const
{
    int n_yakuman = 0; // 役満の数

    // 通常手、七対子手、国士無双手共通
    if (yaku_list & Yaku::Tenho) { // 天和
        result.yaku_list.emplace_back(Yaku::Tenho, -1);
        n_yakuman++;
    } else if (yaku_list & Yaku::Tiho) { // 地和
        result.yaku_list.emplace_back(Yaku::Tiho, -1);
        n_yakuman++;
    } else if (yaku_list & Yaku::Renho) { // 人和
        result.yaku_list.emplace_back(Yaku::Renho, -1);
        n_yakuman++;
    }

    if (syanten_type == SyantenType::Normal) {
        // 通常手
        if (check_tyurenpoto9(tehai, huro_blocks, agarihai)) {
            result.yaku_list.emplace_back(Yaku::Tyurenpoto9, -1);
            n_yakuman += 2;
        } else if (check_tyurenpoto(tehai, huro_blocks, agarihai)) {
            result.yaku_list.emplace_back(Yaku::Tyurenpoto, -1);
            n_yakuman += 1;
        } else if (check_sukantu(tehai, huro_blocks, agarihai)) {
            result.yaku_list.emplace_back(Yaku::Sukantu, -1);
            n_yakuman += 1;
        } else if (check_suanko_tanki(tehai, huro_blocks, agarihai)) {
            result.yaku_list.emplace_back(Yaku::SuankoTanki, -1);
            n_yakuman += 2;
        } else if (check_suanko(tehai, huro_blocks, agarihai)) {
            result.yaku_list.emplace_back(Yaku::Suanko, -1);
            n_yakuman += 1;
        } else if (check_ryuiso(tehai, huro_blocks, agarihai)) {
            result.yaku_list.emplace_back(Yaku::Ryuiso, -1);
            n_yakuman += 1;
        } else if (check_daisangen(tehai, huro_blocks, agarihai)) {
            result.yaku_list.emplace_back(Yaku::Daisangen, -1);
            n_yakuman += 1;
        } else if (check_tinroto(tehai, huro_blocks, agarihai)) {
            result.yaku_list.emplace_back(Yaku::Tinroto, -1);
            n_yakuman += 1;
        } else if (check_daisusi(tehai, huro_blocks, agarihai)) {
            result.yaku_list.emplace_back(Yaku::Daisusi, -1);
            n_yakuman += 2;
        } else if (check_syosusi(tehai, huro_blocks, agarihai)) {
            result.yaku_list.emplace_back(Yaku::Syosusi, -1);
            n_yakuman += 1;
        } else if (check_tuiso(tehai, huro_blocks, agarihai)) {
            result.yaku_list.emplace_back(Yaku::Tuiso, -1);
            n_yakuman += 1;
        }
    } else if (syanten_type == SyantenType::Tiitoi) {
        // 七対子手
        if (check_tuiso(tehai, huro_blocks, agarihai)) {
            result.yaku_list.emplace_back(Yaku::Tuiso, -1);
            n_yakuman += 1;
        }
    } else {
        // 国士無双手
        if (check_kokusi13(tehai, huro_blocks, agarihai)) {
            result.yaku_list.emplace_back(Yaku::Kokusimuso13, -1);
            n_yakuman += 2;
        } else {
            result.yaku_list.emplace_back(Yaku::Kokusimuso, -1);
            n_yakuman += 1;
        }
    }

    if (n_yakuman > 0)
        result.score_name = ScoreName::Yakuman + n_yakuman - 1;

    return n_yakuman > 0;
}

} // namespace mahjong
