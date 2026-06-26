#include <optional>
#include <string>
#include <vector>

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>

#include <boost/filesystem.hpp>
#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include <catch2/catch.hpp>

#include "mahjong/mahjong.hpp"
#include "tools/score_testcase/score_testcase_converter.hpp"

using namespace mahjong;
using mahjong::tools::tenhou::ScoreTestcase;
using mahjong::tools::tenhou::ScoreTestcaseExpected;
using mahjong::tools::tenhou::ScoreTestcaseWin;

const YakuValueTable &tenhou_yaku_value_table()
{
    // 天鳳は、四暗刻単騎、大四喜、純正九蓮宝燈、国士無双13面待ちはシングル役満
    static const YakuValueTable table = [] {
        YakuValueTable ret = ScoreCalculator::default_yaku_value_table();
        for (auto &entry : ret.yakuman_multipliers) {
            if (entry.yaku == Yaku::SingleWaitFourConcealedTriplets ||
                entry.yaku == Yaku::BigFourWinds || entry.yaku == Yaku::TrueNineGates ||
                entry.yaku == Yaku::ThirteenWaitThirteenOrphans) {
                entry.multiplier = 1;
            }
        }
        return ret;
    }();
    return table;
}

std::vector<int> parse_int_array(const rapidjson::Value &value)
{
    std::vector<int> ret;
    for (const auto &x : value.GetArray()) {
        ret.push_back(x.GetInt());
    }
    return ret;
}

std::vector<Meld> parse_melds(const rapidjson::Value &value)
{
    std::vector<Meld> ret;
    for (const auto &x : value.GetArray()) {
        Meld meld;
        meld.type = x["type"].GetInt();
        meld.tiles = parse_int_array(x["tiles"]);
        meld.discarded_tile = x["discarded_tile"].GetInt();
        meld.from = x["from"].GetInt();
        ret.push_back(meld);
    }
    return ret;
}

std::vector<YakuEntry> parse_yaku_list(const rapidjson::Value &value)
{
    std::vector<YakuEntry> ret;
    for (const auto &x : value.GetArray()) {
        ret.push_back({x.GetArray()[0].GetUint64(), x.GetArray()[1].GetInt()});
    }
    return ret;
}

void parse_case(const rapidjson::Value &v, ScoreTestcase &testcase)
{
    testcase.schema_version = v["schema_version"].GetInt();
    testcase.source = v["source"].GetString();

    const auto &table_config = v["table_config"];
    testcase.table_config.rule_flags = table_config["rule_flags"].GetUint();
    testcase.table_config.game_mode = table_config["game_mode"].GetInt();

    const auto &round_state = v["round_state"];
    testcase.round_state.round_wind = round_state["round_wind"].GetInt();
    testcase.round_state.round_number = round_state["round_number"].GetInt();
    testcase.round_state.honba = round_state["honba"].GetInt();
    testcase.round_state.dealer = round_state["dealer"].GetInt();

    const auto &table_state = v["table_state"];
    testcase.table_state.kyotaku = table_state["kyotaku"].GetInt();
    testcase.table_state.dora_indicators =
        parse_int_array(table_state["dora_indicators"]);
    testcase.table_state.uradora_indicators =
        parse_int_array(table_state["uradora_indicators"]);

    const auto &player_state = v["player_state"];
    testcase.player_state.hand = to_hand(parse_int_array(player_state["hand_tiles"]));
    testcase.player_state.melds = parse_melds(player_state["melds"]);
    testcase.player_state.seat_wind = player_state["seat_wind"].GetInt();
    testcase.player_state.nuki_count = player_state["nuki_count"].GetInt();
    testcase.player_state.score = player_state["score"].GetInt();

    const auto &win = v["win"];
    testcase.win.winner = win["winner"].GetInt();
    testcase.win.loser = win["loser"].IsNull()
                             ? std::nullopt
                             : std::optional<int>{win["loser"].GetInt()};
    testcase.win.pao_player = win["pao_player"].IsNull()
                                  ? std::nullopt
                                  : std::optional<int>{win["pao_player"].GetInt()};
    testcase.win.winning_tile = win["winning_tile"].GetInt();
    testcase.win.win_flags = win["win_flags"].GetInt();

    const auto &expected = v["expected"];
    testcase.expected.han = expected["han"].GetInt();
    testcase.expected.fu = expected["fu"].GetInt();
    testcase.expected.score_limit = expected["score_limit"].GetInt();
    testcase.expected.yaku_list = parse_yaku_list(expected["yaku_list"]);
    testcase.expected.score_deltas = parse_int_array(expected["score_deltas"]);
}

int num_players(const ScoreTestcase &testcase)
{
    return testcase.table_config.game_mode == GameMode::Sanma ? 3 : 4;
}

