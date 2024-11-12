#ifndef MAHJONG_CPP_UTILS
#define MAHJONG_CPP_UTILS

#include <string>

#include <spdlog/spdlog.h>

#include "mahjong/core/score_calculator.hpp"

namespace mahjong
{

/**
 * @brief ドラとドラ表示牌の対応表
 */
static inline const std::map<int, int> Dora2Indicator = {
    {Tile::Manzu1, Tile::Manzu9},    {Tile::Manzu2, Tile::Manzu1},
    {Tile::Manzu3, Tile::Manzu2},    {Tile::Manzu4, Tile::Manzu3},
    {Tile::Manzu5, Tile::Manzu4},    {Tile::Manzu6, Tile::Manzu5},
    {Tile::Manzu7, Tile::Manzu6},    {Tile::Manzu8, Tile::Manzu7},
    {Tile::Manzu9, Tile::Manzu8},    {Tile::Pinzu1, Tile::Pinzu9},
    {Tile::Pinzu2, Tile::Pinzu1},    {Tile::Pinzu3, Tile::Pinzu2},
    {Tile::Pinzu4, Tile::Pinzu3},    {Tile::Pinzu5, Tile::Pinzu4},
    {Tile::Pinzu6, Tile::Pinzu5},    {Tile::Pinzu7, Tile::Pinzu6},
    {Tile::Pinzu8, Tile::Pinzu7},    {Tile::Pinzu9, Tile::Pinzu8},
    {Tile::Souzu1, Tile::Souzu9},    {Tile::Souzu2, Tile::Souzu1},
    {Tile::Souzu3, Tile::Souzu2},    {Tile::Souzu4, Tile::Souzu3},
    {Tile::Souzu5, Tile::Souzu4},    {Tile::Souzu6, Tile::Souzu5},
    {Tile::Souzu7, Tile::Souzu6},    {Tile::Souzu8, Tile::Souzu7},
    {Tile::Souzu9, Tile::Souzu8},    {Tile::East, Tile::North},
    {Tile::South, Tile::East},       {Tile::West, Tile::South},
    {Tile::North, Tile::West},       {Tile::White, Tile::Red},
    {Tile::Green, Tile::White},      {Tile::Red, Tile::Green},
    {Tile::RedManzu5, Tile::Manzu4}, {Tile::RedPinzu5, Tile::Pinzu4},
    {Tile::RedSouzu5, Tile::Souzu4}};

/**
 * @brief ドラ表示牌とドラの対応表
 */
static inline const std::map<int, int> Indicator2Dora = {
    {Tile::Manzu1, Tile::Manzu2},    {Tile::Manzu2, Tile::Manzu3},
    {Tile::Manzu3, Tile::Manzu4},    {Tile::Manzu4, Tile::Manzu5},
    {Tile::Manzu5, Tile::Manzu6},    {Tile::Manzu6, Tile::Manzu7},
    {Tile::Manzu7, Tile::Manzu8},    {Tile::Manzu8, Tile::Manzu9},
    {Tile::Manzu9, Tile::Manzu1},    {Tile::Pinzu1, Tile::Pinzu2},
    {Tile::Pinzu2, Tile::Pinzu3},    {Tile::Pinzu3, Tile::Pinzu4},
    {Tile::Pinzu4, Tile::Pinzu5},    {Tile::Pinzu5, Tile::Pinzu6},
    {Tile::Pinzu6, Tile::Pinzu7},    {Tile::Pinzu7, Tile::Pinzu8},
    {Tile::Pinzu8, Tile::Pinzu9},    {Tile::Pinzu9, Tile::Pinzu1},
    {Tile::Souzu1, Tile::Souzu2},    {Tile::Souzu2, Tile::Souzu3},
    {Tile::Souzu3, Tile::Souzu4},    {Tile::Souzu4, Tile::Souzu5},
    {Tile::Souzu5, Tile::Souzu6},    {Tile::Souzu6, Tile::Souzu7},
    {Tile::Souzu7, Tile::Souzu8},    {Tile::Souzu8, Tile::Souzu9},
    {Tile::Souzu9, Tile::Souzu1},    {Tile::East, Tile::South},
    {Tile::South, Tile::West},       {Tile::West, Tile::North},
    {Tile::North, Tile::East},       {Tile::White, Tile::Green},
    {Tile::Green, Tile::Red},        {Tile::Red, Tile::White},
    {Tile::RedManzu5, Tile::Manzu6}, {Tile::RedPinzu5, Tile::Pinzu6},
    {Tile::RedSouzu5, Tile::Souzu6}};

/**
 * @brief 設定を文字列にして返す。
 *
 * @return std::string 文字列
 */
inline std::string print_round_info(const ScoreCalculator &score)
{
    std::string s;

    s += "[ルール]\n";
    for (auto rule : {RuleFlag::RedDora, RuleFlag::OpenTanyao}) {
        s += fmt::format("  {}: {}\n", RuleFlag::Name.at(rule),
                         (score.rules() & rule) ? "有り" : "無し");
    }

    s +=
        fmt::format("[場] 場風: {}, 自風: {}, 積み棒の数: {}, 供託棒の数: {}\n",
                    Tile::Name.at(score.round_wind()), Tile::Name.at(score.self_wind()),
                    score.bonus_sticks(), score.deposit_sticks());

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

static bool check_exclusive(unsigned long long x)
{
    return !x || !(x & (x - 1));
}

} // namespace mahjong

#endif /* MAHJONG_CPP_UTILS */
