#undef NDEBUG
#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_ENABLE_BENCHMARKING

#include <array>
#include <cassert>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/dll.hpp>
#include <catch2/catch.hpp>
#include <spdlog/spdlog.h>

#include "compare/settile.hpp"
#include "compare/win_prob1.hpp"
#include "compare/win_prob2.hpp"
#include "compare/win_prob3.hpp"
#include "mahjong/mahjong.hpp"

using namespace mahjong;

using TestCase = Player;

/**
 * Loads a test case from the specified file.
 *
 * @param filepath The path to the file containing the test case data.
 * @param cases list of test cases.
 */
bool load_testcase(std::vector<TestCase> &cases)
{
    cases.clear();

    boost::filesystem::path path = boost::filesystem::path(CMAKE_TESTCASE_DIR) /
                                   "test_unnecessary_tile_calculator.txt";

    std::ifstream ifs(path.string());
    if (!ifs) {
        std::cerr << "Failed to open " << path.string() << "." << std::endl;
        return false;
    }

    // The format is `<tile1> <tile2> ... <tile14>`
    std::string line;
    while (std::getline(ifs, line)) {
        if (line.empty()) {
            continue;
        }

        std::vector<std::string> tokens;
        boost::split(tokens, line, boost::is_any_of(" "));

        std::vector<int> tiles(14);
        for (int i = 0; i < 14; ++i) {
            tiles[i] = std::stoi(tokens[i]);
        }
        Player player(tiles, Tile::East);

        cases.emplace_back(player);
    }

    spdlog::info("{} testcases loaded.", cases.size());

    return true;
}

void print_stats(const ExpectedScoreCalculator::Config &params,
                 const std::vector<ExpectedScoreCalculator::Stat> &stats,
                 const Player &player, int shanten, int elapsed_ms, int searched)
{
    // clang-format off
    const int pad = 15;
    std::cout << std::boolalpha;
    std::cout << "=== Params ===" << std::endl;
    std::cout << std::setw(pad) << std::right << "min turn: " << std::left << params.t_min << std::endl;
    std::cout << std::setw(pad) << std::right << "max turn: " << std::left << params.t_max << std::endl;
    std::cout << std::setw(pad) << std::right << "wall tiles: " << std::left << params.sum << std::endl;
    std::cout << std::setw(pad) << std::right << "extra: " << std::left << params.extra << std::endl;
    std::cout << std::setw(pad) << std::right << "shanten type: " << std::left << params.shanten_type << std::endl;
    std::cout << std::setw(pad) << std::right << "shanten down: " << std::left << params.enable_shanten_down << std::endl;
    std::cout << std::setw(pad) << std::right << "tegawari: " << std::left << params.enable_tegawari << std::endl;
    std::cout << std::setw(pad) << std::right << "reddora: " << std::left << params.enable_reddora << std::endl;
    std::cout << std::setw(pad) << std::right << "uradora: " << std::left << params.enable_uradora << std::endl;
    std::cout << std::setw(pad) << std::right << "riichi: " << std::left << params.enable_riichi << std::endl;
    std::cout << "=== Input ===" << std::endl;
    std::cout << std::setw(pad) << std::right << "hand: " << std::left << to_mpsz(player.hand) << std::endl;
    std::cout << "=== Info ===" << std::endl;
    std::cout << std::setw(pad) << std::right << "shanten: " << std::left << shanten << std::endl;
    std::cout << std::setw(pad) << std::right << "time: " << std::left << elapsed_ms << " ms" << std::endl;
    std::cout << std::setw(pad) << std::right << "searched: " << std::left << searched << " hands" << std::endl;
    // // clang-format on
    // std::cout << "=== Necessary Tiles ===" << std::endl;
    // for (const auto &stat : stats) {
    //     std::cout << fmt::format("{:>2} ", Tile::Name.at(stat.tile));

    //     int sum =
    //         std::accumulate(stat.necessary_tiles.begin(), stat.necessary_tiles.end(), 0,
    //                         [](int s, const auto &x) { return s + std::get<1>(x); });
    //     std::cout << fmt::format(
    //         "type: {:2<}, sum: {:3<}, tiles: ", stat.necessary_tiles.size(), sum);

    //     for (const auto [tile, count] : stat.necessary_tiles) {
    //         std::cout << fmt::format("{}({}) ", Tile::Name.at(tile), count);
    //     }
    //     std::cout << std::endl;
    // }
    // std::cout << std::endl;

    // std::cout << "=== Tenpai Probability ===" << std::endl;
    // std::cout << fmt::format("{:>4}", "turn");
    // for (const auto &stat : stats) {
    //     std::cout << fmt::format("{:>8}", Tile::Name.at(stat.tile));
    // }
    // std::cout << std::endl;

    // std::cout << std::fixed;
    // for (int t = params.t_min; t <= params.t_max; ++t) {
    //     std::cout << fmt::format("{:>4}", t);
    //     for (const auto &stat : stats) {
    //         std::cout << fmt::format("{:>7.2f}%", stat.tenpai_prob[t] * 100);
    //     }
    //     std::cout << std::endl;
    // }

    // std::cout << "=== Win Probability ===" << std::endl;
    // std::cout << fmt::format("{:>4}", "turn");
    // for (const auto &stat : stats) {
    //     std::cout << fmt::format("{:>8}", Tile::Name.at(stat.tile));
    // }
    // std::cout << std::endl;

    // std::cout << std::fixed;
    // for (int t = params.t_min; t <= params.t_max; ++t) {
    //     std::cout << fmt::format("{:>4}", t);
    //     for (const auto &stat : stats) {
    //         std::cout << fmt::format("{:>7.2f}%", stat.win_prob[t] * 100);
    //     }
    //     std::cout << std::endl;
    // }

    std::cout << "=== Expected Score ===" << std::endl;
    std::cout << fmt::format("{:>4}", "turn");
    for (const auto &stat : stats) {
        std::cout << fmt::format("{:>9}", Tile::Name.at(stat.tile));
    }
    std::cout << std::endl;

    std::cout << std::fixed;
    for (int t = params.t_min; t <= params.t_max; ++t) {
        std::cout << fmt::format("{:>4}", t);
        for (const auto &stat : stats) {
            std::cout << fmt::format("{:>9.2f}", stat.exp_value[t]);
        }
        std::cout << std::endl;
    }
}

