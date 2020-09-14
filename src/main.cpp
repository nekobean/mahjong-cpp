#include <iostream>
#include <sstream>

#include <spdlog/spdlog.h>

#include "syanten.hpp"

using namespace mahjong;

int main(int, char **)
{
    // 手牌入力
    Tehai tehai({Tile::Manzu1, Tile::Manzu1, Tile::Manzu1, Tile::Manzu2, Tile::Manzu5, Tile::Manzu6,
                 Tile::Manzu7, Tile::Manzu8, Tile::Manzu9, Tile::Pinzu1, Tile::Pinzu1, Tile::Pinzu2,
                 Tile::Pinzu2});

    int syanten = SyantenCalculator::calc(tehai);
    spdlog::info("Tehai: {}, Syanten: {}", tehai.to_string(), syanten);
}
