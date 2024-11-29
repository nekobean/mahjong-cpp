#include "json_parser.hpp"

#include <algorithm>
#include <chrono>
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

using namespace mahjong;

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
    writer.SetMaxDecimalPlaces(4);
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
    writer.SetMaxDecimalPlaces(4);
    doc.Accept(writer);

    return buffer.GetString();
}

void parse_json(const std::string &json, rapidjson::Document &doc)
{
    if (doc.Parse(json.c_str()).HasParseError()) {
        throw std::runtime_error(
            "Failed to parse json string. (reason invalid json format)");
    }

    boost::filesystem::path schema_path =
        boost::dll::program_location().parent_path() / "request_schema.json";

    // Open JSON schema.
    std::ifstream ifs(schema_path.string());
    if (!ifs.is_open()) {
        throw std::runtime_error(fmt::format("Failed to open json schema. (path: {})",
                                             schema_path.string()));
    }

    // Parse JSON schema.
    rapidjson::Document schema_doc;
    rapidjson::IStreamWrapper isw(ifs);
    if (schema_doc.ParseStream(isw).HasParseError()) {
        throw std::runtime_error(fmt::format("Failed to parse json schema. (path: {})",
                                             schema_path.string()));
    }
    rapidjson::SchemaDocument schema(schema_doc);

    // Validate JSON schema.
    rapidjson::SchemaValidator validator(schema);
    if (!doc.Accept(validator)) {
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
    if (strcmp(doc["version"].GetString(), PROJECT_VERSION) != 0) {
        throw std::runtime_error(fmt::format(
            u8"リクエストのバージョンの不一致です。"
            u8"ブラウザのキャッシュの影響と思われるので、ページを更新してください。",
            PROJECT_VERSION, doc["version"].GetString()));
    }
}

Request parse_request_doc(const rapidjson::Document &doc)
{
    // Parse request.
    Request req = create_request(doc);

    // Check request.
    validate_request(req);

    return req;
}

Request create_request(const rapidjson::Value &doc)
{
    Request req;
    req.config.enable_reddora = doc["enable_reddora"].GetBool();
    req.config.enable_uradora = doc["enable_uradora"].GetBool();
    req.config.enable_shanten_down = doc["enable_shanten_down"].GetBool();
    req.config.enable_tegawari = doc["enable_tegawari"].GetBool();
    req.config.enable_riichi = doc["enable_riichi"].GetBool();

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

rapidjson::Value
dump_expected_score(const std::vector<ExpectedScoreCalculator::Stat> &stats,
                    rapidjson::Document &doc)
{
    // 確率値が100%を1%程度超えることがあるので、100%を超えないようにする
    // 原因については要調査
    rapidjson::Value value(rapidjson::kArrayType);
    for (const auto &stat : stats) {
        rapidjson::Value x(rapidjson::kObjectType);

        x.AddMember("tile", stat.tile, doc.GetAllocator());

        rapidjson::Value tenpai_prob(rapidjson::kArrayType);
        for (const auto prob : stat.tenpai_prob) {
            tenpai_prob.PushBack(std::min(prob, 1.), doc.GetAllocator());
        }
        x.AddMember("tenpai_prob", tenpai_prob, doc.GetAllocator());

        rapidjson::Value win_prob(rapidjson::kArrayType);
        for (const auto prob : stat.win_prob) {
            win_prob.PushBack(std::min(prob, 1.), doc.GetAllocator());
        }
        x.AddMember("win_prob", win_prob, doc.GetAllocator());

        rapidjson::Value exp_value(rapidjson::kArrayType);
        for (const auto value : stat.exp_value) {
            exp_value.PushBack(value, doc.GetAllocator());
        }
        x.AddMember("exp_score", exp_value, doc.GetAllocator());

        x.AddMember("necessary_tiles", dump_necessary_tiles(stat.necessary_tiles, doc),
                    doc.GetAllocator());

        x.AddMember("shanten", stat.shanten, doc.GetAllocator());

        value.PushBack(x, doc.GetAllocator());
    }

    return value;
}

rapidjson::Value dump_string(const std::string &str, rapidjson::Document &doc)
{
    rapidjson::Value value;
    value.SetString(str.c_str(), static_cast<rapidjson::SizeType>(str.length()),
                    doc.GetAllocator());

    return value;
}

rapidjson::Value create_response(const Request &req, rapidjson::Document &doc)
{
    using namespace mahjong;

    rapidjson::Value res_val(rapidjson::kObjectType);

    // shanten number
    ///////////////////////////////
    const int shanten = std::get<1>(ShantenCalculator::calc(
        req.player.hand, req.player.num_melds(), ShantenFlag::All));
    const int regular_shanten = std::get<1>(ShantenCalculator::calc(
        req.player.hand, req.player.num_melds(), ShantenFlag::Regular));
    const int seven_pairs_shanten = std::get<1>(ShantenCalculator::calc(
        req.player.hand, req.player.num_melds(), ShantenFlag::SevenPairs));
    const int thirteen_orphans_shanten = std::get<1>(ShantenCalculator::calc(
        req.player.hand, req.player.num_melds(), ShantenFlag::ThirteenOrphans));

    if (shanten == -1) {
        throw std::runtime_error(u8"手牌はすでに和了形です。");
    }

    rapidjson::Value shanten_val(rapidjson::kObjectType);
    shanten_val.AddMember("all", shanten, doc.GetAllocator());
    shanten_val.AddMember("regular", regular_shanten, doc.GetAllocator());
    shanten_val.AddMember("seven_pairs", seven_pairs_shanten, doc.GetAllocator());
    shanten_val.AddMember("thirteen_orphans", thirteen_orphans_shanten,
                          doc.GetAllocator());
    res_val.AddMember("shanten", shanten_val, doc.GetAllocator());

    // expected score
    ///////////////////////////////
    ExpectedScoreCalculator::Config config;
    const int num_tiles = req.player.num_tiles() + req.player.num_melds() * 3;
    config.t_min = 1;
    config.t_max = num_tiles == 14 ? 17 : 18;
    config.sum = std::accumulate(req.wall.begin(), req.wall.begin() + 34, 0);
    config.extra = shanten <= 1 ? 2 : 1;
    config.shanten_type = ShantenFlag::All;
    config.calc_stats = shanten <= 3;
    config.enable_reddora = req.config.enable_reddora;
    config.enable_uradora = req.config.enable_uradora;
    config.enable_shanten_down = req.config.enable_shanten_down;
    config.enable_tegawari = req.config.enable_tegawari;
    config.enable_riichi = req.config.enable_riichi;

    const auto start = std::chrono::system_clock::now();
    const auto [stats, searched] =
        ExpectedScoreCalculator::calc(config, req.round, req.player, req.wall);
    const auto end = std::chrono::system_clock::now();
    const auto elapsed_ms =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    res_val.AddMember("stats", dump_expected_score(stats, doc), doc.GetAllocator());
    res_val.AddMember("searched", searched, doc.GetAllocator());
    res_val.AddMember("time", elapsed_ms, doc.GetAllocator());

    rapidjson::Value config_val(rapidjson::kObjectType);
    config_val.AddMember("t_min", config.t_min, doc.GetAllocator());
    config_val.AddMember("t_max", config.t_max, doc.GetAllocator());
    config_val.AddMember("sum", config.sum, doc.GetAllocator());
    config_val.AddMember("extra", config.extra, doc.GetAllocator());
    config_val.AddMember("shanten_type", config.shanten_type, doc.GetAllocator());
    config_val.AddMember("calc_stats", config.calc_stats, doc.GetAllocator());
    config_val.AddMember("num_tiles", num_tiles, doc.GetAllocator());
    res_val.AddMember("config", config_val, doc.GetAllocator());

    return res_val;
}
