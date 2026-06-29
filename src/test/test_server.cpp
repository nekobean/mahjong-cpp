#define CATCH_CONFIG_MAIN

#include <atomic>
#include <condition_variable>
#include <future>
#include <mutex>
#include <string>

#include <catch2/catch.hpp>
#include <rapidjson/document.h>
#include <spdlog/sinks/null_sink.h>
#include <spdlog/spdlog.h>

#include "server/server.hpp"

namespace
{

void initialize_test_logger()
{
    if (spdlog::get("logger") != nullptr) {
        return;
    }

    auto sink = std::make_shared<spdlog::sinks::null_sink_mt>();
    auto logger = std::make_shared<spdlog::logger>("logger", sink);
    spdlog::register_logger(logger);
}

std::string make_valid_request_json()
{
    return R"({
        "game_mode": 0,
        "round_wind": 27,
        "seat_wind": 27,
        "dora_indicators": [31],
        "enable_reddora": true,
        "enable_uradora": true,
        "enable_shanten_down": true,
        "enable_tegawari": true,
        "hand": [0, 0, 0, 1, 1, 1, 2, 3, 5, 7, 8, 9, 9],
        "melds": [],
        "version": "0.9.1"
    })";
}

} // namespace

TEST_CASE("process_http_post returns service unavailable when the queue is full")
{
    initialize_test_logger();

    std::promise<void> release_promise;
    std::shared_future<void> release_future = release_promise.get_future().share();

    std::mutex started_mutex;
    std::condition_variable started_cv;
    std::atomic<int> started_tasks = 0;

    auto blocking_task = [&] {
        {
            std::lock_guard<std::mutex> lock(started_mutex);
            ++started_tasks;
        }
        started_cv.notify_all();
        release_future.wait();
        return std::string("done");
    };

    Server test_server;

    for (int i = 0; i < 3; ++i) {
        test_server.pool_.enqueue(blocking_task);
    }

    {
        std::unique_lock<std::mutex> lock(started_mutex);
        started_cv.wait(lock, [&] { return started_tasks.load() == 3; });
    }

    for (int i = 0; i < 20; ++i) {
        test_server.pool_.enqueue(blocking_task);
    }

    const Server::HttpResponse response =
        test_server.process_http_post(make_valid_request_json());

    release_promise.set_value();

    REQUIRE(response.status == 503);
    REQUIRE(response.content_type == "application/json");

    rapidjson::Document body;
    body.Parse(response.body.c_str());
    REQUIRE_FALSE(body.HasParseError());
    REQUIRE_FALSE(body["success"].GetBool());
    REQUIRE(std::string(body["err_msg"].GetString()) == "Server busy.");
}
