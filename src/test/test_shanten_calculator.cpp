#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include <cassert>
#include <filesystem>
#include <fstream>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <catch2/catch.hpp>
#include <spdlog/spdlog.h>

#include "mahjong/mahjong.hpp"

using namespace mahjong;

using TestCase = std::tuple<Hand, int, int, int>;

namespace
{

std::filesystem::path reference_case_path()
{
    return std::filesystem::path(CMAKE_TESTCASE_DIR) / "test_shanten_calculator.txt";
}

bool load_testcase(const std::filesystem::path &filepath, std::vector<TestCase> &cases)
{
    cases.clear();

    std::ifstream ifs(filepath);
    if (!ifs) {
        spdlog::error("Failed to open {}.", filepath.string());
        return false;
    }

    // The format is `<tile1> <tile2> ... <tile14> <shanten number of standard hand>
    //                <shanten number of Thirteen Orphans> <shanten number of Seven Pairs>`
    std::string line;
    while (std::getline(ifs, line)) {
        if (line.empty()) {
            continue;
        }

        std::vector<std::string> tokens;
        boost::split(tokens, line, boost::is_any_of(" "));

        Hand hand{0};
        for (int i = 0; i < 14; ++i) {
            int tile = std::stoi(tokens[i]);
            ++hand[Tile::to_normal(tile)];
        }
        assert(std::accumulate(hand.begin(), hand.begin() + 34, 0) == 14);
        cases.emplace_back(hand, std::stoi(tokens[14]), std::stoi(tokens[15]),
                           std::stoi(tokens[16]));
    }

    spdlog::info("{} testcases loaded.", cases.size());

    return true;
}

const std::vector<TestCase> &reference_cases()
{
    static const std::vector<TestCase> cases = []() {
        std::vector<TestCase> loaded_cases;
        const bool ok = load_testcase(reference_case_path(), loaded_cases);
        REQUIRE(ok);
        return loaded_cases;
    }();

    return cases;
}

} // namespace

TEST_CASE("Shanten number of standard hand")
{
    const auto &cases = reference_cases();

    SECTION("Shanten number of standard hand")
    {
        for (const auto &[hand, regular, thirteen_orphans, seven_pairs] : cases) {
            INFO(fmt::format("手牌: {}", to_mpsz(hand)));
            REQUIRE(
                std::get<1>(ShantenCalculator::calc(hand, 0, ShantenFlag::StandardHand,
                                                    GameMode::Yonma)) == regular);
        }
    }

    BENCHMARK("Shanten number of standard hand")
    {
        for (const auto &[hand, regular, thirteen_orphans, seven_pairs] : cases) {
            ShantenCalculator::calc(hand, 0, ShantenFlag::StandardHand,
                                    GameMode::Yonma);
        }
    };
}

TEST_CASE("Shanten number of Seven Pairs")
{
    const auto &cases = reference_cases();

    SECTION("Shanten number of Seven Pairs")
    {
        for (const auto &[hand, regular, thirteen_orphans, seven_pairs] : cases) {
            INFO(fmt::format("手牌: {}", to_mpsz(hand)));
            REQUIRE(
                std::get<1>(ShantenCalculator::calc(hand, 0, ShantenFlag::SevenPairs,
                                                    GameMode::Yonma)) == seven_pairs);
        }
    }

    BENCHMARK("Shanten number of Seven Pairs")
    {
        for (const auto &[hand, regular, thirteen_orphans, seven_pairs] : cases) {
            ShantenCalculator::calc(hand, 0, ShantenFlag::SevenPairs, GameMode::Yonma);
        }
    };
}

TEST_CASE("Shanten number of Thirteen Orphans")
{
    const auto &cases = reference_cases();

    SECTION("Shanten number of Thirteen Orphans")
    {
        for (const auto &[hand, regular, thirteen_orphans, seven_pairs] : cases) {
            INFO(fmt::format("手牌: {}", to_mpsz(hand)));
            REQUIRE(std::get<1>(ShantenCalculator::calc(
                        hand, 0, ShantenFlag::ThirteenOrphans, GameMode::Yonma)) ==
                    thirteen_orphans);
        }
    }

    BENCHMARK("Shanten number of Thirteen Orphans")
    {
        for (const auto &[hand, regular, thirteen_orphans, seven_pairs] : cases) {
            ShantenCalculator::calc(hand, 0, ShantenFlag::ThirteenOrphans,
                                    GameMode::Yonma);
        }
    };
}

TEST_CASE("Shanten number")
{
    const auto &cases = reference_cases();

    SECTION("Shanten number")
    {
        for (const auto &[hand, regular, thirteen_orphans, seven_pairs] : cases) {
            const int true_shanten = std::min({regular, thirteen_orphans, seven_pairs});
            const int true_type =
                (true_shanten == regular ? ShantenFlag::StandardHand : 0) |
                (true_shanten == thirteen_orphans ? ShantenFlag::ThirteenOrphans : 0) |
                (true_shanten == seven_pairs ? ShantenFlag::SevenPairs : 0);
            const auto [type, shanten] =
                ShantenCalculator::calc(hand, 0, ShantenFlag::All, GameMode::Yonma);

            INFO(fmt::format("手牌: {}", to_mpsz(hand)));
            REQUIRE(shanten == true_shanten);
            REQUIRE(type == true_type);
        }
    }

    BENCHMARK("Shanten number")
    {
        for (const auto &[hand, regular, thirteen_orphans, seven_pairs] : cases) {
            ShantenCalculator::calc(hand, 0, ShantenFlag::All, GameMode::Yonma);
        }
    };
}
