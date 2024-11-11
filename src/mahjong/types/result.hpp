#ifndef MAHJONG_CPP_RESULT
#define MAHJONG_CPP_RESULT

#include <map>
#include <string>
#include <vector>

#include <spdlog/spdlog.h>

#include "hand2.hpp"
#include "scoretitle.hpp"
#include "tile.hpp"
#include "yaku.hpp"

namespace mahjong
{

/**
 * @brief 手牌に関するフラグ
 *
 *    自摸和了の場合は門前かどうかに関わらず、Tumo を指定します。
 *    立直、ダブル立直はどれか1つのみ指定できます。
 *    搶槓、嶺上開花、海底撈月、河底撈魚はどれか1つのみ指定できます。
 *    天和、地和、人和はどれか1つのみ指定できます。
 */
namespace HandFlag
{
enum
{
    Null = 0,
    Tumo = 1 << 1,         /* 自摸和了 */
    Reach = 1 << 2,        /* 立直成立 */
    Ippatu = 1 << 3,       /* 一発成立 */
    Tyankan = 1 << 4,      /* 搶槓成立 */
    Rinsyankaiho = 1 << 5, /* 嶺上開花成立 */
    Haiteitumo = 1 << 6,   /* 海底撈月成立 */
    Hoteiron = 1 << 7,     /* 河底撈魚成立 */
    DoubleReach = 1 << 8,  /* ダブル立直成立 */
    NagasiMangan = 1 << 9, /* 流し満貫成立 */
    Tenho = 1 << 10,       /* 天和成立 */
    Tiho = 1 << 11,        /* 地和成立 */
    Renho = 1 << 12,       /* 人和成立 */
};

static inline const std::map<int, std::string> Name = {{Null, "Null"},
                                                       {Tumo, "自摸和了"},
                                                       {Reach, "立直成立"},
                                                       {Ippatu, "一発成立"},
                                                       {Tyankan, "搶槓成立"},
                                                       {Rinsyankaiho, "嶺上開花成立"},
                                                       {Haiteitumo, "海底撈月成立"},
                                                       {Hoteiron, "河底撈魚成立"},
                                                       {DoubleReach, "ダブル立直成立"},
                                                       {NagasiMangan, "流し満貫成立"},
                                                       {Tenho, "天和成立"},
                                                       {Tiho, "地和成立"},
                                                       {Renho, "人和成立"}};
} // namespace HandFlag

/**
 * @brief 結果
 */
struct Result2
{
    /**
     * @brief 通常役の結果
     *
     * @param[in] hand 手牌
     * @param[in] win_tile 和了牌
     * @param[in] flag フラグ
     * @param[in] yaku_list (成立した役, 飜) の一覧
     * @param[in] han 翻
     * @param[in] hu 符
     * @param[in] score_title 点数のタイトル
     * @param[in] score 点数
     * @param[in] blocks 面子構成
     * @param[in] wait_type 待ちの種類
     */
    Result2(const Hand2 &hand, int win_tile, int flag,
            const std::vector<std::tuple<YakuList, int>> &yaku_list, int han, int hu,
            int score_title, const std::vector<int> &score,
            const std::vector<Block> &blocks, int wait_type)
        : success(true)
        , hand(hand)
        , win_tile(win_tile)
        , flag(flag)
        , yaku_list(yaku_list)
        , han(han)
        , fu(Hu::Values.at(hu))
        , score_title(score_title)
        , score(score)
        , blocks(blocks)
        , wait_type(wait_type)
    {
    }

    /**
     * @brief 役満、流し満貫の結果
     *
     * @param[in] hand 手牌
     * @param[in] win_tile 和了牌
     * @param[in] flag フラグ
     * @param[in] yaku_list (成立した役, 飜) の一覧
     * @param[in] score_title 点数のタイトル
     * @param[in] score 点数
     */
    Result2(const Hand2 &hand, int win_tile, int flag,
            const std::vector<std::tuple<YakuList, int>> &yaku_list, int score_title,
            const std::vector<int> &score)
        : success(true)
        , hand(hand)
        , win_tile(win_tile)
        , flag(flag)
        , yaku_list(yaku_list)
        , han(0)
        , fu(Hu::Null)
        , score_title(score_title)
        , score(score)
        , wait_type(WaitType::Null)
    {
    }

    /**
     * @brief エラーの結果
     *
     * @param[in] hand 手牌
     * @param[in] win_tile 和了牌
     * @param[in] flag フラグ
     * @param[in] err_msg エラーメッセージ
     */
    Result2(const Hand2 &hand, int win_tile, int flag, const std::string &err_msg)
        : success(false)
        , err_msg(err_msg)
        , hand(hand)
        , win_tile(win_tile)
        , flag(flag)
        , yaku_list(Yaku::Null)
        , han(0)
        , fu(Hu::Null)
        , score_title(ScoreTitle::Null)
        , wait_type(WaitType::Null)
    {
    }

    /* 正常終了したかどうか */
    bool success;

    /* 異常終了した場合のエラーメッセージ */
    std::string err_msg;

    /**
     * 入力情報
     */

    /* 手牌 */
    Hand2 hand;

    /* 和了牌 */
    int win_tile;

    /* フラグ */
    int flag;

    /**
     * 結果情報
     */

    /* (成立した役, 飜) の一覧 */
    std::vector<std::tuple<YakuList, int>> yaku_list;

    /* 飜 */
    int han;

    /* 符 */
    int fu;

    /* 点数の種類 */
    int score_title;

    /* 点数 */
    std::vector<int> score;

    /* 面子構成 */
    std::vector<Block> blocks;

    /* 待ちの種類 */
    int wait_type;

    std::string to_string();
};

inline std::string Result2::to_string()
{
    std::string s;

    if (!success) {
        s += fmt::format("エラー: {}", err_msg);
        return s;
    }

    s += "[結果]\n";
    s +=
        fmt::format("手牌: {}, 和了牌: {}, {}\n", hand.to_string(),
                    Tile::Name.at(win_tile), (flag & HandFlag::Tumo) ? "自摸" : "ロン");

    if (han > 0) {
        if (!blocks.empty()) {
            s += "面子構成:\n";
            for (const auto &block : blocks)
                s += fmt::format("  {}\n", block.to_string());
        }
        s += fmt::format("待ち: {}\n", WaitType::Name.at(wait_type));

        // 役
        s += "役:\n";
        for (auto &[yaku, n] : yaku_list)
            s += fmt::format(" {} {}翻\n", Yaku::Info[yaku].name, n);

        // 飜、符
        s += fmt::format("{}符{}翻{}\n", fu, han,
                         score_title != ScoreTitle::Null
                             ? " " + ScoreTitle::Name.at(score_title)
                             : "");
    }
    else {
        // 流し満貫、役満
        s += "役:\n";
        for (auto &[yaku, n] : yaku_list)
            s += fmt::format(" {}\n", Yaku::Info[yaku].name);
        s += ScoreTitle::Name[score_title] + "\n";
    }

    if (score.size() == 3)
        s += fmt::format(
            "和了者の獲得点数: {}点, 親の支払い点数: {}, 子の支払い点数: {}\n",
            score[0], score[1], score[2]);
    else
        s += fmt::format("和了者の獲得点数: {}点, 放銃者の支払い点数: {}\n", score[0],
                         score[1]);

    return s;
}

} // namespace mahjong

#endif /* MAHJONG_CPP_RESULT */
