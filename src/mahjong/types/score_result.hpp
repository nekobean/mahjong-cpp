#ifndef MAHJONG_CPP_SCORE_RESULT_HPP
#define MAHJONG_CPP_SCORE_RESULT_HPP

#include <string>
#include <vector>

#include "mahjong/types/block.hpp"
#include "mahjong/types/constants.hpp"

namespace mahjong
{

/**
 * @brief Yaku with han value.
 */
struct YakuEntry
{
    /*! Yaku flag. */
    YakuFlags yaku = Yaku::None;

    /*! Han value. */
    int han = 0;
};

/**
 * @brief Score calculation result.
 */
struct ScoreResult
{
    /*! Whether the calculation succeeded. */
    bool success = false;

    /*! Error message when calculation failed. */
    std::string err_msg;

    /*! Yaku entries. */
    std::vector<YakuEntry> yaku_list;

    /*! Total han. */
    int han = 0;

    /*! Total fu. */
    int fu = 0;

    /*! Score limit. */
    int score_limit = ScoreLimit::Null;

    /*! Winner gain followed by payer payments. */
    std::vector<int> payments;

    /*! Hand blocks used for fu and yaku calculation. */
    std::vector<Block> blocks;

    /*! Wait type. */
    int wait_type = WaitType::Null;
};

} // namespace mahjong

#endif // MAHJONG_CPP_SCORE_RESULT_HPP
