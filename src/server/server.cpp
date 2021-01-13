#include "server.hpp"

#include <chrono>
#include <fstream>
#include <sstream>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http  = beast::http;          // from <boost/beast/http.hpp>
namespace net   = boost::asio;          // from <boost/asio.hpp>
using tcp       = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

using namespace mahjong;

Server server;

Server::Server()
    : pool_(3)
{
}

rapidjson::Value Server::json_dumps(int total_count,
                                    const std::vector<std::tuple<int, int>> &tiles,
                                    rapidjson::Document &doc)
{
    rapidjson::Value value(rapidjson::kArrayType);
    for (auto [tile, count] : tiles) {
        rapidjson::Value v(rapidjson::kObjectType);
        v.AddMember("tile", tile, doc.GetAllocator());
        v.AddMember("count", count, doc.GetAllocator());

        value.PushBack(v, doc.GetAllocator());
    }

    return value;
}

rapidjson::Value Server::json_dumps(const Candidate &candidate,
                                    rapidjson::Document &doc)
{
    rapidjson::Value value(rapidjson::kObjectType);
    value.AddMember("tile", candidate.tile, doc.GetAllocator());
    value.AddMember("syanten_down", candidate.syanten_down, doc.GetAllocator());
    value.AddMember(
        "required_tiles",
        json_dumps(candidate.sum_required_tiles, candidate.required_tiles, doc),
        doc.GetAllocator());

    if (!candidate.exp_values.empty()) {
        value.AddMember("exp_values", rapidjson::kArrayType, doc.GetAllocator());
        for (auto p : candidate.exp_values)
            value["exp_values"].PushBack(p, doc.GetAllocator());
    }

    if (!candidate.win_probs.empty()) {
        value.AddMember("win_probs", rapidjson::kArrayType, doc.GetAllocator());
        for (auto p : candidate.win_probs)
            value["win_probs"].PushBack(p, doc.GetAllocator());
    }

    if (!candidate.tenpai_probs.empty()) {
        value.AddMember("tenpai_probs", rapidjson::kArrayType, doc.GetAllocator());
        for (auto p : candidate.tenpai_probs)
            value["tenpai_probs"].PushBack(p, doc.GetAllocator());
    }

    return value;
}

rapidjson::Document Server::create_response(const RequestData &req)
{
    ScoreCalculator score_calc;
    ExpectedValueCalculator exp_value_calc;

    rapidjson::Document doc;
    doc.SetObject();

    // 手牌の枚数を求める。
    int n_tiles = req.hand.num_tiles() + int(req.hand.melds.size()) * 3;

    if (n_tiles == 13) {
        // 13枚の場合は有効牌を求める。
        auto begin = std::chrono::steady_clock::now();

        // 向聴数を計算する。
        auto [syanten_type, syanten] =
            SyantenCalculator::calc(req.hand, req.syanten_type);

        // 各牌の残り枚数を数える。
        std::vector<int> counts =
            ExpectedValueCalculator::count_left_tiles(req.hand, req.dora_tiles);

        // 有効牌を求める。
        auto [total_count, required_tiles] =
            ExpectedValueCalculator::get_required_tiles(req.hand, req.syanten_type,
                                                        counts);

        auto end = std::chrono::steady_clock::now();
        auto elapsed_ms =
            std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();

        doc.AddMember("result_type", 0, doc.GetAllocator());
        doc.AddMember("syanten", syanten, doc.GetAllocator());
        doc.AddMember("time", elapsed_ms, doc.GetAllocator());
        doc.AddMember("required_tiles", json_dumps(total_count, required_tiles, doc),
                      doc.GetAllocator());
    }
    else if (n_tiles == 14) {
        auto begin = std::chrono::steady_clock::now();

        // 向聴数を計算する。
        auto [syanten_type, syanten] =
            SyantenCalculator::calc(req.hand, req.syanten_type);

        // 点数計算の設定
        score_calc.set_bakaze(req.bakaze);
        score_calc.set_zikaze(req.zikaze);
        score_calc.set_num_tumibo(0);
        score_calc.set_num_kyotakubo(0);
        score_calc.set_dora_tiles(req.dora_tiles);

        // 各打牌を分析する。
        auto [success, candidates] =
            exp_value_calc.calc(req.hand, score_calc, req.syanten_type, req.flag);

        auto end = std::chrono::steady_clock::now();
        auto elapsed_ms =
            std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();

        doc.AddMember("result_type", 1, doc.GetAllocator());
        doc.AddMember("syanten", syanten, doc.GetAllocator());
        doc.AddMember("time", elapsed_ms, doc.GetAllocator());
        doc.AddMember("candidates", rapidjson::kArrayType, doc.GetAllocator());
        for (const auto &candidate : candidates)
            doc["candidates"].PushBack(json_dumps(candidate, doc), doc.GetAllocator());
    }

    return doc;
}

