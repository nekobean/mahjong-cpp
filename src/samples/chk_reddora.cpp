#include <algorithm>
#include <iostream>
#include <string>

#include "mahjong/mahjong.hpp"

using namespace mahjong;

static double best_exp(const std::string &mpsz, bool enable_reddora)
{
    Round round;
    round.rules = RuleFlag::OpenTanyao | RuleFlag::RedDora;
    round.wind = Tile::East;
    round.dora_indicators = {Tile::East};

    Player player;
    player.hand = from_mpsz(mpsz);
    player.wind = Tile::East;

    ExpectedScoreCalculator::Config config;
    config.enable_reddora = enable_reddora;
    const MergedCount wall = create_wall(round, player, config.enable_reddora);

    const auto [stats, searched] =
        ExpectedScoreCalculator::calc(config, round, player, wall);

    double best = 0.0;
    for (const auto &s : stats)
        for (double v : s.exp_score)
            best = std::max(best, v);
    return best;
}

int main()
{
    // Same 14-tile hand: one with red 5p (0p), one with normal 5p.
    const std::string red = "13m12407899p1367s";   // contains 0p (red 5p)
    const std::string nored = "13m12457899p1367s"; // normal 5p

    std::cout << "red5 hand, reddora=ON : best_exp=" << best_exp(red, true) << "\n";
    std::cout << "red5 hand, reddora=OFF: best_exp=" << best_exp(red, false) << "\n";
    std::cout << "no-red hand, reddora=OFF: best_exp=" << best_exp(nored, false)
              << "\n";
    std::cout << "no-red hand, reddora=ON : best_exp=" << best_exp(nored, true) << "\n";
    return 0;
}
