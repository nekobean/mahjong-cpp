#define CATCH_CONFIG_MAIN

#include <functional>
#include <fstream>
#include <string>
#include <vector>

#include <boost/dll.hpp>
#include <catch2/catch.hpp>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/schema.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "mahjong/mahjong.hpp"
#include "server/json_parser.hpp"

using namespace mahjong;

namespace
{

std::string stringify_json(const rapidjson::Value &value)
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    value.Accept(writer);
    return buffer.GetString();
}

rapidjson::Document parse_raw_json(const std::string &json)
{
    rapidjson::Document doc;
    doc.Parse(json.c_str());
    if (doc.HasParseError()) {
        throw std::runtime_error("Failed to parse raw JSON in test helper.");
    }
    return doc;
}

rapidjson::Document make_valid_request_document(bool include_wall = true,
                                                bool include_ip = true)
{
    rapidjson::Document doc;
    doc.SetObject();
    auto &allocator = doc.GetAllocator();

    doc.AddMember("round_wind", 27, allocator);
    doc.AddMember("seat_wind", 28, allocator);

    rapidjson::Value dora_indicators(rapidjson::kArrayType);
    dora_indicators.PushBack(31, allocator);
    dora_indicators.PushBack(32, allocator);
    doc.AddMember("dora_indicators", dora_indicators, allocator);

    doc.AddMember("enable_reddora", true, allocator);
    doc.AddMember("enable_uradora", false, allocator);
    doc.AddMember("enable_shanten_down", true, allocator);
    doc.AddMember("enable_tegawari", false, allocator);
    doc.AddMember("objective", 2, allocator);

    rapidjson::Value hand(rapidjson::kArrayType);
    for (const int tile : {0, 0, 5, 11, 12, 20, 21, 22}) {
        hand.PushBack(tile, allocator);
    }
    doc.AddMember("hand", hand, allocator);

    rapidjson::Value melds(rapidjson::kArrayType);
    {
        rapidjson::Value meld(rapidjson::kObjectType);
        meld.AddMember("type", 1, allocator);
        rapidjson::Value tiles(rapidjson::kArrayType);
        for (const int tile : {1, 1, 1}) {
            tiles.PushBack(tile, allocator);
        }
        meld.AddMember("tiles", tiles, allocator);
        melds.PushBack(meld, allocator);
    }
    {
        rapidjson::Value meld(rapidjson::kObjectType);
        meld.AddMember("type", 2, allocator);
        rapidjson::Value tiles(rapidjson::kArrayType);
        for (const int tile : {4, 5, 6}) {
            tiles.PushBack(tile, allocator);
        }
        meld.AddMember("tiles", tiles, allocator);
        melds.PushBack(meld, allocator);
    }
    doc.AddMember("melds", melds, allocator);

    if (include_wall) {
        Round round;
        round.wind = 27;
        round.dora_indicators = {31, 32};

        Player player;
        player.wind = 28;
        player.hand = from_array({0, 0, 5, 11, 12, 20, 21, 22});
        player.melds.emplace_back(1, std::vector<int>{1, 1, 1});
        player.melds.emplace_back(2, std::vector<int>{4, 5, 6});

        const MergedCount wall_counts =
            create_wall(round, player, true);

        rapidjson::Value wall(rapidjson::kArrayType);
        for (const int count : wall_counts) {
            wall.PushBack(count, allocator);
        }
        doc.AddMember("wall", wall, allocator);
    }

    if (include_ip) {
        doc.AddMember("ip", "127.0.0.1", allocator);
    }

    doc.AddMember("version", rapidjson::StringRef(PROJECT_VERSION), allocator);

    return doc;
}

std::string make_valid_request_json(bool include_wall = true, bool include_ip = true)
{
    const rapidjson::Document doc = make_valid_request_document(include_wall, include_ip);
    return stringify_json(doc);
}

template <typename Modifier>
std::string make_request_json(Modifier modifier, bool include_wall = true,
                              bool include_ip = true)
{
    rapidjson::Document doc = make_valid_request_document(include_wall, include_ip);
    modifier(doc);
    return stringify_json(doc);
}

void require_runtime_error_contains(const std::function<void()> &fn,
                                    const std::string &expected_substring)
{
    try {
        fn();
        FAIL("Expected std::runtime_error.");
    }
    catch (const std::runtime_error &e) {
        const std::string message = e.what();
        INFO("Actual error message: " << message);
        REQUIRE(message.find(expected_substring) != std::string::npos);
    }
}

