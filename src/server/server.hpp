#ifndef MAHJONG_CPP_SERVER
#define MAHJONG_CPP_SERVER

#include <memory>

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/schema.h>
#include <rapidjson/stringbuffer.h>

#include "ThreadPool.hpp"
#include "mahjong/mahjong.hpp"

struct RequestData
{
    int zikaze;
    int bakaze;
    int turn;
    int syanten_type;
    std::vector<int> dora_tiles;
    mahjong::Hand hand;
    int flag;
    std::string ip;
};

class Server
{
  public:
    Server();

    int run();
    std::string process_request(const std::string &json);
    ThreadPool pool_;

  private:
    rapidjson::Value json_dumps(int total_count, const std::vector<std::tuple<int, int>> &tiles,
                                rapidjson::Document &doc);
    rapidjson::Value json_dumps(const mahjong::Candidate &candidate, rapidjson::Document &doc);
    rapidjson::Document create_response(const RequestData &req);
    std::tuple<bool, RequestData> parse_json(const rapidjson::Document &doc);
    bool validate_json(const rapidjson::Document &doc);
};

#endif /* MAHJONG_CPP_SERVER */
