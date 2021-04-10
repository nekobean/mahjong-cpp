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

#include "mahjong/json_parser.hpp"

class Server
{
  public:
    Server();

    int run();
    std::string process_request(const std::string &json);
    ThreadPool pool_;

  private:
    std::tuple<bool, RequestData> parse_json(const rapidjson::Document &doc);
    bool validate_json(const rapidjson::Document &doc);
};

#endif /* MAHJONG_CPP_SERVER */
