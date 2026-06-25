#ifndef MAHJONG_CPP_JSON_PARSER_H
#define MAHJONG_CPP_JSON_PARSER_H

#include <rapidjson/document.h>

#include "mahjong/mahjong.hpp"

struct Request
{
    mahjong::ExpectedScoreCalculator::Config config;
    mahjong::TableConfig table_config;
    mahjong::RoundState round_state;
    mahjong::TableState table_state;
    mahjong::PlayerState player;
    mahjong::MergedCount wall;
    int objective = 0;
    std::string ip;
    std::string version;
};

struct CalculationResult
{
    mahjong::ExpectedScoreCalculator::Config config;
    int shanten;
    int regular_shanten;
    int seven_pairs_shanten;
    int thirteen_orphans_shanten;
    std::vector<mahjong::ExpectedScoreCalculator::Stat> stats;
    int searched;
    long long time_us;
};

std::string dump_json(const rapidjson::Document &doc);
void parse_json(const std::string &json, rapidjson::Document &doc);
Request deserialize_request(const rapidjson::Document &doc);
void build_success_response(const Request &req, const CalculationResult &result,
                            rapidjson::Document &doc);
void build_error_response(const std::string &message, rapidjson::Document &doc);

#endif // MAHJONG_CPP_JSON_PARSER_H