TEST_CASE("Expected score calculator")
{
    std::vector<TestCase> cases;
    if (!load_testcase(cases)) {
        return;
    }

    CalshtDW calsht;
    win_prob::win_prob2::WinProb2 win_prob(calsht);
    ExpectedScoreCalculator my_prob;
    auto table = std::make_shared<player_impl::Table>();
    auto player = std::make_shared<player_impl::Player>(table);
    std::string hand_str = "2345m56p6667s5z";
    // params.extra = 0;
    // params.enable_reddora = false;
    // set_hand("222567m345p33667s", *player); // 6s が 4124.43 点になるはず

    // Round information
    //////////////////////////////////////////
    Round round;
    round.rules = RuleFlag::OpenTanyao | RuleFlag::RedDora;
    round.wind = Tile::East;
    round.honba = 0;
    round.kyotaku = 0;
    round.dora_indicators = {};
    round.uradora_indicators = {};

    // Player information
    //////////////////////////////////////////
    Player my_player;
    my_player.wind = Tile::East;
    my_player.hand = from_mpsz(hand_str);
    my_player.melds = {{MeldType::Pong, {Tile::Red, Tile::Red, Tile::Red}}};

    // Calculation Settings
    //////////////////////////////////////////
    ExpectedScoreCalculator::Config params;
    params.t_min = 1;
    params.t_max = 18;
    params.sum = 121; // 136 - 14 - 1
    params.extra = 1;
    params.shanten_type = 7;
    params.enable_reddora = false;
    params.enable_shanten_down = true;
    params.enable_tegawari = true;
    params.enable_uradora = true;
    params.enable_riichi = true;

    // 同じのを設定
    table->wind2 = round.wind;
    player->wind1 = my_player.wind;
    player->tsumo = true;
    player->num = 14;
    set_hand("2345m56p6667s5777z", *player);
    player->call_pong({Tile::Red, false}, {{Tile::Red, false}, {Tile::Red, false}, {Tile::Red, false}});

    // ドラ設定
    // set_dora("1m", *table);
    // round.dora_indicators = {Tile::Manzu1};
    std::cout << "==========>" << mahjong::to_string(my_player) << std::endl;

    auto [mode, shanten] =
        ShantenCalculator::calc(my_player.hand, 0, params.shanten_type);

    const auto start = std::chrono::system_clock::now();
    const auto [stats, searched] = win_prob(round, *player, params, my_player);
    const auto end = std::chrono::system_clock::now();
    const int elapsed_ms = static_cast<int>(
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());

    const auto start3 = std::chrono::system_clock::now();
    const auto [stats3, searched3] = my_prob.calc(params, round, my_player);
    const auto end3 = std::chrono::system_clock::now();
    const int elapsed_ms2 = static_cast<int>(
        std::chrono::duration_cast<std::chrono::milliseconds>(end3 - start3).count());

    print_stats(params, stats3, my_player, shanten, elapsed_ms2, int(searched));

    std::cout << "elapsed_ms: " << elapsed_ms << " us" << std::endl;
    std::cout << "elapsed_ms2: " << elapsed_ms2 << " us" << std::endl;

    REQUIRE(stats.size() == stats3.size());
    for (int i = 0; i < stats.size(); ++i) {
        //REQUIRE(stats[i].tile == stats3[i].tile);
        for (int j = 0; j <= params.t_max; ++j) {
            REQUIRE(stats[i].prob[j] == Approx(stats3[i].exp_value[j]));
        }
    }
}

