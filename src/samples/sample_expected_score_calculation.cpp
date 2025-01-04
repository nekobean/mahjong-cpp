#include <chrono>
#include <iostream>

#include <spdlog/spdlog.h>

#include "mahjong/mahjong.hpp"

int main(int argc, char *argv[])
{
    using namespace mahjong;

    // Player information
    //////////////////////////////////////////
    Player player;
    // Create hand by mpsz notation or vector of tiles.
    //player.hand = from_mpsz("13m12457899p1367s");
    player.hand = from_mpsz("222567m345p33667s");
    // Hand hand = from_array({Tile::Manzu2, Tile::Manzu2, Tile::Manzu2, Tile::Manzu5,
    //                         Tile::Manzu6, Tile::Manzu7, Tile::Pinzu3, Tile::Pinzu4,
    //                         Tile::Souzu3, Tile::Souzu3, Tile::Souzu6, Tile::Souzu6,
    //                         Tile::Souzu7, Tile::Souzu7});

    // player.hand = from_mpsz("1238p1345579s");
    // player.melds = {
    //     {MeldType::OpenKong, {Tile::Manzu2, Tile::Manzu2, Tile::Manzu2, Tile::Manzu2}}};
    player.wind = Tile::East;
    if (player.num_tiles() + player.num_melds() * 3 != 14) {
        spdlog::error("Number of tiles should be 14.");
        return 1;
    }

    // Round information
    //////////////////////////////////////////
    Round round;
    round.rules = RuleFlag::OpenTanyao | RuleFlag::RedDora;
    round.wind = Tile::East;
    round.kyoku = 1;
    round.honba = 0;
    round.kyotaku = 0;
    round.dora_indicators = {Tile::East};
    round.uradora_indicators = {};

    // Calculation Settings
    //////////////////////////////////////////
    ExpectedScoreCalculator::Config config;
    config.t_min = 1;
    config.t_max = 18;
    config.extra = 1;
    config.shanten_type = ShantenFlag::All;
    config.enable_reddora = true;
    config.enable_uradora = true;
    config.enable_shanten_down = true;
    config.enable_tegawari = true;
    config.enable_riichi = false;

    // Calculation
    //////////////////////////////////////////
    const auto [type, shanten] =
        ShantenCalculator::calc(player.hand, player.num_melds(), config.shanten_type);

    // Calculate tenpai probability, win probability, and expected score.
    const auto start = std::chrono::steady_clock::now();
    const auto [stats, searched] = ExpectedScoreCalculator::calc(config, round, player);
    const auto end = std::chrono::steady_clock::now();
    const int elapsed_ms = static_cast<int>(
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());

    // Output
    //////////////////////////////////////////
    std::cout << std::boolalpha;
    std::cout << "=== Config ===" << std::endl;
    std::cout << fmt::format("{:>15}{}", "min turn: ", config.t_min) << std::endl;
    std::cout << fmt::format("{:>15}{}", "max turn: ", config.t_max) << std::endl;
    std::cout << fmt::format("{:>15}{}", "wall tiles: ", config.sum) << std::endl;
    std::cout << fmt::format("{:>15}{}", "extra: ", config.extra) << std::endl;
    std::cout << fmt::format("{:>15}{}", "shanten type: ", config.shanten_type)
              << std::endl;
    std::cout << fmt::format("{:>15}{}", "reddora: ", config.enable_reddora)
              << std::endl;
    std::cout << fmt::format("{:>15}{}", "uradora: ", config.enable_uradora)
              << std::endl;
    std::cout << fmt::format("{:>15}{}", "shanten down: ", config.enable_shanten_down)
              << std::endl;
    std::cout << fmt::format("{:>15}{}", "tegawari: ", config.enable_tegawari)
              << std::endl;
    std::cout << fmt::format("{:>15}{}", "riichi: ", config.enable_riichi) << std::endl;

    std::cout << "=== Round ===" << std::endl;
    std::cout << to_string(round) << std::endl;

    std::cout << "=== Player ===" << std::endl;
    std::cout << to_string(player) << std::endl;

    std::cout << "=== Necessary Tiles ===" << std::endl;
    for (const auto &stat : stats) {
        std::cout << fmt::format("{:>2} ", Tile::Name.at(stat.tile));

        int sum =
            std::accumulate(stat.necessary_tiles.begin(), stat.necessary_tiles.end(), 0,
                            [](int s, const auto &x) { return s + std::get<1>(x); });
        std::cout << fmt::format("type: {:2<}, sum: {:3<}, shanten: {}->{} tiles: ",
                                 stat.necessary_tiles.size(), sum, shanten,
                                 stat.shanten);

        for (const auto [tile, count] : stat.necessary_tiles) {
            std::cout << fmt::format("{}({}) ", Tile::Name.at(tile), count);
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;

    std::cout << "=== Tenpai Probability ===" << std::endl;
    std::cout << fmt::format("{:>4}", "turn");
    for (const auto &stat : stats) {
        std::cout << fmt::format("{:>8}", Tile::Name.at(stat.tile));
    }
    std::cout << std::endl;

    std::cout << std::fixed;
    for (int t = config.t_min; t <= config.t_max; ++t) {
        std::cout << fmt::format("{:>4}", t);
        for (const auto &stat : stats) {
            std::cout << fmt::format("{:>7.2f}%", stat.tenpai_prob[t] * 100);
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;

    std::cout << "=== Win Probability ===" << std::endl;
    std::cout << fmt::format("{:>4}", "turn");
    for (const auto &stat : stats) {
        std::cout << fmt::format("{:>8}", Tile::Name.at(stat.tile));
    }
    std::cout << std::endl;

    std::cout << std::fixed;
    for (int t = config.t_min; t <= config.t_max; ++t) {
        std::cout << fmt::format("{:>4}", t);
        for (const auto &stat : stats) {
            std::cout << fmt::format("{:>7.2f}%", stat.win_prob[t] * 100);
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;

    std::cout << "=== Expected Score ===" << std::endl;
    std::cout << fmt::format("{:>4}", "turn");
    for (const auto &stat : stats) {
        std::cout << fmt::format("{:>9}", Tile::Name.at(stat.tile));
    }
    std::cout << std::endl;

    std::cout << std::fixed;
    for (int t = config.t_min; t <= config.t_max; ++t) {
        std::cout << fmt::format("{:>4}", t);
        for (const auto &stat : stats) {
            std::cout << fmt::format("{:>9.2f}", stat.exp_score[t]);
        }
        std::cout << std::endl;
    }

    std::cout << "=== Info ===" << std::endl;
    std::cout << fmt::format("{:>15}{}", "shanten: ", shanten) << std::endl;
    std::cout << fmt::format("{:>15}{} ms", "time: ", elapsed_ms) << std::endl;
    std::cout << fmt::format("{:>15}{} hands", "searched: ", searched) << std::endl;
}