std::tuple<bool, RequestData> Server::parse_json(const rapidjson::Document &doc)
{
    RequestData req;

    req.zikaze       = doc["zikaze"].GetInt();
    req.bakaze       = doc["bakaze"].GetInt();
    req.turn         = doc["turn"].GetInt();
    req.syanten_type = doc["syanten_type"].GetInt();

    for (auto &v : doc["dora_tiles"].GetArray())
        req.dora_tiles.push_back(v.GetInt());

    std::vector<int> hand_tiles;
    for (auto &v : doc["hand_tiles"].GetArray())
        hand_tiles.push_back(v.GetInt());

    std::vector<MeldedBlock> melded_blocks;
    for (auto &v : doc["melded_blocks"].GetArray()) {
        int meld_type = v["type"].GetInt();
        std::vector<int> tiles;
        for (auto &v : v["tiles"].GetArray())
            tiles.push_back(v.GetInt());
        int discarded_tile = v["discarded_tile"].GetInt();
        int from           = v["from"].GetInt();
        melded_blocks.emplace_back(meld_type, tiles, discarded_tile, from);
    }
    req.hand = Hand(hand_tiles, melded_blocks);
    req.flag = doc["flag"].GetInt();

    std::cout
        << fmt::format(
               "場風牌: {}, 自風牌: {}, 巡目: {}, 手牌の種類: {}, 手牌: {}, フラグ: {}",
               Tile::Name.at(req.bakaze), Tile::Name.at(req.zikaze), req.turn,
               req.syanten_type, req.hand.to_string(), req.flag)
        << std::endl;

    return {true, req};
}

std::string to_json_str(rapidjson::Document &doc)
{
    std::stringstream ss;
    rapidjson::OStreamWrapper osw(ss);
    rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);
    doc.Accept(writer);

    return ss.str();
}

std::string Server::process_request(const std::string &json)
{
    rapidjson::Document doc(rapidjson::kObjectType);

    // JSON を読み込む。
    rapidjson::Document req_doc;
    req_doc.Parse(json.c_str());
    if (req_doc.HasParseError()) {
        doc.AddMember("success", false, doc.GetAllocator());
        doc.AddMember("err_msg", "Failed to parse request json due to invalid format.",
                      doc.GetAllocator());
    }

    // JSON を解析する。
    auto [success, req] = parse_json(req_doc);
    if (!success) {
        doc.AddMember("success", false, doc.GetAllocator());
        doc.AddMember("request", req_doc.GetObject(), doc.GetAllocator());
        doc.AddMember("err_msg", "Failed to parse request json due to invalid value.",
                      doc.GetAllocator());
        return to_json_str(doc);
    }

    // 計算する。
    rapidjson::Document res_doc = create_response(req);

    // 出力用 JSON を作成する。
    doc.AddMember("success", true, doc.GetAllocator());
    doc.AddMember("request", req_doc.GetObject(), doc.GetAllocator());
    doc.AddMember("response", res_doc.GetObject(), doc.GetAllocator());

    return to_json_str(doc);
}

void Server::test()
{
    std::ifstream ifs("request.json");
    std::stringstream ss;
    ss << ifs.rdbuf();

    auto future = pool_.enqueue([&] { return process_request(ss.str()); });
    std::cout << "enqueue complete" << std::endl;
    future.get();
    std::cout << "finished" << std::endl;
}

