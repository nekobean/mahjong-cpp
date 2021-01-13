#ifndef MAHJONG_CPP_HAND
#define MAHJONG_CPP_HAND

#include <bitset>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include <spdlog/spdlog.h>

#include "bitutils.hpp"
#include "meld.hpp"
#include "tile.hpp"

namespace mahjong {

/**
 * @brief 手牌
 */
class Hand {
public:
    using key_type = unsigned int;

    Hand();
    Hand(const std::vector<int> &tiles);
    Hand(const std::vector<int> &tiles, const std::vector<MeldedBlock> &melds);

    bool is_menzen() const;
    bool is_melded() const;
    bool contains(int tile) const;
    int num_tiles(int hai) const;
    int num_tiles() const;
    std::string to_string() const;

private:
    void convert_from_hai34(const std::vector<int> &tiles);
    bool check_arguments(const std::vector<int> &tiles,
                         const std::vector<MeldedBlock> &melds);

    friend std::ostream &operator<<(std::ostream &os, const Hand &hand);

public:
    /*! ビット列にした手牌
     *  例: [0, 2, 0, 2, 2, 1, 1, 1, 4] -> 69510160 (00|000|100|001|001|001|010|010|000|010|000)
     *                                                      牌9 牌8 牌7 牌6 牌5 牌4 牌3 牌2 牌1
     */

    /**
     * 00|000|000|000|000|000|000|000|000|000|000
     *        萬9 萬8 萬7 萬6 萬5 萬4 萬3 萬2 萬1
     */
    key_type manzu;
    /**
     * 00|000|000|000|000|000|000|000|000|000|000
     *        筒9 筒8 筒7 筒6 筒5 筒4 筒3 筒2 筒1
     */
    key_type pinzu;
    /**
     * 00|000|000|000|000|000|000|000|000|000|000
     *        索9 索8 索7 索6 索5 索4 索3 索2 索1
     */
    key_type sozu;
    /**
     * 00|000|000|000|000|000|000|000|000|000|000
     *                中  發  白  北  西  南  東
     */
    key_type zihai;

    /* 赤の萬子5を持ってるかどうか */
    bool aka_manzu5;

    /* 赤の筒子5を持ってるかどうか */
    bool aka_pinzu5;

    /* 赤の索子5を持ってるかどうか */
    bool aka_sozu5;

