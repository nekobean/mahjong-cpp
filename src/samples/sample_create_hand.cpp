#include <iostream>

#include "mahjong/mahjong.hpp"

int main()
{
    using namespace mahjong;

    // Create hand by mpsz notation.
    // 1m~9m: manzu, 0m: red 5m
    // 1p~9p: pinzu, 0p: red 5p
    // 1s~9s: souzu, 0s: red 5s
    // 1z=East, 2z=South, 3z=West, 4z=North, 5z=WhiteDragon,
    // 6z=GreenDragon, 7z=RedDragon
    const Hand hand1 = from_mpsz("222567345p333s22z");
    // Convert a hand to an mpsz notation string.
    std::cout << to_mpsz(hand1) << std::endl;

    // Create hand by list of tiles.
    const Hand hand2 =
        from_array({Tile::Manzu2, Tile::Manzu2, Tile::Manzu2, Tile::Manzu5,
                    Tile::Manzu6, Tile::Manzu7, Tile::Pinzu3, Tile::Pinzu4,
                    Tile::Pinzu5, Tile::Souzu3, Tile::Souzu3, Tile::Souzu3,
                    Tile::South, Tile::South});
    std::cout << to_mpsz(hand2) << std::endl;

    // Create melds by specifying meld type and tiles.
    // MeldType::Pon      : pon (ポン)
    // MeldType::Chi      : chi (チー)
    // MeldType::Ankan    : closed kan (暗槓)
    // MeldType::Daiminkan: open kan (大明槓)
    // MeldType::Kakan    : added kan (加槓)
    const std::vector<Meld> melds = {
        {MeldType::Kakan, {Tile::East, Tile::East, Tile::East, Tile::East}},
        {MeldType::Pon, {Tile::Manzu1, Tile::Manzu1, Tile::Manzu1}},
    };

    for (const auto &meld : melds) {
        std::cout << to_string(meld) << " ";
    }
    std::cout << std::endl;
}
