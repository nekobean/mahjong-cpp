#ifndef MAHJONG_CPP_YAKU
#define MAHJONG_CPP_YAKU

#include <array>
#include <map>
#include <string>

namespace mahjong {

using YakuList = unsigned long long;

/**
 * @brief 役
 */
struct Yaku {
    /**
     * @brief 役の種類
     */
    enum Type : YakuList {
        Null           = 0ull,
        Tumo           = 1ull,       /* 門前清自摸和 */
        Reach          = 1ull << 1,  /* 立直 */
        Ippatu         = 1ull << 2,  /* 一発 */
        Tanyao         = 1ull << 3,  /* 断幺九 */
        Pinhu          = 1ull << 4,  /* 平和 */
        Ipeko          = 1ull << 5,  /* 一盃口 */
        Tyankan        = 1ull << 6,  /* 槍槓 */
        Rinsyankaiho   = 1ull << 7,  /* 嶺上開花 */
        Haiteitumo     = 1ull << 8,  /* 海底摸月 */
        Hoteiron       = 1ull << 9,  /* 河底撈魚 */
        Dora           = 1ull << 10, /* ドラ */
        UraDora        = 1ull << 11, /* 裏ドラ */
        AkaDora        = 1ull << 12, /* 赤ドラ */
        SangenhaiHaku  = 1ull << 13, /* 三元牌 (白) */
        SangenhaiHatu  = 1ull << 14, /* 三元牌 (發) */
        SangenhaiTyun  = 1ull << 15, /* 三元牌 (中) */
        ZikazeTon      = 1ull << 16, /* 自風 (東) */
        ZikazeNan      = 1ull << 17, /* 自風 (南) */
        ZikazeSya      = 1ull << 18, /* 自風 (西) */
        ZikazePe       = 1ull << 19, /* 自風 (北) */
        BakazeTon      = 1ull << 20, /* 場風 (東) */
        BakazeNan      = 1ull << 21, /* 場風 (南) */
        BakazeSya      = 1ull << 22, /* 場風 (西) */
        BakazePe       = 1ull << 23, /* 場風 (北) */
        DoubleReach    = 1ull << 24, /* ダブル立直 */
        Tiitoitu       = 1ull << 25, /* 七対子 */
        Toitoiho       = 1ull << 26, /* 対々和 */
        Sananko        = 1ull << 27, /* 三暗刻 */
        SansyokuDoko   = 1ull << 28, /* 三色同刻 */
        SansyokuDozyun = 1ull << 29, /* 三色同順 */
        Honroto        = 1ull << 30, /* 混老頭 */
        IkkiTukan      = 1ull << 31, /* 一気通貫 */
        Tyanta         = 1ull << 32, /* 混全帯幺九 */
        Syosangen      = 1ull << 33, /* 小三元 */
        Sankantu       = 1ull << 34, /* 三槓子 */
        Honiso         = 1ull << 35, /* 混一色 */
        Zyuntyanta     = 1ull << 36, /* 純全帯幺九 */
        Ryanpeko       = 1ull << 37, /* 二盃口 */
        NagasiMangan   = 1ull << 38, /* 流し満貫 */
        Tiniso         = 1ull << 39, /* 清一色 */
        Tenho          = 1ull << 40, /* 天和 */
        Tiho           = 1ull << 41, /* 地和 */
        Renho          = 1ull << 42, /* 人和 */
        Ryuiso         = 1ull << 43, /* 緑一色 */
        Daisangen      = 1ull << 44, /* 大三元 */
        Syosusi        = 1ull << 45, /* 小四喜 */
        Tuiso          = 1ull << 46, /* 字一色 */
        Kokusimuso     = 1ull << 47, /* 国士無双 */
        Tyurenpoto     = 1ull << 48, /* 九連宝燈 */
        Suanko         = 1ull << 49, /* 四暗刻 */
        Tinroto        = 1ull << 50, /* 清老頭 */
        Sukantu        = 1ull << 51, /* 四槓子 */
        SuankoTanki    = 1ull << 52, /* 四暗刻単騎 */
        Daisusi        = 1ull << 53, /* 大四喜 */
        Tyurenpoto9    = 1ull << 54, /* 純正九連宝燈 */
        Kokusimuso13   = 1ull << 55, /* 国士無双13面待ち */
        Length         = 56ull,
    };

    struct YakuInfo {
        /* 名前 */
        std::string name;
        /* 通常役: (門前の飜数, 非門前の飜数)、役満: (何倍役満か, 未使用) */
        std::array<int, 2> han;
    };

    static inline std::vector<YakuList> NormalYaku = {
        Tumo,          Reach,          Ippatu,        Tanyao,     Pinhu,
        Ipeko,         Tyankan,        Rinsyankaiho,  Haiteitumo, Hoteiron,
        SangenhaiHaku, SangenhaiHatu,  SangenhaiTyun, ZikazeTon,  ZikazeNan,
        ZikazeSya,     ZikazePe,       BakazeTon,     BakazeNan,  BakazeSya,
        BakazePe,      DoubleReach,    Tiitoitu,      Toitoiho,   Sananko,
        SansyokuDoko,  SansyokuDozyun, Honroto,       IkkiTukan,  Tyanta,
        Syosangen,     Sankantu,       Honiso,        Zyuntyanta, Ryanpeko,
        Tiniso,
    };