const rapidjson::SchemaDocument &get_response_schema()
{
    static const rapidjson::SchemaDocument schema = []() {
        const auto schema_path =
            boost::dll::program_location().parent_path() / "response_schema.json";
        std::ifstream ifs(schema_path.string());
        if (!ifs.is_open()) {
            throw std::runtime_error("Failed to open response schema in test helper.");
        }

        rapidjson::Document schema_doc;
        rapidjson::IStreamWrapper isw(ifs);
        schema_doc.ParseStream(isw);
        if (schema_doc.HasParseError()) {
            throw std::runtime_error("Failed to parse response schema in test helper.");
        }

        return rapidjson::SchemaDocument(schema_doc);
    }();

    return schema;
}

void validate_response_schema(const rapidjson::Document &doc)
{
    rapidjson::SchemaValidator validator(get_response_schema());
    if (!doc.Accept(validator)) {
        rapidjson::StringBuffer buffer;
        validator.GetInvalidDocumentPointer().StringifyUriFragment(buffer);
        FAIL("Response schema validation failed at " + std::string(buffer.GetString()));
    }
}

std::vector<int> to_int_vector(const rapidjson::Value &array)
{
    std::vector<int> values;
    values.reserve(array.Size());
    for (const auto &value : array.GetArray()) {
        values.push_back(value.GetInt());
    }
    return values;
}

std::vector<double> to_double_vector(const rapidjson::Value &array)
{
    std::vector<double> values;
    values.reserve(array.Size());
    for (const auto &value : array.GetArray()) {
        values.push_back(value.GetDouble());
    }
    return values;
}

Request make_sample_request()
{
    Request req;
    req.config.enable_reddora = true;
    req.config.enable_uradora = false;
    req.config.enable_shanten_down = true;
    req.config.enable_tegawari = false;
    req.config.enable_riichi = true;

    req.round.wind = 27;
    req.round.dora_indicators = {31, 32};

    req.player.wind = 28;
    req.player.hand = from_array({0, 0, 5, 11, 12, 20, 21, 22});
    req.player.melds.emplace_back(1, std::vector<int>{1, 1, 1});
    req.player.melds.emplace_back(2, std::vector<int>{4, 5, 6});

    req.wall =
        create_wall(req.round, req.player, req.config.enable_reddora);
    req.objective = 2;
    req.ip = "127.0.0.1";
    req.version = PROJECT_VERSION;

    return req;
}

CalculationResult make_sample_result()
{
    CalculationResult result;
    result.config.enable_reddora = true;
    result.config.enable_uradora = false;
    result.config.enable_shanten_down = true;
    result.config.enable_tegawari = false;
    result.config.enable_riichi = true;
    result.config.t_min = 1;
    result.config.t_max = 18;
    result.config.sum = 62;
    result.config.extra = 1;
    result.config.shanten_type = ShantenFlag::All;
    result.config.calc_stats = true;

    result.shanten = 1;
    result.regular_shanten = 1;
    result.seven_pairs_shanten = 3;
    result.thirteen_orphans_shanten = 7;

    ExpectedScoreCalculator::Stat stat1;
    stat1.tile = 5;
    stat1.tenpai_prob = {0.1, 0.25};
    stat1.win_prob = {0.05, 0.125};
    stat1.exp_score = {1234.5678, 2000.0};
    stat1.necessary_tiles = {{3, 2}, {31, 1}};
    stat1.shanten = 0;

    ExpectedScoreCalculator::Stat stat2;
    stat2.tile = -1;
    stat2.tenpai_prob = {1.0};
    stat2.win_prob = {0.5};
    stat2.exp_score = {8000.0};
    stat2.necessary_tiles = {};
    stat2.shanten = -1;

    result.stats = {stat1, stat2};
    result.searched = 42;
    result.time_us = 123456;

    return result;
}

} // namespace

TEST_CASE("dump_json serializes a document to valid JSON")
{
    rapidjson::Document doc;
    doc.SetObject();
    auto &allocator = doc.GetAllocator();
    doc.AddMember("success", true, allocator);
    doc.AddMember("value", 1.23456, allocator);

    const std::string json = dump_json(doc);

    rapidjson::Document parsed = parse_raw_json(json);
    REQUIRE(parsed["success"].GetBool());
    REQUIRE(parsed["value"].GetDouble() == Approx(1.23456).margin(0.0001));
}

TEST_CASE("parse_json accepts valid requests")
{
    SECTION("with all supported fields")
    {
        rapidjson::Document doc;
        REQUIRE_NOTHROW(parse_json(make_valid_request_json(), doc));
        REQUIRE(doc.IsObject());
        REQUIRE(doc.HasMember("wall"));
        REQUIRE(doc.HasMember("ip"));
    }

    SECTION("without optional wall and ip")
    {
        rapidjson::Document doc;
        REQUIRE_NOTHROW(parse_json(make_valid_request_json(false, false), doc));
        REQUIRE_FALSE(doc.HasMember("wall"));
        REQUIRE_FALSE(doc.HasMember("ip"));
    }
}

