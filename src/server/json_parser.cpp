#include "json_parser.hpp"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <string>

#include <boost/dll.hpp>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/schema.h>
#include <rapidjson/stringbuffer.h>
#include <spdlog/spdlog.h>

#include "mahjong/mahjong.hpp"

using namespace mahjong;

namespace
{

const rapidjson::SchemaDocument &get_request_schema()
{
    static const rapidjson::SchemaDocument schema = []() {
        boost::filesystem::path schema_path =
            boost::dll::program_location().parent_path() / "request_schema.json";

        std::ifstream ifs(schema_path.string());
        if (!ifs.is_open()) {
            throw std::runtime_error(fmt::format("Failed to open JSON schema: path={}.",
                                                 schema_path.string()));
        }

        rapidjson::Document schema_doc;
        rapidjson::IStreamWrapper isw(ifs);
        if (schema_doc.ParseStream(isw).HasParseError()) {
            throw std::runtime_error(fmt::format(
                "Failed to parse JSON schema: path={}.", schema_path.string()));
        }

        return rapidjson::SchemaDocument(schema_doc);
    }();

    return schema;
}

// Build the internal request representation from a parsed JSON value.
Request make_request(const rapidjson::Value &doc)
{
    Request req;
    if (doc.HasMember("mode")) {
        req.round.mode = doc["mode"].GetInt();
    }
    req.round.wind = doc["round_wind"].GetInt();
    req.player.wind = doc["seat_wind"].GetInt();

    const auto dora_indicators = doc["dora_indicators"].GetArray();
    req.round.dora_indicators.reserve(dora_indicators.Size());
    for (const auto &x : dora_indicators) {
        req.round.dora_indicators.push_back(x.GetInt());
    }

    std::vector<int> hand;
    const auto hand_array = doc["hand"].GetArray();
    hand.reserve(hand_array.Size());
    for (const auto &x : hand_array) {
        hand.push_back(x.GetInt());
    }
    req.player.hand = from_array(hand);

    const auto melds = doc["melds"].GetArray();
    req.player.melds.reserve(melds.Size());
    for (const auto &meld : melds) {
        int meld_type = meld["type"].GetInt();
        std::vector<int> meld_tiles;
        const auto meld_tile_array = meld["tiles"].GetArray();
        meld_tiles.reserve(meld_tile_array.Size());
        for (const auto &x : meld_tile_array) {
            meld_tiles.push_back(x.GetInt());
        }
        req.player.melds.emplace_back(meld_type, meld_tiles);
    }

    if (doc.HasMember("nuki")) {
        req.player.num_nuki = doc["nuki"].GetInt();
    }

    req.config.enable_reddora = doc["enable_reddora"].GetBool();
    req.config.enable_shanten_down = doc["enable_shanten_down"].GetBool();
    req.config.enable_tegawari = doc["enable_tegawari"].GetBool();
    req.config.enable_uradora = doc["enable_uradora"].GetBool();

    req.objective = doc["objective"].GetInt();

    if (doc.HasMember("wall")) {
        for (int i = 0; i < 37; ++i) {
            req.wall[i] = doc["wall"][i].GetInt();
        }
    }
    else {
        req.wall = create_wall(req.round, req.player, req.config.enable_reddora);
    }

    if (doc.HasMember("ip")) {
        req.ip = doc["ip"].GetString();
    }

    if (doc.HasMember("version")) {
        req.version = doc["version"].GetString();
    }

    return req;
}

void validate_tile_counts(const Request &req)
{
    MergedCount wall = create_wall(req.round, req.player, req.config.enable_reddora);

    for (int i = 0; i < 37; ++i) {
        if (wall[i] < 0) {
            throw std::runtime_error(
                fmt::format("Too many tiles are used: tile={}, count={}", Tile::name(i),
                            4 - wall[i]));
        }
    }

    for (int i = 0; i < 37; ++i) {
        if (req.wall[i] > wall[i]) {
            throw std::runtime_error(
                fmt::format("More tiles are requested than remain in the wall: "
                            "tile={}, wall={}, used={}",
                            Tile::name(i), req.wall[i], 4 - wall[i]));
        }
    }

    int total_count = req.player.num_tiles() + req.player.num_melds() * 3;
    if (total_count % 3 == 0 || total_count > 14) {
        throw std::runtime_error("Invalid tile count.");
    }
}

void validate_sanma_tiles(const Request &req)
{
    if (req.round.mode != GameMode::Sanma) {
        if (req.player.num_nuki > 0) {
            throw std::runtime_error("Nuki dora is only allowed in sanma.");
        }
        return;
    }

    if (has_sanma_disabled_tiles(req.player.hand)) {
        throw std::runtime_error("Sanma hand contains disabled tiles.");
    }

    for (const auto &meld : req.player.melds) {
        if (meld.type == MeldType::Chi) {
            throw std::runtime_error("Sanma hand contains a chow meld.");
        }
        for (const auto tile : meld.tiles) {
            if (is_sanma_disabled_tile(tile)) {
                throw std::runtime_error(fmt::format(
                    "Sanma meld contains a disabled tile: tile={}.", Tile::name(tile)));
            }
        }
    }

    for (const auto tile : req.round.dora_indicators) {
        if (is_sanma_disabled_tile(tile)) {
            throw std::runtime_error(
                fmt::format("Sanma dora indicator contains a disabled tile: tile={}.",
                            Tile::name(tile)));
        }
    }

    for (int tile = 0; tile < Tile::Length; ++tile) {
        if (is_sanma_disabled_tile(tile) && req.wall[tile] > 0) {
            throw std::runtime_error(
                fmt::format("Sanma wall contains disabled tiles: tile={}, count={}.",
                            Tile::name(tile), req.wall[tile]));
        }
    }
}

rapidjson::Value
serialize_necessary_tiles(const std::vector<std::tuple<int, int>> &tiles,
                          rapidjson::Document &doc)
{
    auto &allocator = doc.GetAllocator();
    rapidjson::Value value(rapidjson::kArrayType);
    for (const auto [tile, count] : tiles) {
        rapidjson::Value x(rapidjson::kObjectType);
        x.AddMember("tile", tile, allocator);
        x.AddMember("count", count, allocator);
        value.PushBack(x, allocator);
    }

    return value;
}

rapidjson::Value
serialize_expected_score(const std::vector<ExpectedScoreCalculator::Stat> &stats,
                         rapidjson::Document &doc)
{
    auto &allocator = doc.GetAllocator();

    rapidjson::Value value(rapidjson::kArrayType);
    for (const auto &stat : stats) {
        rapidjson::Value x(rapidjson::kObjectType);

        x.AddMember("tile", stat.tile, allocator);

        rapidjson::Value tenpai_prob(rapidjson::kArrayType);
        for (const auto prob : stat.tenpai_prob) {
            tenpai_prob.PushBack(std::clamp(prob, 0.0, 1.0), allocator);
        }
        x.AddMember("tenpai_prob", tenpai_prob, allocator);

        rapidjson::Value win_prob(rapidjson::kArrayType);
        for (const auto prob : stat.win_prob) {
            win_prob.PushBack(std::clamp(prob, 0.0, 1.0), allocator);
        }
        x.AddMember("win_prob", win_prob, allocator);

        rapidjson::Value exp_score(rapidjson::kArrayType);
        for (const auto value : stat.exp_score) {
            exp_score.PushBack(value, allocator);
        }
        x.AddMember("exp_score", exp_score, allocator);

        x.AddMember("necessary_tiles",
                    serialize_necessary_tiles(stat.necessary_tiles, doc), allocator);

        x.AddMember("shanten", stat.shanten, allocator);

        value.PushBack(x, allocator);
    }

    return value;
}

rapidjson::Value serialize_string(const std::string &str, rapidjson::Document &doc)
{
    auto &allocator = doc.GetAllocator();
    rapidjson::Value value;
    value.SetString(str.c_str(), static_cast<rapidjson::SizeType>(str.length()),
                    allocator);

    return value;
}

rapidjson::Value serialize_input(const Request &req, rapidjson::Document &doc)
{
    auto &allocator = doc.GetAllocator();
    rapidjson::Value input_val(rapidjson::kObjectType);

    input_val.AddMember("mode", static_cast<int>(req.round.mode), allocator);
    input_val.AddMember("round_wind", req.round.wind, allocator);
    input_val.AddMember("seat_wind", req.player.wind, allocator);

    rapidjson::Value dora_indicators(rapidjson::kArrayType);
    for (const auto tile : req.round.dora_indicators) {
        dora_indicators.PushBack(tile, allocator);
    }
    input_val.AddMember("dora_indicators", dora_indicators, allocator);

    rapidjson::Value hand(rapidjson::kArrayType);
    for (int i = 0; i < 37; ++i) {
        int count = req.player.hand[i];
        if (i == Tile::Manzu5) {
            count -= req.player.hand[Tile::RedManzu5];
        }
        else if (i == Tile::Pinzu5) {
            count -= req.player.hand[Tile::RedPinzu5];
        }
        else if (i == Tile::Souzu5) {
            count -= req.player.hand[Tile::RedSouzu5];
        }
        for (int j = 0; j < count; ++j) {
            hand.PushBack(i, allocator);
        }
    }
    input_val.AddMember("hand", hand, allocator);

    rapidjson::Value melds(rapidjson::kArrayType);
    for (const auto &meld : req.player.melds) {
        rapidjson::Value meld_val(rapidjson::kObjectType);
        meld_val.AddMember("type", meld.type, allocator);

        rapidjson::Value tiles(rapidjson::kArrayType);
        for (const auto tile : meld.tiles) {
            tiles.PushBack(tile, allocator);
        }
        meld_val.AddMember("tiles", tiles, allocator);

        melds.PushBack(meld_val, allocator);
    }
    input_val.AddMember("melds", melds, allocator);

    input_val.AddMember("nuki", req.player.num_nuki, allocator);

    rapidjson::Value wall(rapidjson::kArrayType);
    for (const auto count : req.wall) {
        wall.PushBack(count, allocator);
    }
    input_val.AddMember("wall", wall, allocator);

    return input_val;
}

} // namespace

