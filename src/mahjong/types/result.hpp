#ifndef MAHJONG_CPP_RESULT
#define MAHJONG_CPP_RESULT

#include <vector>

#include "const.hpp"

namespace mahjong
{

/**
 * @brief Score calculation result
 */
struct Result
{
    /**
     * @brief Result for normal yaku
     *
     * @param[in] player player
     * @param[in] win_tile win tile
     * @param[in] win_flag win flag
     * @param[in] yaku_list list of (yaku, han)
     * @param[in] han han
     * @param[in] fu fu
     * @param[in] score_title score tile
     * @param[in] score score
     * @param[in] blocks list of blocks
     * @param[in] wait_type wait type
     */
    Result(const Player &player, int win_tile, int win_flag,
           const std::vector<std::tuple<YakuList, int>> &yaku_list, int han, int fu,
           int score_title, const std::vector<int> &score,
           const std::vector<Block> &blocks, int wait_type)
        : success(true)
        , player(player)
        , win_tile(win_tile)
        , win_flag(win_flag)
        , yaku_list(yaku_list)
        , han(han)
        , fu(fu)
        , score_title(score_title)
        , score(score)
        , blocks(blocks)
        , wait_type(wait_type)
    {
    }

    /**
     * @brief Result for Nagashi Mangan or Yakuman
     *
     * @param[in] player player
     * @param[in] win_tile win tile
     * @param[in] win_flag win flag
     * @param[in] yaku_list list of (yaku, han)
     * @param[in] score_title score tile
     * @param[in] score score
     */
    Result(const Player &player, int win_tile, int win_flag,
           const std::vector<std::tuple<YakuList, int>> &yaku_list, int score_title,
           const std::vector<int> &score)
        : success(true)
        , player(player)
        , win_tile(win_tile)
        , win_flag(win_flag)
        , yaku_list(yaku_list)
        , han(0)
        , fu(0)
        , score_title(score_title)
        , score(score)
        , wait_type(WaitType::Null)
    {
    }

    /**
     * @brief Result for error
     *
     * @param[in] player player
     * @param[in] win_tile win tile
     * @param[in] win_flag win flag
     * @param[in] err_msg error message
     */
    Result(const Player &player, int win_tile, int win_flag, const std::string &err_msg)
        : success(false)
        , err_msg(err_msg)
        , player(player)
        , win_tile(win_tile)
        , win_flag(win_flag)
        , yaku_list(Yaku::Null)
        , han(0)
        , fu(0)
        , score_title(ScoreTitle::Null)
        , wait_type(WaitType::Null)
    {
    }

    // Error information
    ////////////////////////

    /* Whether the calculation is successful */
    bool success;

    /* Error message */
    std::string err_msg;

    // Input information
    ////////////////////////

    /* Player infomation */
    Player player;

    /* Win tile */
    int win_tile;

    /* Win flag */
    int win_flag;

    // Output information
    ////////////////////////

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
