#include <algorithm>
#include <tuple>
#include <vector>

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include "mahjong/mahjong.hpp"

using namespace mahjong;

TEST_CASE("Score detail argument validation")
{
    SECTION("valid hand")
    {
        const std::vector<int> tiles = {
            Tile::Manzu1, Tile::Manzu1, Tile::Manzu1, Tile::Manzu2, Tile::Manzu3,
            Tile::Manzu4, Tile::Pinzu2, Tile::Pinzu3, Tile::Pinzu4, Tile::Souzu2,
            Tile::Souzu3, Tile::Souzu4, Tile::East,   Tile::East};
        const PlayerState player{to_hand(tiles), {}, Tile::East};
        const auto [success, message] =
            score_calculator_detail::check_arguments(player, Tile::East, WinFlag::None);
        REQUIRE(success);
        REQUIRE(message.empty());
    }

    SECTION("invalid win tile")
    {
        const std::vector<int> tiles = {
            Tile::Manzu1, Tile::Manzu1, Tile::Manzu1, Tile::Manzu2, Tile::Manzu3,
            Tile::Manzu4, Tile::Pinzu2, Tile::Pinzu3, Tile::Pinzu4, Tile::Souzu2,
            Tile::Souzu3, Tile::Souzu4, Tile::East,   Tile::East};
        const PlayerState player{to_hand(tiles), {}, Tile::East};
        const auto [success, message] = score_calculator_detail::check_arguments(
            player, Tile::RedDragon, WinFlag::None);
        REQUIRE_FALSE(success);
        REQUIRE_FALSE(message.empty());
    }

    SECTION("exclusive flags")
    {
        const std::vector<int> tiles = {
            Tile::Manzu1, Tile::Manzu1, Tile::Manzu1, Tile::Manzu2, Tile::Manzu3,
            Tile::Manzu4, Tile::Pinzu2, Tile::Pinzu3, Tile::Pinzu4, Tile::Souzu2,
            Tile::Souzu3, Tile::Souzu4, Tile::East,   Tile::East};
        const PlayerState player{to_hand(tiles), {}, Tile::East};
        const auto [success, message] = score_calculator_detail::check_arguments(
            player, Tile::East, WinFlag::Riichi | WinFlag::DoubleRiichi);
        REQUIRE_FALSE(success);
        REQUIRE_FALSE(message.empty());
    }
}

TEST_CASE("Score detail score title")
{
    REQUIRE(score_calculator_detail::get_score_title(30, 3) == ScoreLimit::Null);
    REQUIRE(score_calculator_detail::get_score_title(40, 4) == ScoreLimit::Mangan);
    REQUIRE(score_calculator_detail::get_score_title(30, 6) == ScoreLimit::Haneman);
    REQUIRE(score_calculator_detail::get_score_title(30, 8) == ScoreLimit::Baiman);
    REQUIRE(score_calculator_detail::get_score_title(30, 11) == ScoreLimit::Sanbaiman);
    REQUIRE(score_calculator_detail::get_score_title(30, 13) ==
            ScoreLimit::CountedYakuman);
}

TEST_CASE("Score detail payment table")
{
    using score_calculator_detail::calc_score;

    SECTION("ron is identical between yonma and sanma")
    {
        REQUIRE(calc_score(false, false, 0, 0, GameMode::Yonma,
                           ScoreLimit::Mangan) == std::vector<int>({8000, 8000}));
        REQUIRE(calc_score(false, false, 0, 0, GameMode::Sanma,
                           ScoreLimit::Mangan) == std::vector<int>({8000, 8000}));
    }

    SECTION("yonma tsumo")
    {
        REQUIRE(calc_score(true, true, 0, 0, GameMode::Yonma, ScoreLimit::Mangan) ==
                std::vector<int>({12000, 4000}));
        REQUIRE(calc_score(false, true, 0, 0, GameMode::Yonma, ScoreLimit::Mangan) ==
                std::vector<int>({8000, 4000, 2000}));
    }

    SECTION("sanma tsumo loss")
    {
        REQUIRE(calc_score(true, true, 0, 0, GameMode::Sanma, ScoreLimit::Mangan) ==
                std::vector<int>({8000, 4000}));
        REQUIRE(calc_score(false, true, 0, 0, GameMode::Sanma, ScoreLimit::Mangan) ==
                std::vector<int>({6000, 4000, 2000}));
    }

    SECTION("honba and kyotaku")
    {
        REQUIRE(calc_score(false, false, 2, 1, GameMode::Yonma, ScoreLimit::Null, 3,
                           40) == std::vector<int>({6800, 5800}));
        REQUIRE(calc_score(true, true, 1, 0, GameMode::Yonma, ScoreLimit::Null, 3,
                           40) == std::vector<int>({8100, 2700}));
        REQUIRE(calc_score(false, false, 2, 2, GameMode::Sanma, ScoreLimit::Null, 2,
                           30) == std::vector<int>({4400, 2400}));
    }
}

