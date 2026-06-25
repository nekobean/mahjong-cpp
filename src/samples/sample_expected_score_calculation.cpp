#include <chrono>
#include <iostream>
#include <numeric>

#include "mahjong/mahjong.hpp"

int main()
{
    using namespace mahjong;

    TableConfig table_config;
    table_config.rule_flags = RuleFlag::Default;
    table_config.game_mode = GameMode::Yonma;

    RoundState round;
    round.round_wind = Tile::East;
    round.round_number = 1;
    round.honba = 0;

    TableState table;
    table.kyotaku = 0;
    table.dora_indicators = {Tile::East};
    table.uradora_indicators = {};

    PlayerState player;
    player.hand = from_mpsz("222567m345p33667s");
    player.seat_wind = Tile::East;

    ExpectedScoreCalculator::Config calc_config;
    const MergedCount wall =
        create_wall(table_config, table, player, calc_config.enable_reddora);
    calc_config.sum = std::accumulate(wall.begin(), wall.begin() + 34, 0);

    // Calculate the shanten number.
    const int shanten = std::get<1>(
        ShantenCalculator::calc(player.hand, player.num_melds(),
                                calc_config.shanten_type, table_config.game_mode));

    // Calculate tenpai probability, win probability, and expected score.
    const auto start = std::chrono::steady_clock::now();
    const auto [stats, searched] = ExpectedScoreCalculator::calc(
        calc_config, table_config, round, table, player, wall);
    const auto end = std::chrono::steady_clock::now();
    const int elapsed_ms = static_cast<int>(
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());

    std::cout << std::boolalpha;
    std::cout << "=== Config ===" << std::endl;
    std::cout << fmt::format("{:>15}{}", "min turn: ", calc_config.t_min) << std::endl;
    std::cout << fmt::format("{:>15}{}", "max turn: ", calc_config.t_max) << std::endl;
    std::cout << fmt::format("{:>15}{}", "wall tiles: ", calc_config.sum) << std::endl;
    std::cout << fmt::format("{:>15}{}", "extra: ", calc_config.extra) << std::endl;
    std::cout << fmt::format("{:>15}{}", "shanten type: ", calc_config.shanten_type)
              << std::endl;
    std::cout << fmt::format("{:>15}{}", "red dora: ", calc_config.enable_reddora)
              << std::endl;
    std::cout << fmt::format("{:>15}{}", "uradora: ", calc_config.enable_uradora)
              << std::endl;
    std::cout << fmt::format("{:>15}{}",
                             "shanten down: ", calc_config.enable_shanten_down)
              << std::endl;
    std::cout << fmt::format("{:>15}{}", "tegawari: ", calc_config.enable_tegawari)
              << std::endl;

    std::cout << "=== Round ===" << std::endl;
    std::cout << to_string(table_config, round, table) << std::endl;

    std::cout << "=== Player ===" << std::endl;
    std::cout << to_string(player) << std::endl;

    std::cout << "=== Necessary Tiles ===" << std::endl;
    for (const auto &stat : stats) {
        std::cout << fmt::format("{:>2} ", Tile::name(stat.tile));

        int sum =
            std::accumulate(stat.necessary_tiles.begin(), stat.necessary_tiles.end(), 0,
                            [](int s, const auto &x) { return s + std::get<1>(x); });
        std::cout << fmt::format("type: {:2<}, sum: {:3<}, shanten: {}->{} tiles: ",
                                 stat.necessary_tiles.size(), sum, shanten,
                                 stat.shanten);

        for (const auto [tile, count] : stat.necessary_tiles) {
            std::cout << fmt::format("{}({}) ", Tile::name(tile), count);
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;

    std::cout << "=== Tenpai Probability ===" << std::endl;
    std::cout << fmt::format("{:>4}", "turn");
    for (const auto &stat : stats) {
        std::cout << fmt::format("{:>8}", Tile::name(stat.tile));
    }
    std::cout << std::endl;

    std::cout << std::fixed;
    for (int t = calc_config.t_min; t <= calc_config.t_max; ++t) {
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
        std::cout << fmt::format("{:>8}", Tile::name(stat.tile));
    }
    std::cout << std::endl;

    std::cout << std::fixed;
    for (int t = calc_config.t_min; t <= calc_config.t_max; ++t) {
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
        std::cout << fmt::format("{:>9}", Tile::name(stat.tile));
    }
    std::cout << std::endl;

    std::cout << std::fixed;
    for (int t = calc_config.t_min; t <= calc_config.t_max; ++t) {
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
