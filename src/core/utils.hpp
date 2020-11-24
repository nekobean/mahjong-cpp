#ifndef MAHJONG_CPP_UTILS
#define MAHJONG_CPP_UTILS

#include <string>

#include <spdlog/spdlog.h>

#include "score.hpp"

namespace mahjong {

/**
 * @brief 手牌を表すビット表記の文字列を取得する。
 *        例: 111256789m 1122p
 * 
 * @return std::string 文字列
 */
inline std::string to_bit_string(const Hand &hand)
{
    std::string s;
    s += fmt::format("manzu: {:027b}\npinzu: {:027b}\n sozu: {:027b}\nzihai: {:027b}",
                     hand.manzu, hand.pinzu, hand.sozu, hand.zihai);

    return s;
}

inline std::string to_count_string(const Hand &hand)
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

/**
 * @brief 設定を文字列にして返す。
 * 
 * @return std::string 文字列
 */
inline std::string print_round_info(const ScoreCalculator &score)
{
    std::string s;

    s += "[ルール]\n";
    for (auto rule : {RuleFlag::AkaDora, RuleFlag::OpenTanyao}) {
        s += fmt::format("  {}: {}\n", RuleFlag::Name.at(rule),
                         (score.rules() & rule) ? "有り" : "無し");
    }

    s += fmt::format("[場] 場風: {}, 自風: {}, 積み棒の数: {}, 供託棒の数: {}\n",
                     Tile::Name.at(score.bakaze()), Tile::Name.at(score.zikaze()),
                     score.num_tumibo(), score.num_kyotakubo());

    s += "[表ドラ] ";
    for (const auto &tile : score.dora_tiles())
        s += fmt::format("{}{}", Tile::Name.at(tile),
                         &tile == &score.dora_tiles().back() ? "\n" : ", ");

    s += "[裏ドラ] ";
    for (const auto &tile : score.uradora_tiles())
        s += fmt::format("{}{}", Tile::Name.at(tile),
                         &tile == &score.uradora_tiles().back() ? "\n" : ", ");

    return s;
}

} // namespace mahjong

#endif /* MAHJONG_CPP_UTILS */
