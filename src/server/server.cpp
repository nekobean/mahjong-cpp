#include "server.hpp"

#include <fstream>
#include <iostream>

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/dll.hpp>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/schema.h>
#include <rapidjson/stringbuffer.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

//#define OUTPUT_DETAIL_LOG
namespace beast = boost::beast;   // from <boost/beast.hpp>
namespace http = beast::http;     // from <boost/beast/http.hpp>
namespace net = boost::asio;      // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

Server server;

Server::Server() : pool_(3)
{
}

void Server::log_request(const Request &req)
{
    // std::string dora_indicators = "";
    // for (const auto &tile : req.dora_indicators)
    //     dora_indicators +=
    //         Tile::Name.at(tile) + (&tile != &req.dora_indicators.back() ? " " : "");

    // std::string counts = "";
    // for (const auto &count : req.counts)
    //     counts += std::to_string(count);

    using namespace mahjong;

    const std::string round_wind = Tile::Name.at(req.round.wind);
    const std::string seat_wind = Tile::Name.at(req.player.wind);
    const std::string hand = to_mpsz(req.player.hand);
    std::string melds;
    for (const auto &meld : req.player.melds) {
        melds += to_string(meld);
    }
    std::string dora_indicators = to_mpsz(req.round.dora_indicators);
    std::string wall;
    for (const auto c : req.wall) {
        wall += std::to_string(c);
    }

    spdlog::get("logger")->info(
        "ip: {}, version: {}, "
        "round: {}, seat: {}, indicators: {}, "
        "hand: {}, melds: {}, wall: {}, "
        "reddora: {}, uradora: {}, shantendown: {}, tegawari: {}, riichi: {}",
        req.ip, req.version, round_wind, seat_wind, dora_indicators, hand, melds, wall,
        req.config.enable_reddora, req.config.enable_uradora,
        req.config.enable_shanten_down, req.config.enable_tegawari,
        req.config.enable_riichi);

#ifdef OUTPUT_DETAIL_LOG
    {
        boost::filesystem::path save_dir =
            boost::dll::program_location().parent_path() / "requests";
        if (!boost::filesystem::exists(save_dir))
            boost::filesystem::create_directory(save_dir);
        std::string req_save_path =
            save_dir.string() + "\\" + (to_mpsz(req.hand.counts) + ".json");
        std::filesystem::path pa =
            std::filesystem::u8path((const char *)req_save_path.c_str());
        std::ofstream ofs(pa.string());
        ofs << json;
        ofs.close();
    }

    {
        boost::filesystem::path save_dir =
            boost::dll::program_location().parent_path() / "response";
        if (!boost::filesystem::exists(save_dir))
            boost::filesystem::create_directory(save_dir);
        std::string req_save_path =
            save_dir.string() + "\\" + (to_mpsz(req.hand.counts) + ".json");
        std::filesystem::path pa =
            std::filesystem::u8path((const char *)req_save_path.c_str());
        std::ofstream ofs(pa.string());

        std::cout << to_json_str(doc) << std::endl;

        ofs << to_json_str(doc);
        ofs.close();
    }
#endif
}

std::string Server::process_request(const std::string &json)
{
    rapidjson::Document req_doc;
    rapidjson::Document res_doc;
    res_doc.SetObject();

    // Parse request JSON.
    try {
        parse_json(json, req_doc);
    }
    catch (const std::exception &e) {
        res_doc.AddMember("success", false, res_doc.GetAllocator());
        res_doc.AddMember("request", req_doc, res_doc.GetAllocator());
        res_doc.AddMember("err_msg", dump_string(e.what(), res_doc),
                          res_doc.GetAllocator());
        spdlog::get("logger")->error("Failed to parse json. ({})", e.what());
        std::cout << to_json_str(res_doc) << std::endl;
        return to_json_str(res_doc);
    }

    const std::string ip = req_doc.HasMember("ip") ? req_doc["ip"].GetString() : "";

    // Create request object.
    try {
        Request req = parse_request_doc(req_doc);
        log_request(req);
        rapidjson::Value res_val = create_response(req, res_doc);
        res_doc.AddMember("success", true, res_doc.GetAllocator());
        res_doc.AddMember("request", req_doc, res_doc.GetAllocator());
        res_doc.AddMember("response", res_val, res_doc.GetAllocator());
    }
    catch (const std::exception &e) {
        res_doc.AddMember("success", false, res_doc.GetAllocator());
        res_doc.AddMember("request", req_doc, res_doc.GetAllocator());
        res_doc.AddMember("err_msg", dump_string(e.what(), res_doc),
                          res_doc.GetAllocator());
        if (std::string(u8"手牌はすでに和了形です。") == e.what()) {
            spdlog::get("logger")->info("ip: {}, error: {}", ip, e.what());
        }
        else {
            spdlog::get("logger")->error("ip: {}, error: {}", ip, e.what());
        }
        return to_json_str(res_doc);
    }

    return to_json_str(res_doc);
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
template <class Stream> struct send_lambda
{
    Stream &stream_;
    bool &close_;
    beast::error_code &ec_;

    explicit send_lambda(Stream &stream, bool &close, beast::error_code &ec)
        : stream_(stream), close_(close), ec_(ec)
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
    spdlog::get("logger")->info("Launching server...");

    try {
        auto const address = net::ip::make_address("0.0.0.0");
        auto const port = static_cast<unsigned short>(std::atoi("8888"));
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
        spdlog::get("logger")->error("Error: {}", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int main()
{
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto file_sink =
        std::make_shared<spdlog::sinks::basic_file_sink_mt>("log.txt", false);
    std::vector<spdlog::sink_ptr> sinks = {console_sink, file_sink};
    auto logger =
        std::make_shared<spdlog::logger>("logger", sinks.begin(), sinks.end());
    spdlog::register_logger(logger);
    spdlog::flush_every(std::chrono::seconds(3));

    spdlog::get("logger")->info("{} {}", PROJECT_NAME, PROJECT_VERSION);

    return server.run();
}
