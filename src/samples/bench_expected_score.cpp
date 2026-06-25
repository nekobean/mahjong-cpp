#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "mahjong/mahjong.hpp"

using namespace mahjong;

// Representative 4-player (Yonma) hands for benchmarking the hot path.
static const std::vector<std::string> kHands = {
    "222567m345p33667s",
    "13m12457899p1367s",
    "3456788m23457p11s",
    "1112345678999p1s",
};

int main(int argc, char *argv[])
{
    const int iterations = argc > 1 ? std::atoi(argv[1]) : 50;
    const int extra = argc > 2 ? std::atoi(argv[2]) : 0;

    Round round;
    round.rules = RuleFlag::OpenTanyao | RuleFlag::RedDora;
    round.wind = Tile::East;
    round.dora_indicators = {Tile::East};

    ExpectedScoreCalculator::Config config;
    config.extra = extra;

    // Warm up (table load, caches).
    for (const auto &mpsz : kHands) {
        Player p;
        p.hand = from_mpsz(mpsz);
        p.wind = Tile::East;
        const MergedCount wall = create_wall(round, p, config.enable_reddora);
        ExpectedScoreCalculator::calc(config, round, p, wall);
    }

    std::cout << "iterations=" << iterations << " extra=" << extra << "\n";
    long long total_us = 0;
    int total_searched = 0;
    for (const auto &mpsz : kHands) {
        Player p;
        p.hand = from_mpsz(mpsz);
        p.wind = Tile::East;
        const MergedCount wall = create_wall(round, p, config.enable_reddora);

        std::vector<long long> samples;
        int searched = 0;
        for (int i = 0; i < iterations; ++i) {
            const auto start = std::chrono::steady_clock::now();
            const auto [stats, s] =
                ExpectedScoreCalculator::calc(config, round, p, wall);
            const auto end = std::chrono::steady_clock::now();
            searched = s;
            samples.push_back(
                std::chrono::duration_cast<std::chrono::microseconds>(end - start)
                    .count());
        }
        std::sort(samples.begin(), samples.end());
        const long long best = samples.front();
        const long long median = samples[samples.size() / 2];
        total_us += median;
        total_searched += searched;
        std::cout << mpsz << "  best=" << best / 1000.0
                  << "ms  median=" << median / 1000.0 << "ms  searched=" << searched
                  << "\n";
    }
    std::cout << "TOTAL median(sum)=" << total_us / 1000.0
              << "ms  searched(sum)=" << total_searched << "\n";
    return 0;
}