TEST_CASE("parse_json rejects malformed or invalid requests")
{
    SECTION("invalid JSON syntax")
    {
        rapidjson::Document doc;
        require_runtime_error_contains(
            [&] { parse_json("{\"round_wind\":27", doc); },
            "Failed to parse JSON string");
    }

    SECTION("missing a required field")
    {
        rapidjson::Document doc;
        const std::string json = make_request_json(
            [](rapidjson::Document &request) { request.RemoveMember("seat_wind"); });
        require_runtime_error_contains([&] { parse_json(json, doc); },
                                       "JSON schema validation failed");
    }

    SECTION("contains an unknown field")
    {
        rapidjson::Document doc;
        const std::string json = make_request_json([](rapidjson::Document &request) {
            request.AddMember("unknown", true, request.GetAllocator());
        });
        require_runtime_error_contains([&] { parse_json(json, doc); },
                                       "JSON schema validation failed");
    }

    SECTION("has a version mismatch")
    {
        rapidjson::Document doc;
        const std::string json = make_request_json([](rapidjson::Document &request) {
            request["version"].SetString("0.0.0", request.GetAllocator());
        });
        require_runtime_error_contains([&] { parse_json(json, doc); },
                                       "Request version mismatch");
    }
}

TEST_CASE("deserialize_request maps validated JSON to Request")
{
    SECTION("maps all explicit fields")
    {
        rapidjson::Document doc;
        parse_json(make_valid_request_json(), doc);

        const Request req = deserialize_request(doc);

        REQUIRE(req.round.wind == 27);
        REQUIRE(req.player.wind == 28);
        REQUIRE(req.round.dora_indicators == std::vector<int>({31, 32}));
        REQUIRE(req.player.hand == from_array({0, 0, 5, 11, 12, 20, 21, 22}));
        REQUIRE(req.player.melds.size() == 2);
        REQUIRE(req.player.melds[0].type == 1);
        REQUIRE(req.player.melds[0].tiles == std::vector<int>({1, 1, 1}));
        REQUIRE(req.player.melds[1].type == 2);
        REQUIRE(req.player.melds[1].tiles == std::vector<int>({4, 5, 6}));
        REQUIRE(req.config.enable_reddora);
        REQUIRE_FALSE(req.config.enable_uradora);
        REQUIRE(req.config.enable_shanten_down);
        REQUIRE_FALSE(req.config.enable_tegawari);
        REQUIRE(req.config.enable_riichi);
        REQUIRE(req.objective == 2);
        REQUIRE(req.ip == "127.0.0.1");
        REQUIRE(req.version == PROJECT_VERSION);
        REQUIRE(req.wall == create_wall(
            req.round, req.player, req.config.enable_reddora));
    }

    SECTION("builds the wall when it is omitted")
    {
        rapidjson::Document doc;
        parse_json(make_valid_request_json(false, false), doc);

        const Request req = deserialize_request(doc);
        const MergedCount expected_wall =
            create_wall(req.round, req.player, req.config.enable_reddora);

        REQUIRE(req.wall == expected_wall);
        REQUIRE(req.ip.empty());
    }
}

TEST_CASE("deserialize_request rejects inconsistent tile counts")
{
    SECTION("more than four copies of the same tile are used")
    {
        rapidjson::Document doc;
        parse_json(make_request_json([](rapidjson::Document &request) {
                       request["hand"][0].SetInt(1);
                       request["hand"][1].SetInt(1);
                   }),
                   doc);

        require_runtime_error_contains([&] { deserialize_request(doc); },
                                       "Too many tiles are used");
    }

    SECTION("requested wall exceeds the remaining tiles")
    {
        rapidjson::Document doc;
        parse_json(make_request_json([](rapidjson::Document &request) {
                       request["wall"][4].SetInt(4);
                   }),
                   doc);

        require_runtime_error_contains(
            [&] { deserialize_request(doc); },
            "More tiles are requested than remain in the wall");
    }

    SECTION("total tile count is invalid")
    {
        rapidjson::Document doc;
        parse_json(make_request_json([](rapidjson::Document &request) {
                       rapidjson::Value hand(rapidjson::kArrayType);
                       for (const int tile : {0, 0, 5, 11, 12, 20, 21, 22, 30, 31,
                                              32}) {
                           hand.PushBack(tile, request.GetAllocator());
                       }
                       request["hand"] = hand;
                   },
                   false),
                   doc);

        require_runtime_error_contains([&] { deserialize_request(doc); },
                                       "Invalid tile count");
    }
}

