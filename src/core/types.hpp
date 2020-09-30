#ifndef MAHJONG_CPP_TYPES

#include <map>
#include <string>
#include <vector>

#include "bitutils.hpp"

namespace mahjong
{

/**
 * @brief 牌 (はい) の種類
 */
struct Tile {
    enum Type {
        Manzu1, /*! 一萬 */
        Manzu2, /*! 二萬 */
        Manzu3, /*! 三萬 */
        Manzu4, /*! 四萬 */
        Manzu5, /*! 五萬 */
        Manzu6, /*! 六萬 */
        Manzu7, /*! 七萬 */
        Manzu8, /*! 八萬 */
        Manzu9, /*! 九萬 */
        Pinzu1, /*! 一筒 */
        Pinzu2, /*! 二筒 */
        Pinzu3, /*! 三筒 */
        Pinzu4, /*! 四筒 */
        Pinzu5, /*! 五筒 */
        Pinzu6, /*! 六筒 */
        Pinzu7, /*! 七筒 */
        Pinzu8, /*! 八筒 */
        Pinzu9, /*! 九筒 */
        Sozu1,  /*! 一索 */
        Sozu2,  /*! 二索 */
        Sozu3,  /*! 三索 */
        Sozu4,  /*! 四索 */
        Sozu5,  /*! 五索 */
        Sozu6,  /*! 六索 */
        Sozu7,  /*! 七索 */
        Sozu8,  /*! 八索 */
        Sozu9,  /*! 九索 */
        Ton,    /*! 東 (とん) */
        Nan,    /*! 南 (なん) */
        Sya,    /*! 西 (しゃー) */
        Pe,     /*! 北 (ぺー) */
        Haku,   /*! 白 (はく) */
        Hatsu,  /*! 発 (はつ) */
        Tyun,   /*! 中 (ちゅん) */
        Length,
        Null,
        AkaManzu5, /*! 赤五萬 */
        AkaPinzu5, /*! 赤五筒 */
        AkaSozu5,  /*! 赤五索 */
    };

    static const std::vector<std::string> Names;
};

/**
 * @brief 副露 (ふーろ) の種類
 */
struct Huro {
    enum Type {
        Pon,    /* ポン (ぽん) */
        Ti,     /* チー (ちー) */
        Ankan,  /* 暗槓 (あんかん) */
        Minkan, /* 明槓 (みんかん) */
        Kakan,  /* 加槓 (かかん) */
        Length,
        Null,
    };

    static const std::vector<std::string> Names;
};

/**
 * @brief 手牌
 */
class Tehai
{
public:
    enum class StringFormat {
        Kanji,
        MPS,
    };

    Tehai();
    Tehai(const std::vector<int> &tehai);

    std::string to_kanji_string() const;
    std::string to_mps_string() const;
    std::string to_string(StringFormat format = StringFormat::MPS) const;
    void print_bit() const;
    void print_num_tiles() const;

    friend std::ostream &operator<<(std::ostream &os, const Tehai &tehai);

public:
    /*! ビット列にした手牌
     *  例: [0, 2, 0, 2, 2, 1, 1, 1, 4] -> 69510160 (00|000|100|001|001|001|010|010|000|010|000)
     *                                                      牌9 牌8 牌7 牌6 牌5 牌4 牌3 牌2 牌1
     */
    int manzu;
    int pinzu;
    int sozu;
    int zihai;

    int count() const
    {
        return Bit::sum(manzu) + Bit::sum(pinzu) + Bit::sum(sozu) + Bit::sum(zihai);
    }

    bool contains(int hai) const
    {
        if (Tile::Manzu1 <= hai && hai <= Tile::Manzu9)
            return Bit::mask[hai] & manzu;
        else if (Tile::Pinzu1 <= hai && hai <= Tile::Pinzu9)
            return Bit::mask[hai] & pinzu;
        else if (Tile::Sozu1 <= hai && hai <= Tile::Sozu9)
            return Bit::mask[hai] & sozu;
        else
            return Bit::mask[hai] & zihai;
    }
};

/**
 * @brief 副露ブロック
 */
struct HuroBlock {
    HuroBlock() : type(Huro::Null), nakihai(Tile::Null), from(0)
    {
    }

