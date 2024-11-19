#ifndef MAHJONG_CPP_STRING
#define MAHJONG_CPP_STRING

#include <string>
#include <vector>

#include "mahjong/types/types.hpp"

namespace mahjong
{

inline std::string to_mpsz(const Hand &hand)
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
inline Hand from_mpsz(const std::string &tiles)
{
    Hand hand{0};

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

// /**
//  * @brief 設定を文字列にして返す。
//  *
//  * @return std::string 文字列
//  */
// inline std::string print_round_info(const ScoreCalculator &score)
// {
//     // std::string s;

//     // s += "[ルール]\n";
//     // for (auto rule : {RuleFlag::RedDora, RuleFlag::OpenTanyao}) {
//     //     s += fmt::format("  {}: {}\n", RuleFlag::Name.at(rule),
//     //                      (score.rules() & rule) ? "有り" : "無し");
//     // }

//     // s +=
//     //     fmt::format("[場] 場風: {}, 自風: {}, 積み棒の数: {}, 供託棒の数: {}\n",
//     //                 Tile::Name.at(score.wind()), Tile::Name.at(score.self_wind()),
//     //                 score.bonus_sticks(), score.deposit_sticks());

//     // s += "[表ドラ] ";
//     // for (const auto &tile : score.dora_tiles())
//     //     s += fmt::format("{}{}", Tile::Name.at(tile),
//     //                      &tile == &score.dora_tiles().back() ? "\n" : ", ");

//     // s += "[裏ドラ] ";
//     // for (const auto &tile : score.uradora_tiles())
//     //     s += fmt::format("{}{}", Tile::Name.at(tile),
//     //                      &tile == &score.uradora_tiles().back() ? "\n" : ", ");

//     // return s;
// }

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

// /**
//  * @brief 文字列に変換する。
//  *
//  * @return std::string ブロックを表す文字列
//  */
// inline std::string Block::to_string() const
// {
//     std::vector<int> tiles;
//     if (type & BlockType::Triplet) {
//         for (int i = 0; i < 3; ++i)
//             tiles.push_back(min_tile);
//     }
//     else if (type & BlockType::Sequence) {
//         for (int i = 0; i < 3; ++i)
//             tiles.push_back(min_tile + i);
//     }
//     else if (type & BlockType::Kong) {
//         for (int i = 0; i < 4; ++i)
//             tiles.push_back(min_tile);
//     }
//     else if (type & BlockType::Pair) {
//         for (int i = 0; i < 2; ++i)
//             tiles.push_back(min_tile);
//     }

//     std::string s;

//     s += "[";
//     for (auto tile : tiles) {
//         if (is_manzu(tile))
//             s += std::to_string(tile + 1);
//         else if (is_pinzu(tile))
//             s += std::to_string(tile - 8);
//         else if (is_souzu(tile))
//             s += std::to_string(tile - 17);
//         else
//             s += Tile::Name.at(tile);
//     }

//     if (is_manzu(tiles[0]))
//         s += "m";
//     else if (is_pinzu(tiles[0]))
//         s += "p";
//     else if (is_souzu(tiles[0]))
//         s += "s";
//     s += fmt::format(", {}]", BlockType::Name.at(type));

//     return s;
// }

// /**
//  * @brief 文字列に変換する。
//  *
//  * @return std::string ブロックを表す文字列
//  */
// inline std::string MeldedBlock::to_string() const
// {
//     std::string s;

//     s += "[";
//     for (auto tile : tiles) {
//         if (is_reddora(tile))
//             s += "r5";
//         else if (is_manzu(tile))
//             s += std::to_string(tile + 1);
//         else if (is_pinzu(tile))
//             s += std::to_string(tile - 8);
//         else if (is_souzu(tile))
//             s += std::to_string(tile - 17);
//         else
//             s += Tile::Name.at(tile);
//     }

//     if (is_manzu(tiles[0]))
//         s += "m";
//     else if (is_pinzu(tiles[0]))
//         s += "p";
//     else if (is_souzu(tiles[0]))
//         s += "s";
//     s += fmt::format(", {}]", MeldType::Name.at(type));

//     return s;
// }

} // namespace mahjong

#endif /* MAHJONG_CPP_STRING */
