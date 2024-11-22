#include "json_parser.hpp"

#include <algorithm>
#include <sstream>
#include <string>

#include <boost/dll.hpp>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/schema.h>
#include <rapidjson/stringbuffer.h>
#include <spdlog/spdlog.h>

#include "mahjong/mahjong.hpp"

namespace mahjong
{

/**
 * @brief Convert JSON value to string.
 *
 * @param[in] value document
 * @return JSON value as string
 */
std::string to_json_str(rapidjson::Value &value)
{
    std::stringstream ss;
    rapidjson::OStreamWrapper osw(ss);
    rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);
    value.Accept(writer);

    return ss.str();
}

/**
 * @brief Convert JSON document to string.
 *
 * @param[in] doc document
 * @return JSON document as string
 */
std::string to_json_str(rapidjson::Document &doc)
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);

    return buffer.GetString();
}

Request parse_json_str(const std::string &json, rapidjson::Document &doc)
{
    rapidjson::Document req_doc;
    Request req;

    // Parse JSON string.
    req_doc.Parse(json.c_str());
    if (req_doc.HasParseError()) {
        throw std::runtime_error("Failed to parse request json.");
    }

    // Load JSON schema.
    boost::filesystem::path schema_path =
        boost::dll::program_location().parent_path() / "request_schema.json";
    rapidjson::Document schema_doc;

    std::ifstream ifs(schema_path.string());
    rapidjson::IStreamWrapper isw(ifs);
    schema_doc.ParseStream(isw);
    if (schema_doc.HasParseError()) {
        throw std::runtime_error(fmt::format("Failed to parse json schema. (path: {})",
                                             schema_path.string()));
    }
    rapidjson::SchemaDocument schema(schema_doc);

    // Validate JSON schema.
    rapidjson::SchemaValidator validator(schema);
    if (!req_doc.Accept(validator)) {
        rapidjson::StringBuffer sb;
        validator.GetInvalidSchemaPointer().StringifyUriFragment(sb);
        const std::string invalid_schema = sb.GetString();
        const std::string invalid_keyword = validator.GetInvalidSchemaKeyword();
        sb.Clear();
        validator.GetInvalidDocumentPointer().StringifyUriFragment(sb);
        const std::string invalid_doc = sb.GetString();

        throw std::runtime_error(fmt::format(
            "Failed to validate json schema. (schema: {}, keyword: {}, doc: {})",
            invalid_schema, invalid_keyword, invalid_doc));
    }

    // Check version.
    if (strcmp(req_doc["version"].GetString(), PROJECT_VERSION) != 0) {
        throw std::runtime_error(
            fmt::format("Version mismatch detected. (software: {}, json: {})",
                        PROJECT_VERSION, req_doc["version"].GetString()));
    }

    // Parse request.
    req = create_request(req_doc);
    validate_request(req);

    return req;
}

/**
 * @brief Create request from JSON document.
 *
 * @param[in] doc document
 * @return Request
 */
Request create_request(const rapidjson::Value &doc)
{
    Request req;
    req.config.enable_reddora = doc["enable_reddora"].GetBool();
    req.config.enable_uradora = doc["enable_uradora"].GetBool();
    req.config.enable_shanten_down = doc["enable_shanten_down"].GetBool();
    req.config.enable_tegawari = doc["enable_tegawari"].GetBool();

    req.round.wind = doc["round_wind"].GetInt();
    for (const auto &x : doc["dora_indicators"].GetArray()) {
        req.round.dora_indicators.push_back(x.GetInt());
    }

    std::vector<int> hand;
    for (const auto &x : doc["hand"].GetArray()) {
        hand.push_back(x.GetInt());
    }
    req.player.hand = from_array(hand);

    for (const auto &meld : doc["melds"].GetArray()) {
        int meld_type = meld["type"].GetInt();
        std::vector<int> meld_tiles;
        for (const auto &x : meld["tiles"].GetArray()) {
            meld_tiles.push_back(x.GetInt());
        }
        req.player.melds.emplace_back(meld_type, meld_tiles);
    }
    req.player.wind = doc["seat_wind"].GetInt();

    if (doc.HasMember("wall")) {
        for (int i = 0; i < 37; ++i) {
            req.wall[i] = doc["wall"][i].GetInt();
        }
    }
    else {
        req.wall = ExpectedScoreCalculator::create_wall(req.round, req.player,
                                                        req.config.enable_reddora);
    }

    if (doc.HasMember("ip")) {
        req.ip = doc["ip"].GetString();
    }

    if (doc.HasMember("version")) {
        req.version = doc["version"].GetString();
    }

    return req;
}

/**
 * @brief Create value from necessary tiles.
 *
 * @param[in] tiles list of (tile, count)
 * @param[in] doc document
 * @return value
 */
