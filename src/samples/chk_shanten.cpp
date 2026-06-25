#include <iostream>
#include <string>
#include <vector>

#include "mahjong/mahjong.hpp"

using namespace mahjong;

int main()
{
    // (mpsz, expected regular shanten) — all concealed (num_melds = 0).
    const std::vector<std::pair<std::string, int>> cases = {
        {"11144445555777z", 0},    // E3 N4 Wh4 R3  (reported bug case)
        {"11114444555777z", 0},    // E4 N4 Wh3 R3  (two quads)
        {"11144455557772z", 0},    // E3 N3 Wh4 R3 S1 (one quad)
        {"11122233344455z", -1},   // E3 S3 W3 N3 Wh2 (complete, no quad)
        {"1112223334455z", 0},     // E3 S3 W3 N2 Wh2 -> wait, count
        {"1111m4444m7777m99m", 1}, // suit quads, isolated (genuinely 1)
        {"1111m2222m333m99m", 0},  // suit quads but sequences available
        // Suit analog of the honor bug: 1p/9p triplets + 4p/7p quads, no sequences.
        {"111p4444p7777p999p", 0}, // 1p3 4p4 7p4 9p3 -> tenpai (analog of bug)
        {"111m4444m7777m999m", 0}, // same in manzu
    };

    for (const auto &[mpsz, expected] : cases) {
        Hand hand = from_mpsz(mpsz);
        int total = 0;
        for (int i = 0; i < 34; ++i)
            total += hand[i];
        const auto [type, shanten] =
            ShantenCalculator::calc(hand, 0, ShantenFlag::StandardHand, GameMode::Yonma);
        const bool ok = shanten == expected;
        std::cout << (ok ? "[ OK ] " : "[BUG ] ") << mpsz << "  tiles=" << total
                  << "  regular_shanten=" << shanten << "  expected=" << expected
                  << "\n";
    }
    return 0;
}