/**
 * @brief Convert a JSON document to a string.
 *
 * @param[in] doc JSON document to serialize.
 * @return Serialized JSON string.
 */
std::string dump_json(const rapidjson::Document &doc)
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    writer.SetMaxDecimalPlaces(4);
    doc.Accept(writer);

    return buffer.GetString();
}

/**
 * @brief Parse and validate a request JSON string.
 *
 * @param[in] json Request JSON string.
 * @param[out] doc Parsed JSON document.
 * @throw std::runtime_error If parsing, schema validation, or version check fails.
 */
void parse_json(const std::string &json, rapidjson::Document &doc)
{
    if (doc.Parse(json.c_str()).HasParseError()) {
        throw std::runtime_error("Failed to parse JSON string: invalid JSON format.");
    }

    // Validate JSON schema.
    rapidjson::SchemaValidator validator(get_request_schema());
    if (!doc.Accept(validator)) {
        rapidjson::StringBuffer sb;
        validator.GetInvalidSchemaPointer().StringifyUriFragment(sb);
        const std::string invalid_schema = sb.GetString();
        const std::string invalid_keyword = validator.GetInvalidSchemaKeyword();
        sb.Clear();
        validator.GetInvalidDocumentPointer().StringifyUriFragment(sb);
        const std::string invalid_doc = sb.GetString();

        throw std::runtime_error(
            fmt::format("JSON schema validation failed: schema={}, keyword={}, doc={}",
                        invalid_schema, invalid_keyword, invalid_doc));
    }

    if (std::strcmp(doc["version"].GetString(), PROJECT_VERSION) != 0) {
        throw std::runtime_error(
            fmt::format("Request version mismatch: expected={}, actual={}.",
                        PROJECT_VERSION, doc["version"].GetString()));
    }
}