    HuroBlock(Huro::Type type, const std::vector<int> tiles, int nakihai, int from)
        : type(type), tiles(tiles), nakihai(nakihai), from(from)
    {
    }

    /*! 副露の種類 */
    Huro::Type type;

    /*! 構成牌 */
    Tehai tiles;

    /*! 何番目の牌を鳴いたか */
    int nakihai;

    /*! どこから鳴いたか */
    int from;
};

/**
 * @brief ブロックの種類
 */
struct Block {
    enum Type {
        kotu,      /* 刻子 (こーつ) */
        Ankotu,    /* 暗刻子 (あんこーつ) */
        Minkotu,   /* 明刻子 (みんこーつ) */
        Syuntsu,   /* 順子 (しゅんつ) */
        Ansyuntu,  /* 暗順子 (あんしゅんつ) */
        Minsyuntu, /* 明順子 (みんしゅんつ) */
        Tatu,      /* 塔子 (たーつ) */
        Toitu,     /* 対子 (といつ) */
        Zyanto,    /* 雀頭 (じゃんとう) */
        Ryanmen,   /* 両面 (りゃんめん) */
        Pentyan,   /* 辺張 (ぺんちゃん) */
        Kantyan,   /* 嵌張 (かんちゃん) */
        Ryankan,   /* 両嵌 (りゃんかん) */
        Koritu,    /* 孤立牌 (こりつはい) */
        Length,
        Null,
    };
};

/**
 * @brief 役 (Yaku) の種類
 */
struct Yaku {
    enum Type : unsigned long long {
        /* 1翻 */
        Tumo = 1ull,                 /* 門前清自摸和 (めんぜんつも) */
        Reach = 1ull << 1,           /* 立直 (りーち) */
        Ippatu = 1ull << 2,          /* 一発 (いっぱつ) */
        Tanyao = 1ull << 3,          /* 断幺九 (たんやおちゅー) */
        Pinhu = 1ull << 4,           /* 平和 (ぴんふ) */
        Ipeko = 1ull << 5,           /* 一盃口 (いーぺーこー) */
        Tyankan = 1ull << 6,         /* 槍槓 (ちゃんかん) */
        Rinsyankaiho = 1ull << 7,    /* 嶺上開花 (りんしゃんかいほー) */
        Haiteitumo = 1ull << 8,      /* 海底摸月 (はいていらおゆえ) */
        HouteiRon = 1ull << 9,       /* 河底撈魚 (ほおていらおゆい) */
        Dora = 1ull << 10,           /* ドラ (どら) */
        UraDora = 1ull << 11,        /* 裏ドラ (うらどら) */
        AkaDora = 1ull << 12,        /* 赤ドラ (あかどら) */
        SangenhaiHaku = 1ull << 13,  /* 三元牌 (白) (やくはい (はく)) */
        SangenhaiHatsu = 1ull << 14, /* 三元牌 (發) (やくはい (はつ)) */
        SangenhaiTyun = 1ull << 15,  /* 三元牌 (中) (やくはい (ちゅん)) */
        ZikazeTon = 1ull << 16,      /* 自風 (東) (じかぜ (とん)) */
        ZikazeNan = 1ull << 17,      /* 自風 (南) (じかぜ (なん)) */
        ZikazeSya = 1ull << 18,      /* 自風 (西) (じかぜ (しゃー)) */
        ZikazePe = 1ull << 19,       /* 自風 (北) (じかぜ (ぺー)) */
        BakazeTon = 1ull << 20,      /* 場風 (東) (ばかぜ (とん)) */
        BakazeNan = 1ull << 21,      /* 場風 (南) (ばかぜ (なん)) */
        BakazeSya = 1ull << 22,      /* 場風 (西) (ばかぜ (しゃー)) */
        BakazePe = 1ull << 23,       /* 場風 (北) (ばかぜ (ぺー)) */

