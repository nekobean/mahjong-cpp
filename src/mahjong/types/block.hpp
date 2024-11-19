#ifndef MAHJONG_CPP_BLOCK
#define MAHJONG_CPP_BLOCK

#include "mahjong/types/const.hpp"

namespace mahjong
{

/**
 * @brief Block that composes a hand
 */
struct Block
{
    Block() : type(BlockType::Null), min_tile(Tile::Null)
    {
    }

    Block(int type, int min_tile) : type(type), min_tile(min_tile)
    {
    }

    /*! block type */
    int type;

    /*! The smallest tile that composes the block */
    int min_tile;
};

} // namespace mahjong

#endif /* MAHJONG_CPP_BLOCK */