/**
 * @brief Deserialize and validate a request from a parsed JSON document.
 *
 * @param[in] doc Parsed request JSON document.
 * @return Validated request object.
 * @throw std::runtime_error If the request content is invalid.
 */
Request deserialize_request(const rapidjson::Document &doc)
{
    Request req = make_request(doc);

    validate_sanma_tiles(req);
    validate_tile_counts(req);

    return req;
}

/**
 * @brief Build a success response JSON document from a request.
 *
 * @param[in] req Validated request object.
 * @param[in] result Calculated result to serialize.
 * @param[in,out] doc Response document to populate.
 */
void build_success_response(const Request &req, const CalculationResult &result,
                            rapidjson::Document &doc)
{
    auto &allocator = doc.GetAllocator();
    doc.SetObject();
    doc.AddMember("success", true, allocator);
    doc.AddMember("input", serialize_input(req, doc), allocator);

    rapidjson::Value shanten_val(rapidjson::kObjectType);
    shanten_val.AddMember("all", result.shanten, allocator);
    shanten_val.AddMember("regular", result.regular_shanten, allocator);
    shanten_val.AddMember("seven_pairs", result.seven_pairs_shanten, allocator);
    shanten_val.AddMember("thirteen_orphans", result.thirteen_orphans_shanten,
                          allocator);
    doc.AddMember("shanten", shanten_val, allocator);
    doc.AddMember("stats", serialize_expected_score(result.stats, doc), allocator);
    doc.AddMember("searched", result.searched, allocator);
    doc.AddMember("time", static_cast<int64_t>(result.time_us), allocator);

    rapidjson::Value config_val(rapidjson::kObjectType);
    config_val.AddMember("enable_reddora", result.config.enable_reddora, allocator);
    config_val.AddMember("enable_uradora", result.config.enable_uradora, allocator);
    config_val.AddMember("enable_shanten_down", result.config.enable_shanten_down,
                         allocator);
    config_val.AddMember("enable_tegawari", result.config.enable_tegawari, allocator);
    config_val.AddMember("objective", req.objective, allocator);
    config_val.AddMember("t_min", result.config.t_min, allocator);
    config_val.AddMember("t_max", result.config.t_max, allocator);
    config_val.AddMember("sum", result.config.sum, allocator);
    config_val.AddMember("extra", result.config.extra, allocator);
    config_val.AddMember("shanten_type", result.config.shanten_type, allocator);
    config_val.AddMember("calc_stats", result.config.calc_stats, allocator);
    config_val.AddMember(
        "num_tiles", req.player.num_tiles() + req.player.num_melds() * 3, allocator);
    doc.AddMember("config", config_val, allocator);
}

/**
 * @brief Build an error response JSON document.
 *
 * @param[in] message Error message to serialize.
 * @param[in,out] doc Response document to populate.
 */
void build_error_response(const std::string &message, rapidjson::Document &doc)
{
    auto &allocator = doc.GetAllocator();
    doc.SetObject();
    doc.AddMember("success", false, allocator);
    doc.AddMember("err_msg", serialize_string(message, doc), allocator);
}
