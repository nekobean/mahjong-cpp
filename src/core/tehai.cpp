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

/**
 * @brief マスク
 */
const std::vector<int> Tile::mask = {
    7, 7 << 3, 7 << 6, 7 << 9, 7 << 12, 7 << 15, 7 << 18, 7 << 21, 7 << 24,
};

/**
 * @brief 1枚の牌
 */
const std::vector<int> Tile::hai1 = {
    1, 1 << 3, 1 << 6, 1 << 9, 1 << 12, 1 << 15, 1 << 18, 1 << 21, 1 << 24,
};

/**
 * @brief 2枚の牌
 */
const std::vector<int> Tile::hai2 = {
    2, 2 << 3, 2 << 6, 2 << 9, 2 << 12, 2 << 15, 2 << 18, 2 << 21, 2 << 24,
};

/**
 * @brief 3枚の牌
 */
const std::vector<int> Tile::hai3 = {
    3, 3 << 3, 3 << 6, 3 << 9, 3 << 12, 3 << 15, 3 << 18, 3 << 21, 3 << 24,
};

/**
 * @brief 4枚の牌
 */
const std::vector<int> Tile::hai4 = {
    4, 4 << 3, 4 << 6, 4 << 9, 4 << 12, 4 << 15, 4 << 18, 4 << 21, 4 << 24,
};

/**
 * @brief 2枚以上かどうかを調べるときに使うマスク
 */
const std::vector<int> Tile::ge2 = {
    6, 6 << 3, 6 << 6, 6 << 9, 6 << 12, 6 << 15, 6 << 18, 6 << 21, 6 << 24,
};

////////////////////////////////////////////////////////////////////////////////

/**
 * @brief 手牌オブジェクトを作成する。
 */
Tehai::Tehai() : tiles(Tile::Length), manzu(0), pinzu(0), souzu(0), zihai(0)
{
}

/**
 * @brief 手牌オブジェクトを作成する。
 * 
 * @param[in] tehai 手牌の一覧
 */
Tehai::Tehai(const std::vector<int> &tehai)
    : tiles(Tile::Length), manzu(0), pinzu(0), souzu(0), zihai(0)
{
    for (auto hai : tehai)
        tiles[hai]++;

#ifdef CHECK_ARGUMENT
    for (int i = 0; i < Tile::Length; ++i) {
        if (tiles[i] < 0 || tiles[i] > 4)
            return;
    }

    if (tehai.empty() || tehai.size() > 14)
        return; // 手牌の最小枚数は裸単騎の1枚、最大枚数は打牌前の14枚
#endif

    // ビット列にする
    for (auto hai : tehai) {
        if (Tile::Manzu1 <= hai && hai <= Tile::Manzu9)
            manzu += Tile::hai1[hai];
        if (Tile::Pinzu1 <= hai && hai <= Tile::Pinzu9)
            pinzu += Tile::hai1[hai - 9];
        if (Tile::Souzu1 <= hai && hai <= Tile::Souzu9)
            souzu += Tile::hai1[hai - 18];
        if (Tile::Ton <= hai && hai <= Tile::Tyun)
            zihai += Tile::hai1[hai - 27];
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
        int n = (manzu & Tile::mask[i]) >> 3 * i;
        for (int j = 0; j < n; ++j)
            s += Tile::KANJI_NAMES[i];
    }

    // 筒子
    if (!s.empty() && pinzu)
        s += " ";
    for (int i = 0; i < 9; ++i) {
        int n = (pinzu & Tile::mask[i]) >> 3 * i;
        for (int j = 0; j < n; ++j)
            s += Tile::KANJI_NAMES[9 + i];
    }

    // 索子
    if (!s.empty() && souzu)
        s += " ";
    for (int i = 0; i < 9; ++i) {
        int n = (souzu & Tile::mask[i]) >> 3 * i;
        for (int j = 0; j < n; ++j)
            s += Tile::KANJI_NAMES[18 + i];
    }

    // 字牌
    if (!s.empty() && zihai)
        s += " ";
    for (int i = 0; i < 9; ++i) {
        int n = (zihai & Tile::mask[i]) >> 3 * i;
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
        int n = (manzu & Tile::mask[i]) >> 3 * i;
        for (int j = 0; j < n; ++j)
            s += std::to_string(i + 1);
    }
    if (manzu)
        s += "m";

    // 筒子
    if (!s.empty() && pinzu)
        s += " ";
    for (int i = 0; i < 9; ++i) {
        int n = (pinzu & Tile::mask[i]) >> 3 * i;
        for (int j = 0; j < n; ++j)
            s += std::to_string(i + 1);
    }
    if (pinzu)
        s += "p";

    // 索子
    if (!s.empty() && souzu)
        s += " ";
    for (int i = 0; i < 9; ++i) {
        int n = (souzu & Tile::mask[i]) >> 3 * i;
        for (int j = 0; j < n; ++j)
            s += std::to_string(i + 1);
    }
    if (souzu)
        s += "s";

    // 字牌
    if (!s.empty() && zihai)
        s += " ";
    for (int i = 0; i < 9; ++i) {
        int n = (zihai & Tile::mask[i]) >> 3 * i;
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
    for (size_t i = 0; i < 9; ++i)
        std::cout << tiles[i] << (i != 8 ? ", " : "|");
    for (size_t i = 9; i < 18; ++i)
        std::cout << tiles[i] << (i != 17 ? ", " : "|");
    for (size_t i = 18; i < 27; ++i)
        std::cout << tiles[i] << (i != 26 ? ", " : "|");
    for (size_t i = 27; i < 34; ++i)
        std::cout << tiles[i] << (i != 33 ? ", " : "");
    std::cout << std::endl;
}

} // namespace mahjong
