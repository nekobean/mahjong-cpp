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

TEST_CASE("Expected Calculation")
{
    std::vector<RequestData> req_data_list;
    if (!load_input_data(req_data_list))
        return;

    std::vector<DiscardResponseData> res_data_list;
    if (!load_output_data(res_data_list))
        return;

    //write_output_data(req_data_list);

    for (size_t i = 0; i < req_data_list.size(); ++i) {
        DiscardResponseData actual = create_discard_response(req_data_list[i]);
        const DiscardResponseData &expected = res_data_list[i];

        REQUIRE(actual.syanten == expected.syanten);
        REQUIRE(double(expected.time_us) / double(actual.time_us) < 1.1);

        for (size_t i = 0; i < actual.candidates.size(); ++i)
            test_candidate(expected.candidates[i], actual.candidates[i]);
    }
}