    static inline std::vector<YakuList> Yakuman = {
        Tenho,       Tiho,       Renho,       Ryuiso,       Daisangen, Syosusi,
        Tuiso,       Kokusimuso, Tyurenpoto,  Suanko,       Tinroto,   Sukantu,
        SuankoTanki, Daisusi,    Tyurenpoto9, Kokusimuso13,
    };

    /**
     * @brief 役の情報
     */
    // clang-format off
    static inline std::map<YakuList, YakuInfo> Info = {
        {Null,           {"役なし",           {0, 0}}},
        {Tumo,           {"門前清自摸和",     {1, 0}}},  // 門前限定
        {Reach,          {"立直",             {1, 0}}},  // 門前限定
        {Ippatu,         {"一発",             {1, 0}}},  // 門前限定
        {Tanyao,         {"断幺九",           {1, 1}}},
        {Pinhu,          {"平和",             {1, 0}}},  // 門前限定
        {Ipeko,          {"一盃口",           {1, 0}}},  // 門前限定
        {Tyankan,        {"槍槓",             {1, 1}}},
        {Rinsyankaiho,   {"嶺上開花",         {1, 1}}},
        {Haiteitumo,     {"海底摸月",         {1, 1}}},
        {Hoteiron,       {"河底撈魚",         {1, 1}}},
        {Dora,           {"ドラ",             {0, 0}}},
        {UraDora,        {"裏ドラ",           {0, 0}}},
        {AkaDora,        {"赤ドラ",           {0, 0}}},
        {SangenhaiHaku,  {"三元牌 (白)",      {1, 1}}},
        {SangenhaiHatu,  {"三元牌 (發)",      {1, 1}}},
        {SangenhaiTyun,  {"三元牌 (中)",      {1, 1}}},
        {ZikazeTon,      {"自風 (東)",        {1, 1}}},
        {ZikazeNan,      {"自風 (南)",        {1, 1}}},
        {ZikazeSya,      {"自風 (西)",        {1, 1}}},
        {ZikazePe,       {"自風 (北)",        {1, 1}}},
        {BakazeTon,      {"場風 (東)",        {1, 1}}},
        {BakazeNan,      {"場風 (南)",        {1, 1}}},
        {BakazeSya,      {"場風 (西)",        {1, 1}}},
        {BakazePe,       {"場風 (北)",        {1, 1}}},
        {DoubleReach,    {"ダブル立直",       {2, 0}}},  // 門前限定
        {Tiitoitu,       {"七対子",           {2, 0}}},  // 門前限定
        {Toitoiho,       {"対々和",           {2, 2}}},
        {Sananko,        {"三暗刻",           {2, 2}}},
        {SansyokuDoko,   {"三色同刻",         {2, 2}}},
        {SansyokuDozyun, {"三色同順",         {2, 1}}},  // 喰い下がり
        {Honroto,        {"混老頭",           {2, 2}}},
        {IkkiTukan,      {"一気通貫",         {2, 1}}},  // 喰い下がり
        {Tyanta,         {"混全帯幺九",       {2, 1}}},  // 喰い下がり
        {Syosangen,      {"小三元",           {2, 2}}},
        {Sankantu,       {"三槓子",           {2, 2}}},
        {Honiso,         {"混一色",           {3, 2}}},  // 喰い下がり
        {Zyuntyanta,     {"純全帯幺九",       {3, 2}}},  // 喰い下がり
        {Ryanpeko,       {"二盃口",           {3, 0}}},  // 門前限定
        {NagasiMangan,   {"流し満貫",         {0, 0}}},  // 満貫扱い
        {Tiniso,         {"清一色",           {6, 5}}},  // 喰い下がり
        {Tenho,          {"天和",             {1, 0}}},  // 役満
        {Tiho,           {"地和",             {1, 0}}},  // 役満
        {Renho,          {"人和",             {1, 0}}},  // 役満
        {Ryuiso,         {"緑一色",           {1, 0}}},  // 役満
        {Daisangen,      {"大三元",           {1, 0}}},  // 役満
        {Syosusi,        {"小四喜",           {1, 0}}},  // 役満
        {Tuiso,          {"字一色",           {1, 0}}},  // 役満
        {Kokusimuso,     {"国士無双",         {1, 0}}},  // 役満
        {Tyurenpoto,     {"九連宝燈",         {1, 0}}},  // 役満
        {Suanko,         {"四暗刻",           {1, 0}}},  // 役満
        {Tinroto,        {"清老頭",           {1, 0}}},  // 役満
        {Sukantu,        {"四槓子",           {1, 0}}},  // 役満
        {SuankoTanki,    {"四暗刻単騎",       {2, 0}}},  // ダブル役満
        {Daisusi,        {"大四喜",           {2, 0}}},  // ダブル役満
        {Tyurenpoto9,    {"純正九連宝燈",     {2, 0}}},  // ダブル役満
        {Kokusimuso13,   {"国士無双13面待ち", {2, 0}}},  // ダブル役満
    };
    // clang-format on
};

} // namespace mahjong

#endif /* MAHJONG_CPP_YAKU */
