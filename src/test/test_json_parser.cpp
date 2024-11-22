#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_ENABLE_BENCHMARKING

#include <iostream>

#include <catch2/catch.hpp>
#include <spdlog/spdlog.h>

#include "mahjong/json_parser.hpp"
#include "mahjong/mahjong.hpp"

using namespace mahjong;

TEST_CASE("create_request()")
{
    const std::string json = R"(
        {
            "enable_reddora": true,
            "enable_uradora": true,
            "enable_shanten_down": true,
            "enable_tegawari": true,
            "round_wind": 27,
            "dora_indicators": [27],
            "hand": [11, 12, 20, 20, 23, 23, 24, 30],
            "melds": [{"type": 1, "tiles": [1, 1, 1]}, {"type": 2, "tiles": [4, 5, 6]}],
            "seat_wind": 27,
            "wall": [4, 1, 4, 4, 3, 3, 3, 4, 4, 4,
                       4, 3, 3, 4, 4, 4, 4, 4, 4, 4,
                       2, 4, 4, 2, 3, 4, 4, 3, 4, 4,
                       3, 4, 4, 4, 1, 1, 1],
            "version": "0.9.1"
        }
    )";

    rapidjson::Document doc;
    doc.Parse(json.c_str());
    if (doc.HasParseError()) {
        spdlog::error("Error parsing JSON");
        return;
    }

    Request req = create_request(doc);
    REQUIRE(req.config.enable_reddora == true);
    REQUIRE(req.config.enable_uradora == true);
    REQUIRE(req.config.enable_shanten_down == true);
    REQUIRE(req.config.enable_tegawari == true);
    REQUIRE(req.round.wind == 27);
    REQUIRE(req.round.dora_indicators.size() == 1);
    REQUIRE(req.round.dora_indicators[0] == 27);
    REQUIRE(req.player.hand == from_array({11, 12, 20, 20, 23, 23, 24, 30}));
    REQUIRE(req.player.melds.size() == 2);
    REQUIRE(req.player.melds[0].type == 1);
    REQUIRE(req.player.melds[0].tiles == std::vector<int>({1, 1, 1}));
    REQUIRE(req.player.melds[1].type == 2);
    REQUIRE(req.player.melds[1].tiles == std::vector<int>({4, 5, 6}));
    REQUIRE(req.player.wind == 27);
    REQUIRE(req.wall == Hand{4, 1, 4, 4, 3, 3, 3, 4, 4, 4, 4, 3, 3, 4, 4, 4, 4, 4, 4,
                             4, 2, 4, 4, 2, 3, 4, 4, 3, 4, 4, 3, 4, 4, 4, 1, 1, 1});
    REQUIRE(req.version == "0.9.1");
}

TEST_CASE("dump_necessary_tiles()")
{
    Hand hand = from_mpsz("222567m34p33667s");
    int num_melds = 0;
    Hand count;
    for (int i = 0; i < 34; ++i) {
        count[i] = 4 - hand[i];
    }

    // Calculate necessary tiles.
    const auto [shanten_type, shanten, tiles] =
        NecessaryTileCalculator::select(hand, num_melds, ShantenFlag::All);

    std::vector<std::tuple<int, int>> neccessary_tiles;
    for (const auto tile : tiles) {
        neccessary_tiles.emplace_back(tile, count[tile]);
    }

    rapidjson::Document doc;
    rapidjson::Value value = dump_necessary_tiles(neccessary_tiles, doc);

    std::cout << to_json_str(value) << std::endl;
}

TEST_CASE("dump_expected_score()")
{
    // Player information
    //////////////////////////////////////////
    Player player;
    player.hand = from_mpsz("056m3458p1345579s");
    player.melds = {};
    player.wind = Tile::East;
    if (player.num_tiles() + player.num_melds() * 3 != 14) {
        spdlog::error("Number of tiles should be 14.");
        return;
    }

    // Round information
    //////////////////////////////////////////
    Round round;
    round.rules = RuleFlag::OpenTanyao | RuleFlag::RedDora;
    round.wind = Tile::East;
    round.kyoku = 1;
    round.honba = 0;
    round.kyotaku = 0;
    round.dora_indicators = {};
    round.uradora_indicators = {};

    // Calculation Settings
    //////////////////////////////////////////
    ExpectedScoreCalculator::Config config;
    config.t_min = 1;
    config.t_max = 18;
    config.sum = 121; // 136 - 14 - 1
    config.extra = 1;
    config.shanten_type = ShantenFlag::All;
    config.enable_reddora = true;
    config.enable_shanten_down = true;
    config.enable_tegawari = true;
    config.enable_uradora = true;

    // Calculation
    //////////////////////////////////////////
    const auto [type, shanten] =
        ShantenCalculator::calc(player.hand, player.num_melds(), config.shanten_type);

    // Calculate tenpai probability, win probability, and expected score.
    const auto [stats, searched] = ExpectedScoreCalculator::calc(config, round, player);

    rapidjson::Document doc;
    rapidjson::Value value = dump_expected_score(stats, doc);

    std::cout << to_json_str(value) << std::endl;
}

void create_string(const std::string &json)
{
    rapidjson::Document req_doc;
    rapidjson::Document res_doc;
    res_doc.SetObject();

    Request req;
    try {
        req = parse_json_str(json, req_doc);
        create_response(req, res_doc);
    }
    catch (const std::exception &e) {
        res_doc.AddMember("success", false, res_doc.GetAllocator());
        res_doc.AddMember("err_msg", dump_string(e.what(), res_doc),
                          res_doc.GetAllocator());
    }

    std::cout << to_json_str(res_doc) << std::endl;
}

