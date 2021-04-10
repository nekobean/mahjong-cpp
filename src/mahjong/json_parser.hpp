#ifndef MAHJONG_CPP_JSON_PARSER
#define MAHJONG_CPP_JSON_PARSER

#include <rapidjson/document.h>

#include "mahjong.hpp"

struct RequestData
{
    int zikaze;
    int bakaze;
    int turn;
    int syanten_type;
    std::vector<int> dora_indicators;
    mahjong::Hand hand;
    int flag;
    std::string ip;
};

struct DrawResponseData
{
    int syanten;
    size_t time_us;
    std::vector<std::tuple<int, int>> required_tiles;
};

struct DiscardResponseData
{
    int syanten;
    size_t time_us;
    std::vector<mahjong::Candidate> candidates;
};

RequestData parse_request(const rapidjson::Value &doc);
DiscardResponseData parse_response(const rapidjson::Value &doc);
std::string to_json_str(rapidjson::Document &doc);
rapidjson::Value dump_required_tiles(const std::vector<std::tuple<int, int>> &tiles,
                                     rapidjson::Document &doc);
rapidjson::Value dump_candidate(const mahjong::Candidate &candidate, rapidjson::Document &doc);
DrawResponseData create_draw_response(const RequestData &req);
DiscardResponseData create_discard_response(const RequestData &req);
rapidjson::Value dump_draw_response(const DrawResponseData &res, rapidjson::Document &doc);
rapidjson::Value dump_discard_response(const DiscardResponseData &res, rapidjson::Document &doc);
rapidjson::Value create_response(const RequestData &req, rapidjson::Document &doc);

#endif /* MAHJONG_CPP_JSON_PARSER */
