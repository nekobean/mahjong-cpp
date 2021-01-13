#ifndef MAHJONG_CPP_UTILS
#define MAHJONG_CPP_UTILS

#include <string>

#include <spdlog/spdlog.h>

#include "score.hpp"

namespace mahjong {

/**
 * @brief 牌一覧を表す MPS 表記の文字列を取得する。
 *        例: 1112r56789m 1122p
 * 
 * @return std::string 文字列
 */
// inline std::string Hand::to_string(const std::vector<int> &tiles) const
// {
//     std::string s;

//     // 萬子
//     for (int i = 0; i < 9; ++i) {
//         int n = Bit::num_tiles(manzu, i);

//         s += aka_manzu5 && i == 4 ? "r" : "";
//         for (int j = 0; j < n; ++j)
//             s += std::to_string(i + 1);
//     }
//     if (manzu)
//         s += "m";

//     // 筒子
//     if (!s.empty() && pinzu)
//         s += " ";
//     for (int i = 0; i < 9; ++i) {
//         int n = Bit::num_tiles(pinzu, i);

//         s += aka_pinzu5 && i == 4 ? "r" : "";
//         for (int j = 0; j < n; ++j)
//             s += std::to_string(i + 1);
//     }
//     if (pinzu)
//         s += "p";

//     // 索子
//     if (!s.empty() && sozu)
//         s += " ";
//     for (int i = 0; i < 9; ++i) {
//         int n = Bit::num_tiles(sozu, i);

//         s += aka_sozu5 && i == 4 ? "r" : "";
//         for (int j = 0; j < n; ++j)
//             s += std::to_string(i + 1);
//     }
//     if (sozu)
//         s += "s";

//     // 字牌
//     if (!s.empty() && zihai)
//         s += " ";
//     for (int i = 0; i < 9; ++i) {
//         int n = Bit::num_tiles(zihai, i);
//         for (int j = 0; j < n; ++j)
//             s += Tile::Name.at(27 + i);
//     }

//     // 副露ブロック
//     if (!s.empty() && !melds.empty())
//         s += " ";
//     for (const auto &block : melds)
//         s += block.to_string();

//     return s;
// }

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
        s += fmt::format("{}{}", Bit::num_tiles(hand.manzu, i), i != 8 ? ", " : "|");

    // 筒子
    for (int i = 0; i < 9; ++i)
        s += fmt::format("{}{}", Bit::num_tiles(hand.pinzu, i), i != 8 ? ", " : "|");

    // 索子
    for (int i = 0; i < 9; ++i)
        s += fmt::format("{}{}", Bit::num_tiles(hand.sozu, i), i != 8 ? ", " : "|");

    // 字牌
    for (int i = 0; i < 7; ++i)
        s += fmt::format("{}{}", Bit::num_tiles(hand.sozu, i), i != 6 ? ", " : "");
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
