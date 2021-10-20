#include <fstream>

#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include <boost/dll.hpp>
#include <boost/range/iterator_range.hpp>
#include <catch2/catch.hpp>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

#include "mahjong/json_parser.hpp"
#include "mahjong/mahjong.hpp"
#include "naiveexpectedvaluecalculator.hpp"

using namespace mahjong;
namespace fs = boost::filesystem;

bool load_input_data(const std::string &path, RequestData &req_data)
{
    std::ifstream ifs(path);
    if (!ifs.good()) {
        spdlog::error("Failed to open request data. {}", path);
        return false;
    }

    rapidjson::IStreamWrapper isw(ifs);
    rapidjson::Document req_doc;
    req_doc.ParseStream(isw);
    if (req_doc.HasParseError()) {
        spdlog::error("Failed to parse request data (invalid json format). {}", path);
        return false;
    }

    req_data = parse_request(req_doc);

    return true;
}

bool load_input_data(std::vector<RequestData> &req_list, std::vector<std::string> &paths)
{
    paths.clear();
    req_list.clear();

    fs::path requests_dir = fs::path(CMAKE_TESTCASE_DIR) / "requests";

    for (const auto &entry : boost::make_iterator_range(fs::directory_iterator(requests_dir))) {
        paths.push_back(entry.path().string());
    }
    std::sort(paths.begin(), paths.end());

    for (const auto &path : paths) {
        RequestData req_data;
        load_input_data(path, req_data);

        req_list.push_back(req_data);
    }

    return true;
}

bool load_output_data(const std::string &path, DiscardResponseData &res_data)
{
    std::ifstream ifs(path);
    if (!ifs.good()) {
        spdlog::error("Failed to open response data. {}", path);
        return false;
    }

    rapidjson::IStreamWrapper isw(ifs);
    rapidjson::Document res_doc;
    res_doc.ParseStream(isw);
    if (res_doc.HasParseError()) {
        spdlog::error("Failed to parse response data (invalid json format). {}", path);
        return false;
    }

    res_data = parse_response(res_doc);

    return true;
}

void write_output_data()
{
    fs::path requests_dir = fs::path(CMAKE_TESTCASE_DIR) / "requests";

    std::vector<std::string> paths;
    for (const auto &entry : boost::make_iterator_range(fs::directory_iterator(requests_dir))) {
        paths.push_back(entry.path().string());
    }
    std::sort(paths.begin(), paths.end());

    for (const auto &req_path : paths) {
        RequestData req_data;
        load_input_data(req_path, req_data);
        spdlog::info(req_data.hand.to_string());

        DiscardResponseData res_data = create_discard_response(req_data);

        fs::path res_path = fs::path(req_path).parent_path().parent_path() / "responses" /
                            fs::path(req_path).filename();

        rapidjson::Document res_doc(rapidjson::kObjectType);
        res_doc.AddMember("result_type", 1, res_doc.GetAllocator());
        res_doc.AddMember("syanten", res_data.syanten, res_doc.GetAllocator());
        res_doc.AddMember("time", res_data.time_us, res_doc.GetAllocator());
        res_doc.AddMember("candidates", rapidjson::kArrayType, res_doc.GetAllocator());
        for (const auto &candidate : res_data.candidates)
            res_doc["candidates"].PushBack(dump_candidate(candidate, res_doc),
                                           res_doc.GetAllocator());

        std::ofstream ofs(res_path.string());
        rapidjson::OStreamWrapper osw(ofs);
        rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);
        res_doc.Accept(writer);
    }

    exit(1);
}

void test_candidate(const Candidate &expected, const Candidate &actual)
{
    REQUIRE(expected.tile == actual.tile);
    for (size_t i = 0; i < expected.required_tiles.size(); ++i) {
        REQUIRE(std::get<0>(expected.required_tiles[i]) == std::get<0>(actual.required_tiles[i]));
        REQUIRE(std::get<1>(expected.required_tiles[i]) == std::get<1>(actual.required_tiles[i]));
    }
    for (size_t i = 0; i < 17; ++i) {
        REQUIRE(expected.tenpai_probs[i] == Approx(actual.tenpai_probs[i]));
        REQUIRE(expected.win_probs[i] == Approx(actual.win_probs[i]));
        REQUIRE(expected.exp_values[i] == Approx(actual.exp_values[i]));
    }
    REQUIRE(expected.syanten_down == actual.syanten_down);
}