TEST_CASE("create_response()")
{
    SECTION("error (Failed to parse json schema)")
    {
        const std::string json = R"(
            {
                "enable_reddora": true,
                "enable_uradora": true,
                "enable_shanten_down": true,
                "enable_tegawari": true,
                "round_wind": 27,
                "dora_indicators": [27],
                "hand": [1, 1, 20, 20, 23, 23, 24, 30],
                "melds": [{"type": 1, "tiles": [1, 1, 1]}, {"type": 2, "tiles": [4, 5, 6]}],
                "seat_wind": 27,
                "version": "0.9.1,
            }
        )";
        create_string(json);
    }

    SECTION("error (Failed to validate json schema)")
    {
        // seat_wind is missing
        const std::string json = R"(
            {
                "enable_reddora": true,
                "enable_uradora": true,
                "enable_shanten_down": true,
                "enable_tegawari": true,
                "round_wind": 27,
                "dora_indicators": [27],
                "hand": [1, 1, 20, 20, 23, 23, 24, 30],
                "melds": [{"type": 1, "tiles": [1, 1, 1]}, {"type": 2, "tiles": [4, 5, 6]}],
                "wall": [4, 1, 4, 4, 3, 3, 3, 4, 4, 4,
                        4, 3, 3, 4, 4, 4, 4, 4, 4, 4,
                        2, 4, 4, 2, 3, 4, 4, 3, 4, 4,
                        3, 4, 4, 4, 1, 1, 1],
                "version": "0.9.1"
            }
        )";
        create_string(json);
    }

    SECTION("error (Version mismatch detected)")
    {
        const std::string json = R"(
            {
                "enable_reddora": true,
                "enable_uradora": true,
                "enable_shanten_down": true,
                "enable_tegawari": true,
                "round_wind": 27,
                "dora_indicators": [27],
                "hand": [1, 1, 20, 20, 23, 23, 24, 30],
                "melds": [{"type": 1, "tiles": [1, 1, 1]}, {"type": 2, "tiles": [4, 5, 6]}],
                "seat_wind": 27,
                "wall": [4, 1, 4, 4, 3, 3, 3, 4, 4, 4,
                        4, 3, 3, 4, 4, 4, 4, 4, 4, 4,
                        2, 4, 4, 2, 3, 4, 4, 3, 4, 4,
                        3, 4, 4, 4, 1, 1, 1],
                "version": "0.9.0"
            }
        )";
        create_string(json);
    }

    SECTION("error (More than 5 tiles are used)")
    {
        const std::string json = R"(
            {
                "enable_reddora": true,
                "enable_uradora": true,
                "enable_shanten_down": true,
                "enable_tegawari": true,
                "round_wind": 27,
                "dora_indicators": [27],
                "hand": [1, 1, 20, 20, 23, 23, 24, 30],
                "melds": [{"type": 1, "tiles": [1, 1, 1]}, {"type": 2, "tiles": [4, 5, 6]}],
                "seat_wind": 27,
                "wall": [4, 1, 4, 4, 3, 3, 3, 4, 4, 4,
                        4, 3, 3, 4, 4, 4, 4, 4, 4, 4,
                        2, 4, 4, 2, 3, 4, 4, 3, 4, 4,
                        3, 4, 4, 4, 1, 1, 1],
                "version": "0.9.1"
            }
        )";
        create_string(json);
    }

    SECTION("error (More tiles than wall are used)")
    {
        const std::string json = R"(
            {
                "enable_reddora": true,
                "enable_uradora": true,
                "enable_shanten_down": true,
                "enable_tegawari": true,
                "round_wind": 27,
                "dora_indicators": [27],
                "hand": [11, 12, 20, 20, 23, 23, 24, 30],
                "melds": [{"type": 1, "tiles": [1, 1, 1]}, {"type": 2, "tiles": [4, 5, 6]}],
                "seat_wind": 27,
                "wall": [4, 1, 4, 4, 4, 3, 3, 4, 4, 4,
                        4, 3, 3, 4, 4, 4, 4, 4, 4, 4,
                        2, 4, 4, 2, 3, 4, 4, 3, 4, 4,
                        3, 4, 4, 4, 1, 1, 1],
                "version": "0.9.1"
            }
        )";
        create_string(json);
    }

    SECTION("success (wall not specified)")
    {
        const std::string json = R"(
            {
                "enable_reddora": true,
                "enable_uradora": true,
                "enable_shanten_down": true,
                "enable_tegawari": true,
                "round_wind": 27,
                "dora_indicators": [27],
                "hand": [11, 12, 20, 20, 23, 23, 24, 30],
                "melds": [{"type": 1, "tiles": [1, 1, 1]}, {"type": 2, "tiles": [4, 5, 6]}],
                "seat_wind": 27,
                "version": "0.9.1"
            }
        )";
        create_string(json);
    }

    SECTION("success")
    {
        const std::string json = R"(
            {
                "enable_reddora": true,
                "enable_uradora": true,
                "enable_shanten_down": true,
                "enable_tegawari": true,
                "round_wind": 27,
                "dora_indicators": [27],
                "hand": [11, 12, 20, 20, 23, 23, 24, 30],
                "melds": [{"type": 1, "tiles": [1, 1, 1]}, {"type": 2, "tiles": [4, 5, 6]}],
                "seat_wind": 27,
                "wall": [4, 1, 4, 4, 3, 3, 3, 4, 4, 4,
                        4, 3, 3, 4, 4, 4, 4, 4, 4, 4,
                        2, 4, 4, 2, 3, 4, 4, 3, 4, 4,
                        3, 4, 4, 4, 1, 1, 1],
                "version": "0.9.1"
            }
        )";
        create_string(json);
    }
}
