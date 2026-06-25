#include <iostream>

#include "mahjong/mahjong.hpp"

int main()
{
    using namespace mahjong;

    const Hand hand = from_mpsz("222567m34p33667s");
    const int num_melds = 0;

    const auto [shanten_type, shanten, tiles] = NecessaryTileCalculator::select(
        hand, num_melds, ShantenFlag::All, GameMode::Yonma);

    std::cout << "shanten: " << shanten << std::endl;
    for (const int tile : tiles) {
        std::cout << Tile::name(tile) << " ";
    }
    std::cout << std::endl;
}