DiscardResponseData create_discard_response_navie(const RequestData &req)
{
    ScoreCalculator score_calc;
    NaiveExpectedValueCalculator exp_value_calc;

    auto begin = std::chrono::steady_clock::now();

    // 向聴数を計算する。
    auto [syanten_type, syanten] = SyantenCalculator::calc(req.hand, req.syanten_type);

    // 点数計算の設定
    score_calc.set_bakaze(req.bakaze);
    score_calc.set_zikaze(req.zikaze);
    score_calc.set_num_tumibo(0);
    score_calc.set_num_kyotakubo(0);
    score_calc.set_dora_indicators(req.dora_indicators);

    // 各打牌を分析する。
    auto [success, candidates] = exp_value_calc.calc(req.hand, score_calc, req.dora_indicators,
                                                     req.syanten_type, req.turn, req.flag);

    auto end = std::chrono::steady_clock::now();
    auto elapsed_us = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();

    DiscardResponseData res;
    res.syanten = syanten;
    res.time_us = elapsed_us;
    res.candidates = candidates;

    return res;
}

TEST_CASE("期待値計算がナイーブな実装と一致するか")
{
    fs::path path = fs::path(CMAKE_TESTCASE_DIR) / "test_expected_calclation_input.json";

    RequestData req_data;
    if (!load_input_data(path.string(), req_data))
        return;

#ifdef TEST_NAIVE
    spdlog::info("任意の巡目で結果が一致するかどうか");
    for (int turn : {1, 5, 15, 16, 17}) {
        req_data.turn = turn;

        DiscardResponseData result1 = create_discard_response(req_data);
        DiscardResponseData result2 = create_discard_response_navie(req_data);

        spdlog::info("{} {} -> {}", double(result1.time_us) / double(result2.time_us),
                     result1.time_us / 1000, result2.time_us / 1000);

        // 聴牌確率、和了確率、期待値が一致しているかどうか
        for (size_t i = 0; i < result1.candidates.size(); ++i) {
            int t = req_data.turn - 1;
            REQUIRE(result1.candidates[i].win_probs[t] ==
                    Approx(result2.candidates[i].win_probs[t]));
            REQUIRE(result1.candidates[i].exp_values[t] ==
                    Approx(result2.candidates[i].exp_values[t]));
            REQUIRE(result1.candidates[i].tenpai_probs[t] ==
                    Approx(result2.candidates[i].tenpai_probs[t]));
        }
    }

    spdlog::info("任意のフラグで結果が一致するかどうか");
    for (int flag = 0; flag < 255; ++flag) {
        req_data.turn = 14;
        req_data.flag = flag;

        DiscardResponseData result1 = create_discard_response(req_data);
        DiscardResponseData result2 = create_discard_response_navie(req_data);

        spdlog::info("{} {} -> {}", double(result1.time_us) / double(result2.time_us),
                     result1.time_us / 1000, result2.time_us / 1000);

        // 聴牌確率、和了確率、期待値が一致しているかどうか
        for (size_t i = 0; i < result1.candidates.size(); ++i) {
            int t = req_data.turn - 1;
            REQUIRE(result1.candidates[i].win_probs[t] ==
                    Approx(result2.candidates[i].win_probs[t]));
            REQUIRE(result1.candidates[i].exp_values[t] ==
                    Approx(result2.candidates[i].exp_values[t]));
            REQUIRE(result1.candidates[i].tenpai_probs[t] ==
                    Approx(result2.candidates[i].tenpai_probs[t]));
        }
    }

    spdlog::info("任意の場風で結果が一致するかどうか");
    for (int kazehai = Tile::Ton; kazehai <= Tile::Pe; ++kazehai) {
        req_data.turn = 14;
        req_data.bakaze = kazehai;

        DiscardResponseData result1 = create_discard_response(req_data);
        DiscardResponseData result2 = create_discard_response_navie(req_data);

        spdlog::info("{} {} -> {}", double(result1.time_us) / double(result2.time_us),
                     result1.time_us / 1000, result2.time_us / 1000);

        // 聴牌確率、和了確率、期待値が一致しているかどうか
        for (size_t i = 0; i < result1.candidates.size(); ++i) {
            int t = req_data.turn - 1;
            REQUIRE(result1.candidates[i].win_probs[t] ==
                    Approx(result2.candidates[i].win_probs[t]));
            REQUIRE(result1.candidates[i].exp_values[t] ==
                    Approx(result2.candidates[i].exp_values[t]));
            REQUIRE(result1.candidates[i].tenpai_probs[t] ==
                    Approx(result2.candidates[i].tenpai_probs[t]));
        }
    }

    spdlog::info("任意の自風で結果が一致するかどうか");
    for (int kazehai = Tile::Ton; kazehai <= Tile::Pe; ++kazehai) {
        req_data.turn = 14;
        req_data.zikaze = kazehai;

        DiscardResponseData result1 = create_discard_response(req_data);
        DiscardResponseData result2 = create_discard_response_navie(req_data);

        spdlog::info("{} {} -> {}", double(result1.time_us) / double(result2.time_us),
                     result1.time_us / 1000, result2.time_us / 1000);

        // 聴牌確率、和了確率、期待値が一致しているかどうか
        for (size_t i = 0; i < result1.candidates.size(); ++i) {
            int t = req_data.turn - 1;
            REQUIRE(result1.candidates[i].win_probs[t] ==
                    Approx(result2.candidates[i].win_probs[t]));
            REQUIRE(result1.candidates[i].exp_values[t] ==
                    Approx(result2.candidates[i].exp_values[t]));
            REQUIRE(result1.candidates[i].tenpai_probs[t] ==
                    Approx(result2.candidates[i].tenpai_probs[t]));
        }
    }
#endif
    //
    // naive のほうでドラ表示牌が1枚以外のときにエラー発生
    //
    spdlog::info("任意の枚数のドラ表示牌で結果が一致するかどうか");
    std::vector<int> dora_indicators = {Tile::Ton, Tile::Nan, Tile::Sya, Tile::Pe, Tile::Tyun};
    for (int i = 1; i < 2; ++i) {
        req_data.turn = 14;
        req_data.dora_indicators =
            std::vector(dora_indicators.begin(), dora_indicators.begin() + i);
        std::cout << req_data.dora_indicators.size() << std::endl;

        DiscardResponseData result1 = create_discard_response(req_data);
        DiscardResponseData result2 = create_discard_response_navie(req_data);

        spdlog::info("{} {} -> {}", double(result1.time_us) / double(result2.time_us),
                     result1.time_us / 1000, result2.time_us / 1000);

        // 聴牌確率、和了確率、期待値が一致しているかどうか
        for (size_t i = 0; i < result1.candidates.size(); ++i) {
            int t = req_data.turn - 1;
            REQUIRE(result1.candidates[i].win_probs[t] ==
                    Approx(result2.candidates[i].win_probs[t]));
            REQUIRE(result1.candidates[i].exp_values[t] ==
                    Approx(result2.candidates[i].exp_values[t]));
            REQUIRE(result1.candidates[i].tenpai_probs[t] ==
                    Approx(result2.candidates[i].tenpai_probs[t]));
        }
    }

    spdlog::info("いくつかの手牌で結果が一致するかどうか");
    std::vector<RequestData> req_data_list;
    std::vector<std::string> paths;
    if (!load_input_data(req_data_list, paths))
        return;

    for (size_t i = 0; i < req_data_list.size(); ++i) {
        RequestData req_data = req_data_list[i];
        spdlog::info("{} 手牌: {}", i, req_data.hand.to_string());

        req_data.turn = 14;

        DiscardResponseData result1 = create_discard_response(req_data);
        DiscardResponseData result2 = create_discard_response_navie(req_data);

        spdlog::info("{} {} -> {}", double(result1.time_us) / double(result2.time_us),
                     result1.time_us / 1000, result2.time_us / 1000);

        // 聴牌確率、和了確率、期待値が一致しているかどうか
        for (size_t i = 0; i < result1.candidates.size(); ++i) {
            int t = req_data.turn - 1;
            REQUIRE(result1.candidates[i].win_probs[t] ==
                    Approx(result2.candidates[i].win_probs[t]));
            REQUIRE(result1.candidates[i].exp_values[t] ==
                    Approx(result2.candidates[i].exp_values[t]));
            REQUIRE(result1.candidates[i].tenpai_probs[t] ==
                    Approx(result2.candidates[i].tenpai_probs[t]));
        }
    }
}

