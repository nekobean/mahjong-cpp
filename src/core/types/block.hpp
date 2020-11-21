#ifndef MAHJONG_CPP_BLOCK
#define MAHJONG_CPP_BLOCK

#include <map>
#include <string>
#include <vector>

#include "tile.hpp"

namespace mahjong {

/**
 * @brief ブロック
 */
struct Block {
    /**
     * @brief ブロックの種類
     */
    enum Type {
        Null   = 0,
        Kotu   = 1,  /* 刻子 */
        Syuntu = 2,  /* 順子 */
        Kantu  = 4,  /* 槓子 */
        Toitu  = 8,  /* 対子 */
        Huro   = 16, /* 副露して完成したブロックかどうか */
        Length = 6,
    };

    Block()
        : type(Block::Null)
        , min_tile(Tile::Null)
    {
    }

    Block(int type, int min_tile)
        : type(type)
        , min_tile(min_tile)
    {
    }

    /*! ブロックの種類 */
    int type;

    /*! 最小の構成牌 */
    int min_tile;

    static inline std::map<int, std::string> Names = {
        {Block::Kotu, "暗刻子"},   {Block::Kotu | Block::Huro, "明刻子"},
        {Block::Syuntu, "暗順子"}, {Block::Syuntu | Block::Huro, "明順子"},
        {Block::Kantu, "暗槓子"},  {Block::Kantu | Block::Huro, "明槓子"},
        {Block::Toitu, "暗対子"},  {Block::Toitu | Block::Huro, "明対子"},
    };

    std::string to_string() const;
};

/**
 * @brief 文字列に変換する。
 *
 * @return std::string ブロックを表す文字列
 */
inline std::string Block::to_string() const
{
    std::string s;

    s += "[";
    if (type & Kotu) {
        for (int i = 0; i < 3; ++i)
            s += Tile::Names.at(min_tile);
    }
    else if (type & Syuntu) {
        for (int i = 0; i < 3; ++i)
            s += Tile::Names.at(min_tile + i);
    }
    else if (type & Kantu) {
        for (int i = 0; i < 4; ++i)
            s += Tile::Names.at(min_tile);
    }
    else if (type & Toitu) {
        for (int i = 0; i < 2; ++i)
            s += Tile::Names.at(min_tile);
    }
    s += fmt::format(", {}]", Names[type]);

    return s;
}

/**
 * @brief 待ちの種類
 */
struct WaitType {
    enum Type {
        Null = -1,
        Ryanmen, /* 両面待ち */
        Pentyan, /* 辺張待ち */
        Kantyan, /* 嵌張待ち */
        Syanpon, /* 双ポン待ち */
        Tanki,   /* 単騎待ち */
    };

    static inline const std::map<int, std::string> Names = {
        {Null, "Null"},        {Ryanmen, "両面待ち"},   {Pentyan, "辺張待ち"},
        {Kantyan, "嵌張待ち"}, {Syanpon, "双ポン待ち"}, {Tanki, "単騎待ち"}};
};

/**
 * @brief 待ちの種類を取得する。
 * 
 * @param[in] blocks 面子構成
 * @param[in] winning_tile 和了牌
 * @return int 待ちの種類
 */
inline int get_wait_type(const std::vector<Block> &blocks, int winning_tile)
{
    for (const auto &block : blocks) {
        if ((block.type & Block::Kotu) && block.min_tile == winning_tile) {
            return WaitType::Syanpon; // 刻子の場合、双ポン待ち
        }
        else if (block.type & Block::Syuntu) {
            if (block.min_tile + 1 == winning_tile) {
                return WaitType::Kantyan; // 嵌張待ち
            }
            else if (block.min_tile == winning_tile ||
                     block.min_tile + 2 == winning_tile) {
                if (block.min_tile == Tile::Manzu1 || block.min_tile == Tile::Pinzu1 ||
                    block.min_tile == Tile::Sozu1) {
                    if (block.min_tile == winning_tile)
                        return WaitType::Ryanmen; // 123 で和了牌が1の場合、両面待ち
                    else
                        return WaitType::Pentyan; // 123 で和了牌が3の場合、辺張待ち
                }
                else if (block.min_tile == Tile::Manzu7 ||
                         block.min_tile == Tile::Pinzu7 ||
                         block.min_tile == Tile::Sozu7) {
                    if (block.min_tile == winning_tile)
                        return WaitType::Pentyan; // 789 で和了牌が7の場合、辺張待ち
                    else
                        return WaitType::Ryanmen; // 789 で和了牌が9の場合、両面待ち
                }
                else {
                    return WaitType::Ryanmen;
                }
            }
        }
        else if (block.type & Block::Toitu && block.min_tile == winning_tile) {
            return WaitType::Tanki; // 対子の場合、単騎待ち
        }
    }

    return WaitType::Null;
}

} // namespace mahjong

#endif /* MAHJONG_CPP_BLOCK */
