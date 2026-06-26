#define CATCH_CONFIG_MAIN
#include <algorithm>
#include <vector>

#include <catch2/catch.hpp>

#include "mahjong/mahjong.hpp"

using namespace mahjong;

namespace
{

std::vector<int> make_all_tiles()
{
    std::vector<int> tiles;
    tiles.reserve(Tile::Length);
    for (int tile = 0; tile < Tile::Length; ++tile) {
        tiles.push_back(tile);
    }
    return tiles;
}

bool contains(const std::vector<int> &tiles, int tile)
{
    return std::find(tiles.begin(), tiles.end(), tile) != tiles.end();
}

int expected_yonma_indicator(int tile)
{
    switch (tile) {
    case Tile::Manzu1:
        return Tile::Manzu9;
    case Tile::Manzu2:
        return Tile::Manzu1;
    case Tile::Manzu3:
        return Tile::Manzu2;
    case Tile::Manzu4:
        return Tile::Manzu3;
    case Tile::Manzu5:
        return Tile::Manzu4;
    case Tile::Manzu6:
        return Tile::Manzu5;
    case Tile::Manzu7:
        return Tile::Manzu6;
    case Tile::Manzu8:
        return Tile::Manzu7;
    case Tile::Manzu9:
        return Tile::Manzu8;
    case Tile::Pinzu1:
        return Tile::Pinzu9;
    case Tile::Pinzu2:
        return Tile::Pinzu1;
    case Tile::Pinzu3:
        return Tile::Pinzu2;
    case Tile::Pinzu4:
        return Tile::Pinzu3;
    case Tile::Pinzu5:
        return Tile::Pinzu4;
    case Tile::Pinzu6:
        return Tile::Pinzu5;
    case Tile::Pinzu7:
        return Tile::Pinzu6;
    case Tile::Pinzu8:
        return Tile::Pinzu7;
    case Tile::Pinzu9:
        return Tile::Pinzu8;
    case Tile::Souzu1:
        return Tile::Souzu9;
    case Tile::Souzu2:
        return Tile::Souzu1;
    case Tile::Souzu3:
        return Tile::Souzu2;
    case Tile::Souzu4:
        return Tile::Souzu3;
    case Tile::Souzu5:
        return Tile::Souzu4;
    case Tile::Souzu6:
        return Tile::Souzu5;
    case Tile::Souzu7:
        return Tile::Souzu6;
    case Tile::Souzu8:
        return Tile::Souzu7;
    case Tile::Souzu9:
        return Tile::Souzu8;
    case Tile::East:
        return Tile::North;
    case Tile::South:
        return Tile::East;
    case Tile::West:
        return Tile::South;
    case Tile::North:
        return Tile::West;
    case Tile::WhiteDragon:
        return Tile::RedDragon;
    case Tile::GreenDragon:
        return Tile::WhiteDragon;
    case Tile::RedDragon:
        return Tile::GreenDragon;
    case Tile::RedManzu5:
        return Tile::Manzu4;
    case Tile::RedPinzu5:
        return Tile::Pinzu4;
    case Tile::RedSouzu5:
        return Tile::Souzu4;
    default:
        return Tile::Null;
    }
}

int expected_sanma_indicator(int tile)
{
    switch (tile) {
    case Tile::Manzu1:
        return Tile::Manzu9;
    case Tile::Manzu9:
        return Tile::Manzu1;
    case Tile::Manzu2:
    case Tile::Manzu3:
    case Tile::Manzu4:
    case Tile::Manzu5:
    case Tile::Manzu6:
    case Tile::Manzu7:
    case Tile::Manzu8:
    case Tile::RedManzu5:
        return Tile::Null;
    default:
        return expected_yonma_indicator(tile);
    }
}

int expected_yonma_dora(int tile)
{
    switch (tile) {
    case Tile::Manzu1:
        return Tile::Manzu2;
    case Tile::Manzu2:
        return Tile::Manzu3;
    case Tile::Manzu3:
        return Tile::Manzu4;
    case Tile::Manzu4:
        return Tile::Manzu5;
    case Tile::Manzu5:
        return Tile::Manzu6;
    case Tile::Manzu6:
        return Tile::Manzu7;
    case Tile::Manzu7:
        return Tile::Manzu8;
    case Tile::Manzu8:
        return Tile::Manzu9;
    case Tile::Manzu9:
        return Tile::Manzu1;
    case Tile::Pinzu1:
        return Tile::Pinzu2;
    case Tile::Pinzu2:
        return Tile::Pinzu3;
    case Tile::Pinzu3:
        return Tile::Pinzu4;
    case Tile::Pinzu4:
        return Tile::Pinzu5;
    case Tile::Pinzu5:
        return Tile::Pinzu6;
    case Tile::Pinzu6:
        return Tile::Pinzu7;
    case Tile::Pinzu7:
        return Tile::Pinzu8;
    case Tile::Pinzu8:
        return Tile::Pinzu9;
    case Tile::Pinzu9:
        return Tile::Pinzu1;
    case Tile::Souzu1:
        return Tile::Souzu2;
    case Tile::Souzu2:
        return Tile::Souzu3;
    case Tile::Souzu3:
        return Tile::Souzu4;
    case Tile::Souzu4:
        return Tile::Souzu5;
    case Tile::Souzu5:
        return Tile::Souzu6;
    case Tile::Souzu6:
        return Tile::Souzu7;
    case Tile::Souzu7:
        return Tile::Souzu8;
    case Tile::Souzu8:
        return Tile::Souzu9;
    case Tile::Souzu9:
        return Tile::Souzu1;
    case Tile::East:
        return Tile::South;
    case Tile::South:
        return Tile::West;
    case Tile::West:
        return Tile::North;
    case Tile::North:
        return Tile::East;
    case Tile::WhiteDragon:
        return Tile::GreenDragon;
    case Tile::GreenDragon:
        return Tile::RedDragon;
    case Tile::RedDragon:
        return Tile::WhiteDragon;
    case Tile::RedManzu5:
        return Tile::Manzu6;
    case Tile::RedPinzu5:
        return Tile::Pinzu6;
    case Tile::RedSouzu5:
        return Tile::Souzu6;
    default:
        return Tile::Null;
    }
}

int expected_sanma_dora(int tile)
{
    switch (tile) {
    case Tile::Manzu1:
        return Tile::Manzu9;
    case Tile::Manzu9:
        return Tile::Manzu1;
    case Tile::Manzu2:
    case Tile::Manzu3:
    case Tile::Manzu4:
    case Tile::Manzu5:
    case Tile::Manzu6:
    case Tile::Manzu7:
    case Tile::Manzu8:
    case Tile::RedManzu5:
        return Tile::Null;
    default:
        return expected_yonma_dora(tile);
    }
}

} // namespace

