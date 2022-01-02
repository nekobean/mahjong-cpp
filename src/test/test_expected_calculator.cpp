#include "mahjong/mahjong.hpp"

#include <boost/dll.hpp>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include <catch2/catch.hpp>

using namespace mahjong;
namespace fs = boost::filesystem;

struct TestCase
{
    std::vector<int> before_counts;
    std::vector<int> after_counts;
    Hand before_hand;
    Hand after_hand;
    int tile;
};

bool load_testcase(const std::string &path, std::vector<TestCase> &testcases)
{
    std::ifstream ifs(path);
    if (!ifs.good()) {
        spdlog::error("Failed to open request data. {}", path);
        return false;
    }

    rapidjson::IStreamWrapper isw(ifs);
    rapidjson::Document doc;
    doc.ParseStream(isw);
    if (doc.HasParseError()) {
        spdlog::error("Failed to parse request data (invalid json format). {}", path);
        return false;
    }

    for (const auto &x : doc.GetArray()) {
        TestCase testcase;

        for (const auto &tile : x["counts_before"].GetArray())
            testcase.before_counts.push_back(tile.GetInt());

        for (const auto &tile : x["counts_after"].GetArray())
            testcase.after_counts.push_back(tile.GetInt());

        {
            std::vector<int> hand_tiles;
            for (const auto &tile : x["hand_before"].GetArray())
                hand_tiles.push_back(tile.GetInt());
            testcase.before_hand = Hand(hand_tiles);
        }

        {
            std::vector<int> hand_tiles;
            for (const auto &tile : x["hand_after"].GetArray())
                hand_tiles.push_back(tile.GetInt());
            testcase.after_hand = Hand(hand_tiles);
        }

        testcase.tile = x["tile"].GetInt();

        testcases.push_back(testcase);
    }

    return true;
}

TEST_CASE("add_tile")
{
    fs::path testcases_path = fs::path(CMAKE_TESTCASE_DIR) / "testcase_add_tile.json";
    std::vector<TestCase> testcases;
    load_testcase(testcases_path.string(), testcases);

    SECTION("void add_tile(Hand &hand, int tile, std::vector<int> &counts)")
    {
        for (const auto &testcase : testcases) {
            Hand hand = testcase.before_hand;
            std::vector<int> counts = testcase.before_counts;
            add_tile(hand, testcase.tile, counts);

            REQUIRE(hand == testcase.after_hand);
            REQUIRE(counts.size() == testcase.after_counts.size());
            REQUIRE(std::equal(counts.begin(), counts.end(), testcase.after_counts.begin()));
        }
    }

    SECTION("void add_tile(Hand &hand, int tile)")
    {
        for (const auto &testcase : testcases) {
            Hand hand = testcase.before_hand;
            add_tile(hand, testcase.tile);

            REQUIRE(hand == testcase.after_hand);
        }
    }
}

TEST_CASE("remove_tile")
{
    fs::path testcases_path = fs::path(CMAKE_TESTCASE_DIR) / "testcase_remove_tile.json";
    std::vector<TestCase> testcases;
    load_testcase(testcases_path.string(), testcases);

    SECTION("void remove_tile(Hand &hand, int tile, std::vector<int> &counts)")
    {
        for (const auto &testcase : testcases) {
            Hand hand = testcase.before_hand;
            std::vector<int> counts = testcase.before_counts;
            remove_tile(hand, testcase.tile, counts);

            REQUIRE(hand == testcase.after_hand);
            REQUIRE(counts.size() == testcase.after_counts.size());
            REQUIRE(std::equal(counts.begin(), counts.end(), testcase.after_counts.begin()));
        }
    }

    SECTION("void remove_tile(Hand &hand, int tile)")
    {
        for (const auto &testcase : testcases) {
            Hand hand = testcase.before_hand;
            remove_tile(hand, testcase.tile);

            REQUIRE(hand == testcase.after_hand);
        }
    }
}

TEST_CASE("count_left_tiles")
{
    fs::path testcases_path = fs::path(CMAKE_TESTCASE_DIR) / "testcase_remove_tile.json";
    std::vector<TestCase> testcases;
    load_testcase(testcases_path.string(), testcases);

    for (const auto &testcase : testcases) {
        std::vector<int> dora_indicators;
        std::vector<int> counts =
            ExpectedValueCalculator::count_left_tiles(testcase.before_hand, dora_indicators);

        REQUIRE(counts.size() == testcase.before_counts.size());
        REQUIRE(std::equal(counts.begin(), counts.end(), testcase.before_counts.begin()));
    }
}

TEST_CASE("create_prob_table")
{
    SECTION("tumo_prob_table")
    {
        ExpectedValueCalculator caclulator;
        caclulator.max_tumo_ = 17;

        int N = 121; // 残り牌の合計枚数
        caclulator.create_prob_table(N);
        for (int r = 0; r <= 4; ++r) {
            for (int i = 0; i < 17; ++i) {
                double expected = double(r) / double(N - i);
                REQUIRE(caclulator.tumo_prob_table_[r][i] == Approx(expected));
            }
        }
    }

    SECTION("not_tumo_prob_table")
    {
        ExpectedValueCalculator caclulator;
        caclulator.max_tumo_ = 17;

        for (int N = 100; N < 121; ++N) { // N は残り牌の合計枚数
            caclulator.create_prob_table(N);

            // for (int R = 0; R < N; ++R) {
            //     for (int i = 0; i < 17; ++i)
            //         std::cout << fmt::format("{:4.2f} ", caclulator.not_tumo_prob_table_[R][i]);
            //     std::cout << std::endl;
            // }

            for (int R = 0; R < N; ++R) {
                for (int i = 0; i < 17; ++i) { // t は現在の巡目
                    // i 巡目までに有効牌以外を連続して引く確率
                    double not_tumo_prob = 1;
                    for (int j = 0; j < i && N - R - j > 0; ++j) {
                        REQUIRE(N - R - j >= 0);
                        REQUIRE(N - j > 0);
                        not_tumo_prob *= double(N - R - j) / double(N - j);
                    }

                    if (caclulator.not_tumo_prob_table_[R][i] > 10e-12 && not_tumo_prob > 10e-12) {
                        INFO(fmt::format("R={}, i={}, {} {}", R, i,
                                         caclulator.not_tumo_prob_table_[R][i], not_tumo_prob));
                        REQUIRE(caclulator.not_tumo_prob_table_[R][i] == Approx(not_tumo_prob));
                    }
                }
            }

            for (int R = 1; R < N; ++R) {
                for (int t = 0; t < 17; ++t) { // t は現在の巡目
                    for (int i = t; i < 17 && N - R - i > 0; ++i) {
                        // i 巡目までに有効牌以外を連続して引く確率
                        double expected = 1;
                        for (int j = t; j < i; ++j)
                            expected *= double(N - R - j) / double(N - j);

                        double actual = caclulator.not_tumo_prob_table_[R][i] /
                                        caclulator.not_tumo_prob_table_[R][t];

                        REQUIRE(caclulator.not_tumo_prob_table_[R][t] > 0);
                        if (expected > 10e-12 && actual > 10e-12) {
                            INFO(fmt::format("R={}, i={}, t={}, {}", R, i, t,
                                             caclulator.not_tumo_prob_table_[R][t]));
                            REQUIRE(expected == Approx(actual));
                        }
                    }
                }
            }
        }
    }
}
