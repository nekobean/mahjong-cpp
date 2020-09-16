#include "tehai.hpp"

#include <iostream>

namespace mahjong
{

/**
 * @brief 牌の名前 (漢字)
 */
const std::vector<std::string> Tile::KANJI_NAMES = {
    "一萬", "二萬", "三萬", "四萬", "五萬", "六萬", "七萬", "八萬", "九萬", // 萬子
    "一筒", "二筒", "三筒", "四筒", "五筒", "六筒", "七筒", "八筒", "九筒", // 筒子
    "一索", "二索", "三索", "四索", "五索", "六索", "七索", "八索", "九索", // 索子
    "東",   "南",   "西",   "北",   "白",   "發",   "中",                   // 字牌
};

/**
 * @brief すべての牌
 */
const std::vector<int> Tile::All = {
    Tile::Manzu1, Tile::Manzu2, Tile::Manzu3, Tile::Manzu4, Tile::Manzu5, Tile::Manzu6,
    Tile::Manzu7, Tile::Manzu8, Tile::Manzu9, Tile::Pinzu1, Tile::Pinzu2, Tile::Pinzu3,
    Tile::Pinzu4, Tile::Pinzu5, Tile::Pinzu6, Tile::Pinzu7, Tile::Pinzu8, Tile::Pinzu9,
    Tile::Souzu1, Tile::Souzu2, Tile::Souzu3, Tile::Souzu4, Tile::Souzu5, Tile::Souzu6,
    Tile::Souzu7, Tile::Souzu8, Tile::Souzu9, Tile::Ton,    Tile::Nan,    Tile::Sya,
    Tile::Pei,    Tile::Haku,   Tile::Hatsu,  Tile::Tyun};

/**
 * @brief 萬子
 */
const std::vector<int> Tile::Pinzu = {Tile::Manzu1, Tile::Manzu2, Tile::Manzu3,
                                      Tile::Manzu4, Tile::Manzu5, Tile::Manzu6,
                                      Tile::Manzu7, Tile::Manzu8, Tile::Manzu9};

/**
 * @brief 筒子
 */
const std::vector<int> Tile::Souzu = {Tile::Pinzu1, Tile::Pinzu2, Tile::Pinzu3,
                                      Tile::Pinzu4, Tile::Pinzu5, Tile::Pinzu6,
                                      Tile::Pinzu7, Tile::Pinzu8, Tile::Pinzu9};

/**
 * @brief 索子
 */
const std::vector<int> Tile::Manzu = {Tile::Souzu1, Tile::Souzu2, Tile::Souzu3,
                                      Tile::Souzu4, Tile::Souzu5, Tile::Souzu6,
                                      Tile::Souzu7, Tile::Souzu8, Tile::Souzu9};

/**
 * @brief 老頭牌
 */
const std::vector<int> Tile::Routouhai = {Tile::Manzu1, Tile::Pinzu1, Tile::Souzu1,
                                          Tile::Manzu9, Tile::Pinzu9, Tile::Souzu9};

/**
 * @brief 字牌
 */
const std::vector<int> Tile::Zihai{
    Tile::Ton, Tile::Nan, Tile::Sya, Tile::Pei, Tile::Haku, Tile::Hatsu, Tile::Tyun,
};

/**
 * @brief 幺九牌
 */
const std::vector<int> Tile::Yaochuhai = {
    Tile::Manzu1, Tile::Pinzu1, Tile::Souzu1, Tile::Manzu9, Tile::Pinzu9, Tile::Souzu9, Tile::Ton,
    Tile::Nan,    Tile::Sya,    Tile::Pei,    Tile::Haku,   Tile::Hatsu,  Tile::Tyun,
};

////////////////////////////////////////////////////////////////////////////////

/**
 * @brief 手牌オブジェクトを作成する。
 */
Tehai::Tehai() : manzu(0), pinzu(0), souzu(0), zihai(0)
{
}

/**
 * @brief 手牌オブジェクトを作成する。
 * 
 * @param[in] tehai 手牌の一覧
 */
Tehai::Tehai(const std::vector<int> &tehai) : manzu(0), pinzu(0), souzu(0), zihai(0)
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
        if (Tile::Manzu1 <= hai && hai <= Tile::Manzu9)
            manzu += Bit::hai1[hai];
        if (Tile::Pinzu1 <= hai && hai <= Tile::Pinzu9)
            pinzu += Bit::hai1[hai - 9];
        if (Tile::Souzu1 <= hai && hai <= Tile::Souzu9)
            souzu += Bit::hai1[hai - 18];
        if (Tile::Ton <= hai && hai <= Tile::Tyun)
            zihai += Bit::hai1[hai - 27];
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
            s += Tile::KANJI_NAMES[i];
    }

    // 筒子
    if (!s.empty() && pinzu)
        s += " ";
    for (int i = 0; i < 9; ++i) {
        int n = Bit::get_n_tile(pinzu, i);
        for (int j = 0; j < n; ++j)
            s += Tile::KANJI_NAMES[9 + i];
    }

    // 索子
    if (!s.empty() && souzu)
        s += " ";
    for (int i = 0; i < 9; ++i) {
        int n = Bit::get_n_tile(souzu, i);
        for (int j = 0; j < n; ++j)
            s += Tile::KANJI_NAMES[18 + i];
    }

    // 字牌
    if (!s.empty() && zihai)
        s += " ";
    for (int i = 0; i < 9; ++i) {
        int n = Bit::get_n_tile(zihai, i);
        for (int j = 0; j < n; ++j)
            s += Tile::KANJI_NAMES[27 + i];
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
    if (!s.empty() && souzu)
        s += " ";
    for (int i = 0; i < 9; ++i) {
        int n = Bit::get_n_tile(souzu, i);
        for (int j = 0; j < n; ++j)
            s += std::to_string(i + 1);
    }
    if (souzu)
        s += "s";

    // 字牌
    if (!s.empty() && zihai)
        s += " ";
    for (int i = 0; i < 9; ++i) {
        int n = Bit::get_n_tile(zihai, i);
        for (int j = 0; j < n; ++j)
            s += Tile::KANJI_NAMES[27 + i];
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
    std::cout << "manzu: " << manzu << ", ";
    std::cout << "pinzu: " << pinzu << ", ";
    std::cout << "souzu: " << souzu << ", ";
    std::cout << "zihai: " << zihai << std::endl;
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
        std::cout << Bit::get_n_tile(souzu, i) << (i != 8 ? ", " : "|");

    // 字牌
    for (int i = 0; i < 7; ++i)
        std::cout << Bit::get_n_tile(zihai, i) << (i != 6 ? ", " : "");
    std::cout << std::endl;
}

} // namespace mahjong