rapidjson::Value dump_necessary_tiles(const std::vector<std::tuple<int, int>> &tiles,
                                      rapidjson::Document &doc)
{
    rapidjson::Value value(rapidjson::kArrayType);
    for (const auto [tile, count] : tiles) {
        rapidjson::Value x(rapidjson::kObjectType);
        x.AddMember("tile", tile, doc.GetAllocator());
        x.AddMember("count", count, doc.GetAllocator());
        value.PushBack(x, doc.GetAllocator());
    }

    return value;
}

/**
 * @brief Create value from expected scores.
 *
 * @param[in] tiles list of stats
 * @param[in] doc document
 * @return value
 */
rapidjson::Value
dump_expected_score(const std::vector<ExpectedScoreCalculator::Stat> &stats,
                    rapidjson::Document &doc)
{
    rapidjson::Value value(rapidjson::kArrayType);
    for (const auto &stat : stats) {
        rapidjson::Value x(rapidjson::kObjectType);

        x.AddMember("tile", stat.tile, doc.GetAllocator());

        rapidjson::Value tenpai_prob(rapidjson::kArrayType);
        for (const auto prob : stat.tenpai_prob) {
            tenpai_prob.PushBack(prob, doc.GetAllocator());
        }
        x.AddMember("tenpai_prob", tenpai_prob, doc.GetAllocator());

        rapidjson::Value win_prob(rapidjson::kArrayType);
        for (const auto prob : stat.win_prob) {
            win_prob.PushBack(prob, doc.GetAllocator());
        }
        x.AddMember("win_prob", win_prob, doc.GetAllocator());

        rapidjson::Value exp_value(rapidjson::kArrayType);
        for (const auto prob : stat.exp_value) {
            exp_value.PushBack(prob, doc.GetAllocator());
        }
        x.AddMember("exp_value", exp_value, doc.GetAllocator());

        rapidjson::Value necessary_tiles =
            dump_necessary_tiles(stat.necessary_tiles, doc);
        x.AddMember("necessary_tiles", necessary_tiles, doc.GetAllocator());

        value.PushBack(x, doc.GetAllocator());
    }

    return value;
}

void validate_request(const Request &req)
{
    Count wall = ExpectedScoreCalculator::create_wall(req.round, req.player,
                                                      req.config.enable_reddora);

    for (int i = 0; i < 37; ++i) {
        if (wall[i] < 0) {
            throw std::runtime_error(
                fmt::format("More than 5 tiles are used. (tile: {}, count: {}) ",
                            Tile::Name.at(i), 4 - wall[i]));
        }
    }

    for (int i = 0; i < 37; ++i) {
        if (req.wall[i] > wall[i]) {
            throw std::runtime_error(fmt::format(
                "More tiles than wall are used. (tile: {}, wall: {}, used: {}",
                Tile::Name.at(i), req.wall[i], 4 - wall[i]));
        }
    }

    int total_count = req.player.num_tiles() + req.player.num_melds() * 3;
    if (total_count % 3 == 0 || total_count > 14) {
        throw std::runtime_error("Invalid number of tiles.");
    }
}

rapidjson::Value dump_string(const std::string &str, rapidjson::Document &doc)
{
    rapidjson::Value value;
    value.SetString(str.c_str(), static_cast<rapidjson::SizeType>(str.length()),
                    doc.GetAllocator());

    return value;
}

/**
 * @brief Create value from expected scores.
 *
 * @param[in] tiles list of stats
 * @param[in] doc document
 * @return value
 */
void create_response(const Request &req, rapidjson::Document &doc)
{
    // shanten number
    ///////////////////////////////
    const int regular_shanten = std::get<1>(ShantenCalculator::calc(
        req.player.hand, req.player.num_tiles(), ShantenFlag::Regular));
    const int seven_pairs_shanten =
        req.player.num_melds() == 0
            ? std::get<1>(ShantenCalculator::calc(
                  req.player.hand, req.player.num_tiles(), ShantenFlag::SevenPairs))
            : -2;
    const int thirteen_orphans_shanten =
        req.player.num_melds() == 0
            ? std::get<1>(ShantenCalculator::calc(req.player.hand,
                                                  req.player.num_tiles(),
                                                  ShantenFlag::ThirteenOrphans))
            : -2;
    ;

    rapidjson::Value shanten(rapidjson::kObjectType);
    shanten.AddMember("regular", regular_shanten, doc.GetAllocator());
    shanten.AddMember("seven_pairs", seven_pairs_shanten, doc.GetAllocator());
    shanten.AddMember("thirteen_orphans", thirteen_orphans_shanten, doc.GetAllocator());
    doc.AddMember("shanten", shanten, doc.GetAllocator());

    // expected score
    ///////////////////////////////
    const auto [stats, searched] =
        ExpectedScoreCalculator::calc(req.config, req.round, req.player);
    doc.AddMember("searched", searched, doc.GetAllocator());
    doc.AddMember("stats", dump_expected_score(stats, doc), doc.GetAllocator());

    doc.AddMember("success", true, doc.GetAllocator());
}

} // namespace mahjong
