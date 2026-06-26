#include "score_testcase_converter.hpp"

namespace mahjong::tools::tenhou
{

namespace
{

using JsonWriter = rapidjson::Writer<rapidjson::OStreamWrapper>;

std::vector<int> hand_to_tiles(const Hand &hand)
{
    std::vector<int> ret;
    for (int tile = 0; tile < static_cast<int>(hand.size()); ++tile) {
        for (int i = 0; i < hand[tile]; ++i) {
            ret.push_back(tile);
        }
    }
    return ret;
}

void write_int_array(JsonWriter &writer, const std::vector<int> &values)
{
    writer.StartArray();
    for (const int value : values) {
        writer.Int(value);
    }
    writer.EndArray();
}

void write_nullable_int(JsonWriter &writer, const std::optional<int> &value)
{
    if (value) {
        writer.Int(*value);
        return;
    }
    writer.Null();
}

void write_table_config(JsonWriter &writer, const mahjong::TableConfig &table)
{
    writer.StartObject();
    writer.Key("rule_flags");
    writer.Uint(table.rule_flags);
    writer.Key("game_mode");
    writer.Int(table.game_mode);
    writer.EndObject();
}

void write_round_state(JsonWriter &writer, const RoundState &round)
{
    writer.StartObject();
    writer.Key("round_wind");
    writer.Int(round.round_wind);
    writer.Key("round_number");
    writer.Int(round.round_number);
    writer.Key("honba");
    writer.Int(round.honba);
    writer.Key("dealer");
    writer.Int(round.dealer);
    writer.EndObject();
}

void write_table_state(JsonWriter &writer, const TableState &table)
{
    writer.StartObject();
    writer.Key("kyotaku");
    writer.Int(table.kyotaku);
    writer.Key("dora_indicators");
    write_int_array(writer, table.dora_indicators);
    writer.Key("uradora_indicators");
    write_int_array(writer, table.uradora_indicators);
    writer.EndObject();
}

void write_yaku_list(JsonWriter &writer, const std::vector<YakuEntry> &values)
{
    writer.StartArray();
    for (const auto &entry : values) {
        writer.StartArray();
        writer.Uint64(entry.yaku);
        writer.Int(entry.han);
        writer.EndArray();
    }
    writer.EndArray();
}

void write_melds(JsonWriter &writer, const std::vector<Meld> &melds)
{
    writer.StartArray();
    for (const auto &meld : melds) {
        writer.StartObject();
        writer.Key("type");
        writer.Int(meld.type);
        writer.Key("tiles");
        write_int_array(writer, meld.tiles);
        writer.Key("discarded_tile");
        writer.Int(meld.discarded_tile);
        writer.Key("from");
        writer.Int(meld.from);
        writer.EndObject();
    }
    writer.EndArray();
}

void write_player_state(JsonWriter &writer, const PlayerState &player)
{
    writer.StartObject();
    writer.Key("hand_tiles");
    write_int_array(writer, hand_to_tiles(player.hand));
    writer.Key("melds");
    write_melds(writer, player.melds);
    writer.Key("seat_wind");
    writer.Int(player.seat_wind);
    writer.Key("nuki_count");
    writer.Int(player.nuki_count);
    writer.Key("score");
    writer.Int(player.score);
    writer.EndObject();
}

} // namespace

ScoreTestcase convert_score_testcase(const GameRecord &game, const WinResult &result)
{
    ScoreTestcase testcase;
    testcase.source = game.meta.source_file;
    testcase.table_config = {game.table.rule_flags, game.table.game_mode};
    testcase.round_state = result.result_round;
    testcase.table_state = result.result_table;
    testcase.player_state = result.player;
    testcase.win.winner = result.winner;
    testcase.win.loser = result.loser;
    if (result.pao) {
        testcase.win.pao_player = result.pao->player;
    }
    testcase.win.winning_tile = result.winning_tile;
    testcase.win.win_flags = result.win_flags;
    testcase.expected.han = result.han;
    testcase.expected.fu = result.fu;
    testcase.expected.score_limit = result.score_limit;
    testcase.expected.yaku_list = result.yaku;
    testcase.expected.score_deltas = result.score_deltas;

    return testcase;
}

void write_score_testcase(JsonWriter &writer, const ScoreTestcase &t)
{
    writer.StartObject();
    writer.Key("schema_version");
    writer.Int(t.schema_version);
    writer.Key("source");
    writer.String(t.source.c_str());
    writer.Key("table_config");
    write_table_config(writer, t.table_config);
    writer.Key("round_state");
    write_round_state(writer, t.round_state);
    writer.Key("table_state");
    write_table_state(writer, t.table_state);
    writer.Key("player_state");
    write_player_state(writer, t.player_state);

    writer.Key("win");
    writer.StartObject();
    writer.Key("winner");
    writer.Int(t.win.winner);
    writer.Key("loser");
    write_nullable_int(writer, t.win.loser);
    writer.Key("pao_player");
    write_nullable_int(writer, t.win.pao_player);
    writer.Key("winning_tile");
    writer.Int(t.win.winning_tile);
    writer.Key("win_flags");
    writer.Int(t.win.win_flags);
    writer.EndObject();

    writer.Key("expected");
    writer.StartObject();
    writer.Key("han");
    writer.Int(t.expected.han);
    writer.Key("fu");
    writer.Int(t.expected.fu);
    writer.Key("score_limit");
    writer.Int(t.expected.score_limit);
    writer.Key("yaku_list");
    write_yaku_list(writer, t.expected.yaku_list);
    writer.Key("score_deltas");
    write_int_array(writer, t.expected.score_deltas);
    writer.EndObject();

    writer.EndObject();
}

} // namespace mahjong::tools::tenhou
