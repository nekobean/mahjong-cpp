#ifndef MAHJONG_CPP_TYPES

#include <map>
#include <string>
#include <vector>

#include <spdlog/spdlog.h>

#include "bitutils.hpp"
#include "block.hpp"
#include "hand.hpp"
#include "meld.hpp"
#include "result.hpp"
#include "scoreboard.hpp"
#include "tile.hpp"
#include "yaku.hpp"

namespace mahjong {

/**
 * @brief 結果
 */
struct Result {
    // 通常役
    Result(const Hand &tehai, int agarihai, bool tumo,
           const std::vector<std::tuple<YakuList, int>> &yaku_list,
           const std::vector<std::tuple<std::string, int>> &hu_list, int score_type,
           int han, int hu, const std::vector<Block> &blocks, int ko2oya_ron,
           int ko2oya_tumo, int ko2ko_tumo, int oya2ko_ron, int oya2ko_tumo)
        : success(true)
        , tehai(tehai)
        , agarihai(agarihai)
        , tumo(tumo)
        , yaku_list(yaku_list)
        , hu_list(hu_list)
        , score_type(score_type)
        , han(han)
        , hu(hu)
        , blocks(blocks)
        , ko2oya_ron(ko2oya_ron)
        , ko2oya_tumo(ko2oya_tumo)
        , ko2ko_tumo(ko2ko_tumo)
        , oya2ko_ron(oya2ko_ron)
        , oya2ko_tumo(oya2ko_tumo)
    {
    }

    // 役満、流し満貫
    Result(const Hand &tehai, int agarihai, bool tumo,
           const std::vector<std::tuple<YakuList, int>> &yaku_list, int score_type,
           int ko2oya_ron, int ko2oya_tumo, int ko2ko_tumo, int oya2ko_ron,
           int oya2ko_tumo)
        : success(true)
        , tehai(tehai)
        , agarihai(agarihai)
        , tumo(tumo)
        , yaku_list(yaku_list)
        , score_type(score_type)
        , han(-1)
        , hu(-1)
        , blocks(blocks)
        , ko2oya_ron(ko2oya_ron)
        , ko2oya_tumo(ko2oya_tumo)
        , ko2ko_tumo(ko2ko_tumo)
        , oya2ko_ron(oya2ko_ron)
        , oya2ko_tumo(oya2ko_tumo)
    {
    }

    // エラー
    Result(const Hand &tehai, int agarihai, const std::string &err_msg)
        : success(false)
        , tehai(tehai)
        , agarihai(agarihai)
        , tumo(false)
        , err_msg(err_msg)
        , yaku_list(Yaku::Null)
        , score_type(Score::Null)
        , han(-1)
        , hu(-1)
    {
    }

    /* 正常終了したかどうか */
    bool success;

    /* 手牌 */
    Hand tehai;

    /* 和了り牌 */
    int agarihai;

    /* 自摸和了りかどうか */
    bool tumo;

    /* 異常終了した場合のエラーメッセージ */
    std::string err_msg;

    /* 成立した役 */
    std::vector<std::tuple<YakuList, int>> yaku_list;
    std::vector<std::tuple<std::string, int>> hu_list;

    /* 点数の種類 */
    int score_type;

    /* 飜 */
    int han;

    /* 符 */
    int hu;

    int ko2oya_ron;
    int ko2oya_tumo;
    int ko2ko_tumo;
    int oya2ko_ron;
    int oya2ko_tumo;

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
        s += fmt::format("手牌: {}, 和了り牌: {} {}\n", tehai.to_string(),
                         Tile::Names.at(agarihai), tumo ? "ツモ" : "ロン");

        if (hu != -1) {
            if (!blocks.empty()) {
                s += "面子構成: ";
                for (const auto &block : blocks)
                    s += block.to_string() + (&block != &blocks.back() ? " " : "\n");
            }

            // 符
            for (const auto &[type, hu] : hu_list)
                s += fmt::format("{} {}符\n", type, hu);

            // 通常役
            s += "役:\n";
            for (auto &[yaku, n] : yaku_list)
                s += fmt::format(" {} {}翻\n", Yaku::Info[yaku].name, n);

            s += fmt::format("{}符{}翻\n", Hu::Names[hu], han);
            if (score_type != Score::Null)
                s += Score::Names[score_type] + "\n";
        }
        else {
            // 流し満貫、役満
            s += "役:\n";
            for (auto &[yaku, n] : yaku_list)
                s += fmt::format(" {}\n", Yaku::Info[yaku].name);
            s += Score::Names[score_type] + "\n";
        }

        s += fmt::format("親のロン: {}点\n", ko2oya_ron);
        s += fmt::format("子のロン: {}点\n", oya2ko_ron);
        s += fmt::format("親のツモ: {}点オール\n", ko2oya_tumo);
        s += fmt::format("子のツモ: 親 {}点 / 子 {}点\n", oya2ko_tumo, ko2ko_tumo);

        return s;
    }
};

} // namespace mahjong

#define MAHJONG_CPP_TYPES
#endif /* MAHJONG_CPP_TYPES */