std::vector<int> to_pao_score_deltas(const ScoreResult &ret,
                                     const ScoreTestcase &testcase)
{
    const int n = num_players(testcase);
    std::vector<int> deltas(n, 0);
    deltas[testcase.win.winner] = ret.payments.front();
    const int loser = testcase.win.loser.value_or(testcase.win.winner);

    // ツモ
    if (testcase.win.winner == loser) {
        deltas[*testcase.win.pao_player] =
            -(ret.payments.front() - testcase.table_state.kyotaku * 1000);
        return deltas;
    }

    // ロン
    const int honba_payment =
        testcase.round_state.honba *
        (testcase.table_config.game_mode == GameMode::Sanma ? 200 : 300);
    const int base_payment = ret.payments[1] - honba_payment;
    const int split_payment = base_payment / 2;
    deltas[loser] = -split_payment;
    deltas[*testcase.win.pao_player] = -(split_payment + honba_payment);
    return deltas;
}

std::vector<int> to_score_deltas(const ScoreResult &ret, const ScoreTestcase &testcase)
{
    if (testcase.win.pao_player) {
        return to_pao_score_deltas(ret, testcase);
    }

    const int n = num_players(testcase);
    std::vector<int> deltas(n, 0);
    deltas[testcase.win.winner] = ret.payments[0];
    const int loser = testcase.win.loser.value_or(testcase.win.winner);

    // ロン
    if (testcase.win.winner != loser) {
        deltas[loser] = -ret.payments[1];
        return deltas;
    }

    // 親ツモ
    if (testcase.win.winner == testcase.round_state.dealer) {
        for (int player = 0; player < n; ++player) {
            if (player != testcase.win.winner) {
                deltas[player] = -ret.payments[1];
            }
        }
        return deltas;
    }

    // 子ツモ
    for (int player = 0; player < n; ++player) {
        if (player != testcase.win.winner) {
            deltas[player] = player == testcase.round_state.dealer ? -ret.payments[1]
                                                                   : -ret.payments[2];
        }
    }
    return deltas;
}

bool load_cases(const std::string &filename, std::vector<ScoreTestcase> &cases)
{
    cases.clear();

    boost::filesystem::path path =
        boost::filesystem::path(CMAKE_TESTCASE_DIR) / filename;

    std::FILE *fp = std::fopen(path.string().c_str(), "rb");
    if (!fp) {
        spdlog::error("Failed to open {}.", path.string());
        return false;
    }

    std::vector<char> buffer(100000000);
    rapidjson::FileReadStream is(fp, buffer.data(), buffer.size());
    rapidjson::Document doc;
    doc.ParseStream(is);
    std::fclose(fp);
    if (doc.HasParseError()) {
        spdlog::error("Failed to parse test data: {}.", path.string());
        return false;
    }

    REQUIRE(doc.IsArray());

    for (auto &v : doc.GetArray()) {
        ScoreTestcase testcase;
        parse_case(v, testcase);
        cases.push_back(testcase);
    }

    spdlog::info("Loaded {} test cases.", cases.size());

    return true;
}

void check_score_case(const ScoreTestcase &testcase,
                      const YakuValueTable &yaku_value_table =
                          ScoreCalculator::default_yaku_value_table())
{
    ScoreResult ret = ScoreCalculator::calc(testcase.table_config, testcase.round_state,
                                            testcase.table_state, testcase.player_state,
                                            testcase.win.winning_tile,
                                            testcase.win.win_flags, yaku_value_table);

    INFO(fmt::format("Source: {}", testcase.source));
    INFO(to_string(ret));

    REQUIRE(ret.success);
    REQUIRE(ret.err_msg.empty());

    std::string s;
    for (const auto &yaku : testcase.expected.yaku_list) {
        s += fmt::format(" {} {}翻\n", Yaku::name(yaku.yaku), yaku.han);
    }
    INFO(s);

    REQUIRE(ret.han == testcase.expected.han);                 // 飜
    REQUIRE(ret.fu == testcase.expected.fu);                   // 符
    REQUIRE(ret.score_limit == testcase.expected.score_limit); // 点数区分

    // 成立役
    REQUIRE(ret.yaku_list.size() == testcase.expected.yaku_list.size());
    for (size_t i = 0; i < ret.yaku_list.size(); ++i) {
        REQUIRE(ret.yaku_list[i].yaku == testcase.expected.yaku_list[i].yaku);
        REQUIRE(ret.yaku_list[i].han == testcase.expected.yaku_list[i].han);
    }

    REQUIRE(to_score_deltas(ret, testcase) == testcase.expected.score_deltas);
}

TEST_CASE("MJLOG Score Calculator")
{
    std::vector<ScoreTestcase> cases;
    REQUIRE(load_cases("test_score_mjlog.json", cases));

    for (const auto &testcase : cases) {
        check_score_case(testcase, tenhou_yaku_value_table());
    }
}
