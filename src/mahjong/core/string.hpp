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

// inline std::string Result::to_string()
// {
//     std::string s;

//     if (!success) {
//         s += fmt::format("エラー: {}", err_msg);
//         return s;
//     }

//     s += "[結果]\n";
//     s +=
//         fmt::format("手牌: {}, 和了牌: {}, {}\n", to_mpsz(hand.counts),
//                     Tile::Name.at(win_tile), (flag & WinFlag::Tsumo) ? "自摸" : "ロン");

//     if (han > 0) {
//         if (!blocks.empty()) {
//             s += "面子構成:\n";
//             for (const auto &block : blocks)
//                 s += fmt::format("  {}\n", block.to_string());
//         }
//         s += fmt::format("待ち: {}\n", WaitType::Name.at(wait_type));

//         // 役
//         s += "役:\n";
//         for (auto &[yaku, n] : yaku_list)
//             s += fmt::format(" {} {}翻\n", Yaku::Name[yaku], n);

//         // 飜、符
//         s += fmt::format("{}符{}翻{}\n", fu, han,
//                          score_title != ScoreTitle::Null
//                              ? " " + ScoreTitle::Name.at(score_title)
//                              : "");
//     }
//     else {
//         // 流し満貫、役満
//         s += "役:\n";
//         for (auto &[yaku, n] : yaku_list)
//             s += fmt::format(" {}\n", Yaku::Name[yaku]);
//         s += ScoreTitle::Name[score_title] + "\n";
//     }

//     if (score.size() == 3)
//         s += fmt::format(
//             "和了者の獲得点数: {}点, 親の支払い点数: {}, 子の支払い点数: {}\n",
//             score[0], score[1], score[2]);
//     else
//         s += fmt::format("和了者の獲得点数: {}点, 放銃者の支払い点数: {}\n", score[0],
//                          score[1]);

//     return s;
// }

} // namespace mahjong

#endif /* MAHJONG_CPP_STRING */