        /* 2翻 */
        WReach = 1ull << 24,         /* W 立直 (だぶるりーち) */
        Tiitoitu = 1ull << 25,       /* 七対子 (ちーといつ) */
        Toitoiho = 1ull << 26,       /* 対々和 (といといほう) */
        Sananko = 1ull << 27,        /* 三暗刻 (さんあんこー) */
        SansyokuDoko = 1ull << 28,   /* 三色同刻 (さんしょくどーこー) */
        SansyokuDozyun = 1ull << 29, /* 三色同順 (さんしょくどうじゅん) */
        Honroto = 1ull << 30,        /* 混老頭 (ほんろうとう) */
        IkkiTukan = 1ull << 31,      /* 一気通貫 (いっきつうかん) */
        Tyanta = 1ull << 32,         /* 混全帯幺 (ほんちゃんたいやおちゅー) */
        Syosangen = 1ull << 33,      /* 小三元 (しょうさんげん) */
        Sankantu = 1ull << 34,       /* 三槓子 (さんかんつ) */

        /* 3翻 */
        Honiso = 1ull << 35,     /* 混一色 (ほんいーそー) */
        Zyuntyanta = 1ull << 36, /* 混全帯么九 (じゅんちゃんたいやお) */
        Ryanpeko = 1ull << 37,   /* 二盃口 (りゃんぺーこー  */

        /* 5翻 */
        NagasiMangan = 1ull << 38, /* 流し満貫 (ながしまんがん) */
        Tiniso = 1ull << 39,       /* 清一色 (ほんいーそー) */

        /* 役満 */
        Tenho = 1ull << 40,      /* 天和 (てんほー) */
        Tiho = 1ull << 41,       /* 地和 (ちーほー) */
        Renho = 1ull << 42,      /* 人和 (れんほー) */
        Ryuiso = 1ull << 43,     /* 緑一色 (りゅーいーそー) */
        Daisangen = 1ull << 44,  /* 大三元 (だいさんげん) */
        Syosusi = 1ull << 45,    /* 小四喜 (しょうすーしー) */
        Tuiso = 1ull << 46,      /* 字一色 (つーいーそー) */
        Kokusimuso = 1ull << 47, /* 国士無双 (こくしむそう) */
        Tyurenpoto = 1ull << 48, /* 九連宝燈 (ちゅーれんぽーとー) */
        Suanko = 1ull << 49,     /* 四暗刻 (すーあんこー) */
        Tinroto = 1ull << 50,    /* 清老頭 (ちんろうとう) */
        Sukantu = 1ull << 51,    /* 四槓子 (すーかんつ) */
        Daisyarin = 1ull << 52,  /* 大車輪 (だいしゃりん) */

        /* ダブル役満 */
        SuankoTanki = 1ull << 53,  /* 四暗刻単騎 (すーあんこーたんき) */
        Daisusi = 1ull << 54,      /* 大四喜 (だいすーしー) */
        Tyurenpoto9 = 1ull << 55,  /* 純正九連宝燈 */
        Kokusimuso13 = 1ull << 56, /* 国士無双13面待ち */

        Null = 0ull,
        Length = 57ull,
    };

    /* 名前 */
    static const std::map<unsigned long long, std::string> Names;
};

struct ScoreName {
    enum Type {
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
        Null,
    };

    static int han_to_score(int han)
    {
        if (han == 5)
            return Mangan;
        else if (6 <= han && han <= 7)
            return Haneman;
        else if (8 <= han && han <= 10)
            return Baiman;
        else if (11 <= han && han <= 12)
            return Sanbaiman;
        else if (han >= 13)
            return KazoeYakuman;
    }

    static int yakuman_to_score(int n)
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

    /* 名前 */
    static const std::vector<std::string> Names;
};

} // namespace mahjong

#define MAHJONG_CPP_TYPES
#endif /* MAHJONG_CPP_TYPES */
