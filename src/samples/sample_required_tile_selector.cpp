#include "mahjong/mahjong.hpp"

using namespace mahjong;

/**
 * @brief 各牌の残り枚数を数える。
 *
 * @param[in] hand 手牌
 * @return std::vector<int> 各牌の残り枚数
 */
std::vector<int> count_left_tiles(const Hand &hand)
{
    // 残り牌の枚数
    std::vector<int> count(34, 4);

    for (int i = 0; i < 34; ++i)
        count[i] -= hand.num_tiles(i);

    for (const auto &block : hand.melds) {
        for (auto tile : block.tiles) {
            tile = red2normal(tile);
            count[tile] -= hand.num_tiles(tile);
        }
    }

    return count;
}

/**
 * @brief 有効牌の合計枚数を数える。
 *
 * @param[in] count 各牌の残り枚数
 * @param[in] tiles 有効牌の一覧
 * @return int 有効牌の合計枚数
 */
int count_num_required_tiles(const std::vector<int> &count,
                             const std::vector<int> &tiles)
{
    int n_tiles = 0;
    for (auto tile : tiles)
        n_tiles += count[tile];

    return n_tiles;
}

int main(int, char **)
{
    // 一般手の有効牌を計算する
    {
        Hand hand({Tile::Manzu1, Tile::Manzu1, Tile::Manzu1, Tile::Manzu2,
                   Tile::RedManzu5, Tile::Manzu6, Tile::Manzu7, Tile::Manzu8,
                   Tile::Manzu9, Tile::Pinzu1, Tile::Pinzu1, Tile::Pinzu2,
                   Tile::Pinzu2});

        // 有効牌を計算する。
        auto tiles = RequiredTileSelector::select(hand, SyantenType::Normal);

        // 各牌の残り枚数を数える。
        auto count = count_left_tiles(hand);

        std::cout << fmt::format("[一般手の有効牌] 手牌: {}", hand.to_string())
                  << std::endl;

        int n_tiles = 0;
        for (auto tile : tiles) {
            n_tiles += count[tile];
            std::cout << fmt::format("{}: {}枚 ", Tile::Name.at(tile), count[tile]);
        }
        std::cout << std::endl;
        std::cout << fmt::format("有効牌合計: {}枚", n_tiles) << std::endl;
    }

    // 七対子手の有効牌を計算する
    {
        Hand hand({Tile::Manzu1, Tile::Manzu1, Tile::Manzu2, Tile::Manzu2,
                   Tile::RedManzu5, Tile::Manzu5, Tile::Manzu8, Tile::Manzu8,
                   Tile::Manzu8, Tile::East, Tile::South, Tile::West, Tile::West});

        // 有効牌を計算する。
        auto tiles = RequiredTileSelector::select(hand, SyantenType::Tiitoi);

        // 各牌の残り枚数を数える。
        auto count = count_left_tiles(hand);

        std::cout << fmt::format("[七対子手の有効牌] 手牌: {}", hand.to_string())
                  << std::endl;

        int n_tiles = 0;
        for (auto tile : tiles) {
            n_tiles += count[tile];
            std::cout << fmt::format("{}: {}枚 ", Tile::Name.at(tile), count[tile]);
        }
        std::cout << std::endl;
        std::cout << fmt::format("有効牌合計: {}枚", n_tiles) << std::endl;
    }

    // 国士無双手の有効牌を計算する
    {
        Hand hand({Tile::Manzu1, Tile::Manzu1, Tile::Pinzu9, Tile::Souzu1, Tile::Souzu9,
                   Tile::Souzu9, Tile::East, Tile::South, Tile::West, Tile::West,
                   Tile::White, Tile::Green, Tile::Red});

        // 有効牌を計算する。
        auto tiles = RequiredTileSelector::select(hand, SyantenType::Kokusi);

        // 各牌の残り枚数を数える。
        auto count = count_left_tiles(hand);

        std::cout << fmt::format("[国士手の有効牌] 手牌: {}", hand.to_string())
                  << std::endl;

        int n_tiles = 0;
        for (auto tile : tiles) {
            n_tiles += count[tile];
            std::cout << fmt::format("{}: {}枚 ", Tile::Name.at(tile), count[tile]);
        }
        std::cout << std::endl;
        std::cout << fmt::format("有効牌合計: {}枚", n_tiles) << std::endl;
    }
}
