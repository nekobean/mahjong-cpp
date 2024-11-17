#ifndef MAHJONG_CPP_STRING
#define MAHJONG_CPP_STRING

#include <string>
#include <vector>

#include "mahjong/types/types.hpp"

namespace mahjong
{
inline std::string to_mpsz(const std::vector<int> &hand)
{
    std::string s;
    const std::string suffix = "mpsz";
    bool has_suit[4] = {false, false, false, false};

    // 萬子
    for (int i = 0; i < 34; ++i) {
        int type = i / 9;
        int num = i % 9 + 1;

        if (hand.size() > 34 && ((i == Tile::Manzu1 && hand[Tile::RedManzu5]) ||
                                 (i == Tile::Pinzu1 && hand[Tile::RedPinzu5]) ||
                                 (i == Tile::Souzu1 && hand[Tile::RedSouzu5]))) {
            s += "0";
        }

        if (hand[i] > 0) {
            int count = hand[i];
            if (hand.size() > 34 && ((i == Tile::Manzu5 && hand[Tile::RedManzu5]) ||
                                     (i == Tile::Pinzu5 && hand[Tile::RedPinzu5]) ||
                                     (i == Tile::Souzu5 && hand[Tile::RedSouzu5]))) {
                --count;
            }

            for (int j = 0; j < count; ++j) {
                s += std::to_string(num);
            }
            has_suit[type] = true;
        }

        if ((i == 8 || i == 17 || i == 26 || i == 33) && has_suit[type]) {
            s += suffix[type];
        }
    }

    return s;
}

} // namespace mahjong

#endif /* MAHJONG_CPP_STRING */
