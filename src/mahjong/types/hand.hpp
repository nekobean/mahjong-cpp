#ifndef MAHJONG_CPP_HAND
#define MAHJONG_CPP_HAND

#include <algorithm>
#include <array>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

#include <spdlog/spdlog.h>

#include "meld.hpp"
#include "tile.hpp"

using HandType = std::array<int, 37>;

namespace mahjong
{

/**
 * @brief 手牌
 */
class Hand
{
  public:
    Hand();
    Hand(const std::vector<int> &tiles);
    Hand(const std::vector<int> &tiles, const std::vector<MeldedBlock> &melds);

    bool is_closed() const;
    int num_tiles() const;
    static Hand from_mpsz(const std::string &mpsz_str);

    static Hand from_counts(const std::vector<int> &tiles)
    {
        Hand hand;
        for (int i = 0; i < 34; ++i) {
            hand.counts[i] = tiles[i];
        }
        return hand;
    }

  private:
    void check_arguments(const std::vector<int> &tiles,
                         const std::vector<MeldedBlock> &melds);
    friend bool operator==(const Hand &a, const Hand &b);

  public:
    /* 牌数 */
    HandType counts;

    /* 副露ブロック */
    std::vector<MeldedBlock> melds;

    int32_t manzu;
    int32_t pinzu;
    int32_t souzu;
    int32_t honors;
};

/**
 * @brief Create an empty hand.
 */
inline Hand::Hand() : counts{0}
{
}

/**
 * @brief Create a hand from a list of tiles.
 *
 * @param[in] tiles list of tiles
 */
inline Hand::Hand(const std::vector<int> &tiles) : counts{0}
{
#ifdef CHECK_ARGUMENT
    if (!check_arguments(tiles, melds))
        return;
#endif

    for (int tile : tiles) {
        if (tile == Tile::RedManzu5) {
            ++counts[Tile::Manzu5];
        }
        else if (tile == Tile::RedPinzu5) {
            ++counts[Tile::Pinzu5];
        }
        else if (tile == Tile::RedSouzu5) {
            ++counts[Tile::Souzu5];
        }

        ++counts[tile];
    }
}

/**
 * @brief Create a hand from a list of tiles and melded blocks.
 *
 * @param[in] tiles list of tiles
 * @param[in] melds list of melded blocks
 */
inline Hand::Hand(const std::vector<int> &tiles, const std::vector<MeldedBlock> &melds)
    : counts{0}, melds(melds)
{
#ifdef CHECK_ARGUMENT
    if (!check_arguments(tiles, melds))
        return;
#endif

    for (int tile : tiles) {
        if (tile == Tile::RedManzu5) {
            ++counts[Tile::Manzu5];
        }
        else if (tile == Tile::RedPinzu5) {
            ++counts[Tile::Pinzu5];
        }
        else if (tile == Tile::RedSouzu5) {
            ++counts[Tile::Souzu5];
        }

        ++counts[tile];
    }
}

/**
 * @brief 引数が問題ないかどうかを調べる。
 *
 * @param[in] tiles 牌の一覧
 * @param melds 副露ブロックの一覧
 * @return 引数が問題ない場合は true、そうでない場合は false を返す。
 */
inline void Hand::check_arguments(const std::vector<int> &tiles,
                                  const std::vector<MeldedBlock> &melds)
{
    // Check if the total number of tiles is 13 or 14.
    int num_tiles = std::accumulate(tiles.begin(), tiles.end(), int(melds.size()) * 3);
    if (num_tiles != 13 && num_tiles != 14) {
        throw std::invalid_argument("The total number of tiles must be 13 or 14.");
    }

    // Check if the number of each tile is 4 or less.
    std::vector<int> merged_tiles = tiles;
    for (const auto &meld : melds) {
        merged_tiles.insert(merged_tiles.end(), meld.tiles.begin(), meld.tiles.end());
    }

    std::vector<int> tile_counts(37);
    for (int tile : merged_tiles) {
        if (tile < 0 || tile >= Tile::Length) {
            throw std::invalid_argument("Invalid tile number found.");
        }

        if (tile == Tile::RedManzu5) {
            ++tile_counts[Tile::Manzu5];
        }
        else if (tile == Tile::RedPinzu5) {
            ++tile_counts[Tile::Pinzu5];
        }
        else if (tile == Tile::RedSouzu5) {
            ++tile_counts[Tile::Souzu5];
        }

        ++tile_counts[tile];
    }

    if (std::any_of(tile_counts.begin(), tile_counts.end(),
                    [](int count) { return count > 4; })) {
        throw std::invalid_argument("The number of each tile must be 4 or less.");
    }

    if (tile_counts[Tile::RedManzu5] > 1 || tile_counts[Tile::RedPinzu5] > 1 ||
        tile_counts[Tile::RedSouzu5] > 1) {
        throw std::invalid_argument("The number of red fives must be 1 or less.");
    }
}

/**
 * @brief Check if the hand is closed.
 *
 * @return Returns true if the hand is closed, otherwise false.
 */
inline bool Hand::is_closed() const
{
    for (const auto &meld : melds) {
        if (meld.type != MeldType::ClosedKong) {
            return false; // contains open meld (not a closed kong)
        }
    }

    return true;
}

/**
 * @brief Get the total number of tiles.
 *
 * @return total number of tiles
 */
inline int Hand::num_tiles() const
{
    return std::accumulate(counts.begin(), counts.begin() + 34, 0);
}

/**
 * @brief Create a hand from a string in MPSZ notation.
 *
 * @param[in] tiles string in MPSZ notation
 * @return Hand object
 */
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

inline bool operator==(const Hand &a, const Hand &b)
{
    return a.melds == b.melds && a.counts == b.counts;
}

} // namespace mahjong

#endif /* MAHJONG_CPP_HAND */
