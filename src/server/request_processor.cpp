#include "request_processor.hpp"

#include <chrono>
#include <numeric>
#include <stdexcept>

using namespace mahjong;

CalculationResult calculate_result(const Request &req)
{
    CalculationResult result;

    result.config.enable_reddora = req.config.enable_reddora;
    result.config.enable_uradora = req.config.enable_uradora;
    result.config.enable_shanten_down = req.config.enable_shanten_down;
    result.config.enable_tegawari = req.config.enable_tegawari;
    result.config.t_min = 1;
    result.config.t_max = 18;
    result.config.sum = std::accumulate(req.wall.begin(), req.wall.begin() + 34, 0);
    result.config.extra = 1;
    result.config.shanten_type = ShantenFlag::All;
    result.shanten = std::get<1>(
        ShantenCalculator::calc(req.player.hand, req.player.num_melds(),
                                ShantenFlag::All, req.table_config.game_mode));
    result.regular_shanten = std::get<1>(
        ShantenCalculator::calc(req.player.hand, req.player.num_melds(),
                                ShantenFlag::StandardHand, req.table_config.game_mode));
    result.seven_pairs_shanten = std::get<1>(
        ShantenCalculator::calc(req.player.hand, req.player.num_melds(),
                                ShantenFlag::SevenPairs, req.table_config.game_mode));
    result.thirteen_orphans_shanten = std::get<1>(ShantenCalculator::calc(
        req.player.hand, req.player.num_melds(), ShantenFlag::ThirteenOrphans,
        req.table_config.game_mode));
    result.config.calc_stats = result.shanten <= 3;

    if (result.shanten == -1) {
        throw std::runtime_error(u8"手牌はすでに和了形です。");
    }

    const auto start = std::chrono::steady_clock::now();
    std::tie(result.stats, result.searched) =
        ExpectedScoreCalculator::calc(result.config, req.table_config, req.round_state,
                                      req.table_state, req.player, req.wall);
    const auto end = std::chrono::steady_clock::now();
    result.time_us =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    return result;
}
