#ifndef MAHJONG_CPP_TOOLS_TENHOU_SCORE_TESTCASE_CONVERTER
#define MAHJONG_CPP_TOOLS_TENHOU_SCORE_TESTCASE_CONVERTER

#include <optional>
#include <ostream>
#include <string>
#include <vector>

#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/writer.h>

#include "mahjong/types/types.hpp"
#include "tools/tenhou/mjlog_types.hpp"

namespace mahjong::tools::tenhou
{

struct ScoreTestcaseWin
{
    int winner;
    std::optional<int> loser;
    std::optional<int> pao_player;
    int winning_tile;
    int win_flags;
};

struct ScoreTestcaseExpected
{
    int han;
    int fu;
    int score_limit;
    std::vector<YakuEntry> yaku_list;
    std::vector<int> score_deltas;
};

struct ScoreTestcase
{
    int schema_version = 1;
    std::string source;
    mahjong::TableConfig table_config;
    RoundState round_state;
    TableState table_state;
    PlayerState player_state;
    ScoreTestcaseWin win;
    ScoreTestcaseExpected expected;
};

/**
 * @brief Converts a replay result into a score calculator test case.
 * @param game Reconstructed mjlog game record.
 * @param replay_result Win result to convert.
 * @param win_index Index in a consecutive AGARI group.
 * @return Converted test case.
 */
ScoreTestcase convert_score_testcase(const GameRecord &game, const WinResult &result,
                                     int win_index);

/**
 * @brief Writes a score calculator test case as JSON.
 * @param writer RapidJSON writer.
 * @param testcase Test case to write.
 */
void write_score_testcase(rapidjson::Writer<rapidjson::OStreamWrapper> &writer,
                          const ScoreTestcase &testcase);

} // namespace mahjong::tools::tenhou

#endif // MAHJONG_CPP_TOOLS_TENHOU_SCORE_TESTCASE_CONVERTER
