#include "mahjong/mahjong.hpp"

using namespace mahjong;

int main(int, char **)
{
    SyantenCalculator::initialize();

    // 一般手の不要牌を計算する
    {
        Hand hand({Tile::Manzu1, Tile::Manzu1, Tile::Manzu1, Tile::Manzu2,
                   Tile::AkaManzu5, Tile::Manzu6, Tile::Manzu7, Tile::Manzu8,
                   Tile::Manzu9, Tile::Pinzu1, Tile::Pinzu1, Tile::Pinzu2,
                   Tile::Pinzu2});

        // 不要牌を計算する。
        auto tiles = UnnecessaryTileSelector::select_normal(hand);

        std::cout << fmt::format("[一般手の不要牌] 手牌: {}", hand.to_string())
                  << std::endl;

        for (auto tile : tiles)
            std::cout << fmt::format("{} ", Tile::Name.at(tile));
        std::cout << std::endl;
    }

    // 七対子手の不要牌を計算する
    {
        Hand hand({Tile::Manzu1, Tile::Manzu1, Tile::Manzu2, Tile::Manzu2,
                   Tile::AkaManzu5, Tile::Manzu5, Tile::Manzu8, Tile::Manzu8,
                   Tile::Manzu8, Tile::Ton, Tile::Nan, Tile::Sya, Tile::Sya});

        // 不要牌を計算する。
        auto tiles = UnnecessaryTileSelector::select_tiitoi(hand);

        std::cout << fmt::format("[七対子手の不要牌] 手牌: {}", hand.to_string())
                  << std::endl;

        for (auto tile : tiles)
            std::cout << fmt::format("{} ", Tile::Name.at(tile));
        std::cout << std::endl;
    }

    // 国士無双手の不要牌を計算する
    {
        Hand hand({Tile::Manzu1, Tile::Manzu1, Tile::Pinzu9, Tile::Sozu1, Tile::Sozu9,
                   Tile::Sozu9, Tile::Ton, Tile::Nan, Tile::Sya, Tile::Sya, Tile::Haku,
                   Tile::Hatu, Tile::Tyun});

        // 不要牌を計算する。
        auto tiles = UnnecessaryTileSelector::select_kokusi(hand);

        std::cout << fmt::format("[国士手の不要牌] 手牌: {}", hand.to_string())
                  << std::endl;

        for (auto tile : tiles)
            std::cout << fmt::format("{} ", Tile::Name.at(tile));
        std::cout << std::endl;
    }
}
