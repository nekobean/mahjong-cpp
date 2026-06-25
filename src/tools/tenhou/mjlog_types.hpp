#ifndef MAHJONG_CPP_TOOLS_TENHOU_MJLOG_TYPES
#define MAHJONG_CPP_TOOLS_TENHOU_MJLOG_TYPES

#include <string>
#include <variant>
#include <vector>

#include "mahjong/types/types.hpp"
#include "mjlog_constants.hpp"

namespace mahjong::tools::tenhou
{

struct MjlogShuffleEvent
{
    std::string seed;
    std::string ref;
};

struct MjlogGoEvent
{
    int type;
    int lobby;
};

struct MjlogUnEvent
{
    std::vector<std::string> names;
    std::vector<int> dan;
    std::vector<double> rate;
    std::vector<std::string> sx;
};

struct MjlogTaikyokuEvent
{
    int oya;
};

struct MjlogInitEvent
{
    std::vector<int> seed;
    std::vector<int> ten;
    int oya;
    std::vector<std::vector<int>> hands;
};

struct MjlogDrawEvent
{
    int player;
    int tile136;
};

struct MjlogDiscardEvent
{
    int player;
    int tile136;
};

struct MjlogMeldEvent
{
    int who;
    int m;
};

struct MjlogDoraEvent
{
    int hai;
};

struct MjlogByeEvent
{
    int who;
};

struct MjlogReachEvent
{
    int who;
    int step;
    std::vector<int> ten;
};

struct MjlogAgariEvent
{
    std::vector<int> ba;
    std::vector<int> hai;
    std::vector<int> m;
    int machi;
    std::vector<int> ten;
    std::vector<int> yaku;
    std::vector<int> yakuman;
    std::vector<int> dora_hai;
    std::vector<int> dora_hai_ura;
    int who;
    int from_who;
    std::vector<int> sc;
    std::vector<int> owari;
};

struct MjlogRyukyokuEvent
{
    std::vector<int> ba;
    std::vector<int> sc;
    std::vector<std::vector<int>> hands;
    std::string type;
    std::vector<int> owari;
};

struct MjlogUnknownEvent
{
    std::string name;
};

using MjlogEvent =
    std::variant<MjlogShuffleEvent, MjlogGoEvent, MjlogUnEvent, MjlogTaikyokuEvent,
                 MjlogInitEvent, MjlogDrawEvent, MjlogDiscardEvent, MjlogMeldEvent,
                 MjlogDoraEvent, MjlogByeEvent, MjlogReachEvent, MjlogAgariEvent,
                 MjlogRyukyokuEvent, MjlogUnknownEvent>;

struct Mjlog
{
    std::string ver;
    std::string source_file;
    std::vector<MjlogEvent> events;
};

struct GameMeta
{
    std::string source_file;
    std::string version;
};

struct TableConfig
{
    int game_mode;
    RuleFlags rule_flags;
    int game_length;
    GameSpeed game_speed;
    TableLevel table_level;
};

struct PlayerProfile
{
    int id;
    std::string name;
    Rank rank;
    double rate;
    Gender gender;
};

struct GameRecord
{
    GameMeta meta;
    TableConfig table;
    std::vector<PlayerProfile> players;
    std::vector<RoundRecord> rounds;
};

} // namespace mahjong::tools::tenhou

#endif // MAHJONG_CPP_TOOLS_TENHOU_MJLOG_TYPES