TEST_CASE("期待値計算の計算時間")
{
    // データを更新する場合
    //write_output_data();

    fs::path response_dir = fs::path(CMAKE_TESTCASE_DIR) / "responses";

    std::vector<RequestData> req_data_list;
    std::vector<std::string> paths;
    if (!load_input_data(req_data_list, paths))
        return;

    for (size_t i = 0; i < req_data_list.size(); ++i) {
        const RequestData &req_data = req_data_list[i];
        const std::string &req_path = paths[i];
        fs::path res_path = response_dir / fs::path(paths[i]).filename();

        spdlog::info(req_data_list[i].hand.to_string());
        DiscardResponseData actual = create_discard_response(req_data);
        DiscardResponseData expected;
        if (!load_output_data(res_path.string(), expected))
            return;

        REQUIRE(actual.syanten == expected.syanten);
        spdlog::info("{} {} -> {}", double(actual.time_us) / double(expected.time_us),
                     expected.time_us / 1000, actual.time_us / 1000);
        // REQUIRE(double(expected.time_us) / double(actual.time_us) < 1.1);

        for (size_t j = 0; j < actual.candidates.size(); ++j)
            test_candidate(expected.candidates[j], actual.candidates[j]);
    }
}

TEST_CASE("Count Left Tiles")
{
    SECTION("赤ドラなし")
    {
        Hand hand({Tile::Manzu1, Tile::Manzu2, Tile::Manzu3, Tile::Manzu3, Tile::Manzu4,
                   Tile::Manzu9, Tile::Pinzu3, Tile::Pinzu6, Tile::Pinzu8, Tile::Pinzu8,
                   Tile::Sozu1, Tile::Sozu2, Tile::Sozu4, Tile::Sozu5});
        std::vector<int> dora_indicators = {Tile::Ton};

        std::vector<int> actual = ExpectedValueCalculator::count_left_tiles(hand, dora_indicators);
        std::vector<int> expected = {3, 3, 2, 3, 4, 4, 4, 4, 3, // 萬子
                                     4, 4, 3, 4, 4, 3, 4, 2, 4, // 筒子
                                     3, 3, 4, 3, 3, 4, 4, 4, 4, // 索子
                                     3, 4, 4, 4, 4, 4, 4,       // 字牌
                                     1, 1, 1};

        for (size_t i = 0; i < 37; ++i)
            REQUIRE(actual[i] == expected[i]);
    }

    SECTION("赤五萬が手牌にある")
    {
        Hand hand({Tile::Manzu1, Tile::Manzu2, Tile::Manzu3, Tile::Manzu3, Tile::AkaManzu5,
                   Tile::Manzu9, Tile::Pinzu3, Tile::Pinzu6, Tile::Pinzu8, Tile::Pinzu8,
                   Tile::Sozu1, Tile::Sozu2, Tile::Sozu4, Tile::Sozu5});
        std::vector<int> dora_indicators = {Tile::Ton};

        std::vector<int> actual = ExpectedValueCalculator::count_left_tiles(hand, dora_indicators);
        std::vector<int> expected = {3, 3, 2, 4, 3, 4, 4, 4, 3, // 萬子
                                     4, 4, 3, 4, 4, 3, 4, 2, 4, // 筒子
                                     3, 3, 4, 3, 3, 4, 4, 4, 4, // 索子
                                     3, 4, 4, 4, 4, 4, 4,       // 字牌
                                     0, 1, 1};

        for (size_t i = 0; i < 37; ++i)
            REQUIRE(actual[i] == expected[i]);
    }

    SECTION("赤五筒が手牌にある")
    {
        Hand hand({Tile::Manzu1, Tile::Manzu2, Tile::Manzu3, Tile::Manzu3, Tile::Manzu4,
                   Tile::Manzu9, Tile::Pinzu3, Tile::AkaPinzu5, Tile::Pinzu8, Tile::Pinzu8,
                   Tile::Sozu1, Tile::Sozu2, Tile::Sozu4, Tile::Sozu5});
        std::vector<int> dora_indicators = {Tile::Ton};

        std::vector<int> actual = ExpectedValueCalculator::count_left_tiles(hand, dora_indicators);
        std::vector<int> expected = {3, 3, 2, 3, 4, 4, 4, 4, 3, // 萬子
                                     4, 4, 3, 4, 3, 4, 4, 2, 4, // 筒子
                                     3, 3, 4, 3, 3, 4, 4, 4, 4, // 索子
                                     3, 4, 4, 4, 4, 4, 4,       // 字牌
                                     1, 0, 1};

        for (size_t i = 0; i < 37; ++i)
            REQUIRE(actual[i] == expected[i]);
    }

    SECTION("赤五索が手牌にある")
    {
        Hand hand({Tile::Manzu1, Tile::Manzu2, Tile::Manzu3, Tile::Manzu3, Tile::Manzu4,
                   Tile::Manzu9, Tile::Pinzu3, Tile::Pinzu6, Tile::Pinzu8, Tile::Pinzu8,
                   Tile::Sozu1, Tile::Sozu2, Tile::Sozu4, Tile::AkaSozu5});
        std::vector<int> dora_indicators = {Tile::Ton};

        std::vector<int> actual = ExpectedValueCalculator::count_left_tiles(hand, dora_indicators);
        std::vector<int> expected = {3, 3, 2, 3, 4, 4, 4, 4, 3, // 萬子
                                     4, 4, 3, 4, 4, 3, 4, 2, 4, // 筒子
                                     3, 3, 4, 3, 3, 4, 4, 4, 4, // 索子
                                     3, 4, 4, 4, 4, 4, 4,       // 字牌
                                     1, 1, 0};

        for (size_t i = 0; i < 37; ++i)
            REQUIRE(actual[i] == expected[i]);
    }

    SECTION("赤五萬が副露ブロックにある")
    {
        MeldedBlock meld(
            {MeldType::Ankan, {Tile::AkaManzu5, Tile::Manzu5, Tile::Manzu5, Tile::Manzu5}});
        Hand hand({Tile::Manzu3, Tile::Manzu4, Tile::Manzu9, Tile::Pinzu3, Tile::Pinzu6,
                   Tile::Pinzu8, Tile::Pinzu8, Tile::Sozu1, Tile::Sozu2, Tile::Sozu4, Tile::Sozu5},
                  {meld});
        std::vector<int> dora_indicators = {Tile::Ton};

        std::vector<int> actual = ExpectedValueCalculator::count_left_tiles(hand, dora_indicators);
        std::vector<int> expected = {4, 4, 3, 3, 0, 4, 4, 4, 3, // 萬子
                                     4, 4, 3, 4, 4, 3, 4, 2, 4, // 筒子
                                     3, 3, 4, 3, 3, 4, 4, 4, 4, // 索子
                                     3, 4, 4, 4, 4, 4, 4,       // 字牌
                                     0, 1, 1};

        for (size_t i = 0; i < 37; ++i)
            REQUIRE(actual[i] == expected[i]);
    }

    SECTION("赤五筒が副露ブロックにある")
    {
        MeldedBlock meld(
            {MeldType::Ankan, {Tile::AkaPinzu5, Tile::Pinzu5, Tile::Pinzu5, Tile::Pinzu5}});
        Hand hand({Tile::Manzu3, Tile::Manzu4, Tile::Manzu9, Tile::Pinzu3, Tile::Pinzu6,
                   Tile::Pinzu8, Tile::Pinzu8, Tile::Sozu1, Tile::Sozu2, Tile::Sozu4, Tile::Sozu4},
                  {meld});
        std::vector<int> dora_indicators = {Tile::Ton};

        std::vector<int> actual = ExpectedValueCalculator::count_left_tiles(hand, dora_indicators);
        std::vector<int> expected = {4, 4, 3, 3, 4, 4, 4, 4, 3, // 萬子
                                     4, 4, 3, 4, 0, 3, 4, 2, 4, // 筒子
                                     3, 3, 4, 2, 4, 4, 4, 4, 4, // 索子
                                     3, 4, 4, 4, 4, 4, 4,       // 字牌
                                     1, 0, 1};

        for (size_t i = 0; i < 37; ++i)
            REQUIRE(actual[i] == expected[i]);
    }

    SECTION("赤五索が副露ブロックにある")
    {
        MeldedBlock meld(
            {MeldType::Ankan, {Tile::AkaSozu5, Tile::Sozu5, Tile::Sozu5, Tile::Sozu5}});
        Hand hand({Tile::Manzu3, Tile::Manzu4, Tile::Manzu9, Tile::Pinzu3, Tile::Pinzu6,
                   Tile::Pinzu8, Tile::Pinzu8, Tile::Sozu1, Tile::Sozu2, Tile::Sozu4, Tile::Sozu4},
                  {meld});
        std::vector<int> dora_indicators = {Tile::Ton};

        std::vector<int> actual = ExpectedValueCalculator::count_left_tiles(hand, dora_indicators);
        std::vector<int> expected = {4, 4, 3, 3, 4, 4, 4, 4, 3, // 萬子
                                     4, 4, 3, 4, 4, 3, 4, 2, 4, // 筒子
                                     3, 3, 4, 2, 0, 4, 4, 4, 4, // 索子
                                     3, 4, 4, 4, 4, 4, 4,       // 字牌
                                     1, 1, 0};

        for (size_t i = 0; i < 37; ++i)
            REQUIRE(actual[i] == expected[i]);
    }

    SECTION("赤五萬がドラ表示牌にある")
    {
        Hand hand({Tile::Manzu1, Tile::Manzu2, Tile::Manzu3, Tile::Manzu3, Tile::Manzu4,
                   Tile::Manzu9, Tile::Pinzu3, Tile::Pinzu6, Tile::Pinzu8, Tile::Pinzu8,
                   Tile::Sozu1, Tile::Sozu2, Tile::Sozu4, Tile::Sozu5});
        std::vector<int> dora_indicators = {Tile::Ton, Tile::AkaManzu5};

        std::vector<int> actual = ExpectedValueCalculator::count_left_tiles(hand, dora_indicators);
        std::vector<int> expected = {3, 3, 2, 3, 3, 4, 4, 4, 3, // 萬子
                                     4, 4, 3, 4, 4, 3, 4, 2, 4, // 筒子
                                     3, 3, 4, 3, 3, 4, 4, 4, 4, // 索子
                                     3, 4, 4, 4, 4, 4, 4,       // 字牌
                                     0, 1, 1};

        for (size_t i = 0; i < 37; ++i)
            REQUIRE(actual[i] == expected[i]);
    }

    SECTION("赤五筒がドラ表示牌にある")
    {
        Hand hand({Tile::Manzu1, Tile::Manzu2, Tile::Manzu3, Tile::Manzu3, Tile::Manzu4,
                   Tile::Manzu9, Tile::Pinzu3, Tile::Pinzu6, Tile::Pinzu8, Tile::Pinzu8,
                   Tile::Sozu1, Tile::Sozu2, Tile::Sozu4, Tile::Sozu5});
        std::vector<int> dora_indicators = {Tile::Ton, Tile::AkaPinzu5};

        std::vector<int> actual = ExpectedValueCalculator::count_left_tiles(hand, dora_indicators);
        std::vector<int> expected = {3, 3, 2, 3, 4, 4, 4, 4, 3, // 萬子
                                     4, 4, 3, 4, 3, 3, 4, 2, 4, // 筒子
                                     3, 3, 4, 3, 3, 4, 4, 4, 4, // 索子
                                     3, 4, 4, 4, 4, 4, 4,       // 字牌
                                     1, 0, 1};

        for (size_t i = 0; i < 37; ++i)
            REQUIRE(actual[i] == expected[i]);
    }

    SECTION("赤五索がドラ表示牌にある")
    {
        Hand hand({Tile::Manzu1, Tile::Manzu2, Tile::Manzu3, Tile::Manzu3, Tile::Manzu4,
                   Tile::Manzu9, Tile::Pinzu3, Tile::Pinzu6, Tile::Pinzu8, Tile::Pinzu8,
                   Tile::Sozu1, Tile::Sozu2, Tile::Sozu4, Tile::Sozu5});
        std::vector<int> dora_indicators = {Tile::Ton, Tile::AkaSozu5};

        std::vector<int> actual = ExpectedValueCalculator::count_left_tiles(hand, dora_indicators);
        std::vector<int> expected = {3, 3, 2, 3, 4, 4, 4, 4, 3, // 萬子
                                     4, 4, 3, 4, 4, 3, 4, 2, 4, // 筒子
                                     3, 3, 4, 3, 2, 4, 4, 4, 4, // 索子
                                     3, 4, 4, 4, 4, 4, 4,       // 字牌
                                     1, 1, 0};

        for (size_t i = 0; i < 37; ++i)
            REQUIRE(actual[i] == expected[i]);
    }
}

