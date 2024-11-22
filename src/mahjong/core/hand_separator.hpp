#ifndef MAHJONG_CPP_HAND_SEPARATOR
#define MAHJONG_CPP_HAND_SEPARATOR

#include "mahjong/types/types.hpp"

namespace mahjong
{

/**
 * @brief 手牌から面子パターンを生成する。
 */
class HandSeparator
{
  public:
    HandSeparator();
    static bool initialize();
    static std::vector<std::tuple<std::vector<Block>, int>>
    separate(const Player &player, const int win_tile, const int win_flag);

  private:
    static bool make_table(const std::string &path,
                           std::map<int, std::vector<std::vector<Block>>> &table);
    static std::vector<Block> get_blocks(const std::string &s);
    static void
    create_block_patterns(const int win_tile, const bool tsumo,
                          std::vector<std::tuple<std::vector<Block>, int>> &patterns,
                          std::vector<Block> &blocks, size_t i, int d,
                          const std::vector<std::vector<Block>> &manzu,
                          const std::vector<std::vector<Block>> &pinzu,
                          const std::vector<std::vector<Block>> &souzu,
                          const std::vector<std::vector<Block>> &honors);

  private:
    static std::map<int, std::vector<std::vector<Block>>> s_tbl_;
    static std::map<int, std::vector<std::vector<Block>>> z_tbl_;
};

} // namespace mahjong

#endif /* MAHJONG_CPP_HAND_SEPARATOR */
