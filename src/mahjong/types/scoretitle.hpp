#ifndef MAHJONG_CPP_SCORETITLE
#define MAHJONG_CPP_SCORETITLE

#include "scoringtable.hpp"

namespace mahjong
{

/**
 * @brief 符の種類
 */
namespace Hu
{

enum Type
{
    Null = -1,
    Hu20,  /* 20符 */
    Hu25,  /* 25符 */
    Hu30,  /* 30符 */
    Hu40,  /* 40符 */
    Hu50,  /* 50符 */
    Hu60,  /* 60符 */
    Hu70,  /* 70符 */
    Hu80,  /* 80符 */
    Hu90,  /* 90符 */
    Hu100, /* 100符 */
    Hu110, /* 110符 */
};

static inline std::map<int, int> Values = {
    {Null, -1}, {Hu20, 20}, {Hu25, 25}, {Hu30, 30}, {Hu40, 40},   {Hu50, 50},
    {Hu60, 60}, {Hu70, 70}, {Hu80, 80}, {Hu90, 90}, {Hu100, 100}, {Hu110, 110}};

static inline std::map<int, int> Keys = {
    {-1, Null}, {20, Hu20}, {25, Hu25}, {30, Hu30}, {40, Hu40},   {50, Hu50},
    {60, Hu60}, {70, Hu70}, {80, Hu80}, {90, Hu90}, {100, Hu100}, {110, Hu110}};

static inline std::map<int, std::string> Name = {
    {Null, "Null"}, {Hu20, "20符"}, {Hu25, "25符"},   {Hu30, "30符"},
    {Hu40, "40符"}, {Hu50, "50符"}, {Hu60, "60符"},   {Hu70, "70符"},
    {Hu80, "80符"}, {Hu90, "90符"}, {Hu100, "100符"}, {Hu110, "110符"}};

/**
 * @brief 符を切り上げる。
 *
 * @param[in] hu 符
 * @return int 切り上げた符
 */
static inline int round_up_fu(int hu)
{
    hu = int(std::ceil(hu / 10.)) * 10;

    switch (hu) {
    case 20:
        return Hu20;
    case 25:
        return Hu25;
    case 30:
        return Hu30;
    case 40:
        return Hu40;
    case 50:
        return Hu50;
    case 60:
        return Hu60;
    case 70:
        return Hu70;
    case 80:
        return Hu80;
    case 90:
        return Hu90;
    case 100:
        return Hu100;
    case 110:
        return Hu110;
    }

    return Null;
}

} // namespace Hu

namespace ScoreTitle
{

enum Type
{
    Null = -1,
    Mangan,       /* 満貫 */
    Haneman,      /* 跳満 */
    Baiman,       /* 倍満 */
    Sanbaiman,    /* 三倍満 */
    KazoeYakuman, /* 数え役満 */
    Yakuman,      /* 役満 */
    TwoYakuman,   /* ダブル役満 */
    ThreeYakuman, /* トリプル役満 */
    FourYakuman,  /* 四倍役満 */
    FiveYakuman,  /* 五倍役満 */
    SixYakuman,   /* 六倍役満 */
    Length,
};

static inline std::map<int, std::string> Name = {{Null, "Null"},
                                                 {Mangan, "満貫"},
                                                 {Haneman, "跳満"},
                                                 {Baiman, "倍満"},
                                                 {Sanbaiman, "三倍満"},
                                                 {KazoeYakuman, "数え役満"},
                                                 {Yakuman, "役満"},
                                                 {TwoYakuman, "ダブル役満"},
                                                 {ThreeYakuman, "トリプル役満"},
                                                 {FourYakuman, "4倍役満"},
                                                 {FiveYakuman, "5倍役満"},
                                                 {SixYakuman, "6倍役満"}};

/**
 * @brief 役満でない点数のタイトルを取得する。
 *
 * @param[in] hu 符
 * @param[in] han 飜
 * @return int タイトル
 */
static int get_score_title(int fu, int han)
{
    if (han < 5)
        return ScoringTable::IsMangan[fu][han - 1] ? Mangan : Null;
    if (han == 5)
        return Mangan;
    else if (han <= 7)
        return Haneman;
    else if (han <= 10)
        return Baiman;
    else if (han <= 12)
        return Sanbaiman;
    else
        return KazoeYakuman;
};

/**
 * @brief 役満のタイトルを取得する。
 *
 * @param[in] n 何倍役満かを指定する
 * @return int タイトル
 */
static int get_score_title(int n)
{
    if (n == 1)
        return Yakuman;
    else if (n == 2)
        return TwoYakuman;
    else if (n == 3)
        return ThreeYakuman;
    else if (n == 4)
        return FourYakuman;
    else if (n == 5)
        return FiveYakuman;
    else if (n == 6)
        return SixYakuman;

    return Null;
};

} // namespace ScoreTitle

} // namespace mahjong

#endif /* MAHJONG_CPP_SCORETITLE */
