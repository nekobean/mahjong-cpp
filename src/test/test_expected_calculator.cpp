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
    std::ofstream ofs("test_expected_calclation_output.json");
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

TEST_CASE("期待値計算")
{
    std::vector<RequestData> req_data_list;
    if (!load_input_data(req_data_list))
        return;
    //write_output_data(req_data_list);

    std::vector<DiscardResponseData> res_data_list;
    if (!load_output_data(res_data_list))
        return;

    for (size_t i = 0; i < req_data_list.size(); ++i) {
        DiscardResponseData actual = create_discard_response(req_data_list[i]);
        const DiscardResponseData &expected = res_data_list[i];

        REQUIRE(actual.syanten == expected.syanten);
        std::cout << double(expected.time_us) / double(actual.time_us) << std::endl;
        //REQUIRE(double(expected.time_us) / double(actual.time_us) < 1.1);

        for (size_t i = 0; i < actual.candidates.size(); ++i)
            test_candidate(expected.candidates[i], actual.candidates[i]);
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
