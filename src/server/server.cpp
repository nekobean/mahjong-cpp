#include "server.hpp"

#include "rapidjson/istreamwrapper.h"
#include "rapidjson/schema.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include <boost/dll.hpp>

#include <chrono>
#include <fstream>
#include <iostream>

//#define OUTPUT_DETAIL_LOG
#ifdef OUTPUT_DETAIL_LOG
#include <filesystem>
#endif

namespace beast = boost::beast;   // from <boost/beast.hpp>
namespace http = beast::http;     // from <boost/beast/http.hpp>
namespace net = boost::asio;      // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

using namespace mahjong;

Server server;

Server::Server() : pool_(3) {}

/**
 * @brief JSON データを解析する。
 *
 * @param[in] doc ドキュメント
 * @return (成功したかどうか, リクエストデータ)
 */
std::tuple<bool, RequestData> Server::parse_json(const rapidjson::Document &doc)
{
    RequestData req = parse_request(doc);

    std::string dora_indicators = "";
    for (const auto &tile : req.dora_indicators)
        dora_indicators += Tile::Name.at(tile) + (&tile != &req.dora_indicators.back() ? " " : "");
    spdlog::get("logger")->info(
        "IP: {}, バージョン: {}, 場風牌: {}, 自風牌: {}, 巡目: {}, 手牌の種類: {}, 手牌: "
        "{}, フラグ: {}, ドラ表示牌: {}",
        req.ip, req.version, Tile::Name.at(req.bakaze), Tile::Name.at(req.zikaze), req.turn,
        req.syanten_type, req.hand.to_string(), req.flag, dora_indicators);

    auto counts = ExpectedValueCalculator::count_left_tiles(req.hand, req.dora_indicators);
    for (auto x : counts) {
        if (x < 0)
            return {false, req};
    }

    return {true, req};
}

std::string Server::process_request(const std::string &json)
{
    rapidjson::Document doc(rapidjson::kObjectType);

    // リクエストデータを読み込む。
    rapidjson::Document req_doc;
    req_doc.Parse(json.c_str());
    if (req_doc.HasParseError()) {
        doc.AddMember("success", false, doc.GetAllocator());
        doc.AddMember("err_msg", "Failed to parse request data (invalid json format).",
                      doc.GetAllocator());
        return to_json_str(doc);
    }

    std::string schema_path =
        (boost::dll::program_location().parent_path() / "request_schema.json").string();
    std::ifstream ifs(schema_path);
    rapidjson::IStreamWrapper isw(ifs);
    rapidjson::Document sd;
    sd.ParseStream(isw);
    rapidjson::SchemaDocument schema(sd);
    rapidjson::SchemaValidator validator(schema);

    std::string req_version;
    if (req_doc.HasMember("version"))
        req_version = req_doc["version"].GetString();

    if (req_version != PROJECT_VERSION) {
        spdlog::get("logger")->error("バージョンが不一致 {} vs {}", req_version, PROJECT_VERSION);
        doc.AddMember("success", false, doc.GetAllocator());
        doc.AddMember("request", req_doc.GetObject(), doc.GetAllocator());
        doc.AddMember("err_msg",
                      "アプリのバージョンが古いです。キャッシュの影響だと思われるので、ブラウザでペ"
                      "ージを再読み込みしてください。",
                      doc.GetAllocator());
        return to_json_str(doc);
    }

    if (!req_doc.Accept(validator)) {
        // rapidjson::StringBuffer sb;
        // validator.GetInvalidSchemaPointer().StringifyUriFragment(sb);
        // printf("Invalid schema: %s\n", sb.GetString());
        // printf("Invalid keyword: %s\n", validator.GetInvalidSchemaKeyword());
        doc.AddMember("success", false, doc.GetAllocator());
        doc.AddMember("request", req_doc.GetObject(), doc.GetAllocator());
        doc.AddMember("err_msg", "Failed to parse request data (invalid value found).",
                      doc.GetAllocator());
        return to_json_str(doc);
    }

    // JSON を解析する。
    auto [success, req] = parse_json(req_doc);
    if (!success) {
        doc.AddMember("success", false, doc.GetAllocator());
        doc.AddMember("request", req_doc.GetObject(), doc.GetAllocator());
        doc.AddMember("err_msg", "Failed to parse request data (invalid hand found).",
                      doc.GetAllocator());
        return to_json_str(doc);
    }

    // 向聴数を計算する。
    auto [syanten_type, syanten] = SyantenCalculator::calc(req.hand, req.syanten_type);
    if (syanten == -1) {
        doc.AddMember("success", false, doc.GetAllocator());
        doc.AddMember("request", req_doc.GetObject(), doc.GetAllocator());
        doc.AddMember("err_msg", "和了形です。", doc.GetAllocator());
        return to_json_str(doc);
    }

    // 計算する。
    rapidjson::Value res_doc = create_response(req, doc);

    // 出力用 JSON を作成する。
    doc.AddMember("success", true, doc.GetAllocator());
    doc.AddMember("request", req_doc.GetObject(), doc.GetAllocator());
    doc.AddMember("response", res_doc, doc.GetAllocator());

#ifdef OUTPUT_DETAIL_LOG
    {
        boost::filesystem::path save_dir =
            boost::dll::program_location().parent_path() / "requests";
        if (!boost::filesystem::exists(save_dir))
            boost::filesystem::create_directory(save_dir);
        std::string req_save_path = save_dir.string() + "\\" + (req.hand.to_string() + ".json");
        std::filesystem::path pa = std::filesystem::u8path((const char *)req_save_path.c_str());
        std::ofstream ofs(pa.string());
        ofs << json;
        ofs.close();
    }

    {
        boost::filesystem::path save_dir =
            boost::dll::program_location().parent_path() / "response";
        if (!boost::filesystem::exists(save_dir))
            boost::filesystem::create_directory(save_dir);
        std::string req_save_path = save_dir.string() + "\\" + (req.hand.to_string() + ".json");
        std::filesystem::path pa = std::filesystem::u8path((const char *)req_save_path.c_str());
        std::ofstream ofs(pa.string());

        std::cout << to_json_str(doc) << std::endl;

        ofs << to_json_str(doc);
        ofs.close();
    }
#endif

    return to_json_str(doc);
}

// This function produces an HTTP response for the given
// request. The type of the response object depends on the
// contents of the request, so the interface requires the
// caller to pass a generic lambda for receiving the response.
template <class Body, class Allocator, class Send>
void handle_request(beast::string_view doc_root,
                    http::request<Body, http::basic_fields<Allocator>> &&req, Send &&send)
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
        http::response<http::string_body> res{http::status::internal_server_error, req.version()};
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

    auto future = server.pool_.enqueue([&] { return server.process_request(req.body()); });
    http::string_body::value_type body;
    body = future.get();

    // Respond to GET request
    http::response<http::string_body> res{std::piecewise_construct,
                                          std::make_tuple(std::move(body)),
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
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("log.txt", false);
    std::vector<spdlog::sink_ptr> sinks = {console_sink, file_sink};
    auto logger = std::make_shared<spdlog::logger>("logger", sinks.begin(), sinks.end());
    spdlog::register_logger(logger);
    spdlog::flush_every(std::chrono::seconds(3));

    spdlog::get("logger")->info("{} {}", PROJECT_NAME, PROJECT_VERSION);

    return server.run();
}
