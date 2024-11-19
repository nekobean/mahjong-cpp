#include <fstream>

#include <boost/dll.hpp>
#include <boost/range/iterator_range.hpp>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

#include "mahjong/core/string.hpp"
#include "mahjong/json_parser.hpp"
#include "mahjong/mahjong.hpp"
//#include "naiveexpectedvaluecalculator.hpp"

#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include <catch2/catch.hpp>

using namespace mahjong;
namespace fs = boost::filesystem;

//#define TEST_NAIVE

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

bool load_input_data(std::vector<RequestData> &req_list,
                     std::vector<std::string> &paths)
{
    paths.clear();
    req_list.clear();

    fs::path requests_dir = fs::path(CMAKE_TESTCASE_DIR) / "requests";

    for (const auto &entry :
         boost::make_iterator_range(fs::directory_iterator(requests_dir))) {
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
    for (const auto &entry :
         boost::make_iterator_range(fs::directory_iterator(requests_dir))) {
        paths.push_back(entry.path().string());
    }
    std::sort(paths.begin(), paths.end());

    for (const auto &req_path : paths) {
        RequestData req_data;
        load_input_data(req_path, req_data);
        spdlog::info(to_mpsz(req_data.hand.counts));

        DiscardResponseData res_data = create_discard_response(req_data);

        fs::path res_path = fs::path(req_path).parent_path().parent_path() /
                            "responses" / fs::path(req_path).filename();

        rapidjson::Document res_doc(rapidjson::kObjectType);

        rapidjson::Value syanten_value(rapidjson::kObjectType);
        syanten_value.AddMember("syanten", res_data.syanten, res_doc.GetAllocator());
        syanten_value.AddMember("normal", res_data.normal_syanten,
                                res_doc.GetAllocator());
        syanten_value.AddMember("tiitoi", res_data.tiitoi_syanten,
                                res_doc.GetAllocator());
        syanten_value.AddMember("kokusi", res_data.kokusi_syanten,
                                res_doc.GetAllocator());
        res_doc.AddMember("result_type", 1, res_doc.GetAllocator());
        res_doc.AddMember("syanten", syanten_value, res_doc.GetAllocator());
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
        REQUIRE(std::get<0>(expected.required_tiles[i]) ==
                std::get<0>(actual.required_tiles[i]));
        REQUIRE(std::get<1>(expected.required_tiles[i]) ==
                std::get<1>(actual.required_tiles[i]));
    }
    //spdlog::info("{}", Tile::Name.at(expected.tile));
    for (size_t i = 0; i < 17; ++i) {
        // if (std::abs(expected.tenpai_probs[i] - actual.tenpai_probs[i]) > 0.05) {
        //     spdlog::info("{:.2f}->{:.2f} {:.2f}->{:.2f} {:.2f}->{:.2f}",
        //                  expected.tenpai_probs[i], actual.tenpai_probs[i],
        //                  expected.win_probs[i], actual.win_probs[i],
        //                  expected.exp_values[i], actual.exp_values[i]);
        // }
        // spdlog::info("{} {:.2f}->{:.2f} {:.2f}->{:.2f} {:.2f}->{:.2f}", i,
        //              expected.tenpai_probs[i], actual.tenpai_probs[i],
        //              expected.win_probs[i], actual.win_probs[i], expected.exp_values[i],
        //              actual.exp_values[i]);
        REQUIRE(expected.tenpai_probs[i] == Approx(actual.tenpai_probs[i]));
        REQUIRE(expected.win_probs[i] == Approx(actual.win_probs[i]));
        REQUIRE(expected.exp_values[i] == Approx(actual.exp_values[i]));
    }
    REQUIRE(expected.syanten_down == actual.syanten_down);
}

// DiscardResponseData create_discard_response_navie(const RequestData &req)
// {
//     ScoreCalculator score_calc;
//     NaiveExpectedValueCalculator exp_value_calc;

//     auto begin = std::chrono::steady_clock::now();

//     // 点数計算の設定
//     score_calc.set_wind(req.bakaze);
//     score_calc.set_self_wind(req.zikaze);
//     score_calc.set_bonus_stick(0);
//     score_calc.set_deposit_stick(0);
//     score_calc.set_dora_indicators(req.dora_indicators);

//     // 各打牌を分析する。
//     auto [success, candidates] =
//         exp_value_calc.calc(req.hand, score_calc, req.dora_indicators, req.syanten_type,
//                             req.turn, req.flag);

//     auto end = std::chrono::steady_clock::now();
//     auto elapsed_us =
//         std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();

//     DiscardResponseData res;
//     auto [_, syanten] = ShantenCalculator::calc(req.hand, req.syanten_type);
//     res.syanten = syanten;
//     res.normal_syanten = ShantenCalculator::calc_normal(req.hand);
//     res.tiitoi_syanten = ShantenCalculator::calc_tiitoi(req.hand);
//     res.kokusi_syanten = ShantenCalculator::calc_kokusi(req.hand);
//     res.time_us = elapsed_us;
//     res.candidates = candidates;

//     return res;
// }

// TEST_CASE("期待値計算がナイーブな実装と一致するか")
// {
//     fs::path path =
//         fs::path(CMAKE_TESTCASE_DIR) / "test_expected_calclation_input.json";

//     RequestData req_data;
//     if (!load_input_data(path.string(), req_data))
//         return;

// #ifdef TEST_NAIVE
//     spdlog::info("任意の巡目で結果が一致するかどうか");
//     for (int turn : {1, 5, 15, 16, 17}) {
//         req_data.turn = turn;

//         DiscardResponseData result1 = create_discard_response(req_data);
//         DiscardResponseData result2 = create_discard_response_navie(req_data);

//         spdlog::info("{} {} -> {}", double(result1.time_us) / double(result2.time_us),
//                      result1.time_us / 1000, result2.time_us / 1000);

//         // 聴牌確率、和了確率、期待値が一致しているかどうか
//         for (size_t i = 0; i < result1.candidates.size(); ++i) {
//             int t = req_data.turn - 1;
//             REQUIRE(result1.candidates[i].win_probs[t] ==
//                     Approx(result2.candidates[i].win_probs[t]));
//             REQUIRE(result1.candidates[i].exp_values[t] ==
//                     Approx(result2.candidates[i].exp_values[t]));
//             REQUIRE(result1.candidates[i].tenpai_probs[t] ==
//                     Approx(result2.candidates[i].tenpai_probs[t]));
//         }
//     }

//     spdlog::info("任意のフラグで結果が一致するかどうか");
//     for (int flag = 0; flag < 255; ++flag) {
//         req_data.turn = 14;
//         req_data.flag = flag;

//         DiscardResponseData result1 = create_discard_response(req_data);
//         DiscardResponseData result2 = create_discard_response_navie(req_data);

//         spdlog::info("{} {} -> {}", double(result1.time_us) / double(result2.time_us),
//                      result1.time_us / 1000, result2.time_us / 1000);

//         // 聴牌確率、和了確率、期待値が一致しているかどうか
//         for (size_t i = 0; i < result1.candidates.size(); ++i) {
//             int t = req_data.turn - 1;
//             REQUIRE(result1.candidates[i].win_probs[t] ==
//                     Approx(result2.candidates[i].win_probs[t]));
//             REQUIRE(result1.candidates[i].exp_values[t] ==
//                     Approx(result2.candidates[i].exp_values[t]));
//             REQUIRE(result1.candidates[i].tenpai_probs[t] ==
//                     Approx(result2.candidates[i].tenpai_probs[t]));
//         }
//     }

//     spdlog::info("任意の場風で結果が一致するかどうか");
//     for (int kazehai = Tile::East; kazehai <= Tile::North; ++kazehai) {
//         req_data.turn = 14;
//         req_data.bakaze = kazehai;

//         DiscardResponseData result1 = create_discard_response(req_data);
//         DiscardResponseData result2 = create_discard_response_navie(req_data);

//         spdlog::info("{} {} -> {}", double(result1.time_us) / double(result2.time_us),
//                      result1.time_us / 1000, result2.time_us / 1000);

//         // 聴牌確率、和了確率、期待値が一致しているかどうか
//         for (size_t i = 0; i < result1.candidates.size(); ++i) {
//             int t = req_data.turn - 1;
//             REQUIRE(result1.candidates[i].win_probs[t] ==
//                     Approx(result2.candidates[i].win_probs[t]));
//             REQUIRE(result1.candidates[i].exp_values[t] ==
//                     Approx(result2.candidates[i].exp_values[t]));
//             REQUIRE(result1.candidates[i].tenpai_probs[t] ==
//                     Approx(result2.candidates[i].tenpai_probs[t]));
//         }
//     }

//     spdlog::info("任意の自風で結果が一致するかどうか");
//     for (int kazehai = Tile::East; kazehai <= Tile::North; ++kazehai) {
//         req_data.turn = 14;
//         req_data.zikaze = kazehai;

//         DiscardResponseData result1 = create_discard_response(req_data);
//         DiscardResponseData result2 = create_discard_response_navie(req_data);

//         spdlog::info("{} {} -> {}", double(result1.time_us) / double(result2.time_us),
//                      result1.time_us / 1000, result2.time_us / 1000);

//         // 聴牌確率、和了確率、期待値が一致しているかどうか
//         for (size_t i = 0; i < result1.candidates.size(); ++i) {
//             int t = req_data.turn - 1;
//             REQUIRE(result1.candidates[i].win_probs[t] ==
//                     Approx(result2.candidates[i].win_probs[t]));
//             REQUIRE(result1.candidates[i].exp_values[t] ==
//                     Approx(result2.candidates[i].exp_values[t]));
//             REQUIRE(result1.candidates[i].tenpai_probs[t] ==
//                     Approx(result2.candidates[i].tenpai_probs[t]));
//         }
//     }

//     spdlog::info("任意の枚数のドラ表示牌で結果が一致するかどうか");
//     std::vector<int> dora_indicators = {Tile::East, Tile::South, Tile::West, Tile::North,
//                                         Tile::Red};
//     for (size_t i = 0; i < dora_indicators.size(); ++i) {
//         req_data.turn = 14;
//         req_data.dora_indicators =
//             std::vector(dora_indicators.begin(), dora_indicators.begin() + i);

//         DiscardResponseData result1 = create_discard_response(req_data);
//         DiscardResponseData result2 = create_discard_response_navie(req_data);

//         spdlog::info("{} {} -> {}", double(result1.time_us) / double(result2.time_us),
//                      result1.time_us / 1000, result2.time_us / 1000);

//         // 聴牌確率、和了確率、期待値が一致しているかどうか
//         for (size_t i = 0; i < result1.candidates.size(); ++i) {
//             int t = req_data.turn - 1;
//             REQUIRE(result1.candidates[i].win_probs[t] ==
//                     Approx(result2.candidates[i].win_probs[t]));
//             REQUIRE(result1.candidates[i].exp_values[t] ==
//                     Approx(result2.candidates[i].exp_values[t]));
//             REQUIRE(result1.candidates[i].tenpai_probs[t] ==
//                     Approx(result2.candidates[i].tenpai_probs[t]));
//         }
//     }

//     spdlog::info("いくつかの手牌で結果が一致するかどうか");
//     std::vector<RequestData> req_data_list;
//     std::vector<std::string> paths;
//     if (!load_input_data(req_data_list, paths))
//         return;

//     for (size_t i = 0; i < req_data_list.size(); ++i) {
//         RequestData req_data = req_data_list[i];
//         spdlog::info("{} 手牌: {}", i, req_data.hand.to_string());

//         req_data.turn = 14;

//         DiscardResponseData result1 = create_discard_response(req_data);
//         DiscardResponseData result2 = create_discard_response_navie(req_data);

//         spdlog::info("{} {} -> {}", double(result1.time_us) / double(result2.time_us),
//                      result1.time_us / 1000, result2.time_us / 1000);

//         // 聴牌確率、和了確率、期待値が一致しているかどうか
//         for (size_t i = 0; i < result1.candidates.size(); ++i) {
//             int t = req_data.turn - 1;
//             REQUIRE(result1.candidates[i].win_probs[t] ==
//                     Approx(result2.candidates[i].win_probs[t]));
//             REQUIRE(result1.candidates[i].exp_values[t] ==
//                     Approx(result2.candidates[i].exp_values[t]));
//             REQUIRE(result1.candidates[i].tenpai_probs[t] ==
//                     Approx(result2.candidates[i].tenpai_probs[t]));
//         }
//     }
// #endif
// }

TEST_CASE("期待値計算の計算時間")
{
    // データを更新する場合
    // write_output_data();

    fs::path response_dir = fs::path(CMAKE_TESTCASE_DIR) / "responses";

    std::vector<RequestData> req_data_list;
    std::vector<std::string> paths;
    if (!load_input_data(req_data_list, paths))
        return;

    for (size_t i = 0; i < req_data_list.size(); ++i) {
        if (to_mpsz(req_data_list[i].hand.counts) == "123m067789p22223s") {
            continue;
        }
        const RequestData &req_data = req_data_list[i];
        const std::string &req_path = paths[i];
        fs::path res_path = response_dir / fs::path(paths[i]).filename();

        spdlog::info(to_mpsz(req_data_list[i].hand.counts));
        DiscardResponseData actual = create_discard_response(req_data);
        DiscardResponseData expected;
        if (!load_output_data(res_path.string(), expected))
            return;

        REQUIRE(actual.syanten == expected.syanten);
        REQUIRE(actual.normal_syanten == expected.normal_syanten);
        REQUIRE(actual.tiitoi_syanten == expected.tiitoi_syanten);
        REQUIRE(actual.kokusi_syanten == expected.kokusi_syanten);
        spdlog::info("{} {} -> {}", double(actual.time_us) / double(expected.time_us),
                     expected.time_us / 1000, actual.time_us / 1000);
        // REQUIRE(double(expected.time_us) / double(actual.time_us) < 1.1);

        for (size_t j = 0; j < actual.candidates.size(); ++j)
            test_candidate(expected.candidates[j], actual.candidates[j]);
    }
}
