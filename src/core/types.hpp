#ifndef MAHJONG_CPP_TYPES

#include <string>
#include <vector>

namespace mahjong
{

/**
 * @brief 牌 (はい) の種類
 */
struct Tile {
    enum Type {
        Manzu1,    /*! 一萬 */
        Manzu2,    /*! 二萬 */
        Manzu3,    /*! 三萬 */
        Manzu4,    /*! 四萬 */
        Manzu5,    /*! 五萬 */
        Manzu6,    /*! 六萬 */
        Manzu7,    /*! 七萬 */
        Manzu8,    /*! 八萬 */
        Manzu9,    /*! 九萬 */
        Pinzu1,    /*! 一筒 */
        Pinzu2,    /*! 二筒 */
        Pinzu3,    /*! 三筒 */
        Pinzu4,    /*! 四筒 */
        Pinzu5,    /*! 五筒 */
        Pinzu6,    /*! 六筒 */
        Pinzu7,    /*! 七筒 */
        Pinzu8,    /*! 八筒 */
        Pinzu9,    /*! 九筒 */
        Sozu1,     /*! 一索 */
        Sozu2,     /*! 二索 */
        Sozu3,     /*! 三索 */
        Sozu4,     /*! 四索 */
        Sozu5,     /*! 五索 */
        Sozu6,     /*! 六索 */
        Sozu7,     /*! 七索 */
        Sozu8,     /*! 八索 */
        Sozu9,     /*! 九索 */
        Ton,       /*! 東 (とん) */
        Nan,       /*! 南 (なん) */
        Sya,       /*! 西 (しゃー) */
        Pe,        /*! 北 (ぺー) */
        Haku,      /*! 白 (はく) */
        Hatsu,     /*! 発 (はつ) */
        Tyun,      /*! 中 (ちゅん) */
        Length,    /*! 牌の種類 */
        AkaManzu5, /*! 赤五萬 */
        AkaPinzu5, /*! 赤五筒 */
        AkaSozu5,  /*! 赤五索 */
    };

    static const std::vector<std::string> Names;
    static const std::vector<int> Manzu;
    static const std::vector<int> Pinzu;
    static const std::vector<int> Sozu;
    static const std::vector<int> ZiHai;
    static const std::vector<int> RotoHai;
    static const std::vector<int> YaotyuHai;
    static const std::vector<int> TanyaoHai;
    static const std::vector<int> YakuHai;
};

/**
 * @brief ブロックの種類
 */
struct BlockType {
    enum Type {
        Anko,    //暗刻
        Minko,   //明刻
        Syuntsu, //順子
        Koutsu,  //刻子
        Toitsu,  //対子
        Zyanto,  //雀頭
        Ryanmen, //両面
        Kantyan, //嵌張
        Pentyan, //辺張
        Ryankan, //両嵌
        Koritsu, //孤立牌
    };
};

/**
 * @brief 役 (Yaku) の種類
 */
struct Yaku {
    enum Type {
        /* 1翻 */
        Tumo,           /* 門前清自摸和 (めんぜんつも) */
        Reach,          /* 立直 (りーち) */
        Ippatu,         /* 一発 (いっぱつ) */
        Tanyao,         /* 断幺九 (たんやおちゅー) */
        Pinhu,          /* 平和 (ぴんふ) */
        Ipeko,          /* 一盃口 (いーぺーこー) */
        Tyankan,        /* 槍槓 (ちゃんかん) */
        Rinsyankaiho,   /* 嶺上開花 (りんしゃんかいほー) */
        Haiteitumo,     /* 海底摸月 (はいていらおゆえ) */
        HouteiRon,      /* 河底撈魚 (ほおていらおゆい) */
        Dora,           /* ドラ (どら) */
        UraDora,        /* 裏ドラ (うらどら) */
        AkaDora,        /* 赤ドラ (あかどら) */
        SangenhaiHaku,  /* 三元牌 (白) (やくはい (はく)) */
        SangenhaiHatsu, /* 三元牌 (發) (やくはい (はつ)) */
        SangenhaiTyun,  /* 三元牌 (中) (やくはい (ちゅん)) */
        ZikazeTon,      /* 自風 (東) (じかぜ (とん)) */
        ZikazeNan,      /* 自風 (南) (じかぜ (なん)) */
        ZikazeSya,      /* 自風 (西) (じかぜ (しゃー)) */
        ZikazePe,       /* 自風 (北) (じかぜ (ぺー)) */
        BakazeTon,      /* 場風 (東) (ばかぜ (とん)) */
        BakazeNan,      /* 場風 (南) (ばかぜ (なん)) */
        BakazeSya,      /* 場風 (西) (ばかぜ (しゃー)) */
        BakazePe,       /* 場風 (北) (ばかぜ (ぺー)) */

        /* 2翻 */
        WReach,         /* W 立直 (だぶるりーち) */
        Tiitoitu,       /* 七対子 (ちーといつ) */
        Toitoiho,       /* 対々和 (といといほう) */
        Sananko,        /* 三暗刻 (さんあんこー) */
        SansyokuDoko,   /* 三色同刻 (さんしょくどーこー) */
        SansyokuDozyun, /* 三色同順 (さんしょくどうじゅん) */
        Honroto,        /* 混老頭 (ほんろうとう) */
        IkkiTukan,      /* 一気通貫 (いっきつうかん) */
        Tyanta,         /* 混全帯幺 (ほんちゃんたいやおちゅー) */
        Syosangen,      /* 小三元 (しょうさんげん) */
        Sankantu,       /* 三槓子 (さんかんつ) */

        /* 3翻 */
        Honiso,     /* 混一色 (ほんいーそー) */
        Zyuntyanta, /* 混全帯么九 (じゅんちゃんたいやお) */
        Ryanpeko,   /* 二盃口 (りゃんぺーこー  */

        /* 5翻 */
        NagasiMangan, /* 流し満貫 (ながしまんがん) */
        Tiniso,       /* 清一色 (ほんいーそー) */

        /* 役満 */
        Tenho,      /* 天和 (てんほー) */
        Tiho,       /* 地和 (ちーほー) */
        Renho,      /* 人和 (れんほー) */
        Ryuiso,     /* 緑一色 (りゅーいーそー) */
        Daisangen,  /* 大三元 (だいさんげん) */
        Syosusi,    /* 小四喜和 (しょうすーしー) */
        Tuiso,      /* 字一色 (つーいーそー) */
        Kokusimuso, /* 国士無双 (こくしむそう) */
        Tyurenpoto, /* 九連宝燈 (ちゅーれんぽーとー) */
        Suanko,     /* 四暗刻 (すーあんこー) */
        Tinroto,    /* 清老頭 (ちんろうとう) */
        Sukantu,    /* 四槓子 (すーかんつ) */
        Daisyarin,  /* 大車輪 (だいしゃりん) */

        /* ダブル役満 */
        SuankoTanki, /* 四暗刻単騎 (すーあんこーたんき) */
        Daisusi,     /* 大四喜 (だいすーしー) */
        Tyurenpoto9, /* 純正九連宝燈 */
        Kokushi13,   /* 国士無双13面待ち */
    };

    /* 名前 */
    static const std::vector<std::string> Name;
};

} // namespace mahjong

#define MAHJONG_CPP_TYPES
#endif /* MAHJONG_CPP_TYPES */