// TEST_CASE("Win probability calculator")
// {
//     std::vector<TestCase> cases;
//     if (!load_testcase(cases)) {
//         return;
//     }

//     CalshtDW calsht;
//     win_prob::win_prob2::WinProb2 win_prob(calsht);
//     ExpectedScoreCalculator my_prob;
//     WinProb3 win_prob3;
//     auto table = std::make_shared<player_impl::Table>();
//     auto player = std::make_shared<player_impl::Player>(table);
//     // params.extra = 0;
//     // params.enable_reddora = false;
//     // set_hand("222567m345p33667s", *player); // 6s が 4124.43 点になるはず

//     SECTION("Expected score calculator")
//     {
//         for (int i = 0; i < 100; ++i) {
//             // Round information
//             //////////////////////////////////////////
//             Round round;
//             round.rules = RuleFlag::OpenTanyao | RuleFlag::RedDora;
//             round.wind = Tile::East;
//             round.honba = 0;
//             round.kyotaku = 0;
//             round.dora_indicators = {};
//             round.uradora_indicators = {};

//             // Player information
//             //////////////////////////////////////////
//             Player my_player = cases[i];
//             my_player.wind = Tile::East;

//             // Calculation Settings
//             //////////////////////////////////////////
//             ExpectedScoreCalculator::Config params;
//             params.t_min = 1;
//             params.t_max = 18;
//             params.sum = 121; // 136 - 14 - 1
//             params.extra = 1;
//             params.shanten_type = 7;
//             params.enable_reddora = false;
//             params.enable_shanten_down = true;
//             params.enable_tegawari = true;
//             params.enable_uradora = true;
//             params.enable_riichi = false;

//             // 同じのを設定
//             table->wind2 = round.wind;
//             player->wind1 = my_player.wind;
//             player->tsumo = true;
//             player->num = 14;

//             auto player = std::make_shared<player_impl::Player>(table);
//             player->wind1 = Tile::East;
//             player->tsumo = true;
//             player->num = 14;
//             std::copy(my_player.hand.begin(), my_player.hand.begin() + 34,
//                       player->hand.begin());
//             if (my_player.hand[mahjong::Tile::RedManzu5]) {
//                 player->closed_reds[mahjong::Tile::Manzu5] = 1;
//             }
//             if (my_player.hand[mahjong::Tile::RedPinzu5]) {
//                 player->closed_reds[mahjong::Tile::Pinzu5] = 1;
//             }
//             if (my_player.hand[mahjong::Tile::RedSouzu5]) {
//                 player->closed_reds[mahjong::Tile::Souzu5] = 1;
//             }

//             auto [type, shanten] = ShantenCalculator::calc(
//                 my_player.hand, int(my_player.melds.size()), ShantenFlag::All);
//             if (shanten > 2) {
//                 continue;
//             }

//             spdlog::info("{} 手牌: {}, shanten: {}", i, to_mpsz(my_player.hand),
//                          shanten);
//             const auto [stats1, searched1] = win_prob(round, *player, params, my_player);
//             const auto [stats2, searched2] = my_prob.calc(params, round, my_player);
//             const auto [stats3, searched3] = win_prob3(player->hand, params);

