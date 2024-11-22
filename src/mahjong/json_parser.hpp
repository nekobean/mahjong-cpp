#ifndef MAHJONG_CPP_JSON_PARSER
#define MAHJONG_CPP_JSON_PARSER

#include <rapidjson/document.h>

#include "mahjong.hpp"

namespace mahjong
{

struct Request
{
    ExpectedScoreCalculator::Config config;
    Round round;
    Player player;
    Count wall;
    std::string ip;
    std::string version;
};

std::string to_json_str(rapidjson::Value &value);
std::string to_json_str(rapidjson::Document &doc);
Request parse_json_str(const std::string &json, rapidjson::Document &doc);
Request create_request(const rapidjson::Value &doc);
rapidjson::Value dump_necessary_tiles(const std::vector<std::tuple<int, int>> &tiles,
                                      rapidjson::Document &doc);
rapidjson::Value dump_string(const std::string &str, rapidjson::Document &doc);
rapidjson::Value
dump_expected_score(const std::vector<ExpectedScoreCalculator::Stat> &stats,
                    rapidjson::Document &doc);
void create_response(const Request &req, rapidjson::Document &doc);
void validate_request(const Request &req);

} // namespace mahjong

#endif // MAHJONG_CPP_JSON_PARSER
