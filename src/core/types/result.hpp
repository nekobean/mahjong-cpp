#ifndef MAHJONG_CPP_RESULT
#define MAHJONG_CPP_RESULT

#include <map>
#include <string>
#include <vector>

#include <spdlog/spdlog.h>

#include "hand.hpp"
#include "scoretitle.hpp"
#include "tile.hpp"
#include "yaku.hpp"

namespace mahjong {

/**
 * @brief 結果
 */
struct Result {
    // 通常役
    Result(const Hand &tehai, int win_tile, bool tumo,
           const std::vector<std::tuple<YakuList, int>> &yaku_list,
           const std::vector<std::tuple<std::string, int>> &hu_list, int score_title,
           int han, int hu, const std::vector<Block> &blocks,
           const std::vector<int> &score)
        : success(true)
        , tehai(tehai)
        , win_tile(win_tile)
        , tumo(tumo)
        , yaku_list(yaku_list)
        , hu_list(hu_list)
        , score_title(score_title)
        , han(han)
        , hu(Hu::Values.at(hu))
        , blocks(blocks)
        , score(score)
    {
    }

    // 役満、流し満貫
    Result(const Hand &tehai, int win_tile, bool tumo,
           const std::vector<std::tuple<YakuList, int>> &yaku_list, int score_title,
           const std::vector<int> &score)
        : success(true)
        , tehai(tehai)
        , win_tile(win_tile)
        , tumo(tumo)
        , yaku_list(yaku_list)
        , score_title(score_title)
        , han(-1)
        , hu(-1)
        , blocks(blocks)
        , score(score)
    {
    }

    // エラー
    Result(const Hand &tehai, int win_tile, const std::string &err_msg)
        : success(false)
        , tehai(tehai)
        , win_tile(win_tile)
        , tumo(false)
        , err_msg(err_msg)
        , yaku_list(Yaku::Null)
        , score_title(ScoreTitle::Null)
        , han(-1)
        , hu(-1)
    {
    }

    /* 正常終了したかどうか */
    bool success;

    /* 手牌 */
    Hand tehai;

    /* 和了牌 */
    int win_tile;

    /* 自摸和了かどうか */
    bool tumo;

    /* 異常終了した場合のエラーメッセージ */
    std::string err_msg;

    /* 成立した役 */
    std::vector<std::tuple<YakuList, int>> yaku_list;
    std::vector<std::tuple<std::string, int>> hu_list;

    /* 点数の種類 */
    int score_title;

    /* 飜 */
    int han;

    /* 符 */
    int hu;

    /* 点数
     * 子自摸の場合: [和了者の獲得点数, 親の支払い点数, 子の支払い点数]
     * 親自摸の場合: [和了者の獲得点数, 獲得点数, 子の支払い点数]
     * ロンの場合: [和了者の獲得点数, 獲得点数, 放銃者の支払い点数]
     */
    std::vector<int> score;

    /* 面子構成 */
    std::vector<Block> blocks;

    std::string to_string()
    {
        std::string s;

        if (!success) {
            s += fmt::format("エラー: {}", err_msg);
            return s;
        }

        s += "[結果]\n";
        s += fmt::format("手牌: {}, 和了牌: {} {}\n", tehai.to_string(),
                         Tile::Names.at(win_tile), tumo ? "自摸" : "ロン");

        if (hu != -1) {
            if (!blocks.empty()) {
                s += "面子構成: ";
                for (const auto &block : blocks)
                    s += block.to_string() + (&block != &blocks.back() ? " " : "\n");
            }

            // 符
            for (const auto &[type, hu] : hu_list)
                s += fmt::format("* {} {}符\n", type, hu);

            // 通常役
            s += "役:\n";
            for (auto &[yaku, n] : yaku_list)
                s += fmt::format(" {} {}翻\n", Yaku::Info[yaku].name, n);

            s += fmt::format("{}符{}翻\n", hu, han);
            if (score_title != ScoreTitle::Null)
                s += ScoreTitle::Names[score_title] + "\n";
        }
        else {
            // 流し満貫、役満
            s += "役:\n";
            for (auto &[yaku, n] : yaku_list)
                s += fmt::format(" {}\n", Yaku::Info[yaku].name);
            s += ScoreTitle::Names[score_title] + "\n";
        }

        if (score.size() == 3)
            s += fmt::format(
                "和了者の獲得点数: {}点, 親の支払い点数: {}, 子の支払い点数: {}\n",
                score[0], score[1], score[2]);
        else
            s += fmt::format("和了者の獲得点数: {}点, 放銃者の支払い点数: {}\n",
                             score[0], score[1]);

        return s;
    }
};

} // namespace mahjong

#endif /* MAHJONG_CPP_RESULT */
