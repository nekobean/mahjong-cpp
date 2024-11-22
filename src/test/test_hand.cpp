#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_ENABLE_BENCHMARKING

#include <fstream>
#include <iostream>

#include <catch2/catch.hpp>

#include "mahjong/mahjong.hpp"

using namespace mahjong;

TEST_CASE("check_hand")
{
    SECTION("valid")
    {
        REQUIRE_NOTHROW(from_mpsz("11122233344455m"));
    }

    SECTION("More than 5 tiles are used")
    {
        REQUIRE_THROWS(from_mpsz("1111133344455m"));
    }

    SECTION("More than 2 red doras are used")
    {
        REQUIRE_THROWS(from_mpsz("11122233344400m"));
    }

    SECTION("0m flag specified but 5m is not included")
    {
        Hand hand = from_mpsz("11122233344466m");
        hand[Tile::RedManzu5] = 1;
        REQUIRE_THROWS(check_hand(hand));
    }

    SECTION("0p flag specified but 5p is not included")
    {
        Hand hand = from_mpsz("11122233344466m");
        hand[Tile::RedPinzu5] = 1;
        REQUIRE_THROWS(check_hand(hand));
    }

    SECTION("0s flag specified but 5s is not included")
    {
        Hand hand = from_mpsz("11122233344466m");
        hand[Tile::RedSouzu5] = 1;
        REQUIRE_THROWS(check_hand(hand));
    }

    SECTION("More than 14 tiles are used")
    {
        REQUIRE_THROWS(from_mpsz("111122233344466m"));
    }

    SECTION("The number of tiles divisible by 3")
    {
        REQUIRE_THROWS(from_mpsz("222333444666m"));
    }
}
