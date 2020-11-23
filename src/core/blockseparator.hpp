#ifndef MAHJONG_CPP_BLOCKSEPARATOR
#define MAHJONG_CPP_BLOCKSEPARATOR

#include "types/types.hpp"

namespace mahjong {

/**
 * @brief 手牌から面子パターンを生成する。
 */
class BlockSeparator {
    static bool initialize();
    static bool make_table(const std::string &path,
                           std::map<int, std::vector<std::vector<Block>>> &table);
    static std::vector<std::vector<Block>>
    create_block_patterns(const Hand &tehai, int win_tile, bool tumo);
    static std::vector<Block> get_blocks(const std::string &s);
    static void create_block_patterns(const Hand &tehai, int win_tile, bool tumo,
                                      std::vector<std::vector<Block>> &patterns,
                                      std::vector<Block> &blocks, size_t i, int d = 0);

private:
    static std::map<int, std::vector<std::vector<Block>>> s_tbl_;
    static std::map<int, std::vector<std::vector<Block>>> z_tbl_;
};
} // namespace mahjong

#endif /* MAHJONG_CPP_BLOCKSEPARATOR */