TEST_CASE("Score detail dora counting")
{
    using score_calculator_detail::count_dora;

    SECTION("hand and meld doras")
    {
        const Hand hand =
            from_array({Tile::Manzu2, Tile::Manzu2, Tile::Pinzu5, Tile::Pinzu5});
        const std::vector<Meld> melds = {
            {MeldType::Pon,
             {Tile::Souzu3, Tile::Souzu3, Tile::Souzu3},
             Tile::Souzu3,
             PlayerIndex::Player1},
        };
        const std::vector<int> indicators = {Tile::Manzu1, Tile::Souzu2};

        REQUIRE(count_dora(hand, melds, indicators, GameMode::Yonma) == 5);
    }

    SECTION("sanma manzu dora loop")
    {
        const Hand hand =
            from_array({Tile::Manzu1, Tile::Manzu9, Tile::Pinzu6});

        REQUIRE(count_dora(hand, {}, {Tile::Manzu1}, GameMode::Sanma) == 1);
        REQUIRE(count_dora(hand, {}, {Tile::Manzu9}, GameMode::Sanma) == 1);
        REQUIRE(count_dora(hand, {}, {Tile::Pinzu5}, GameMode::Sanma) == 1);
        REQUIRE(count_dora(hand, {}, {Tile::Manzu1}, GameMode::Yonma) == 0);
    }

    SECTION("nuki dora")
    {
        const Hand hand = from_array({Tile::North});

        REQUIRE(count_dora(hand, {}, {Tile::Manzu1}, GameMode::Sanma, 2) == 0);
        REQUIRE(count_dora(hand, {}, {Tile::West}, GameMode::Sanma, 2) == 3);
        REQUIRE(count_dora(hand, {}, {Tile::West}, GameMode::Sanma) == 1);
        REQUIRE(count_dora(hand, {}, {Tile::West}, GameMode::Yonma, 2) == 3);
    }
}

TEST_CASE("Score detail nuki dora score calculation")
{
    TableConfig table_config;
    table_config.game_mode = GameMode::Sanma;
    RoundState round_state;
    round_state.round_wind = Tile::East;
    TableState table_state;

    const std::vector<int> tiles = {
        Tile::Pinzu2, Tile::Pinzu3, Tile::Pinzu4, Tile::Pinzu5, Tile::Pinzu6,
        Tile::Pinzu7, Tile::Souzu2, Tile::Souzu3, Tile::Souzu4, Tile::Souzu5,
        Tile::Souzu5, Tile::Souzu5, Tile::Souzu8, Tile::Souzu8};
    PlayerState player{to_hand(tiles), {}, Tile::South};
    player.nuki_count = 2;

    const ScoreResult result = ScoreCalculator::calc(
        table_config, round_state, table_state, player, Tile::Souzu4, WinFlag::Tsumo);

    REQUIRE(result.success);

    const auto itr =
        std::find_if(result.yaku_list.begin(), result.yaku_list.end(),
                     [](const auto &x) { return x.yaku == Yaku::NukiDora; });
    REQUIRE(itr != result.yaku_list.end());
    REQUIRE(itr->han == 2);
    REQUIRE(result.han == 4);
}

TEST_CASE("Score detail yaku value table")
{
    TableConfig table_config;
    RoundState round_state;
    round_state.round_wind = Tile::East;
    TableState table_state;

    const std::vector<int> tiles = {
        Tile::Manzu2, Tile::Manzu2, Tile::Manzu2, Tile::Pinzu3, Tile::Pinzu3,
        Tile::Pinzu3, Tile::Souzu4, Tile::Souzu4, Tile::Souzu4, Tile::East,
        Tile::East,  Tile::East,  Tile::South, Tile::South};
    const PlayerState player{to_hand(tiles), {}, Tile::South};

    const ScoreResult result = ScoreCalculator::calc(
        table_config, round_state, table_state, player, Tile::South, WinFlag::None);
    REQUIRE(result.success);
    REQUIRE(result.score_limit == ScoreLimit::DoubleYakuman);
    REQUIRE(result.yaku_list.front().han == 2);
}
