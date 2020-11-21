#ifndef MAHJONG_CPP_RESUT
#define MAHJONG_CPP_RESUT

namespace mahjong {

struct Hu {
    /**
     * @brief 符の種類
     */
    enum Type {
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

    /**
     * @brief 名前
     */
    static inline const std::vector<std::string> Names = {
        "20符", "25符", "30符", "40符",  "50符",  "60符",
        "70符", "80符", "90符", "100符", "110符",
    };

    static inline int round_up_hu(int hu)
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
};

struct Score {
    enum Type {
        Null = -1,
        Mangan,       /* 満貫 (まんがん) */
        Haneman,      /* 跳満 (はねまん) */
        Baiman,       /* 倍満 (ばいまん)) */
        Sanbaiman,    /* 三倍満 (さんばいまん) */
        KazoeYakuman, /* 数え役満 (かぞえやくまん) */
        Yakuman,      /* 役満 (やくまん) */
        TwoYakuman,   /* ダブル役満 (ダブル役満) */
        ThreeYakuman, /* トリプル役満 (トリプル役満) */
        FourYakuman,  /* 四倍役満 (はねまん) */
        FiveYakuman,  /* 五倍役満 (はねまん) */
        SixYakuman,   /* 六倍役満 (はねまん) */
        Length,
    };

    /**
     * @brief 名前
     */
    static inline const std::vector<std::string> Names = {
        "満貫", "跳満",       "倍満",         "三倍満",  "数え役満", // 通常役
        "役満", "ダブル役満", "トリプル役満", "4倍役満", "5倍役満",  "6倍役満", // 役満
    };

    // clang-format off
    static inline std::vector<std::vector<int>> is_mangan = {
    //   1翻    2翻    3翻    4翻
        {false, false, false, false}, // 20符
        {false, false, false, false}, // 25符
        {false, false, false, false}, // 30符
        {false, false, false, true},  // 40符
        {false, false, false, true},  // 50符
        {false, false, false, true},  // 60符
        {false, false, true,  true},  // 70符
        {false, false, true,  true},  // 80符
        {false, false, true,  true},  // 90符
        {false, false, true,  true},  // 100符
        {false, false, true,  true},  // 110符
    };
    // clang-format on

    static int get_normal_score_type(int han, int hu)
    {
        if (han < 5)
            return is_mangan[hu][han] ? Mangan : Null;
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
    }

    static int get_yakuman_score_type(int n)
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
    }
};
} // namespace mahjong

#endif /* MAHJONG_CPP_RESUT */