//             // ドラや赤牌対応していないため、注意 ドラなし、赤牌なしの場合のみテスト通る
//             if (my_player.hand[Tile::RedManzu5] || my_player.hand[Tile::RedPinzu5] ||
//                 my_player.hand[Tile::RedSouzu5] || round.dora_indicators.size() > 0 ||
//                 params.enable_reddora) {
//                 continue;
//             }

//             REQUIRE(stats1.size() == stats2.size());
//             for (int i = 0; i < stats1.size(); ++i) {
//                 for (int j = 0; j <= params.t_max; ++j) {
//                     REQUIRE(stats1[i].prob[j] == Approx(stats2[i].exp_value[j]));
//                 }
//             }

//             REQUIRE(stats2.size() == stats3.size());
//             for (int i = 0; i < stats3.size(); ++i) {
//                 for (int j = 0; j <= params.t_max; ++j) {
//                     REQUIRE(stats2[i].win_prob[j] == Approx(stats3[i].prob[j]));
//                 }
//             }
//         }
//     };
// }

TEST_CASE("Exp probability calculator")
{
    std::vector<TestCase> cases;
    if (!load_testcase(cases)) {
        return;
    }

    CalshtDW calsht;
    win_prob::win_prob2::WinProb2 win_prob(calsht);
    ExpectedScoreCalculator my_prob;
    // params.extra = 0;
    // params.enable_reddora = false;
    // set_hand("222567m345p33667s", *player); // 6s が 4124.43 点になるはず

    SECTION("Expected score calculator")
    {
        for (int i = 0; i < cases.size(); ++i) {
            // Round information
            //////////////////////////////////////////
            Round round;
            round.rules = RuleFlag::OpenTanyao | RuleFlag::RedDora;
            round.wind = Tile::East;
            round.honba = 0;
            round.kyotaku = 0;
            round.dora_indicators = {};
            round.uradora_indicators = {};

            // Player information
            //////////////////////////////////////////
            Player my_player = cases[i];
            my_player.wind = Tile::East;

            // Calculation Settings
            //////////////////////////////////////////
            ExpectedScoreCalculator::Config params;
            params.t_min = 1;
            params.t_max = 18;
            params.sum = 121; // 136 - 14 - 1
            params.extra = 1;
            params.shanten_type = 7;
            params.enable_reddora = true;
            params.enable_shanten_down = true;
            params.enable_tegawari = true;
            params.enable_uradora = false;
            params.enable_riichi = false;

            // 同じのを設定
            auto table = std::make_shared<player_impl::Table>();
            auto player = std::make_shared<player_impl::Player>(table);
            table->wind2 = round.wind;
            player->wind1 = my_player.wind;
            player->tsumo = true;
            player->num = 14;

            player->wind1 = Tile::East;
            player->tsumo = true;
            player->num = 14;
            std::copy(my_player.hand.begin(), my_player.hand.begin() + 34,
                      player->hand.begin());
            if (my_player.hand[mahjong::Tile::RedManzu5]) {
                player->closed_reds[mahjong::Tile::Manzu5] = 1;
            }
            if (my_player.hand[mahjong::Tile::RedPinzu5]) {
                player->closed_reds[mahjong::Tile::Pinzu5] = 1;
            }
            if (my_player.hand[mahjong::Tile::RedSouzu5]) {
                player->closed_reds[mahjong::Tile::Souzu5] = 1;
            }

            Count wall = ExpectedScoreCalculator::create_wall(round, my_player,
                                                              params.enable_reddora);

            int not_used = 0;
            for (int j = 0; j < 34; ++j) {
                if (wall[j]) {
                    not_used = j;
                    break;
                }
            }

            // ドラ設定
            set_dora(Tile::Name.at(not_used), *table);
            round.dora_indicators = {not_used};

            auto [type, shanten] = ShantenCalculator::calc(
                my_player.hand, int(my_player.melds.size()), ShantenFlag::All);
            if (shanten > 2) {
                continue;
            }

            spdlog::info("{} 手牌: {}, shanten: {}", i, to_mpsz(my_player.hand),
                         shanten);

            const auto [stats1, searched1] = win_prob(round, *player, params, my_player);
            const auto [stats2, searched2] = my_prob.calc(params, round, my_player);

            REQUIRE(stats1.size() == stats2.size());
            for (int i = 0; i < stats1.size(); ++i) {
                for (int j = 0; j <= params.t_max; ++j) {
                    REQUIRE(stats1[i].prob[j] == Approx(stats2[i].exp_value[j]));
                }
            }
        }
    };
}
