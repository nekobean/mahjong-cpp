#ifndef MAHJONG_CPP_HAND
#define MAHJONG_CPP_HAND

#include <algorithm>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

#include <spdlog/spdlog.h>

#include "meld.hpp"
#include "tile.hpp"

namespace mahjong
{

/**
 * @brief 手牌
 */
class Hand : private boost::equality_comparable<Hand, Hand>
{
  public:
    Hand();
    Hand(const std::vector<int> &tiles);
    Hand(const std::vector<int> &tiles, const std::vector<MeldedBlock> &melds);

    bool is_closed() const;
    int num_tiles() const;
    static Hand from_array34(const std::vector<int> &array34);
    static Hand from_mpsz(const std::string &mpsz_str);
    std::string to_string() const;

  private:
    bool check_arguments(const std::vector<int> &tiles,
                         const std::vector<MeldedBlock> &melds);
    friend std::ostream &operator<<(std::ostream &os, const Hand &hand);
    friend bool operator==(const Hand &a, const Hand &b);

  public:
    /* 牌数 */
    std::vector<int> counts;

    /* 副露ブロック */
    std::vector<MeldedBlock> melds;

    /* 赤の萬子5を持ってるかどうか */
    bool aka_manzu5;

    /* 赤の筒子5を持ってるかどうか */
    bool aka_pinzu5;

    /* 赤の索子5を持ってるかどうか */
    bool aka_souzu5;

    int32_t manzu;
    int32_t pinzu;
    int32_t souzu;
    int32_t honors;

