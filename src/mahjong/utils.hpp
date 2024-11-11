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
    {Tile::Sozu1, Tile::Sozu9},      {Tile::Sozu2, Tile::Sozu1},
    {Tile::Sozu3, Tile::Sozu2},      {Tile::Sozu4, Tile::Sozu3},
    {Tile::Sozu5, Tile::Sozu4},      {Tile::Sozu6, Tile::Sozu5},
    {Tile::Sozu7, Tile::Sozu6},      {Tile::Sozu8, Tile::Sozu7},
    {Tile::Sozu9, Tile::Sozu8},      {Tile::Ton, Tile::Pe},
    {Tile::Nan, Tile::Ton},          {Tile::Sya, Tile::Nan},
    {Tile::Pe, Tile::Sya},           {Tile::Haku, Tile::Tyun},
    {Tile::Hatu, Tile::Haku},        {Tile::Tyun, Tile::Hatu},
    {Tile::AkaManzu5, Tile::Manzu4}, {Tile::AkaPinzu5, Tile::Pinzu4},
    {Tile::AkaSozu5, Tile::Sozu4}};

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
    {Tile::Sozu1, Tile::Sozu2},      {Tile::Sozu2, Tile::Sozu3},
    {Tile::Sozu3, Tile::Sozu4},      {Tile::Sozu4, Tile::Sozu5},
    {Tile::Sozu5, Tile::Sozu6},      {Tile::Sozu6, Tile::Sozu7},
    {Tile::Sozu7, Tile::Sozu8},      {Tile::Sozu8, Tile::Sozu9},
    {Tile::Sozu9, Tile::Sozu1},      {Tile::Ton, Tile::Nan},
    {Tile::Nan, Tile::Sya},          {Tile::Sya, Tile::Pe},
    {Tile::Pe, Tile::Ton},           {Tile::Haku, Tile::Hatu},
    {Tile::Hatu, Tile::Tyun},        {Tile::Tyun, Tile::Haku},
    {Tile::AkaManzu5, Tile::Manzu6}, {Tile::AkaPinzu5, Tile::Pinzu6},
    {Tile::AkaSozu5, Tile::Sozu6}};

/**
 * @brief 設定を文字列にして返す。
 *
 * @return std::string 文字列
 */
inline std::string print_round_info(const ScoreCalculator2 &score)
{
    std::string s;

    s += "[ルール]\n";
    for (auto rule : {RuleFlag2::AkaDora, RuleFlag2::OpenTanyao}) {
        s += fmt::format("  {}: {}\n", RuleFlag2::Name.at(rule),
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

static bool check_exclusive(unsigned long long x)
{
    return !x || !(x & (x - 1));
}

} // namespace mahjong

#endif /* MAHJONG_CPP_UTILS */
