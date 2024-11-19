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
    struct Input
    {
        Hand merged_hand;
        Hand hand;
        std::vector<MeldedBlock> melds;

        bool is_closed() const
        {
            for (const auto &meld : melds) {
                if (meld.type != MeldType::ClosedKong) {
                    return false; // contains open meld (not a closed kong)
                }
            }

            return true;
        }

        int manzu;
        int pinzu;
        int souzu;
        int honors;

        int win_tile;
        int win_flag;

        int merged_manzu;
        int merged_pinzu;
        int merged_souzu;
        int merged_honors;
    };

    HandSeparator();
    static bool initialize();
    static std::vector<std::tuple<std::vector<Block>, int>>
    separate(const Input &input);

  private:
    static bool make_table(const std::string &path,
                           std::map<int, std::vector<std::vector<Block>>> &table);
    static std::vector<Block> get_blocks(const std::string &s);
    static void
    create_block_patterns(const Input &input,
                          std::vector<std::tuple<std::vector<Block>, int>> &patterns,
                          std::vector<Block> &blocks, size_t i, int d = 0);

  private:
    static std::map<int, std::vector<std::vector<Block>>> s_tbl_;
    static std::map<int, std::vector<std::vector<Block>>> z_tbl_;
};

} // namespace mahjong

#endif /* MAHJONG_CPP_HAND_SEPARATOR */
