#ifndef MAHJONG_CPP_TABLE_CONFIG_HPP
#define MAHJONG_CPP_TABLE_CONFIG_HPP

#include "mahjong/types/constants.hpp"

namespace mahjong
{

/**
 * @brief Table configuration.
 */
struct TableConfig
{
    /*! Rule flags. */
    RuleFlags rule_flags = RuleFlag::Default;

    /*! Mahjong game mode. */
    int game_mode = GameMode::Yonma;
};

} // namespace mahjong

#endif // MAHJONG_CPP_TABLE_CONFIG_HPP
