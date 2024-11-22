#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_ENABLE_BENCHMARKING

#include <fstream>
#include <iostream>

#include <catch2/catch.hpp>

#include "mahjong/mahjong.hpp"

using namespace mahjong;

TEST_CASE("Player")
{
    SECTION("valid")
    {
        Player player;
        player.hand = from_mpsz("11122233344455m");
        player.melds = {};

        REQUIRE(player.is_closed() == true);
    }

    SECTION("closed kong exists")
    {
        Player player;
        player.hand = from_mpsz("22233344455m");
        player.melds = {{MeldType::ClosedKong,
                         {Tile::Manzu1, Tile::Manzu1, Tile::Manzu1, Tile::Manzu1}}};

        REQUIRE(player.is_closed() == true);
    }

    SECTION("any other closed kong exists")
    {
        Player player;
        player.hand = from_mpsz("22233344455m");
        player.melds = {{MeldType::OpenKong,
                         {Tile::Manzu1, Tile::Manzu1, Tile::Manzu1, Tile::Manzu1}}};

        REQUIRE(player.is_closed() == false);
    }

    SECTION("num_tiles()")
    {
        Player player;
        player.hand = from_mpsz("11122233344455m");
        REQUIRE(player.num_tiles() == 14);

        player.hand = from_mpsz("22233344455m");
        REQUIRE(player.num_tiles() == 11);

        player.hand = from_mpsz("33344455m");
        REQUIRE(player.num_tiles() == 8);

        player.hand = from_mpsz("44455m");
        REQUIRE(player.num_tiles() == 5);

        player.hand = from_mpsz("55m");
        REQUIRE(player.num_tiles() == 2);
    }
}
