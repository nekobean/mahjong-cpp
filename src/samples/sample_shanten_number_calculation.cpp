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

    {
        // Calculate minimum shanten number of regular hand, Seven Pairs and Thirteen Orphans.
        auto [shanten_type, shanten] =
            ShantenCalculator::calc(hand, num_melds, ShantenFlag::All);
        std::cout << "shanten type: " << ShantenFlag::Name.at(shanten_type)
                  << std::endl;
        std::cout << "shanten: " << shanten << std::endl;
    }

    {
        // Calculate shanten number for regular hand (exclude Seven Pairs, Thirteen Orphans).
        auto [shanten_type, shanten] =
            ShantenCalculator::calc(hand, num_melds, ShantenFlag::Regular);
        std::cout << "shanten type: " << ShantenFlag::Name.at(shanten_type)
                  << std::endl;
        std::cout << "shanten: " << shanten << std::endl;
    }

    {
        // Calculate shanten number for Seven Pairs (七対子).
        auto [shanten_type, shanten] =
            ShantenCalculator::calc(hand, num_melds, ShantenFlag::SevenPairs);
        std::cout << "shanten type: " << ShantenFlag::Name.at(shanten_type)
                  << std::endl;
        std::cout << "shanten: " << shanten << std::endl;
    }

    {
        // Calculate shanten number for Thirteen Orphans (国士無双).
        auto [shanten_type, shanten] =
            ShantenCalculator::calc(hand, num_melds, ShantenFlag::ThirteenOrphans);
        std::cout << "shanten type: " << ShantenFlag::Name.at(shanten_type)
                  << std::endl;
        std::cout << "shanten: " << shanten << std::endl;
    }
}