TEST_CASE("create_prob_table")
{
    SECTION("tumo_prob_table")
    {
        ExpectedValueCalculator caclulator;

        int N = 121; // 残り牌の合計枚数
        caclulator.create_prob_table(N);
        for (int r = 0; r <= 4; ++r) {
            for (int i = 0; i < 17; ++i) {
                double expected = double(r) / double(N - i);
                REQUIRE(caclulator.tumo_prob_table_[r][i] == Approx(expected));
            }
        }
    }

    SECTION("not_tumo_prob_table")
    {
        ExpectedValueCalculator caclulator;

        for (int N = 100; N < 121; ++N) { // N は残り牌の合計枚数
            caclulator.create_prob_table(N);

            // for (int R = 0; R < N; ++R) {
            //     for (int i = 0; i < 17; ++i)
            //         std::cout << fmt::format("{:4.2f} ", caclulator.not_tumo_prob_table_[R][i]);
            //     std::cout << std::endl;
            // }

            for (int R = 0; R < N; ++R) {
                for (int i = 0; i < 17; ++i) { // t は現在の巡目
                    // i 巡目までに有効牌以外を連続して引く確率
                    double not_tumo_prob = 1;
                    for (int j = 0; j < i && N - R - j > 0; ++j) {
                        REQUIRE(N - R - j >= 0);
                        REQUIRE(N - j > 0);
                        not_tumo_prob *= double(N - R - j) / double(N - j);
                    }

                    if (caclulator.not_tumo_prob_table_[R][i] > 10e-12 && not_tumo_prob > 10e-12) {
                        INFO(fmt::format("R={}, i={}, {} {}", R, i,
                                         caclulator.not_tumo_prob_table_[R][i], not_tumo_prob));
                        REQUIRE(caclulator.not_tumo_prob_table_[R][i] == Approx(not_tumo_prob));
                    }
                }
            }

            for (int R = 1; R < N; ++R) {
                for (int t = 0; t < 17; ++t) { // t は現在の巡目
                    for (int i = t; i < 17 && N - R - i > 0; ++i) {
                        // i 巡目までに有効牌以外を連続して引く確率
                        double expected = 1;
                        for (int j = t; j < i; ++j)
                            expected *= double(N - R - j) / double(N - j);

                        double actual = caclulator.not_tumo_prob_table_[R][i] /
                                        caclulator.not_tumo_prob_table_[R][t];

                        REQUIRE(caclulator.not_tumo_prob_table_[R][t] > 0);
                        if (expected > 10e-12 && actual > 10e-12) {
                            INFO(fmt::format("R={}, i={}, t={}, {}", R, i, t,
                                             caclulator.not_tumo_prob_table_[R][t]));
                            REQUIRE(expected == Approx(actual));
                        }
                    }
                }
            }
        }
    }
}
