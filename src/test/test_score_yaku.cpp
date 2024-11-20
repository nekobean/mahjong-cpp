#include <fstream>
#include <iostream>

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/writer.h>

#include <boost/dll.hpp>
#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include <catch2/catch.hpp>

#include "mahjong/core/score_calculator.hpp"
#include "mahjong/mahjong.hpp"

using namespace mahjong;

struct TestCase
{
    // 場況
    int rules;
    int wind;
    int honba;
    int kyotaku;
    std::vector<int> dora_tiles;
    std::vector<int> uradora_tiles;

    // 入力
    MyPlayer player;
    int win_tile;
    int win_flag;

    // 結果
    int han;
    int hu;
    int score_title;
    std::vector<std::tuple<YakuList, int>> yaku_list;

    // 点数
    int win_player;
    int lose_player;
    std::vector<int> score;

    std::string url;
};

/**
 * Load a test case from the specified file.
 *
 * @param filepath The path to the file containing the test case data.
 * @param cases list of test cases.
 * @return true if the test case is loaded successfully, false otherwise.
 */
bool load_cases(const std::string &filename, std::vector<TestCase> &cases)
{
    cases.clear();

    boost::filesystem::path path =
        boost::filesystem::path(CMAKE_TESTCASE_DIR) / filename;

    std::FILE *fp = std::fopen(path.string().c_str(), "rb");
    if (!fp) {
        spdlog::error("Failed to open {}.", path.string());
        return false;
    }

    char *buffer = new char[100000000];
    rapidjson::FileReadStream is(fp, buffer, sizeof(buffer));
    rapidjson::Document doc;
    doc.ParseStream(is);
    if (doc.HasParseError()) {
        spdlog::error("Failed to parse {}.", path.string());
        return false;
    }

    for (auto &v : doc.GetArray()) {
        TestCase testcase;

        testcase.wind = v["bakaze"].GetInt();
        testcase.honba = v["num_tumibo"].GetInt();
        testcase.kyotaku = v["num_kyotakubo"].GetInt();
        for (auto &x : v["dora_tiles"].GetArray())
            testcase.dora_tiles.push_back(x.GetInt());
        for (auto &x : v["uradora_tiles"].GetArray())
            testcase.uradora_tiles.push_back(x.GetInt());

        std::vector<int> tiles;
        std::vector<Meld> melded_blocks;
        for (auto &x : v["hand_tiles"].GetArray())
            tiles.push_back(x.GetInt());

        for (auto &x : v["melded_blocks"].GetArray()) {
            Meld melded_block;
            melded_block.type = x["type"].GetInt();
            melded_block.discarded_tile = x["discarded_tile"].GetInt();
            melded_block.from = x["from"].GetInt();

            for (auto &y : x["tiles"].GetArray())
                melded_block.tiles.push_back(y.GetInt());

            melded_blocks.push_back(melded_block);
        }

        MyPlayer player(tiles, melded_blocks, v["zikaze"].GetInt());
        testcase.player = player;
        testcase.win_tile = v["win_tile"].GetInt();
        testcase.win_flag = v["flag"].GetInt();

        testcase.han = v["han"].GetInt();
        testcase.hu = v["hu"].GetInt();
        testcase.score_title = v["score_title"].GetInt();

        for (auto &x : v["yaku_list"].GetArray()) {
            YakuList yaku = x.GetArray()[0].GetInt64();
            int han = x.GetArray()[1].GetInt();

            testcase.yaku_list.emplace_back(yaku, han);
        }

        testcase.win_player = v["win_player"].GetInt();
        testcase.lose_player = v["lose_player"].GetInt();

        for (auto &x : v["score"].GetArray())
            testcase.score.push_back(x.GetInt());

        testcase.url = v["url"].GetString();

        testcase.rules = 0;
        testcase.rules |= v["akadora"].GetBool() ? RuleFlag::RedDora : 0;
        testcase.rules |= v["kuitan"].GetBool() ? RuleFlag::OpenTanyao : 0;

        cases.push_back(testcase);
    }

    fclose(fp);
    delete buffer;

    spdlog::info("Loaded {} test cases.", cases.size());

    return true;
}

TEST_CASE("Score Calculator")
{
    std::vector<TestCase> cases;
    if (!load_cases("test_score_normal_yaku.json", cases))
        return;

    SECTION("Score Calculator")
    {
        for (const auto &testcase : cases) {
            // 設定
            Round params;
            params.wind = testcase.wind;
            params.self_wind = testcase.player.wind;
            params.honba = testcase.honba;
            params.kyotaku = testcase.kyotaku;
            params.dora_tiles = testcase.dora_tiles;
            params.uradora_tiles = testcase.uradora_tiles;
            params.rules = testcase.rules;

            // 計算
            Result ret = ScoreCalculator::calc(params, testcase.player,
                                               testcase.win_tile, testcase.win_flag);

            // 照合
            INFO(fmt::format("URL: {}", testcase.url));
            INFO(to_string(ret));
            std::string s;
            for (auto &[yaku, n] : testcase.yaku_list)
                s += fmt::format(" {} {}翻\n", Yaku::Name[yaku], n);
            INFO(s);
            REQUIRE(ret.han == testcase.han);                 // 飜
            REQUIRE(ret.fu == testcase.hu);                   // 符
            REQUIRE(ret.score_title == testcase.score_title); // タイトル
            // 成立役
            REQUIRE(ret.yaku_list.size() == testcase.yaku_list.size());
            for (size_t i = 0; i < ret.yaku_list.size(); ++i) {
                REQUIRE(std::get<0>(ret.yaku_list[i]) ==
                        std::get<0>(testcase.yaku_list[i]));
                REQUIRE(std::get<1>(ret.yaku_list[i]) ==
                        std::get<1>(testcase.yaku_list[i]));
            }

            // 点数の収支
            REQUIRE(ret.score.size() == testcase.score.size());
            for (size_t i = 0; i < ret.score.size(); ++i)
                REQUIRE(ret.score[i] == testcase.score[i]);
        }
    };

    BENCHMARK("Score Calculator")
    {
        for (const auto &testcase : cases) {
            // 設定
            Round params;
            params.wind = testcase.wind;
            params.self_wind = testcase.player.wind;
            params.honba = testcase.honba;
            params.kyotaku = testcase.kyotaku;
            params.dora_tiles = testcase.dora_tiles;
            params.uradora_tiles = testcase.uradora_tiles;
            params.rules = testcase.rules;

            // 計算
            Result ret = ScoreCalculator::calc(params, testcase.player,
                                               testcase.win_tile, testcase.win_flag);
        }
    };
}
