#ifndef MAHJONG_CPP_TOOLS_TENHOU_REPLAY_BUILDER
#define MAHJONG_CPP_TOOLS_TENHOU_REPLAY_BUILDER

#include "mjlog_types.hpp"

namespace mahjong::tools::tenhou
{

/**
 * @brief Builds a game record by applying parsed mjlog events in order.
 * @param log Parsed mjlog event sequence.
 * @return Reconstructed game record.
 */
GameRecord build_replay(const Mjlog &log);

} // namespace mahjong::tools::tenhou

#endif // MAHJONG_CPP_TOOLS_TENHOU_REPLAY_BUILDER
