#ifndef MAHJONG_CPP_JSON_PARSER
#define MAHJONG_CPP_JSON_PARSER

#include <rapidjson/document.h>

#include "mahjong/mahjong.hpp"

struct Request
{
    mahjong::ExpectedScoreCalculator::Config config;
    mahjong::Round round;
    mahjong::Player player;
    mahjong::Count wall;
    std::string ip;
    std::string version;
};

std::string to_json_str(rapidjson::Value &value);
std::string to_json_str(rapidjson::Document &doc);
void parse_json(const std::string &json, rapidjson::Document &doc);
Request parse_request_doc(const rapidjson::Document &doc);
Request create_request(const rapidjson::Value &doc);
void validate_request(const Request &req);
rapidjson::Value create_response(const Request &req, rapidjson::Document &doc);
rapidjson::Value dump_necessary_tiles(const std::vector<std::tuple<int, int>> &tiles,
                                      rapidjson::Document &doc);
rapidjson::Value dump_string(const std::string &str, rapidjson::Document &doc);
rapidjson::Value
dump_expected_score(const std::vector<mahjong::ExpectedScoreCalculator::Stat> &stats,
                    rapidjson::Document &doc);

#endif // MAHJONG_CPP_JSON_PARSER
