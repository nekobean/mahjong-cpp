#ifndef MAHJONG_CPP_ROUND_RECORD_HPP
#define MAHJONG_CPP_ROUND_RECORD_HPP

#include <vector>

#include "player_state.hpp"
#include "round_event.hpp"
#include "round_result.hpp"
#include "round_state.hpp"
#include "table_state.hpp"

namespace mahjong
{

struct RoundSnapshot
{
    RoundState round;
    TableState table;
    std::vector<PlayerState> players;
};

struct RoundRecord
{
    RoundSnapshot initial;
    RoundSnapshot last;
    std::vector<RoundEvent> events;
    std::vector<RoundResult> results;
};

} // namespace mahjong

#endif // MAHJONG_CPP_ROUND_RECORD_HPP
