#ifndef MAHJONG_CPP_BLOCK
#define MAHJONG_CPP_BLOCK

#include <map>
#include <string>
#include <vector>

#include <spdlog/spdlog.h>

#include "mahjong/types/const.hpp"
#include "tile.hpp"

namespace mahjong
{

/**
 * @brief ブロック
 */
struct Block
{
    Block() : type(BlockType::Null), min_tile(Tile::Null)
    {
    }

    Block(int type, int min_tile) : type(type), min_tile(min_tile)
    {
    }

    /*! ブロックの種類 */
    int type;

    /*! 最小の構成牌 */
    int min_tile;

    std::string to_string() const;
};

/**
 * @brief 文字列に変換する。
 *
 * @return std::string ブロックを表す文字列
 */
inline std::string Block::to_string() const
{
    std::vector<int> tiles;
    if (type & BlockType::Triplet) {
        for (int i = 0; i < 3; ++i)
            tiles.push_back(min_tile);
    }
    else if (type & BlockType::Sequence) {
        for (int i = 0; i < 3; ++i)
            tiles.push_back(min_tile + i);
    }
    else if (type & BlockType::Kong) {
        for (int i = 0; i < 4; ++i)
            tiles.push_back(min_tile);
    }
    else if (type & BlockType::Pair) {
        for (int i = 0; i < 2; ++i)
            tiles.push_back(min_tile);
    }

    std::string s;

    s += "[";
    for (auto tile : tiles) {
        if (is_manzu(tile))
            s += std::to_string(tile + 1);
        else if (is_pinzu(tile))
            s += std::to_string(tile - 8);
        else if (is_sozu(tile))
            s += std::to_string(tile - 17);
        else
            s += Tile::Name.at(tile);
    }

    if (is_manzu(tiles[0]))
        s += "m";
    else if (is_pinzu(tiles[0]))
        s += "p";
    else if (is_sozu(tiles[0]))
        s += "s";
    s += fmt::format(", {}]", BlockType::Name.at(type));

    return s;
}

} // namespace mahjong

#endif /* MAHJONG_CPP_BLOCK */
