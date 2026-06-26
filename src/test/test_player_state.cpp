#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_ENABLE_BENCHMARKING

#include <catch2/catch.hpp>

#include "mahjong/mahjong.hpp"

using namespace mahjong;

TEST_CASE("PlayerState")
{
    SECTION("default values are initialized")
    {
        const PlayerState player;

        REQUIRE(player.num_tiles() == 0);
        REQUIRE(player.num_melds() == 0);
        REQUIRE(player.is_closed() == true);
        REQUIRE(player.seat_wind == Tile::Null);
        REQUIRE(player.nuki_count == 0);
        REQUIRE(player.score == 0);
    }

    SECTION("is_closed returns true for a concealed hand")
    {
        PlayerState player;
        player.hand = from_mpsz("11122233344455m");
        player.melds = {};

        REQUIRE(player.is_closed() == true);
    }

    SECTION("is_closed returns false for a hand with a pon meld")
    {
        PlayerState player;
        player.hand = from_mpsz("22233344455m");
        player.melds = {
            {MeldType::Pon, {Tile::Manzu1, Tile::Manzu1, Tile::Manzu1}},
        };

        REQUIRE(player.is_closed() == false);
    }

    SECTION("is_closed returns false for a hand with a chi meld")
    {
        PlayerState player;
        player.hand = from_mpsz("22233344455m");
        player.melds = {
            {MeldType::Chi, {Tile::Manzu1, Tile::Manzu2, Tile::Manzu3}},
        };

        REQUIRE(player.is_closed() == false);
    }

    SECTION("is_closed returns true for a hand with only concealed kans")
    {
        PlayerState player;
        player.hand = from_mpsz("22233344455m");
        player.melds = {{MeldType::Ankan,
                         {Tile::Manzu1, Tile::Manzu1, Tile::Manzu1, Tile::Manzu1}}};

        REQUIRE(player.is_closed() == true);
    }

    SECTION("is_closed returns false for a hand with an open kan")
    {
        PlayerState player;
        player.hand = from_mpsz("22233344455m");
        player.melds = {{MeldType::Daiminkan,
                         {Tile::Manzu1, Tile::Manzu1, Tile::Manzu1, Tile::Manzu1}}};

        REQUIRE(player.is_closed() == false);
    }

    SECTION("is_closed returns false for a hand with an added kan")
    {
        PlayerState player;
        player.hand = from_mpsz("22233344455m");
        player.melds = {{MeldType::Kakan,
                         {Tile::Manzu1, Tile::Manzu1, Tile::Manzu1, Tile::Manzu1}}};

        REQUIRE(player.is_closed() == false);
    }

    SECTION("is_closed returns false when open and concealed melds coexist")
    {
        PlayerState player;
        player.hand = from_mpsz("33344455m");
        player.melds = {
            {MeldType::Ankan, {Tile::Manzu1, Tile::Manzu1, Tile::Manzu1, Tile::Manzu1}},
            {MeldType::Pon, {Tile::Pinzu1, Tile::Pinzu1, Tile::Pinzu1}},
        };

        REQUIRE(player.is_closed() == false);
    }

    SECTION("num_tiles returns the number of concealed hand tiles")
    {
        PlayerState player;
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

    SECTION("num_tiles counts red fives as concealed hand tiles")
    {
        PlayerState player;
        player.hand = from_mpsz("123m405p789s11z");

        REQUIRE(player.num_tiles() == 11);
    }

    SECTION("num_melds returns the number of melds")
    {
        PlayerState player;
        REQUIRE(player.num_melds() == 0);

        player.melds.push_back(
            Meld{MeldType::Chi, {Tile::Manzu1, Tile::Manzu2, Tile::Manzu3}});
        REQUIRE(player.num_melds() == 1);

        player.melds.push_back(
            Meld{MeldType::Pon, {Tile::Pinzu1, Tile::Pinzu1, Tile::Pinzu1}});
        REQUIRE(player.num_melds() == 2);
    }

    SECTION("num_melds counts all meld types")
    {
        PlayerState player;
        player.melds = {
            {MeldType::Chi, {Tile::Manzu1, Tile::Manzu2, Tile::Manzu3}},
            {MeldType::Pon, {Tile::Pinzu1, Tile::Pinzu1, Tile::Pinzu1}},
            {MeldType::Ankan, {Tile::Souzu1, Tile::Souzu1, Tile::Souzu1, Tile::Souzu1}},
            {MeldType::Kakan, {Tile::East, Tile::East, Tile::East, Tile::East}},
        };

        REQUIRE(player.num_melds() == 4);
    }
}
