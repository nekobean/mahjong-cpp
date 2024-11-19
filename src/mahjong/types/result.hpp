#ifndef MAHJONG_CPP_RESULT
#define MAHJONG_CPP_RESULT

#include <vector>

#include "const.hpp"

namespace mahjong
{

/**
 * @brief 結果
 */
struct Result
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
    Result(const HandType &hand, int win_tile, int flag,
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
    Result(const HandType &hand, int win_tile, int flag,
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
    Result(const HandType &hand, int win_tile, int flag, const std::string &err_msg)
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
    HandType hand;

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
};

} // namespace mahjong

#endif /* MAHJONG_CPP_RESULT */
