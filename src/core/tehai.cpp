#include "tehai.hpp"

#include <iostream>

namespace mahjong
{

////////////////////////////////////////////////////////////////////////////////

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
    tiles.resize(34);
    for (auto hai : tehai)
        tiles[hai]++;

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
        if (Tile::Sozu1 <= hai && hai <= Tile::Sozu9)
            sozu += Bit::hai1[hai - 18];
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
    std::cout << "manzu: " << manzu << ", ";
    std::cout << "pinzu: " << pinzu << ", ";
    std::cout << "sozu: " << sozu << ", ";
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
        std::cout << Bit::get_n_tile(sozu, i) << (i != 8 ? ", " : "|");

    // 字牌
    for (int i = 0; i < 7; ++i)
        std::cout << Bit::get_n_tile(zihai, i) << (i != 6 ? ", " : "");
    std::cout << std::endl;
}

} // namespace mahjong
