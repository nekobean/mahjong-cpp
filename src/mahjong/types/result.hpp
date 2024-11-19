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
     * @param[in] win_flag フラグ
     * @param[in] yaku_list (成立した役, 飜) の一覧
     * @param[in] han 翻
     * @param[in] hu 符
     * @param[in] score_title 点数のタイトル
     * @param[in] score 点数
     * @param[in] blocks 面子構成
     * @param[in] wait_type 待ちの種類
     */
    Result(const MyPlayer &player, int win_tile, int win_flag,
           const std::vector<std::tuple<YakuList, int>> &yaku_list, int han, int hu,
           int score_title, const std::vector<int> &score,
           const std::vector<Block> &blocks, int wait_type)
        : success(true)
        , player(player)
        , win_tile(win_tile)
        , win_flag(win_flag)
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
     * @param[in] win_flag フラグ
     * @param[in] yaku_list (成立した役, 飜) の一覧
     * @param[in] score_title 点数のタイトル
     * @param[in] score 点数
     */
    Result(const MyPlayer &player, int win_tile, int win_flag,
           const std::vector<std::tuple<YakuList, int>> &yaku_list, int score_title,
           const std::vector<int> &score)
        : success(true)
        , player(player)
        , win_tile(win_tile)
        , win_flag(win_flag)
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
     * @param[in] player 手牌
     * @param[in] win_tile 和了牌
     * @param[in] win_flag フラグ
     * @param[in] err_msg エラーメッセージ
     */
    Result(const MyPlayer &player, int win_tile, int win_flag,
           const std::string &err_msg)
        : success(false)
        , err_msg(err_msg)
        , player(player)
        , win_tile(win_tile)
        , win_flag(win_flag)
        , yaku_list(Yaku::Null)
        , han(0)
        , fu(Hu::Null)
        , score_title(ScoreTitle::Null)
        , wait_type(WaitType::Null)
    {
    }

    /* Whether the operation was successful */
    bool success;

    /* Error message */
    std::string err_msg;

    // Input
    ////////////////////////

    /* Player infomation */
    MyPlayer player;

    /* Win tile */
    int win_tile;

    /* Win flag */
    int win_flag;

    // Output

    /* list of (yaku, han) */
    std::vector<std::tuple<YakuList, int>> yaku_list;

    /* han */
    int han;

    /* fu */
    int fu;

    /* score title */
    int score_title;

    /* score */
    std::vector<int> score;

    /* block */
    std::vector<Block> blocks;

    /* wait type */
    int wait_type;
};

} // namespace mahjong

#endif /* MAHJONG_CPP_RESULT */
