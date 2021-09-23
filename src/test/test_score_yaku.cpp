#include <fstream>
#include <iostream>

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/writer.h>

#include <boost/dll.hpp>
#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include <catch2/catch.hpp>

#include "mahjong/mahjong.hpp"

using namespace mahjong;

struct TestCase
{
    // 場況
    int bakaze;
    int zikaze;
    int num_tumibo;
    int num_kyotakubo;
    std::vector<int> dora_tiles;
    std::vector<int> uradora_tiles;

    // 入力
    Hand hand;
    int win_tile;
    int flag;

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

    bool enable_akadora;
    bool enable_kuitan;
};

/**
 * @brief 初期化する。
 *
 * @param[in] path パス
 * @param[out] table テーブル
 * @return 初期化に成功した場合は true、そうでない場合は false を返す。
 */
bool load_cases(const std::string &filename, std::vector<TestCase> &cases)
{
    cases.clear();

    boost::filesystem::path path = boost::dll::program_location().parent_path() / filename;

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

        testcase.bakaze = v["bakaze"].GetInt();
        testcase.zikaze = v["zikaze"].GetInt();
        testcase.num_tumibo = v["num_tumibo"].GetInt();
        testcase.num_kyotakubo = v["num_kyotakubo"].GetInt();
        for (auto &x : v["dora_tiles"].GetArray())
            testcase.dora_tiles.push_back(x.GetInt());
        for (auto &x : v["uradora_tiles"].GetArray())
            testcase.uradora_tiles.push_back(x.GetInt());

        std::vector<int> tiles;
        std::vector<MeldedBlock> melded_blocks;
        for (auto &x : v["hand_tiles"].GetArray())
            tiles.push_back(x.GetInt());

        for (auto &x : v["melded_blocks"].GetArray()) {
            MeldedBlock melded_block;
            melded_block.type = x["type"].GetInt();
            melded_block.discarded_tile = x["discarded_tile"].GetInt();
            melded_block.from = x["from"].GetInt();

            for (auto &y : x["tiles"].GetArray())
                melded_block.tiles.push_back(y.GetInt());

            melded_blocks.push_back(melded_block);
        }

        testcase.hand = Hand(tiles, melded_blocks);
        testcase.win_tile = v["win_tile"].GetInt();
        testcase.flag = v["flag"].GetInt();

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
        testcase.enable_akadora = v["akadora"].GetBool();
        testcase.enable_kuitan = v["kuitan"].GetBool();

        cases.push_back(testcase);
    }

    fclose(fp);
    delete buffer;

    return true;
}

TEST_CASE("一般役の点数計算")
{
    std::vector<TestCase> cases;
    if (!load_cases("test_score_normal_yaku.json", cases))
        return;

    ScoreCalculator score;

    BENCHMARK("一般役の点数計算")
    {
        for (const auto &testcase : cases) {
            // 設定
            score.set_bakaze(testcase.bakaze);
            score.set_zikaze(testcase.zikaze);
            score.set_num_tumibo(testcase.num_tumibo);
            score.set_num_kyotakubo(testcase.num_kyotakubo);
            score.set_dora_tiles(testcase.dora_tiles);
            score.set_uradora_tiles(testcase.uradora_tiles);
            score.set_rule(RuleFlag::AkaDora, testcase.enable_akadora);
            score.set_rule(RuleFlag::OpenTanyao, testcase.enable_kuitan);

            // 計算
            Result ret = score.calc(testcase.hand, testcase.win_tile, testcase.flag);
        }
    };

    SECTION("一般役の点数計算")
    {
        for (const auto &testcase : cases) {
            // 設定
            score.set_bakaze(testcase.bakaze);
            score.set_zikaze(testcase.zikaze);
            score.set_num_tumibo(testcase.num_tumibo);
            score.set_num_kyotakubo(testcase.num_kyotakubo);
            score.set_dora_tiles(testcase.dora_tiles);
            score.set_uradora_tiles(testcase.uradora_tiles);
            score.set_rule(RuleFlag::AkaDora, testcase.enable_akadora);
            score.set_rule(RuleFlag::OpenTanyao, testcase.enable_kuitan);

            // 計算
            Result ret = score.calc(testcase.hand, testcase.win_tile, testcase.flag);

            // 照合
            INFO(fmt::format("URL: {}", testcase.url));
            INFO(ret.to_string());

            REQUIRE(ret.han == testcase.han);                 // 飜
            REQUIRE(ret.fu == testcase.hu);                   // 符
            REQUIRE(ret.score_title == testcase.score_title); // タイトル
            // 成立役
            REQUIRE(ret.yaku_list.size() == testcase.yaku_list.size());
            // for (size_t i = 0; i < ret.yaku_list.size(); ++i) {
            //     REQUIRE(std::get<0>(ret.yaku_list[i]) ==
            //             std::get<0>(testcase.yaku_list[i]));
            //     REQUIRE(std::get<1>(ret.yaku_list[i]) ==
            //             std::get<1>(testcase.yaku_list[i]));
            // }

            // 点数の収支
            REQUIRE(ret.score.size() == testcase.score.size());
            for (size_t i = 0; i < ret.score.size(); ++i)
                REQUIRE(ret.score[i] == testcase.score[i]);
        }
    };
}