TEST_CASE("Tile names are defined for all tile constants")
{
    const std::vector<std::string_view> expected_names = {
        "1m", "2m", "3m", "4m", "5m", "6m", "7m", "8m", "9m", "1p", "2p", "3p", "4p",
        "5p", "6p", "7p", "8p", "9p", "1s", "2s", "3s", "4s", "5s", "6s", "7s", "8s",
        "9s", "1z", "2z", "3z", "4z", "5z", "6z", "7z", "0m", "0p", "0s",
    };

    REQUIRE(expected_names.size() == Tile::Length);

    for (int tile = 0; tile < Tile::Length; ++tile) {
        REQUIRE(Tile::name(tile) == expected_names[tile]);
    }

    REQUIRE(Tile::name(Tile::Null) == "Null");
    REQUIRE(Tile::name(99) == "Unknown");
}

TEST_CASE("Red tile conversion and detection")
{
    REQUIRE(Tile::to_normal(Tile::Manzu5) == Tile::Manzu5);
    REQUIRE(Tile::to_normal(Tile::Pinzu5) == Tile::Pinzu5);
    REQUIRE(Tile::to_normal(Tile::Souzu5) == Tile::Souzu5);
    REQUIRE(Tile::to_normal(Tile::RedManzu5) == Tile::Manzu5);
    REQUIRE(Tile::to_normal(Tile::RedPinzu5) == Tile::Pinzu5);
    REQUIRE(Tile::to_normal(Tile::RedSouzu5) == Tile::Souzu5);

    REQUIRE(Tile::is_red(Tile::RedManzu5) == true);
    REQUIRE(Tile::is_red(Tile::RedPinzu5) == true);
    REQUIRE(Tile::is_red(Tile::RedSouzu5) == true);
    REQUIRE(Tile::is_red(Tile::Manzu5) == false);
    REQUIRE(Tile::is_red(Tile::East) == false);
}

TEST_CASE("Tile suit and honor categories are classified correctly")
{
    const std::vector<int> manzu_tiles = {
        Tile::Manzu1, Tile::Manzu2, Tile::Manzu3, Tile::Manzu4, Tile::Manzu5,
        Tile::Manzu6, Tile::Manzu7, Tile::Manzu8, Tile::Manzu9, Tile::RedManzu5,
    };
    const std::vector<int> pinzu_tiles = {
        Tile::Pinzu1, Tile::Pinzu2, Tile::Pinzu3, Tile::Pinzu4, Tile::Pinzu5,
        Tile::Pinzu6, Tile::Pinzu7, Tile::Pinzu8, Tile::Pinzu9, Tile::RedPinzu5,
    };
    const std::vector<int> souzu_tiles = {
        Tile::Souzu1, Tile::Souzu2, Tile::Souzu3, Tile::Souzu4, Tile::Souzu5,
        Tile::Souzu6, Tile::Souzu7, Tile::Souzu8, Tile::Souzu9, Tile::RedSouzu5,
    };
    const std::vector<int> honor_tiles = {
        Tile::East,        Tile::South,       Tile::West,      Tile::North,
        Tile::WhiteDragon, Tile::GreenDragon, Tile::RedDragon,
    };

    for (const int tile : make_all_tiles()) {
        REQUIRE(Tile::is_manzu(tile) == contains(manzu_tiles, tile));
        REQUIRE(Tile::is_pinzu(tile) == contains(pinzu_tiles, tile));
        REQUIRE(Tile::is_souzu(tile) == contains(souzu_tiles, tile));
        REQUIRE(Tile::is_suit(tile) ==
                (contains(manzu_tiles, tile) || contains(pinzu_tiles, tile) ||
                 contains(souzu_tiles, tile)));
        REQUIRE(Tile::is_honor(tile) == contains(honor_tiles, tile));
    }
}