TEST_CASE("build_error_response creates a schema-compliant error document")
{
    rapidjson::Document doc;
    doc.SetObject();
    doc.AddMember("stale", true, doc.GetAllocator());

    build_error_response("invalid request", doc);

    REQUIRE(doc.MemberCount() == 2);
    REQUIRE_FALSE(doc["success"].GetBool());
    REQUIRE(std::string(doc["err_msg"].GetString()) == "invalid request");
    validate_response_schema(doc);
}

TEST_CASE("build_success_response creates a schema-compliant success document")
{
    const Request req = make_sample_request();
    const CalculationResult result = make_sample_result();

    rapidjson::Document doc;
    doc.SetObject();
    doc.AddMember("stale", true, doc.GetAllocator());

    build_success_response(req, result, doc);

    REQUIRE(doc.MemberCount() == 7);
    REQUIRE(doc["success"].GetBool());

    const rapidjson::Value &input = doc["input"];
    REQUIRE(input.MemberCount() == 6);
    REQUIRE(input["round_wind"].GetInt() == 27);
    REQUIRE(input["seat_wind"].GetInt() == 28);
    REQUIRE(to_int_vector(input["dora_indicators"]) == std::vector<int>({31, 32}));
    REQUIRE(to_int_vector(input["hand"]) == std::vector<int>({0, 0, 5, 11, 12, 20, 21, 22}));
    REQUIRE(input["melds"].Size() == 2);
    REQUIRE(input["melds"][0]["type"].GetInt() == 1);
    REQUIRE(to_int_vector(input["melds"][0]["tiles"]) == std::vector<int>({1, 1, 1}));
    REQUIRE(input["melds"][1]["type"].GetInt() == 2);
    REQUIRE(to_int_vector(input["melds"][1]["tiles"]) == std::vector<int>({4, 5, 6}));
    REQUIRE(input["wall"].Size() == 37);

    const rapidjson::Value &config = doc["config"];
    REQUIRE(config.MemberCount() == 12);
    REQUIRE(config["enable_reddora"].GetBool());
    REQUIRE_FALSE(config["enable_uradora"].GetBool());
    REQUIRE(config["enable_shanten_down"].GetBool());
    REQUIRE_FALSE(config["enable_tegawari"].GetBool());
    REQUIRE(config["objective"].GetInt() == 2);
    REQUIRE(config["t_min"].GetInt() == 1);
    REQUIRE(config["t_max"].GetInt() == 18);
    REQUIRE(config["sum"].GetInt() == 62);
    REQUIRE(config["extra"].GetInt() == 1);
    REQUIRE(config["shanten_type"].GetInt() == ShantenFlag::All);
    REQUIRE(config["calc_stats"].GetBool());
    REQUIRE(config["num_tiles"].GetInt() == 14);

    const rapidjson::Value &shanten = doc["shanten"];
    REQUIRE(shanten["all"].GetInt() == 1);
    REQUIRE(shanten["regular"].GetInt() == 1);
    REQUIRE(shanten["seven_pairs"].GetInt() == 3);
    REQUIRE(shanten["thirteen_orphans"].GetInt() == 7);

    const rapidjson::Value &stats = doc["stats"];
    REQUIRE(stats.Size() == 2);
    REQUIRE(stats[0]["tile"].GetInt() == 5);
    REQUIRE(to_double_vector(stats[0]["tenpai_prob"]) ==
            std::vector<double>({0.1, 0.25}));
    REQUIRE(to_double_vector(stats[0]["win_prob"]) ==
            std::vector<double>({0.05, 0.125}));
    REQUIRE(to_double_vector(stats[0]["exp_score"]) ==
            std::vector<double>({1234.5678, 2000.0}));
    REQUIRE(stats[0]["necessary_tiles"].Size() == 2);
    REQUIRE(stats[0]["necessary_tiles"][0]["tile"].GetInt() == 3);
    REQUIRE(stats[0]["necessary_tiles"][0]["count"].GetInt() == 2);
    REQUIRE(stats[0]["necessary_tiles"][1]["tile"].GetInt() == 31);
    REQUIRE(stats[0]["necessary_tiles"][1]["count"].GetInt() == 1);
    REQUIRE(stats[0]["shanten"].GetInt() == 0);

    REQUIRE(stats[1]["tile"].GetInt() == -1);
    REQUIRE(to_double_vector(stats[1]["tenpai_prob"]) == std::vector<double>({1.0}));
    REQUIRE(to_double_vector(stats[1]["win_prob"]) == std::vector<double>({0.5}));
    REQUIRE(to_double_vector(stats[1]["exp_score"]) == std::vector<double>({8000.0}));
    REQUIRE(stats[1]["necessary_tiles"].Empty());
    REQUIRE(stats[1]["shanten"].GetInt() == -1);

    REQUIRE(doc["searched"].GetInt() == 42);
    REQUIRE(doc["time"].GetInt64() == 123456);

    validate_response_schema(doc);
}
