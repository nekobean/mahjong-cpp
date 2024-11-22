#include "string.hpp"

#include <spdlog/spdlog.h>

#include <mahjong/core/utils.hpp>

namespace mahjong
{

/**
 * @brief Create a string in MPSZ notation from a hand.
 *
 * @param hand hand
 * @return string in MPSZ notation
 */
std::string to_mpsz(const Hand &hand)
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

std::string to_mpsz(const std::vector<int> &tiles)
{
    Hand hand{0};
    for (auto tile : tiles) {
        ++hand[tile];
        if (tile == Tile::RedManzu5) {
            ++hand[Tile::Manzu5];
        }
        else if (tile == Tile::RedPinzu5) {
            ++hand[Tile::Pinzu5];
        }
        else if (tile == Tile::RedSouzu5) {
            ++hand[Tile::Souzu5];
        }
    }

    return to_mpsz(hand);
}

/**
 * @brief Create a hand from a string in MPSZ notation.
 *
 * @param[in] tiles string in MPSZ notation
 * @return hand
 */
Hand from_mpsz(const std::string &tiles)
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

    check_hand(hand);

    return hand;
}

std::string to_string(const Block &block)
{
    Hand tiles{0};
    if (block.type & BlockType::Triplet) {
        tiles[block.min_tile] = 3;
    }
    else if (block.type & BlockType::Sequence) {
        tiles[block.min_tile] = 1;
        tiles[block.min_tile + 1] = 1;
        tiles[block.min_tile + 2] = 1;
    }
    else if (block.type & BlockType::Kong) {
        tiles[block.min_tile] = 4;
    }
    else if (block.type & BlockType::Pair) {
        tiles[block.min_tile] = 2;
    }

    return fmt::format("[{} {}]", to_mpsz(tiles), BlockType::Name.at(block.type));
}

std::string to_string(const Meld &meld)
{
    return fmt::format("[{} {}]", to_mpsz(meld.tiles), MeldType::Name.at(meld.type));
}

std::string to_string(const Round &round)
{
    std::string s;

    s += u8"[ルール]\n";
    for (auto rule : {RuleFlag::RedDora, RuleFlag::OpenTanyao}) {
        s += fmt::format(u8"  {}: {}\n", RuleFlag::Name.at(rule),
                         (round.rules & rule) ? u8"有り" : u8"無し");
    }

    std::string round_wind;
    if (round.wind == Tile::East) {
        round_wind = u8"東";
    }
    else if (round.wind == Tile::South) {
        round_wind = u8"南";
    }
    else if (round.wind == Tile::West) {
        round_wind = u8"西";
    }
    else if (round.wind == Tile::North) {
        round_wind = u8"北";
    }
    s += u8"[場]\n";
    s += fmt::format(u8"{}{}局{}本場\n", round_wind, round.kyoku, round.honba);
    s += fmt::format(u8"供託棒: {}本\n", round.kyotaku);
    s += fmt::format(u8"ドラ表示牌: {}\n", to_mpsz(round.dora_indicators));

    return s;
}

std::string to_string(const Player &player)
{
    std::string s;

    s += fmt::format(u8"手牌: {}\n", to_mpsz(player.hand));
    s += u8"副露牌: ";
    for (const auto &meld : player.melds) {
        s += to_string(meld);
    }
    s += u8"\n";
    s += fmt::format(u8"自風: {}\n", Tile::Name.at(player.wind));

    return s;
}

std::string to_string(const Result &result)
{
    std::string s;

    if (!result.success) {
        s += fmt::format(u8"エラー: {}\n", result.err_msg);
        return s;
    }

    s += u8"[入力]\n";
    s += to_string(result.player);
    s += ((result.win_flag & WinFlag::Tsumo) ? u8"自摸\n" : u8"ロン\n");
    s += u8"[結果]\n";

    if (result.han > 0) {
        s += u8"面子構成: ";
        for (const auto &block : result.blocks) {
            s += to_string(block);
        }
        s += u8"\n";
        s += fmt::format(u8"待ち: {}\n", WaitType::Name.at(result.wait_type));

        // 役
        s += u8"役:\n";
        for (auto &[yaku, han] : result.yaku_list) {
            s += fmt::format(u8" {} {}翻\n", Yaku::Name[yaku], han);
        }

        // 飜、符
        s += fmt::format("{}符{}翻 {}\n", result.fu, result.han,
                         result.score_title != ScoreTitle::Null
                             ? ScoreTitle::Name.at(result.score_title)
                             : "");
    }
    else {
        // 流し満貫、役満
        s += "役:\n";
        for (auto &[yaku, _] : result.yaku_list)
            s += fmt::format(" {}\n", Yaku::Name[yaku]);
        s += ScoreTitle::Name[result.score_title] + "\n";
    }

    if (result.score.size() == 3) {
        s += fmt::format(
            u8"和了者の獲得点数: {}点, 親の支払い点数: {}点, 子の支払い点数: {}点\n",
            result.score[0], result.score[1], result.score[2]);
    }
    else if ((result.win_flag & WinFlag::Tsumo) && result.score.size() == 2) {
        s += fmt::format(u8"和了者の獲得点数: {}点, 子の支払い点数: {}点\n",
                         result.score[0], result.score[1]);
    }
    else {
        s += fmt::format(u8"和了者の獲得点数: {}点, 放銃者の支払い点数: {}点\n",
                         result.score[0], result.score[1]);
    }

    return s;
}

} // namespace mahjong