    static const int TILE_TYPES = 34;
};

/**
 * @brief 手牌を作成する。
 */
inline Hand::Hand()
    : counts(TILE_TYPES, 0), aka_manzu5(false), aka_pinzu5(false), aka_souzu5(false)
{
}

/**
 * @brief 手牌を作成する。
 *
 * @param[in] tiles 牌の一覧
 */
inline Hand::Hand(const std::vector<int> &tiles)
    : counts(TILE_TYPES, 0), aka_manzu5(false), aka_pinzu5(false), aka_souzu5(false)
{
#ifdef CHECK_ARGUMENT
    if (!check_arguments(tiles, melds))
        return;
#endif

    for (int tile : tiles) {
        if (tile == Tile::AkaManzu5) {
            aka_manzu5 = true;
            counts[Tile::Manzu5]++;
        }
        else if (tile == Tile::AkaPinzu5) {
            aka_pinzu5 = true;
            counts[Tile::Pinzu5]++;
        }
        else if (tile == Tile::AkaSozu5) {
            aka_souzu5 = true;
            counts[Tile::Sozu5]++;
        }
        else {
            counts[tile]++;
        }
    }
}

/**
 * @brief 手牌を作成する。
 *
 * @param[in] tiles 牌の一覧
 * @param[in] melds 副露ブロックの一覧
 */
inline Hand::Hand(const std::vector<int> &tiles, const std::vector<MeldedBlock> &melds)
    : counts(TILE_TYPES, 0)
    , melds(melds)
    , aka_manzu5(false)
    , aka_pinzu5(false)
    , aka_souzu5(false)
{
#ifdef CHECK_ARGUMENT
    if (!check_arguments(tiles, melds))
        return;
#endif

    for (int tile : tiles) {
        if (tile == Tile::AkaManzu5) {
            aka_manzu5 = true;
            counts[Tile::Manzu5]++;
        }
        else if (tile == Tile::AkaPinzu5) {
            aka_pinzu5 = true;
            counts[Tile::Pinzu5]++;
        }
        else if (tile == Tile::AkaSozu5) {
            aka_souzu5 = true;
            counts[Tile::Sozu5]++;
        }
        else {
            counts[tile]++;
        }
    }
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
    int num_tiles = int(melds.size()) * 3;
    std::vector<int> tile_counts(34);
    for (auto tile : tiles) {
        if (tile < 0 || tile >= Tile::Length) {
            return false;
        }

        if (tile == Tile::AkaManzu5) {
            tile_counts[Tile::Manzu5]++;
        }
        else if (tile == Tile::AkaPinzu5) {
            tile_counts[Tile::Pinzu5]++;
        }
        else if (tile == Tile::AkaSozu5) {
            tile_counts[Tile::Sozu5]++;
        }
        else {
            tile_counts[tile]++;
        }
        num_tiles++;
    }

    for (const auto &block : melds) {
        for (int tile : block.tiles) {
            if (tile < 0 || tile >= Tile::Length) {
                return false;
            }

            if (tile == Tile::AkaManzu5) {
                tile_counts[Tile::Manzu5]++;
            }
            else if (tile == Tile::AkaPinzu5) {
                tile_counts[Tile::Pinzu5]++;
            }
            else if (tile == Tile::AkaSozu5) {
                tile_counts[Tile::Sozu5]++;
            }
            else {
                tile_counts[tile]++;
            }
        }
    }

    return std::all_of(tile_counts.begin(), tile_counts.end(),
                       [](int count) { return count <= 4; }) &&
           (num_tiles == 13 || num_tiles == 14); // 少牌または多牌かどうか
}

/**
 * @brief 門前かどうかを取得する。
 *
 * @return 門前の場合は true、そうでない場合は false を返す。
 */
inline bool Hand::is_closed() const
{
    for (const auto &meld : melds) {
        if (meld.type != MeldType::Ankan) {
            return false; // 暗槓以外の副露ブロックがある場合
        }
    }

    return true;
}

/**
 * @brief 手牌にある牌の合計枚数を取得する。
 *
 * @return int 牌の合計枚数
 */
inline int Hand::num_tiles() const
{
    return std::accumulate(counts.begin(), counts.end(), 0);
}

/**
 * @brief 牌の一覧からビット列を作成する。
 *
 * @param[in] tiles 牌の一覧
 */
inline Hand Hand::from_array34(const std::vector<int> &array34)
{
    Hand hand;
    hand.counts = array34;

    return hand;
}

inline Hand Hand::from_mpsz(const std::string &tiles)
{
    Hand hand;

    std::string type;
    for (auto it = tiles.rbegin(); it != tiles.rend(); ++it) {
        if (std::isspace(*it)) {
            continue;
        }

        if (*it == 'm' || *it == 'p' || *it == 's' || *it == 'z') {
            type = *it;
        }
        else if (std::isdigit(*it)) {
            int tile = *it - '0' - 1;
            if (type == "m") {
                hand.counts[tile]++;
            }
            else if (type == "p") {
                hand.counts[tile + 9]++;
            }
            else if (type == "s") {
                hand.counts[tile + 18]++;
            }
            else if (type == "z") {
                hand.counts[tile + 27]++;
            }
        }
    }

    return hand;
}

/**
 * @brief 手牌を表す MPS 表記の文字列を取得する。
 *        例: 1112r56789m1122p
 *
 * @return std::string 文字列
 */
inline std::string Hand::to_string() const
{
    std::string s;
    const std::string suffix = "mpsz";
    bool has_suit[4] = {false, false, false, false};

    // 萬子
    for (int i = 0; i < TILE_TYPES; ++i) {
        int type = i / 9;
        int num = i % 9 + 1;

        if (counts[i] > 0) {
            if (num == 5 && ((type == 0 && aka_manzu5) || (type == 1 && aka_pinzu5) ||
                             (type == 2 && aka_souzu5))) {
                s += "r";
            }
            for (int j = 0; j < counts[i]; ++j) {
                s += std::to_string(num);
            }
            has_suit[type] = true;
        }

        if ((i == 8 || i == 17 || i == 26 || i == 33) && has_suit[type]) {
            s += suffix[type];
        }
    }

    // 副露ブロック
    if (!s.empty() && !melds.empty()) {
        s += " ";
        for (const auto &block : melds) {
            s += block.to_string();
        }
    }

    return s;
}

inline std::ostream &operator<<(std::ostream &os, const Hand &hand)
{
    return os << hand.to_string();
}

inline bool operator==(const Hand &a, const Hand &b)
{
    return a.melds == b.melds && a.counts == b.counts && a.aka_manzu5 == b.aka_manzu5 &&
           a.aka_pinzu5 == b.aka_pinzu5 && a.aka_souzu5 == b.aka_souzu5;
}

} // namespace mahjong

#endif /* MAHJONG_CPP_HAND */
