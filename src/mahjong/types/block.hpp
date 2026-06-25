#ifndef MAHJONG_CPP_BLOCK_HPP
#define MAHJONG_CPP_BLOCK_HPP

#include "mahjong/types/constants.hpp"
#include "mahjong/types/tile.hpp"

namespace mahjong
{

/**
 * @brief Block in a hand decomposition.
 */
struct Block
{
    /*! Block type. */
    BlockFlags type = BlockType::None;

    /*! Lowest tile in the block. */
    int min_tile = Tile::Null;
};

} // namespace mahjong

#endif // MAHJONG_CPP_BLOCK_HPP
