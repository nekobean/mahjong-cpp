

#include <array>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include <spdlog/spdlog.h>

#undef NDEBUG
#include <cassert>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/dll.hpp>
#include <spdlog/spdlog.h>
#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include <catch2/catch.hpp>

#include "compare/settile.hpp"
#include "compare/win_prob1.hpp"
#include "compare/win_prob2.hpp"
#include "compare/win_prob3.hpp"
#include "mahjong/mahjong.hpp"

using namespace mahjong;

/**
 * Loads a test case from the specified file.
 *
 * @param filepath The path to the file containing the test case data.
 * @param cases list of test cases.
 */
bool load_testcase(std::vector<Hand> &cases)
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

        cases.emplace_back(tiles);
    }

    spdlog::info("{} testcases loaded.", cases.size());

    return true;
}

void print_stats(const ExpectedScoreCalculator::Params &params,
                 const std::vector<ExpectedScoreCalculator::Stat> &stats,
                 const Hand &hand, int shanten, int elapsed_ms, int searched)
{
    // clang-format off
    const int pad = 15;
    std::cout << std::boolalpha;
    std::cout << "=== Params ===" << std::endl;
    std::cout << std::setw(pad) << std::right << "min turn: " << std::left << params.t_min << std::endl;
    std::cout << std::setw(pad) << std::right << "max turn: " << std::left << params.t_max << std::endl;
    std::cout << std::setw(pad) << std::right << "wall tiles: " << std::left << params.sum << std::endl;
    std::cout << std::setw(pad) << std::right << "extra: " << std::left << params.extra << std::endl;
    std::cout << std::setw(pad) << std::right << "shanten type: " << std::left << params.mode << std::endl;
    std::cout << std::setw(pad) << std::right << "uradora: " << std::left << params.enable_uradora << std::endl;
    std::cout << std::setw(pad) << std::right << "reddora: " << std::left << params.enable_reddora << std::endl;
    std::cout << std::setw(pad) << std::right << "shanten down: " << std::left << params.enable_shanten_down << std::endl;
    std::cout << std::setw(pad) << std::right << "tegawari: " << std::left << params.enable_tegawari << std::endl;
    std::cout << std::setw(pad) << std::right << "Double Riichi: " << std::left << params.enable_double_riichi << std::endl;
    std::cout << std::setw(pad) << std::right << "Ippatsu: " << std::left << params.enable_ippatsu << std::endl;
    std::cout << std::setw(pad) << std::right << "Under the Sea: " << std::left << params.enable_under_the_sea << std::endl;
    std::cout << "=== Input ===" << std::endl;
    std::cout << std::setw(pad) << std::right << "hand: " << std::left << hand.to_string() << std::endl;
    std::cout << "=== Info ===" << std::endl;
    std::cout << std::setw(pad) << std::right << "shanten: " << std::left << shanten << std::endl;
    std::cout << std::setw(pad) << std::right << "time: " << std::left << elapsed_ms << " ms" << std::endl;
    std::cout << std::setw(pad) << std::right << "searched: " << std::left << searched << " hands" << std::endl;
    // clang-format on
    std::cout << "=== Tenpai Probability ===" << std::endl;
    std::cout << fmt::format("{:>4}", "turn");
    for (const auto &stat : stats) {
        std::cout << fmt::format("{:>8}", Tile::Name.at(stat.tile));
    }
    std::cout << std::endl;

    std::cout << std::fixed;
    for (int t = params.t_min; t <= params.t_max; ++t) {
        std::cout << fmt::format("{:>4}", t);
        for (const auto &stat : stats) {
            std::cout << fmt::format("{:>7.2f}%", stat.tenpai_prob[t] * 100);
        }
        std::cout << std::endl;
    }

    std::cout << "=== Win Probability ===" << std::endl;
    std::cout << fmt::format("{:>4}", "turn");
    for (const auto &stat : stats) {
        std::cout << fmt::format("{:>8}", Tile::Name.at(stat.tile));
    }
    std::cout << std::endl;

    std::cout << std::fixed;
    for (int t = params.t_min; t <= params.t_max; ++t) {
        std::cout << fmt::format("{:>4}", t);
        for (const auto &stat : stats) {
            std::cout << fmt::format("{:>7.2f}%", stat.win_prob[t] * 100);
        }
        std::cout << std::endl;
    }

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
    std::vector<Hand> cases;
    if (!load_testcase(cases)) {
        return;
    }

    CalshtDW calsht;
    win_prob::win_prob2::WinProb2 win_prob(calsht);
    ExpectedScoreCalculator my_prob;
    WinProb3 win_prob3;
    auto table = std::make_shared<player_impl::Table>();
    table->wind2 = Tile::East;

    // Player information
    //////////////////////////////////////////
    auto player = std::make_shared<player_impl::Player>(table);
    player->wind1 = Tile::West;
    player->tsumo = true;
    player->num = 14;

    // Round information
    //////////////////////////////////////////
    Round round;
    round.wind = Tile::East;
    round.self_wind = player->wind1;
    round.honba = 0;
    round.kyotaku = 0;
    round.dora_tiles = {};
    round.uradora_tiles = {};
    round.rules = RuleFlag::OpenTanyao | RuleFlag::RedDora;

    // Calculation Settings
    //////////////////////////////////////////
    ExpectedScoreCalculator::Params params;
    params.t_min = 1;
    params.t_max = 18;
    params.sum = 121; // 136 - 14 - 1
    params.extra = 1;
    params.mode = 7;
    params.enable_reddora = false;
    // 以下、未実装
    params.enable_uradora = true;
    params.enable_shanten_down = true;
    params.enable_tegawari = true;
    params.enable_double_riichi = true;
    params.enable_ippatsu = true;
    params.enable_under_the_sea = true;

    //set_dora("1z", *table);
    set_hand("222567m345p33667s", *player);
    //set_hand("222567m345p33667s", *player); // 6s が 4124.43 点になるはず

    auto [mode, shanten] = ShantenCalculator::calc(player->hand, 0, params.mode);

    const auto start = std::chrono::system_clock::now();
    const auto [stats, searched] = win_prob(round, *player, params);
    const auto end = std::chrono::system_clock::now();
    const int elapsed_ms = static_cast<int>(
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());

    const auto start3 = std::chrono::system_clock::now();
    const auto [stats3, searched3] = my_prob.calc(round, *player, params);
    const auto end3 = std::chrono::system_clock::now();
    const int elapsed_ms2 = static_cast<int>(
        std::chrono::duration_cast<std::chrono::milliseconds>(end3 - start3).count());

    print_stats(params, stats3, Hand::from_counts(player->hand), shanten, elapsed_ms2,
                searched);

    std::cout << "elapsed_ms: " << elapsed_ms << " us" << std::endl;
    std::cout << "elapsed_ms2: " << elapsed_ms2 << " us" << std::endl;

    REQUIRE(stats.size() == stats3.size());
    for (int i = 0; i < stats.size(); ++i) {
        REQUIRE(stats[i].tile == stats3[i].tile);
        for (int j = 0; j <= params.t_max; ++j) {
            REQUIRE(stats[i].prob[j] == Approx(stats3[i].exp_value[j]));
        }
    }

    SECTION("Expected score calculator")
    {
        int i = 0;
        for (auto &hand : cases) {
            ++i;
            std::cout << "Test case " << i << std::endl;

            auto player = std::make_shared<player_impl::Player>(table);
            std::copy(hand.counts.begin(), hand.counts.begin() + 34,
                      player->hand.begin());
            if (hand.counts[mahjong::Tile::RedManzu5]) {
                player->closed_reds[mahjong::Tile::Manzu5] = 1;
            }
            if (hand.counts[mahjong::Tile::RedPinzu5]) {
                player->closed_reds[mahjong::Tile::Pinzu5] = 1;
            }
            if (hand.counts[mahjong::Tile::RedSouzu5]) {
                player->closed_reds[mahjong::Tile::Souzu5] = 1;
            }
            player->wind1 = Tile::West;
            player->tsumo = true;
            player->num = 14;

            auto [type, shanten] = ShantenCalculator::calc(
                hand.counts, hand.melds.size(), ShantenFlag::All);
            if (shanten >= 3) {
                continue;
            }
            spdlog::info("手牌: {}, shanten: {}", hand.to_string(), shanten);

            const auto [stats1, searched1] = win_prob(round, *player, params);
            const auto [stats2, searched2] = my_prob.calc(round, *player, params);
            const auto [stats3, searched3] = win_prob3(player->hand, params);
            // print_stats(params, stats2, Hand::from_counts(player->hand), shanten, 0,
            //             searched2);

            if (hand.counts[Tile::RedManzu5] || hand.counts[Tile::RedPinzu5] ||
                hand.counts[Tile::RedSouzu5]) {
                continue;
            }

            INFO(fmt::format("手牌: {}", hand.to_string()));
            REQUIRE(stats1.size() == stats2.size());
            for (int i = 0; i < stats1.size(); ++i) {
                REQUIRE(stats1[i].tile == stats2[i].tile);
                for (int j = 0; j <= params.t_max; ++j) {
                    REQUIRE(stats1[i].prob[j] == Approx(stats2[i].exp_value[j]));
                }
            }

            INFO(fmt::format("手牌: {}", hand.to_string()));
            REQUIRE(stats2.size() == stats3.size());
            for (int i = 0; i < stats3.size(); ++i) {
                REQUIRE(stats2[i].tile == stats3[i].tile);
                for (int j = 0; j <= params.t_max; ++j) {
                    REQUIRE(stats2[i].win_prob[j] == Approx(stats3[i].prob[j]));
                }
            }
        }
    };
}
