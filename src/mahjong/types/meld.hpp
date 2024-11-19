#ifndef MAHJONG_CPP_MELD
#define MAHJONG_CPP_MELD

#include <vector>

#include "mahjong/types/const.hpp"

namespace mahjong
{

/**
 * @brief 副露ブロック
 */
struct MeldedBlock
{
    MeldedBlock()
        : type(MeldType::Null), discarded_tile(Tile::Null), from(PlayerType::Null)
    {
    }

    MeldedBlock(int type, std::vector<int> tiles)
        : type(type)
        , tiles(tiles)
        , discarded_tile(!tiles.empty() ? tiles.front() : Tile::Null)
        , from(PlayerType::Null)
    {
    }

    MeldedBlock(int type, std::vector<int> tiles, int discarded_tile, int from)
        : type(type), tiles(tiles), discarded_tile(discarded_tile), from(from)
    {
    }

    /*! 副露の種類 */
    int type;

    /*! 構成牌 */
    std::vector<int> tiles;

    /*! 鳴いた牌 */
    int discarded_tile;

    /*! 鳴かれたプレイヤー */
    int from;
};

} // namespace mahjong

#endif /* MAHJONG_CPP_MELD */
