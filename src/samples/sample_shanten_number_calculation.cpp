#include <iostream>

#include "mahjong/mahjong.hpp"

int main()
{
    using namespace mahjong;

    const Hand hand = from_mpsz("1m137p1268s135567z");
    const int num_melds = 0;

    {
        // Calculate the minimum shanten number among standard hand,
        // Seven Pairs, and Thirteen Orphans.
        const auto [shanten_type, shanten] =
            ShantenCalculator::calc(hand, num_melds, ShantenFlag::All, GameMode::Yonma);
        std::cout << "shanten type: ";
        for (const ShantenFlags type :
             {ShantenFlag::StandardHand, ShantenFlag::SevenPairs,
              ShantenFlag::ThirteenOrphans}) {
            if (shanten_type & type) {
                std::cout << ShantenFlag::name(type) << " ";
            }
        }
        std::cout << std::endl;
        std::cout << "shanten: " << shanten << std::endl;
    }

    {
        // Calculate the shanten number for standard hand.
        const auto [shanten_type, shanten] = ShantenCalculator::calc(
            hand, num_melds, ShantenFlag::StandardHand, GameMode::Yonma);
        std::cout << "shanten type: " << ShantenFlag::name(shanten_type) << std::endl;
        std::cout << "shanten: " << shanten << std::endl;
    }

    {
        // Calculate shanten number for Seven Pairs.
        const auto [shanten_type, shanten] = ShantenCalculator::calc(
            hand, num_melds, ShantenFlag::SevenPairs, GameMode::Yonma);
        std::cout << "shanten type: " << ShantenFlag::name(shanten_type) << std::endl;
        std::cout << "shanten: " << shanten << std::endl;
    }

    {
        // Calculate shanten number for Thirteen Orphans.
        const auto [shanten_type, shanten] = ShantenCalculator::calc(
            hand, num_melds, ShantenFlag::ThirteenOrphans, GameMode::Yonma);
        std::cout << "shanten type: " << ShantenFlag::name(shanten_type) << std::endl;
        std::cout << "shanten: " << shanten << std::endl;
    }
}
