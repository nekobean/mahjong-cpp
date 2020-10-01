#include "types.hpp"

#include <bitset>
#include <iostream>

#include <spdlog/spdlog.h>

#include "bitutils.hpp"

namespace mahjong
{

/**
 * @brief 牌の名前
 */
const std::vector<std::string> Tile::Names = {
    "一萬", "二萬", "三萬", "四萬", "五萬", "六萬", "七萬", "八萬", "九萬", // 萬子
    "一筒", "二筒", "三筒", "四筒", "五筒", "六筒", "七筒", "八筒", "九筒", // 筒子
    "一索", "二索", "三索", "四索", "五索", "六索", "七索", "八索", "九索", // 索子
    "東",   "南",   "西",   "北",   "白",   "發",   "中",                   // 字牌
};

/**
 * @brief 副露の名前
 */
const std::vector<std::string> Huro::Names = {
    "ポン", "チー", "暗槓", "明槓", "加槓",
};

/**
 * @brief 役の名前
 */
const std::map<uint64_t, std::string> Yaku::Names = {
    {Yaku::Null, "役なし"},
    {Yaku::Tumo, "門前清自摸和"},
    {Yaku::Reach, "立直"},
    {Yaku::Ippatu, "一発"},
    {Yaku::Tanyao, "断幺九"},
    {Yaku::Pinhu, "平和"},
    {Yaku::Ipeko, "一盃口"},
    {Yaku::Tyankan, "槍槓"},
    {Yaku::Rinsyankaiho, "嶺上開花"},
    {Yaku::Haiteitumo, "海底摸月"},
    {Yaku::Hoteiron, "河底撈魚"},
    {Yaku::Dora, "ドラ"},
    {Yaku::UraDora, "裏ドラ"},
    {Yaku::AkaDora, "赤ドラ"},
    {Yaku::SangenhaiHaku, "三元牌"},
    {Yaku::SangenhaiHatu, "三元牌"},
    {Yaku::SangenhaiTyun, "三元牌"},
    {Yaku::ZikazeTon, "自風"},
    {Yaku::ZikazeNan, "自風"},
    {Yaku::ZikazeSya, "自風"},
    {Yaku::ZikazePe, "自風"},
    {Yaku::BakazeTon, "場風"},
    {Yaku::BakazeNan, "場風"},
    {Yaku::BakazeSya, "場風"},
    {Yaku::BakazePe, "場風"},
    {Yaku::WReach, "ダブル立直"},
    {Yaku::Tiitoitu, "七対子"},
    {Yaku::Toitoiho, "対々和"},
    {Yaku::Sananko, "三暗刻"},
    {Yaku::SansyokuDoko, "三色同刻"},
    {Yaku::SansyokuDozyun, "三色同順"},
    {Yaku::Honroto, "混老頭"},
    {Yaku::IkkiTukan, "一気通貫"},
    {Yaku::Tyanta, "混全帯幺"},
    {Yaku::Syosangen, "小三元"},
    {Yaku::Sankantu, "三槓子"},
    {Yaku::Honiso, "混一色"},
    {Yaku::Zyuntyanta, "混全帯么九"},
    {Yaku::Ryanpeko, "二盃口"},
    {Yaku::NagasiMangan, "流し満貫"},
    {Yaku::Tiniso, "清一色"},
    {Yaku::Tenho, "天和"},
    {Yaku::Tiho, "地和"},
    {Yaku::Renho, "人和"},
    {Yaku::Ryuiso, "緑一色"},
    {Yaku::Daisangen, "大三元"},
    {Yaku::Syosusi, "小四喜"},
    {Yaku::Tuiso, "字一色"},
    {Yaku::Kokusimuso, "国士無双"},
    {Yaku::Tyurenpoto, "九連宝燈"},
    {Yaku::Suanko, "四暗刻"},
    {Yaku::Tinroto, "清老頭"},
    {Yaku::Sukantu, "四槓子"},
    {Yaku::Daisyarin, "大車輪"},
    {Yaku::SuankoTanki, "四暗刻単騎"},
    {Yaku::Daisusi, "大四喜"},
    {Yaku::Tyurenpoto9, "純正九連宝燈"},
    {Yaku::Kokusimuso, "国士無双13面待ち"},
};

/**
 * @brief 手牌オブジェクトを作成する。
 */
Tehai::Tehai() : manzu(0), pinzu(0), sozu(0), zihai(0)
{
}

/**
 * @brief 手牌オブジェクトを作成する。
 * 
 * @param[in] tehai 手牌の一覧
 */
Tehai::Tehai(const std::vector<int> &tehai) : manzu(0), pinzu(0), sozu(0), zihai(0)
{
#ifdef CHECK_ARGUMENT
    std::map<int, int> tiles;
    int n_tiles = 0;
    for (auto hai : tehai) {
        tiles[hai]++;
        n_tiles++;
    }

    for (const auto &[hai, n] : tiles) {
        if (n < 0 || n > 4)
            return;
    }

    if (n_tiles < 1 || n_tiles > 14)
        return; // 手牌の最小枚数は裸単騎の1枚、最大枚数は打牌前の14枚
#endif

    // ビット列にする
    for (auto hai : tehai) {
        if (hai <= Tile::Manzu9)
            manzu += Bit::hai1[hai];
        else if (hai <= Tile::Pinzu9)
            pinzu += Bit::hai1[hai];
        else if (hai <= Tile::Sozu9)
            sozu += Bit::hai1[hai];
        else
            zihai += Bit::hai1[hai];
    }
}

std::ostream &operator<<(std::ostream &os, const Tehai &tehai)
{
    os << tehai.to_string();

    return os;
}

/**
 * @brief 手牌を表す漢字表記の文字列を取得する。
 *        例: 一萬一萬一萬二萬五萬六萬七萬八萬九萬一筒一筒二筒二筒
 * 
 * @return std::string 文字列
 */
std::string Tehai::to_kanji_string() const
{
    std::string s;

    // 萬子
    for (int i = 0; i < 9; ++i) {
        int n = Bit::get_n_tile(manzu, i);
        for (int j = 0; j < n; ++j)
            s += Tile::Names[i];
    }

    // 筒子
    if (!s.empty() && pinzu)
        s += " ";
    for (int i = 0; i < 9; ++i) {
        int n = Bit::get_n_tile(pinzu, i);
        for (int j = 0; j < n; ++j)
            s += Tile::Names[9 + i];
    }

    // 索子
    if (!s.empty() && sozu)
        s += " ";
    for (int i = 0; i < 9; ++i) {
        int n = Bit::get_n_tile(sozu, i);
        for (int j = 0; j < n; ++j)
            s += Tile::Names[18 + i];
    }

    // 字牌
    if (!s.empty() && zihai)
        s += " ";
    for (int i = 0; i < 9; ++i) {
        int n = Bit::get_n_tile(zihai, i);
        for (int j = 0; j < n; ++j)
            s += Tile::Names[27 + i];
    }

    return s;
}

/**
 * @brief 手牌を表す MPS 表記の文字列を取得する。
 *        例: 111256789m 1122p
 * 
 * @return std::string 文字列
 */
std::string Tehai::to_mps_string() const
{
    std::string s;

    // 萬子
    for (int i = 0; i < 9; ++i) {
        int n = Bit::get_n_tile(manzu, i);
        for (int j = 0; j < n; ++j)
            s += std::to_string(i + 1);
    }
    if (manzu)
        s += "m";

    // 筒子
    if (!s.empty() && pinzu)
        s += " ";
    for (int i = 0; i < 9; ++i) {
        int n = Bit::get_n_tile(pinzu, i);
        for (int j = 0; j < n; ++j)
            s += std::to_string(i + 1);
    }
    if (pinzu)
        s += "p";

    // 索子
    if (!s.empty() && sozu)
        s += " ";
    for (int i = 0; i < 9; ++i) {
        int n = Bit::get_n_tile(sozu, i);
        for (int j = 0; j < n; ++j)
            s += std::to_string(i + 1);
    }
    if (sozu)
        s += "s";

    // 字牌
    if (!s.empty() && zihai)
        s += " ";
    for (int i = 0; i < 9; ++i) {
        int n = Bit::get_n_tile(zihai, i);
        for (int j = 0; j < n; ++j)
            s += Tile::Names[27 + i];
    }

    return s;
}

/**
 * @brief 手牌を表す文字列を取得する。
 * 
 * @return std::string 文字列
 */
std::string Tehai::to_string(StringFormat format) const
{
    switch (format) {
    case StringFormat::Kanji:
        return to_kanji_string();
    case StringFormat::MPS:
        return to_mps_string();
    }

    return "";
}

void Tehai::print_bit() const
{
    // デバッグ用
    std::cout << *this << std::endl;
    std::cout << "manzu: " << std::bitset<27>(manzu) << ", ";
    std::cout << "pinzu: " << std::bitset<27>(pinzu) << ", ";
    std::cout << "sozu: " << std::bitset<27>(sozu) << ", ";
    std::cout << "zihai: " << std::bitset<21>(zihai) << std::endl;
}

void Tehai::print_num_tiles() const
{
    // デバッグ用
    std::cout << *this << std::endl;

    // 萬子
    for (int i = 0; i < 9; ++i)
        std::cout << Bit::get_n_tile(manzu, i) << (i != 8 ? ", " : "|");

    // 筒子
    for (int i = 0; i < 9; ++i)
        std::cout << Bit::get_n_tile(pinzu, i) << (i != 8 ? ", " : "|");

    // 索子
    for (int i = 0; i < 9; ++i)
        std::cout << Bit::get_n_tile(sozu, i) << (i != 8 ? ", " : "|");

    // 字牌
    for (int i = 0; i < 7; ++i)
        std::cout << Bit::get_n_tile(zihai, i) << (i != 6 ? ", " : "");
    std::cout << std::endl;
}

const std::vector<std::string> ScoreName::Names = {
    "満貫", "跳満",       "倍満",         "三倍満",  "数え役満",            // 通常役
    "役満", "ダブル役満", "トリプル役満", "4倍役満", "5倍役満",  "6倍役満", // 役満
};

} // namespace mahjong