#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_ENABLE_BENCHMARKING

#include <iostream>

#include <catch2/catch.hpp>

#include "mahjong/mahjong.hpp"

using namespace mahjong;

TEST_CASE("to_mpsz")
{
    Hand hand1 = {
        0, 3, 0, 0, 1, 1, 1, 0, 0, // 1m ~ 9m
        0, 0, 1, 1, 0, 0, 0, 0, 0, // 1p ~ 9p
        0, 0, 2, 0, 0, 2, 1, 0, 0, // 1s ~ 9s
        1, 0, 0, 0, 0, 0, 0,       // 1z ~ 7z
        0, 0, 0                    // 0m, 0p, 0s
    };
    REQUIRE(to_mpsz(hand1) == u8"222567m34p33667s1z");

    Hand hand2 = {
        0, 3, 0, 0, 2, 1, 1, 0, 0, // 1m ~ 9m
        0, 0, 1, 1, 0, 0, 0, 0, 0, // 1p ~ 9p
        0, 0, 2, 0, 0, 2, 1, 0, 0, // 1s ~ 9s
        0, 0, 0, 0, 0, 0, 0,       // 1z ~ 7z
        1, 0, 0                    // 0m, 0p, 0s
    };
    REQUIRE(to_mpsz(hand2) == u8"0222567m34p33667s");

    Hand hand3 = {
        0, 3, 0, 0, 1, 1, 1, 0, 0, // 1m ~ 9m
        0, 0, 1, 1, 1, 0, 0, 0, 0, // 1p ~ 9p
        0, 0, 2, 0, 0, 2, 1, 0, 0, // 1s ~ 9s
        0, 0, 0, 0, 0, 0, 0,       // 1z ~ 7z
        0, 1, 0                    // 0m, 0p, 0s
    };
    REQUIRE(to_mpsz(hand3) == u8"222567m034p33667s");

    Hand hand4 = {
        0, 3, 0, 0, 1, 1, 1, 0, 0, // 1m ~ 9m
        0, 0, 1, 1, 0, 0, 0, 0, 0, // 1p ~ 9p
        0, 0, 2, 0, 1, 2, 1, 0, 0, // 1s ~ 9s
        0, 0, 0, 0, 0, 0, 0,       // 1z ~ 7z
        0, 0, 1                    // 0m, 0p, 0s
    };
    REQUIRE(to_mpsz(hand4) == u8"222567m34p033667s");
}

TEST_CASE("from_mpsz")
{
    Hand hand1 = {
        0, 3, 0, 0, 1, 1, 1, 0, 0, // 1m ~ 9m
        0, 0, 1, 1, 0, 0, 0, 0, 0, // 1p ~ 9p
        0, 0, 2, 0, 0, 2, 1, 0, 0, // 1s ~ 9s
        1, 0, 0, 0, 0, 0, 0,       // 1z ~ 7z
        0, 0, 0                    // 0m, 0p, 0s
    };
    REQUIRE(from_mpsz(u8"222567m34p33667s1z") == hand1);

    Hand hand2 = {
        0, 3, 0, 0, 2, 1, 1, 0, 0, // 1m ~ 9m
        0, 0, 1, 1, 0, 0, 0, 0, 0, // 1p ~ 9p
        0, 0, 2, 0, 0, 2, 1, 0, 0, // 1s ~ 9s
        0, 0, 0, 0, 0, 0, 0,       // 1z ~ 7z
        1, 0, 0                    // 0m, 0p, 0s
    };
    REQUIRE(from_mpsz(u8"0222567m34p33667s") == hand2);

    Hand hand3 = {
        0, 3, 0, 0, 1, 1, 1, 0, 0, // 1m ~ 9m
        0, 0, 1, 1, 1, 0, 0, 0, 0, // 1p ~ 9p
        0, 0, 2, 0, 0, 2, 1, 0, 0, // 1s ~ 9s
        0, 0, 0, 0, 0, 0, 0,       // 1z ~ 7z
        0, 1, 0                    // 0m, 0p, 0s
    };
    REQUIRE(from_mpsz(u8"222567m034p33667s") == hand3);

    Hand hand4 = {
        0, 3, 0, 0, 1, 1, 1, 0, 0, // 1m ~ 9m
        0, 0, 1, 1, 0, 0, 0, 0, 0, // 1p ~ 9p
        0, 0, 2, 0, 1, 2, 1, 0, 0, // 1s ~ 9s
        0, 0, 0, 0, 0, 0, 0,       // 1z ~ 7z
        0, 0, 1                    // 0m, 0p, 0s
    };
    REQUIRE(from_mpsz(u8"222567m34p033667s") == hand4);
}

