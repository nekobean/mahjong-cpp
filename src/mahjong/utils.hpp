#ifndef MAHJONG_CPP_UTILS2
#define MAHJONG_CPP_UTILS2

#include <string>

#include <spdlog/spdlog.h>

#include "mahjong/core/score_calculator.hpp"

namespace mahjong
{

/**
 * @brief 設定を文字列にして返す。
 *
 * @return std::string 文字列
 */
inline std::string print_round_info(const ScoreCalculator &score)
{
    // std::string s;

    // s += "[ルール]\n";
    // for (auto rule : {RuleFlag::RedDora, RuleFlag::OpenTanyao}) {
    //     s += fmt::format("  {}: {}\n", RuleFlag::Name.at(rule),
    //                      (score.rules() & rule) ? "有り" : "無し");
    // }

    // s +=
    //     fmt::format("[場] 場風: {}, 自風: {}, 積み棒の数: {}, 供託棒の数: {}\n",
    //                 Tile::Name.at(score.wind()), Tile::Name.at(score.self_wind()),
    //                 score.bonus_sticks(), score.deposit_sticks());

    // s += "[表ドラ] ";
    // for (const auto &tile : score.dora_tiles())
    //     s += fmt::format("{}{}", Tile::Name.at(tile),
    //                      &tile == &score.dora_tiles().back() ? "\n" : ", ");

    // s += "[裏ドラ] ";
    // for (const auto &tile : score.uradora_tiles())
    //     s += fmt::format("{}{}", Tile::Name.at(tile),
    //                      &tile == &score.uradora_tiles().back() ? "\n" : ", ");

    // return s;
}

} // namespace mahjong

#endif /* MAHJONG_CPP_UTILS */
