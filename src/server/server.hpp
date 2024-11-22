#ifndef MAHJONG_CPP_SERVER
#define MAHJONG_CPP_SERVER

#include "ThreadPool.hpp"
#include "json_parser.hpp"
#include "mahjong/mahjong.hpp"

class Server
{
  public:
    Server();
    int run();
    std::string process_request(const std::string &json);
    ThreadPool pool_;

  private:
    void log_request(const Request &req);
};

#endif /* MAHJONG_CPP_SERVER */