TEST_CASE("Terminal, honor, and simple classifications are correct")
{
    const std::vector<int> terminal_tiles = {
        Tile::Manzu1, Tile::Manzu9, Tile::Pinzu1,
        Tile::Pinzu9, Tile::Souzu1, Tile::Souzu9,
    };
    const std::vector<int> honor_tiles = {
        Tile::East,        Tile::South,       Tile::West,      Tile::North,
        Tile::WhiteDragon, Tile::GreenDragon, Tile::RedDragon,
    };
    const std::vector<int> simple_tiles = {
        Tile::Manzu2, Tile::Manzu3,    Tile::Manzu4,    Tile::Manzu5,    Tile::Manzu6,
        Tile::Manzu7, Tile::Manzu8,    Tile::Pinzu2,    Tile::Pinzu3,    Tile::Pinzu4,
        Tile::Pinzu5, Tile::Pinzu6,    Tile::Pinzu7,    Tile::Pinzu8,    Tile::Souzu2,
        Tile::Souzu3, Tile::Souzu4,    Tile::Souzu5,    Tile::Souzu6,    Tile::Souzu7,
        Tile::Souzu8, Tile::RedManzu5, Tile::RedPinzu5, Tile::RedSouzu5,
    };

    for (const int tile : make_all_tiles()) {
        REQUIRE(Tile::is_terminal(tile) == contains(terminal_tiles, tile));
        REQUIRE(Tile::is_terminal_or_honor(tile) ==
                (contains(terminal_tiles, tile) || contains(honor_tiles, tile)));
        REQUIRE(Tile::is_simple(tile) == contains(simple_tiles, tile));
    }
}

TEST_CASE("Sanma disabled tiles are classified correctly")
{
    const std::vector<int> sanma_disabled_tiles = {
        Tile::Manzu2, Tile::Manzu3, Tile::Manzu4, Tile::Manzu5,
        Tile::Manzu6, Tile::Manzu7, Tile::Manzu8, Tile::RedManzu5,
    };

    for (const int tile : make_all_tiles()) {
        REQUIRE(Tile::is_sanma_disabled(tile) == contains(sanma_disabled_tiles, tile));
    }
}

TEST_CASE("Dora conversion is correct for all yonma and sanma tiles")
{
    for (int tile = 0; tile < Tile::Length; ++tile) {
        REQUIRE(Tile::to_indicator(tile, GameMode::Yonma) == expected_yonma_indicator(tile));
        REQUIRE(Tile::to_dora(tile, GameMode::Yonma) == expected_yonma_dora(tile));
        REQUIRE(Tile::to_indicator(tile, GameMode::Sanma) == expected_sanma_indicator(tile));
        REQUIRE(Tile::to_dora(tile, GameMode::Sanma) == expected_sanma_dora(tile));
    }
}

TEST_CASE("TableState dora setters append converted indicators for all tiles")
{
    const std::vector<int> all_tiles = make_all_tiles();

    SECTION("set_dora appends yonma dora indicators")
    {
        TableState table_state;
        table_state.dora_indicators = {Tile::East};

        table_state.set_dora(all_tiles, GameMode::Yonma);

        std::vector<int> expected = {Tile::East};
        for (const int tile : all_tiles) {
            expected.push_back(expected_yonma_indicator(tile));
        }
        REQUIRE(table_state.dora_indicators == expected);
    }

    SECTION("set_dora appends sanma dora indicators")
    {
        TableState table_state;
        table_state.dora_indicators = {Tile::South};

        table_state.set_dora(all_tiles, GameMode::Sanma);

        std::vector<int> expected = {Tile::South};
        for (const int tile : all_tiles) {
            expected.push_back(expected_sanma_indicator(tile));
        }
        REQUIRE(table_state.dora_indicators == expected);
    }

    SECTION("set_uradora appends yonma uradora indicators")
    {
        TableState table_state;
        table_state.uradora_indicators = {Tile::West};

        table_state.set_uradora(all_tiles, GameMode::Yonma);

        std::vector<int> expected = {Tile::West};
        for (const int tile : all_tiles) {
            expected.push_back(expected_yonma_indicator(tile));
        }
        REQUIRE(table_state.uradora_indicators == expected);
    }

    SECTION("set_uradora appends sanma uradora indicators")
    {
        TableState table_state;
        table_state.uradora_indicators = {Tile::North};

        table_state.set_uradora(all_tiles, GameMode::Sanma);

        std::vector<int> expected = {Tile::North};
        for (const int tile : all_tiles) {
            expected.push_back(expected_sanma_indicator(tile));
        }
        REQUIRE(table_state.uradora_indicators == expected);
    }
}
