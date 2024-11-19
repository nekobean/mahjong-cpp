#ifndef MAHJONG_CPP_STRING
#define MAHJONG_CPP_STRING

#include <string>
#include <vector>

#include "mahjong/types/types.hpp"

namespace mahjong
{

inline std::string to_mpsz(const HandType &hand)
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

/**
 * @brief Create a hand from a string in MPSZ notation.
 *
 * @param[in] tiles string in MPSZ notation
 * @return Hand object
 */
inline HandType from_mpsz(const std::string &tiles)
{
    HandType hand{0};

    std::string type;
    for (auto it = tiles.rbegin(); it != tiles.rend(); ++it) {
        if (std::isspace(*it)) {
            continue;
        }

        if (*it == 'm' || *it == 'p' || *it == 's' || *it == 'z') {
            type = *it;
        }
        else if (std::isdigit(*it)) {
            int tile = *it - '0' - 1;
            if (type == "m") {
                if (tile == -1) {
                    hand[Tile::RedManzu5]++;
                    hand[Tile::Manzu5]++;
                }
                else {
                    hand[tile]++;
                }
            }
            else if (type == "p") {
                if (tile == -1) {
                    hand[Tile::RedPinzu5]++;
                    hand[Tile::Pinzu5]++;
                }
                else {
                    hand[tile + 9]++;
                }
            }
            else if (type == "s") {
                if (tile == -1) {
                    hand[Tile::RedSouzu5]++;
                    hand[Tile::Souzu5]++;
                }
                else {
                    hand[tile + 18]++;
                }
            }
            else if (type == "z") {
                hand[tile + 27]++;
            }
        }
    }

    return hand;
}

inline std::string to_string(const Result &result)
{
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
    return "";
}

} // namespace mahjong

#endif /* MAHJONG_CPP_STRING */
