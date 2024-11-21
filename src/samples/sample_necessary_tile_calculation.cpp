#include <iostream>

#include "mahjong/mahjong.hpp"

int main(int argc, char *argv[])
{
    using namespace mahjong;

    // Create hand by mpsz notation or vector of tiles.
    Hand hand = from_mpsz("222567m34p33667s");
    // Hand hand = from_array({Tile::Manzu2, Tile::Manzu2, Tile::Manzu2, Tile::Manzu5,
    //                          Tile::Manzu6, Tile::Manzu7, Tile::Pinzu3, Tile::Pinzu4,
    //                          Tile::Souzu3, Tile::Souzu3, Tile::Souzu6, Tile::Souzu6,
    //                          Tile::Souzu7});
    // number of melds.
    int num_melds = 0;

    // Calculate necessary tiles.
    const auto [shanten_type, shanten, tiles] =
        NecessaryTileCalculator::select(hand, num_melds, ShantenFlag::All);

    std::cout << "shanten: " << shanten << std::endl;
    for (auto tile : tiles) {
        std::cout << Tile::Name.at(tile) + " ";
    }
    std::cout << std::endl;
}