// This function produces an HTTP response for the given
// request. The type of the response object depends on the
// contents of the request, so the interface requires the
// caller to pass a generic lambda for receiving the response.
template <class Body, class Allocator, class Send>
void handle_request(beast::string_view doc_root,
                    http::request<Body, http::basic_fields<Allocator>> &&req,
                    Send &&send)
{
    // Returns a bad request response
    auto const bad_request = [&req](beast::string_view why) {
        http::response<http::string_body> res{http::status::bad_request, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = std::string(why);
        res.prepare_payload();
        return res;
    };

    // Returns a not found response
    auto const not_found = [&req](beast::string_view target) {
        http::response<http::string_body> res{http::status::not_found, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = "The resource '" + std::string(target) + "' was not found.";
        res.prepare_payload();
        return res;
    };

    // Returns a server error response
    auto const server_error = [&req](beast::string_view what) {
        http::response<http::string_body> res{http::status::internal_server_error,
                                              req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = "An error occurred: '" + std::string(what) + "'";
        res.prepare_payload();
        return res;
    };

    // Make sure we can handle the method
    if (req.method() != http::verb::post)
        return send(bad_request("Unknown HTTP-method"));

    // Request path must be absolute and not contain "..".
    if (req.target().empty() || req.target()[0] != '/' ||
        req.target().find("..") != beast::string_view::npos)
        return send(bad_request("Illegal request-target"));

    auto future =
        server.pool_.enqueue([&] { return server.process_request(req.body()); });
    http::string_body::value_type body;
    body = future.get();

    // Respond to GET request
    http::response<http::string_body> res{
        std::piecewise_construct, std::make_tuple(std::move(body)),
        std::make_tuple(http::status::ok, req.version())};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "application/json");
    res.set(http::field::access_control_allow_origin, "*");
    res.prepare_payload();
    res.keep_alive(req.keep_alive());
    return send(std::move(res));
}

// Report a failure
void fail(beast::error_code ec, char const *what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}

// This is the C++11 equivalent of a generic lambda.
// The function object is used to send an HTTP message.
template <class Stream> struct send_lambda {
    Stream &stream_;
    bool &close_;
    beast::error_code &ec_;

    explicit send_lambda(Stream &stream, bool &close, beast::error_code &ec)
        : stream_(stream)
        , close_(close)
        , ec_(ec)
    {
    }

    template <bool isRequest, class Body, class Fields>
    void operator()(http::message<isRequest, Body, Fields> &&msg) const
    {
        // Determine if we should close the connection after
        close_ = msg.need_eof();

        // We need the serializer here because the serializer requires
        // a non-const file_body, and the message oriented version of
        // http::write only works with const messages.
        http::serializer<isRequest, Body, Fields> sr{msg};
        http::write(stream_, sr, ec_);
    }
};

// Handles an HTTP server connection
void do_session(tcp::socket &socket, std::shared_ptr<std::string const> const &doc_root)
{
    bool close = false;
    beast::error_code ec;

    // This buffer is required to persist across reads
    beast::flat_buffer buffer;

    // This lambda is used to send messages
    send_lambda<tcp::socket> lambda{socket, close, ec};

    for (;;) {
        // Read a request
        http::request<http::string_body> req;
        http::read(socket, buffer, req, ec);
        if (ec == http::error::end_of_stream)
            break;
        if (ec)
            return fail(ec, "read");

        // Send the response
        handle_request(*doc_root, std::move(req), lambda);
        if (ec)
            return fail(ec, "write");
        if (close) {
            // This means we should close the connection, usually because
            // the response indicated the "Connection: close" semantic.
            break;
        }
    }

    // Send a TCP shutdown
    socket.shutdown(tcp::socket::shutdown_send, ec);

    // At this point the connection is closed gracefully
}

int Server::run()
{
    try {
        auto const address  = net::ip::make_address("0.0.0.0");
        auto const port     = static_cast<unsigned short>(std::atoi("8888"));
        auto const doc_root = std::make_shared<std::string>(".");

        // The io_context is required for all I/O
        net::io_context ioc{1};

        // The acceptor receives incoming connections
        tcp::acceptor acceptor{ioc, {address, port}};
        for (;;) {
            // This will receive the new connection
            tcp::socket socket{ioc};

            // Block until we get a connection
            acceptor.accept(socket);

            // Launch the session, transferring ownership of the socket
            std::thread{std::bind(&do_session, std::move(socket), doc_root)}.detach();
        }
    }
    catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int main()
{
    return server.run();
}
