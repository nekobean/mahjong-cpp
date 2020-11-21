#ifndef MAHJONG_CPP_UTILS
#define MAHJONG_CPP_UTILS

#include <string>

#include <spdlog/spdlog.h>

/**
 * @brief 手牌を表すビット表記の文字列を取得する。
 *        例: 111256789m 1122p
 * 
 * @return std::string 文字列
 */
inline std::string Hand::to_bit_string(const Hand &hand)
{
    std::string s;
    s += fmt::format("manzu: {:027b}\npinzu: {:027b}\n sozu: {:027b}\nzihai: {:027b}",
                     hand.manzu, hand.pinzu, hand.sozu, hand.zihai);

    return s;
}

inline std::string Hand::to_count_string(const Hand &hand)
{
    std::string s;

    s += "[";
    // 萬子
    for (int i = 0; i < 9; ++i)
        s += fmt::format("{}{}", Bit::get_n_tile(hand.manzu, i), i != 8 ? ", " : "|");

    // 筒子
    for (int i = 0; i < 9; ++i)
        s += fmt::format("{}{}", Bit::get_n_tile(hand.pinzu, i), i != 8 ? ", " : "|");

    // 索子
    for (int i = 0; i < 9; ++i)
        s += fmt::format("{}{}", Bit::get_n_tile(hand.sozu, i), i != 8 ? ", " : "|");

    // 字牌
    for (int i = 0; i < 7; ++i)
        s += fmt::format("{}{}", Bit::get_n_tile(hand.sozu, i), i != 6 ? ", " : "");
    s += "]";

    return s;
}

#endif /* MAHJONG_CPP_UTILS */