    /* 副露ブロック */
    std::vector<MeldedBlock> melds;
};

/**
 * @brief 手牌を作成する。
 */
inline Hand::Hand()
    : manzu(0)
    , pinzu(0)
    , sozu(0)
    , zihai(0)
    , aka_manzu5(false)
    , aka_pinzu5(false)
    , aka_sozu5(false)
{
}

/**
 * @brief 手牌を作成する。
 * 
 * @param[in] tiles 牌の一覧
 */
inline Hand::Hand(const std::vector<int> &tiles)
{
#ifdef CHECK_ARGUMENT
    if (!check_arguments(tiles, melds))
        return;
#endif

    convert_from_hai34(tiles);
}

/**
 * @brief 手牌を作成する。
 * 
 * @param[in] tiles 牌の一覧
 * @param[in] melds 副露ブロックの一覧
 */
inline Hand::Hand(const std::vector<int> &tiles, const std::vector<MeldedBlock> &melds)
    : melds(melds)
{
#ifdef CHECK_ARGUMENT
    if (!check_arguments(tiles, melds))
        return;
#endif

    convert_from_hai34(tiles);
}

/**
 * @brief 引数が問題ないかどうかを調べる。
 * 
 * @param[in] tiles 牌の一覧
 * @param melds 副露ブロックの一覧
 * @return 引数が問題ない場合は true、そうでない場合は false を返す。
 */
inline bool Hand::check_arguments(const std::vector<int> &tiles,
                                  const std::vector<MeldedBlock> &melds)
{
    // 牌ごとの枚数及び合計枚数を数える。
    int n_tiles = 0;
    std::unordered_map<int, int> count;

    for (auto hai : tiles) {
        count[hai]++;
        n_tiles++;
    }

    n_tiles += int(melds.size()) * 3;
    // for (const auto &block : melds) {
    //     if (block.type & (BlockType::Kotu | BlockType::Kantu)) {
    //         count[block.min_tile] += 3;
    //     }
    //     else if (block.type & BlockType::Syuntu) {
    //         count[block.min_tile]++;
    //         count[block.min_tile + 1]++;
    //         count[block.min_tile + 2]++;
    //     }
    // }

    for (const auto &[hai, n] : count) {
        if (n > 4)
            return false; // 5枚以上の牌がある場合
    }

    return n_tiles == 13 || n_tiles == 14; // 少牌または多牌かどうか
}

/**
 * @brief 牌の一覧からビット列を作成する。
 * 
 * @param[in] tiles 牌の一覧
 */
inline void Hand::convert_from_hai34(const std::vector<int> &tiles)
{
    manzu = pinzu = sozu = zihai = 0;
    aka_manzu5 = aka_pinzu5 = aka_sozu5 = false;

    for (auto tile : tiles) {
        aka_manzu5 |= tile == Tile::AkaManzu5;
        aka_pinzu5 |= tile == Tile::AkaPinzu5;
        aka_sozu5 |= tile == Tile::AkaSozu5;

        if (tile <= Tile::Manzu9 || tile == Tile::AkaManzu5)
            manzu += Bit::tile1[tile];
        else if (tile <= Tile::Pinzu9 || tile == Tile::AkaPinzu5)
            pinzu += Bit::tile1[tile];
        else if (tile <= Tile::Sozu9 || tile == Tile::AkaSozu5)
            sozu += Bit::tile1[tile];
        else
            zihai += Bit::tile1[tile];
    }
}

/**
 * @brief 門前かどうかを取得する。
 * 
 * @return 門前の場合は true、そうでない場合は false を返す。
 */
inline bool Hand::is_menzen() const
{
    for (const auto &block : melds) {
        if (block.type != MeldType::Ankan)
            return false; // 暗槓以外の副露ブロックがある場合
    }

    return true;
}

/**
 * @brief 副露しているかどうかを取得する。
 * 
 * @return 副露している場合は true、そうでない場合は false を返す。
 */
inline bool Hand::is_melded() const
{
    return !melds.empty();
}

/**
 * @brief 指定した牌が手牌に含まれるかどうかを調べる。赤牌は通常の牌は区別しません。
 * 
 * @param[in] tile 牌
 * @return 指定した牌が手牌に含まれる場合は true、そうでない場合は false を返す。
 */
inline bool Hand::contains(int tile) const
{
    if (tile <= Tile::Manzu9 || tile == Tile::AkaManzu5)
        return manzu & Bit::mask[tile];
    else if (tile <= Tile::Pinzu9 || tile == Tile::AkaPinzu5)
        return pinzu & Bit::mask[tile];
    else if (tile <= Tile::Sozu9 || tile == Tile::AkaSozu5)
        return sozu & Bit::mask[tile];
    else
        return zihai & Bit::mask[tile];
}

/**
 * @brief 手牌にある指定した牌の枚数を取得する。赤牌は通常の牌は区別しません。
 * 
 * @param[in] tile 牌
 * @return int 牌の枚数
 */
inline int Hand::num_tiles(int tile) const
{
    tile = aka2normal(tile);

    if (tile <= Tile::Manzu9)
        return Bit::get_n_tile(manzu, tile);
    else if (tile <= Tile::Pinzu9)
        return Bit::get_n_tile(pinzu, tile - 9);
    else if (tile <= Tile::Sozu9)
        return Bit::get_n_tile(sozu, tile - 18);
    else
        return Bit::get_n_tile(zihai, tile - 27);
}

/**
 * @brief 手牌にある牌の合計枚数を取得する。
 * 
 * @return int 牌の合計枚数
 */
inline int Hand::num_tiles() const
{
    return Bit::sum(manzu) + Bit::sum(pinzu) + Bit::sum(sozu) + Bit::sum(zihai);
}

/**
 * @brief 手牌を表す MPS 表記の文字列を取得する。
 *        例: 1112r56789m 1122p
 * 
 * @return std::string 文字列
 */
inline std::string Hand::to_string() const
{
    std::string s;

    // 萬子
    for (int i = 0; i < 9; ++i) {
        int n = Bit::get_n_tile(manzu, i);

        s += aka_manzu5 && i == 4 ? "r" : "";
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

        s += aka_pinzu5 && i == 4 ? "r" : "";
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

        s += aka_sozu5 && i == 4 ? "r" : "";
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
            s += Tile::Name.at(27 + i);
    }

    // 副露ブロック
    if (!s.empty() && !melds.empty())
        s += " ";
    for (const auto &block : melds)
        s += block.to_string();

    return s;
}

inline std::ostream &operator<<(std::ostream &os, const Hand &hand)
{
    os << hand.to_string();

    return os;
}

} // namespace mahjong

#endif /* MAHJONG_CPP_HAND */
