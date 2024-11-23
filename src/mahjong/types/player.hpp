#ifndef MAHJONG_CPP_PLAYER
#define MAHJONG_CPP_PLAYER

#include <algorithm> // any_of
#include <numeric>   // accumulate
#include <stdexcept> // invalid_argument
#include <vector>

#include "mahjong/types/const.hpp"
#include "mahjong/types/meld.hpp"

namespace mahjong
{

/**
 * @brief Player
 */
class Player
{
  public:
    Player() : Player(Hand{}, {}, 0)
    {
    }

    Player(const Hand &hand, int wind) : Player(hand, {}, wind)
    {
    }

    Player(const Hand &hand, const std::vector<Meld> &melds, int wind)
        : hand(hand), melds(melds), wind(wind)
    {
#ifdef CHECK_ARGUMENT
        check_arguments(tiles, melds);
#endif
    }

    Player(const std::vector<int> &tiles, int wind) : Player(to_hand(tiles), {}, wind)
    {
    }

    Player(const std::vector<int> &tiles, const std::vector<Meld> &melds, int wind)
        : Player(to_hand(tiles), melds, wind)
    {
    }

    /**
     * @brief Create a hand from a list of tiles and melded blocks.
     *
     * @param[in] tiles list of tiles
     * @param[in] melds list of melded blocks
     */
    inline Hand to_hand(const std::vector<int> &tiles)
    {
        Hand hand{0};

        for (int tile : tiles) {
            if (tile == Tile::RedManzu5) {
                ++hand[Tile::Manzu5];
            }
            else if (tile == Tile::RedPinzu5) {
                ++hand[Tile::Pinzu5];
            }
            else if (tile == Tile::RedSouzu5) {
                ++hand[Tile::Souzu5];
            }
            ++hand[tile];
        }

        return hand;
    }

    // Count of hand tiles (手牌の各牌の枚数)
    Hand hand;

    // List of meld blocks (副露ブロックの一覧)
    std::vector<Meld> melds;

    // Seat wind (自風)
    int wind;

    int num_tiles() const
    {
        return std::accumulate(hand.begin(), hand.begin() + 34, 0);
    }

    int num_melds() const
    {
        return static_cast<int>(melds.size());
    }

    bool is_closed() const
    {
        for (const auto &meld : melds) {
            if (meld.type != MeldType::ClosedKong) {
                return false;
            }
        }

        return true;
    }
};

/**
 * @brief 引数が問題ないかどうかを調べる。
 *
 * @param[in] tiles 牌の一覧
 * @param melds 副露ブロックの一覧
 * @return 引数が問題ない場合は true、そうでない場合は false を返す。
 */
inline void check_arguments(const std::vector<int> &tiles,
                            const std::vector<Meld> &melds)
{
    // Check if the total number of tiles is 13 or 14.
    int num_tiles = std::accumulate(tiles.begin(), tiles.end(), int(melds.size()) * 3);
    if (num_tiles != 13 && num_tiles != 14) {
        throw std::invalid_argument(u8"The total number of tiles must be 13 or 14.");
    }

    // Check if the number of each tile is 4 or less.
    std::vector<int> merged_tiles = tiles;
    for (const auto &meld : melds) {
        merged_tiles.insert(merged_tiles.end(), meld.tiles.begin(), meld.tiles.end());
    }

    std::vector<int> tile_counts(37);
    for (int tile : merged_tiles) {
        if (tile < 0 || tile >= Tile::Length) {
            throw std::invalid_argument(u8"Invalid tile number found.");
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
        throw std::invalid_argument(u8"The number of each tile must be 4 or less.");
    }

    if (tile_counts[Tile::RedManzu5] > 1 || tile_counts[Tile::RedPinzu5] > 1 ||
        tile_counts[Tile::RedSouzu5] > 1) {
        throw std::invalid_argument(u8"The number of red fives must be 1 or less.");
    }
}

} // namespace mahjong

#endif // MAHJONG_CPP_PLAYER