TEST_CASE("to_string(Block)")
{
    Block block1(BlockType::Triplet, Tile::Manzu1);
    REQUIRE(to_string(block1) == u8"[111m 暗刻子]");

    Block block2(BlockType::Triplet | BlockType::Open, Tile::Manzu1);
    REQUIRE(to_string(block2) == u8"[111m 明刻子]");

    Block block3(BlockType::Sequence, Tile::Manzu1);
    REQUIRE(to_string(block3) == u8"[123m 暗順子]");

    Block block4(BlockType::Sequence | BlockType::Open, Tile::Manzu1);
    REQUIRE(to_string(block4) == u8"[123m 明順子]");

    Block block5(BlockType::Kong, Tile::Manzu1);
    REQUIRE(to_string(block5) == u8"[1111m 暗槓子]");

    Block block6(BlockType::Kong | BlockType::Open, Tile::Manzu1);
    REQUIRE(to_string(block6) == u8"[1111m 明槓子]");

    Block block7(BlockType::Pair, Tile::Manzu1);
    REQUIRE(to_string(block7) == u8"[11m 暗対子]");

    Block block8(BlockType::Pair | BlockType::Open, Tile::Manzu1);
    REQUIRE(to_string(block8) == u8"[11m 明対子]");
}

TEST_CASE("to_string(Meld)")
{
    MeldedBlock meld1(MeldType::Pong, {Tile::Manzu1, Tile::Manzu1, Tile::Manzu1},
                      Tile::Manzu1, PlayerType::Player1);
    REQUIRE(to_string(meld1) == u8"[111m ポン]");

    MeldedBlock meld2(MeldType::Chow, {Tile::Manzu1, Tile::Manzu2, Tile::Manzu3},
                      Tile::Manzu1, PlayerType::Player1);
    REQUIRE(to_string(meld2) == u8"[123m チー]");

    MeldedBlock meld3(MeldType::ClosedKong,
                      {Tile::Manzu1, Tile::Manzu1, Tile::Manzu1, Tile::Manzu1},
                      Tile::Manzu1, PlayerType::Player0);
    REQUIRE(to_string(meld3) == u8"[1111m 暗槓]");

    MeldedBlock meld4(MeldType::OpenKong,
                      {Tile::Manzu1, Tile::Manzu1, Tile::Manzu1, Tile::Manzu1},
                      Tile::Manzu1, PlayerType::Player1);
    REQUIRE(to_string(meld4) == u8"[1111m 明槓]");

    MeldedBlock meld5(MeldType::AddedKong,
                      {Tile::Manzu1, Tile::Manzu1, Tile::Manzu1, Tile::Manzu1},
                      Tile::Manzu1, PlayerType::Player1);
    REQUIRE(to_string(meld5) == u8"[1111m 加槓]");
}

TEST_CASE("to_string(Round)")
{
    Round round;
    round.rules = RuleFlag::RedDora | RuleFlag::OpenTanyao;
    round.wind = Tile::East;
    round.kyoku = 1;
    round.honba = 3;
    round.kyotaku = 2;
    round.dora_indicators = {Tile::Manzu1};

    std::cout << to_string(round) << std::endl;
}

TEST_CASE("to_string(Player)")
{
    Hand hand = from_mpsz(u8"222567m34p33667s1z");
    std::vector<MeldedBlock> melds = {{MeldType::Pong,
                                       {Tile::Manzu1, Tile::Manzu1, Tile::Manzu1},
                                       Tile::Manzu1,
                                       PlayerType::Player0}};
    MyPlayer player(hand, melds, Tile::East);

    std::cout << to_string(player) << std::endl;
}

TEST_CASE("to_string(Result)")
{
    Round round;
    round.rules = RuleFlag::RedDora | RuleFlag::OpenTanyao;
    round.wind = Tile::East;
    round.kyoku = 1;
    round.honba = 3;
    round.kyotaku = 2;
    round.dora_indicators = {Tile::Manzu1};

    MyPlayer player(from_mpsz(u8"222345m345p345s11z"), Tile::East);
    Result result = ScoreCalculator::calc(round, player, Tile::East, WinFlag::Tsumo);

    std::cout << to_string(result) << std::endl;
}
