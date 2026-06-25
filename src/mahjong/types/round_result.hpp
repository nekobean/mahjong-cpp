#ifndef MAHJONG_CPP_ROUND_RESULT_HPP
#define MAHJONG_CPP_ROUND_RESULT_HPP

#include <optional>
#include <variant>
#include <vector>

#include "mahjong/types/constants.hpp"
#include "mahjong/types/player_state.hpp"
#include "mahjong/types/round_state.hpp"
#include "mahjong/types/score_result.hpp"
#include "mahjong/types/table_state.hpp"
#include "mahjong/types/tile.hpp"

namespace mahjong
{

/**
 * @brief Pao liability information.
 */
struct PaoInfo
{
    /*! Liable player index. */
    int player = PlayerIndex::Null;

    /*! Yaku that caused pao liability. */
    YakuFlags yaku = Yaku::None;
};

/**
 * @brief Win result in a game record.
 */
struct WinResult
{
    /*! Round state at the result. */
    RoundState result_round;

    /*! Table state at the result. */
    TableState result_table;

    /*! Winning player state at the result. */
    PlayerState player;

    /*! Winning player index. */
    int winner = PlayerIndex::Null;

    /*! Losing player index. Nullopt for tsumo. */
    std::optional<int> loser;

    /*! Winning tile. */
    int winning_tile = Tile::Null;

    /*! Win flags. */
    int win_flags = WinFlag::None;

    /*! Yaku entries. */
    std::vector<YakuEntry> yaku;

    /*! Total han. */
    int han = 0;

    /*! Total fu. */
    int fu = 0;

    /*! Score limit. */
    int score_limit = ScoreLimit::Null;

    /*! Score deltas for each player. */
    std::vector<int> score_deltas;

    /*! Liability payment information for pao. */
    std::optional<PaoInfo> pao;
};

/**
 * @brief Ryukyoku result in a game record.
 */
struct RyukyokuResult
{
    /*! Round state at the result. */
    RoundState result_round;

    /*! Table state at the result. */
    TableState result_table;

    /*! Ryukyoku type. */
    int type = RyukyokuType::Null;

    /*! Score deltas for each player. */
    std::vector<int> score_deltas;
};

using RoundResult = std::variant<WinResult, RyukyokuResult>;

} // namespace mahjong

#endif // MAHJONG_CPP_ROUND_RESULT_HPP
