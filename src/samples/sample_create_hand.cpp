#include <iostream>

#include "mahjong/mahjong.hpp"

int main(int argc, char *argv[])
{
    using namespace mahjong;

    // Create hand by mpsz notation.
    // 1m~9m: manzu, 0m: red5m
    // 1p~9p: pinzu, 0p: red5p
    // 1s~9s: souzu, 0s: red5s
    // 1z=East, 2z=South, 3z=West, 4z=North, 5z=White, 6z=Green, 7z=Red
    Hand hand1 = from_mpsz("222567345p333s22z");
    // Convert hand to mpsz notation string by to_string().
    std::cout << to_mpsz(hand1) << std::endl;

    // Create hand by list of tiles.
    Hand hand2 = from_array({Tile::Manzu2, Tile::Manzu2, Tile::Manzu2, Tile::Manzu5,
                             Tile::Manzu6, Tile::Manzu7, Tile::Pinzu3, Tile::Pinzu4,
                             Tile::Pinzu5, Tile::Souzu3, Tile::Souzu3, Tile::Souzu3,
                             Tile::South, Tile::South});
    std::cout << to_mpsz(hand2) << std::endl;

    // Create melds by specifying meld type and tiles.
    // MeldType::Pong      : pong (ポン)
    // MeldType::Chow      : chow (チー)
    // MeldType::ClosedKong: closed kong (暗槓)
    // MeldType::OpenKong  : open kong (明槓)
    // MeldType::AddedKong : added kong (加槓)
    std::vector<Meld> melds = {
        {MeldType::AddedKong, {Tile::East, Tile::East, Tile::East, Tile::East}},
        {MeldType::Pong, {Tile::Manzu1, Tile::Manzu1, Tile::Manzu1}},
    };

    for (const auto &meld : melds) {
        std::cout << to_string(meld) << " ";
    }
    std::cout << std::endl;
}
