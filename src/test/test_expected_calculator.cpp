#include <fstream>

#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include <boost/dll.hpp>
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

bool load_input_data(std::vector<RequestData> &req_list)
{
    req_list.clear();

    boost::filesystem::path path =
        boost::dll::program_location().parent_path() / "test_expected_calclation_input.json";

    // 入力データを読み込む。
    std::ifstream ifs(path.string());
    rapidjson::IStreamWrapper isw(ifs);
    rapidjson::Document req_doc;
    req_doc.ParseStream(isw);
    if (req_doc.HasParseError()) {
        std::cerr << "Failed to parse request data (invalid json format)." << std::endl;
        return false;
    }

    for (auto &x : req_doc.GetArray())
        req_list.push_back(parse_request(x));

    return true;
}

bool load_output_data(std::vector<DiscardResponseData> &res_list)
{
    res_list.clear();

    boost::filesystem::path path =
        boost::dll::program_location().parent_path() / "test_expected_calclation_output.json";

    // 入力データを読み込む。
    std::ifstream ifs(path.string());
    rapidjson::IStreamWrapper isw(ifs);
    rapidjson::Document res_doc;
    res_doc.ParseStream(isw);
    if (res_doc.HasParseError()) {
        std::cerr << "Failed to parse response data (invalid json format)." << std::endl;
        return false;
    }

    for (auto &x : res_doc.GetArray())
        res_list.push_back(parse_response(x));

    return true;
}

void write_output_data(const std::vector<RequestData> &req_data_list)
{
    rapidjson::Document res_doc(rapidjson::kArrayType);
    for (const auto &req_data : req_data_list) {
        DiscardResponseData res_data = create_discard_response(req_data);
        rapidjson::Value res_value = dump_discard_response(res_data, res_doc);
        res_doc.PushBack(res_value, res_doc.GetAllocator());
    }
    std::ofstream ofs(
        R"(F:\work\cpp-apps\mahjong-cpp\data\testcase\test_expected_calclation_output.json)");
    rapidjson::OStreamWrapper osw(ofs);
    rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);
    res_doc.Accept(writer);
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
    std::vector<RequestData> req_data_list;
    if (!load_input_data(req_data_list))
        return;

    RequestData req_data = req_data_list[3];
    for (int turn = 1; turn < 18; ++turn) {
        req_data.turn = turn;
        req_data.flag = ExpectedValueCalculator::CalcSyantenDown   // 向聴戻し考慮
                        | ExpectedValueCalculator::CalcTegawari    // 手変わり考慮
                        | ExpectedValueCalculator::CalcDoubleReach // ダブル立直考慮
                        | ExpectedValueCalculator::CalcIppatu      // 一発考慮
                        | ExpectedValueCalculator::CalcHaiteitumo  // 海底撈月考慮
                        | ExpectedValueCalculator::CalcUradora     // 裏ドラ考慮
                        | ExpectedValueCalculator::CalcAkaTileTumo // 赤牌自摸考慮
            // | ExpectedValueCalculator::MaximaizeWinProb // 和了確率を最大化
            ;

        DiscardResponseData result1 = create_discard_response(req_data);
        DiscardResponseData result2 = create_discard_response_navie(req_data);

        spdlog::info("{} {} -> {}", double(result1.time_us) / double(result2.time_us),
                     result1.time_us / 1000, result2.time_us / 1000);

        // 聴牌確率、和了確率、期待値が一致しているかどうか
        for (size_t i = 0; i < result1.candidates.size(); ++i) {
            REQUIRE(result1.candidates[i].win_probs[turn - 1] ==
                    Approx(result2.candidates[i].win_probs[turn - 1]));
            REQUIRE(result1.candidates[i].exp_values[turn - 1] ==
                    Approx(result2.candidates[i].exp_values[turn - 1]));
            REQUIRE(result1.candidates[i].tenpai_probs[turn - 1] ==
                    Approx(result2.candidates[i].tenpai_probs[turn - 1]));
        }
    }
}

TEST_CASE("期待値計算の計算時間")
{
    std::vector<RequestData> req_data_list;
    if (!load_input_data(req_data_list))
        return;
    // write_output_data(req_data_list);
    // return;

    std::vector<DiscardResponseData> res_data_list;
    if (!load_output_data(res_data_list))
        return;

    for (size_t i = 0; i < req_data_list.size(); ++i) {
        spdlog::info(req_data_list[i].hand.to_string());
        DiscardResponseData actual = create_discard_response(req_data_list[i]);
        const DiscardResponseData &expected = res_data_list[i];

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
