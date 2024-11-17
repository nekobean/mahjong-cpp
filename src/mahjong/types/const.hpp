#ifndef MAHJONG_CPP_CONST_HPP
#define MAHJONG_CPP_CONST_HPP

#include <array>
#include <map>
#include <string>
#include <vector>

#ifdef USE_ENGLISH
#define ENTRY(id, eng, jp) {id, eng}
#else
#define ENTRY(id, eng, jp) {id, jp}
#endif

namespace mahjong
{
/**
 * @brief Tile types
 */
namespace Tile
{
enum
{
    Null = -1,
    Manzu1,    /*! Manzu1 (一萬) */
    Manzu2,    /*! Manzu2 (二萬) */
    Manzu3,    /*! Manzu3 (三萬) */
    Manzu4,    /*! Manzu4 (四萬) */
    Manzu5,    /*! Manzu5 (五萬) */
    Manzu6,    /*! Manzu6 (六萬) */
    Manzu7,    /*! Manzu7 (七萬) */
    Manzu8,    /*! Manzu8 (八萬) */
    Manzu9,    /*! Manzu9 (九萬) */
    Pinzu1,    /*! Pinzu1 (一筒) */
    Pinzu2,    /*! Pinzu2 (二筒) */
    Pinzu3,    /*! Pinzu3 (三筒) */
    Pinzu4,    /*! Pinzu4 (四筒) */
    Pinzu5,    /*! Pinzu5 (五筒) */
    Pinzu6,    /*! Pinzu6 (六筒) */
    Pinzu7,    /*! Pinzu7 (七筒) */
    Pinzu8,    /*! Pinzu8 (八筒) */
    Pinzu9,    /*! Pinzu9 (九筒) */
    Souzu1,    /*! Souzu1 (一索) */
    Souzu2,    /*! Souzu2 (二索) */
    Souzu3,    /*! Souzu3 (三索) */
    Souzu4,    /*! Souzu4 (四索) */
    Souzu5,    /*! Souzu5 (五索) */
    Souzu6,    /*! Souzu6 (六索) */
    Souzu7,    /*! Souzu7 (七索) */
    Souzu8,    /*! Souzu8 (八索) */
    Souzu9,    /*! Souzu9 (九索) */
    East,      /*! East (東) */
    South,     /*! South (南) */
    West,      /*! West (西) */
    North,     /*! North (北) */
    White,     /*! White (白) */
    Green,     /*! Green (発) */
    Red,       /*! Red (中) */
    RedManzu5, /*! Red Manzu5 (赤五萬) */
    RedPinzu5, /*! Red Pinzu5 (赤五筒) */
    RedSouzu5, /*! Red Souzu5 (赤五索) */
    Length,
};

static inline const std::map<int, std::string> Name = {
    {Null, u8"Null"},    {Manzu1, u8"1m"},    {Manzu2, u8"2m"}, {Manzu3, u8"3m"},
    {Manzu4, u8"4m"},    {Manzu5, u8"5m"},    {Manzu6, u8"6m"}, {Manzu7, u8"7m"},
    {Manzu8, u8"8m"},    {Manzu9, u8"9m"},    {Pinzu1, u8"1p"}, {Pinzu2, u8"2p"},
    {Pinzu3, u8"3p"},    {Pinzu4, u8"4p"},    {Pinzu5, u8"5p"}, {Pinzu6, u8"6p"},
    {Pinzu7, u8"7p"},    {Pinzu8, u8"8p"},    {Pinzu9, u8"9p"}, {Souzu1, u8"1s"},
    {Souzu2, u8"2s"},    {Souzu3, u8"3s"},    {Souzu4, u8"4s"}, {Souzu5, u8"5s"},
    {Souzu6, u8"6s"},    {Souzu7, u8"7s"},    {Souzu8, u8"8s"}, {Souzu9, u8"9s"},
    {East, u8"1z"},      {South, u8"2z"},     {West, u8"3z"},   {North, u8"4z"},
    {White, u8"5z"},     {Green, u8"6z"},     {Red, u8"7z"},    {RedManzu5, u8"0m"},
    {RedPinzu5, u8"0p"}, {RedSouzu5, u8"0s"},
};
}; // namespace Tile

/**
 * @brief Block types
 */
namespace BlockType
{
enum
{
    Null = 0,
    Triplet = 1,  /* Triplet (刻子) */
    Sequence = 2, /* Sequence (順子) */
    Kong = 4,     /* Kong (槓子) */
    Pair = 8,     /* Pair (対子) */
    Open = 16,    /* Open (副露した牌が含まれるかどうか) */
};

static inline const std::map<int, std::string> Name = {
    ENTRY(Triplet, u8"Closed Triplet", u8"暗刻子"),
    ENTRY(Triplet | Open, u8"Open Triplet", u8"明刻子"),
    ENTRY(Sequence, u8"Closed Sequence", u8"暗順子"),
    ENTRY(Sequence | Open, u8"Open Sequence", u8"明順子"),
    ENTRY(Kong, u8"Closed Kong", u8"暗槓子"),
    ENTRY(Kong | Open, u8"Open Kong", u8"明槓子"),
    ENTRY(Pair, u8"Closed Pair", u8"暗対子"),
    ENTRY(Pair | Open, u8"Open Pair", u8"明対子"),
};
} // namespace BlockType

/**
 * @brief Seat types
 */
namespace SeatType
{
enum
{
    Null = -1,
    Myself,         /* Myself (自家) */
    LeftPlayer,     /* Player on the left side (上家) */
    OppositePlayer, /* Palyer on the opposite side (対面) */
    RightPlayer,    /* Player on the right side (下家) */
    Length,
};

static inline const std::map<int, std::string> Name = {
    ENTRY(Null, u8"Null", u8"Null"), ENTRY(Myself, u8"Myself", u8"自家"),
    ENTRY(LeftPlayer, u8"Left Player", u8"上家"),
    ENTRY(OppositePlayer, u8"Opposite Player", u8"対面"),
    ENTRY(RightPlayer, u8"Right Player", u8"下家")};
} // namespace SeatType

/**
 * @brief Meld types
 */
namespace MeldType
{
enum
{
    Null = -1,
    Pong,       /* Pong (ポン) */
    Chow,       /* Chow (チー) */
    ClosedKong, /* Closed kong (暗槓) */
    OpenKong,   /* Open kong (明槓) */
    AddedKong,  /* Added kong (加槓) */
    Length,
};

static inline const std::map<int, std::string> Name = {
    ENTRY(Null, u8"Null", u8"Null"),
    ENTRY(Pong, u8"Pong", u8"ポン"),
    ENTRY(Chow, u8"Chow", u8"チー"),
    ENTRY(ClosedKong, u8"Closed Kong", u8"暗槓"),
    ENTRY(OpenKong, u8"Open Kong", u8"明槓"),
    ENTRY(AddedKong, u8"Added Kong", u8"加槓")};
} // namespace MeldType

/**
 * @brief Player types
 */
namespace PlayerType
{
enum
{
    Null = -1,
    Player0, /* Player0 (プレイヤー1) */
    Player1, /* Player1 (プレイヤー2) */
    Player2, /* Player2 (プレイヤー3) */
    Player3, /* Player3 (プレイヤー4) */
    Length,
};

static inline const std::map<int, std::string> Name = {
    ENTRY(Null, u8"Null", u8"Null"), ENTRY(Player0, u8"Player0", u8"プレイヤー1"),
    ENTRY(Player1, u8"Player1", u8"プレイヤー2"),
    ENTRY(Player2, u8"Player2", u8"プレイヤー3"),
    ENTRY(Player3, u8"Player3", u8"プレイヤー4")};
} // namespace PlayerType

/**
 * @brief Wait types
 */
namespace WaitType
{
enum
{
    Null = -1,
    DoubleEdgeWait, /* waiting for either of two gates (両面待ち) */
    EdgeWait,       /* waiting for three or seven the tile (辺張待ち) */
    ClosedWait,     /* waiting for the middle in a sequence (嵌張待ち) */
    TripletWait,    /* Waiting for a triplet (双ポン待ち) */
    PairWait,       /* Waiting for one of a pair (単騎待ち) */
    Length,
};

static inline const std::map<int, std::string> Name = {
    ENTRY(Null, u8"Null", u8"Null"),
    ENTRY(DoubleEdgeWait, u8"Double Edge Wait", u8"両面待ち"),
    ENTRY(EdgeWait, u8"Edge Wait", u8"辺張待ち"),
    ENTRY(ClosedWait, u8"Closed Wait", u8"嵌張待ち"),
    ENTRY(TripletWait, u8"Triplet Wait", u8"双ポン待ち"),
    ENTRY(PairWait, u8"Pair Wait", u8"単騎待ち")};

} // namespace WaitType

/**
 * @brief Flags related to rules
 */
namespace RuleFlag
{
enum
{
    Null = 0,
    RedDora = 1,    /* Allow red dora (赤ドラ有り) */
    OpenTanyao = 2, /* Allow open Tanyao (喰い断有り) */
};

static inline const std::map<int, std::string> Name = {
    ENTRY(Null, u8"Null", u8"Null"), ENTRY(RedDora, u8"Red Dora", u8"赤ドラ有り"),
    ENTRY(OpenTanyao, u8"Open Tanyao", u8"喰い断有り")};
} // namespace RuleFlag

/**
 * @brief Flags related to which winning form to calculate shanten number for
 */
namespace ShantenFlag
{
enum
{
    Null = 0,
    /* Shanten number for regular form (一般形に対する向聴数) */
    Regular = 1,
    /* Shanten number for Seven Pairs (七対子に対する向聴数) */
    SevenPairs = 2,
    /* Shanten number for Thirteen Orphans (国士無双に対する向聴数) */
    ThirteenOrphans = 4,
    All = Regular | SevenPairs | ThirteenOrphans,
};

static inline const std::map<int, std::string> Name = {
    ENTRY(Null, u8"Null", u8"Null"),
    ENTRY(Regular, u8"Regular", u8"一般形"),
    ENTRY(SevenPairs, u8"Seven Pairs", u8"七対子"),
    ENTRY(ThirteenOrphans, u8"Thirteen Orphans", u8"国士無双"),
};
}; // namespace ShantenFlag

/**
 * @brief Flags related to win
 *
 *    For a Tsumo win, specify Tsumo regardless of whether hand is closed or open.
 *    Only one of Riichi or Double Riichi can be specified.
 *    Only one of RobbingAKong, AfterAKong, UnderTheSea, or UnderTheRiver can be specified.
 *    Only one of BlessingOfHeaven, BlessingOfEarth, or HandOfMan can be specified.
 */
namespace WinFlag
{
enum
{
    Null = 0,
    Tsumo = 1 << 1,             /* Tsumo win (自摸和了) */
    Riichi = 1 << 2,            /* Riich established (立直成立) */
    Ippatsu = 1 << 3,           /* One-shot Win established (一発成立) */
    RobbingAKong = 1 << 4,      /* Robbing a Kong established (搶槓成立) */
    AfterAKong = 1 << 5,        /* After a Kong established (嶺上開花成立) */
    UnderTheSea = 1 << 6,       /* Under the Sea established(海底撈月成立) */
    UnderTheRiver = 1 << 7,     /* Under the River established (河底撈魚成立) */
    DoubleRiichi = 1 << 8,      /* Double Riichi established (ダブル立直成立) */
    NagashiMangan = 1 << 9,     /* Mangan at Draw established(流し満貫成立) */
    BlessingOfHeaven = 1 << 10, /* Blessing of Heaven established (天和成立) */
    BlessingOfEarth = 1 << 11,  /* Blessing of Earth established (地和成立) */
    HandOfMan = 1 << 12,        /* Hand of Man established (人和成立) */
};

static inline const std::map<int, std::string> Name = {
    ENTRY(Null, u8"Null", u8"Null"),
    ENTRY(Tsumo, u8"Tsumo win", u8"自摸和了"),
    ENTRY(Riichi, u8"Riichi established", u8"立直成立"),
    ENTRY(Ippatsu, u8"Ippatsu established", u8"一発成立"),
    ENTRY(RobbingAKong, u8"Robbing a Kong", u8"搶槓成立"),
    ENTRY(AfterAKong, u8"After a Kong established", u8"嶺上開花成立"),
    ENTRY(UnderTheSea, u8"Under the Sea established", u8"海底撈月成立"),
    ENTRY(UnderTheRiver, u8"Under the River established", u8"河底撈魚成立"),
    ENTRY(DoubleRiichi, u8"Double Riichi established", u8"ダブル立直成立"),
    ENTRY(NagashiMangan, u8"Mangan at Draw established", u8"流し満貫成立"),
    ENTRY(BlessingOfHeaven, u8"Blessing of Heaven established", u8"天和成立"),
    ENTRY(BlessingOfEarth, u8"Blessing of Earth established", u8"地和成立"),
    ENTRY(HandOfMan, u8"Hand of Man established", u8"人和成立")};
} // namespace WinFlag

using YakuList = unsigned long long;

/**
 * @brief 役
 */
namespace Yaku
{
/**
 * @brief 役の種類
 */
// clang-format off
enum : YakuList
{
    Null = 0LL,
    Tsumo = 1LL,                                 /* Win by self-draw (門前清自摸和) */
    Riichi = 1LL << 1,                           /* Riichi (立直) */
    Ippatsu = 1LL << 2,                          /* One-shot Win (一発) */
    Tanyao = 1LL << 3,                           /* All Simples (断幺九) */
    Pinfu = 1LL << 4,                            /* Pinfu (平和) */
    PureDoubleSequence = 1LL << 5,               /* Pure Double Sequence (一盃口) */
    RobbingAKong = 1LL << 6,                     /* Robbing a kong (槍槓) */
    AfterAKong = 1LL << 7,                       /* After a Kong (嶺上開花) */
    UnderTheSea = 1LL << 8,                      /* Under the Sea (海底摸月) */
    UnderTheRiver = 1LL << 9,                    /* Under the River (河底撈魚) */
    Dora = 1LL << 10,                            /* Dora (ドラ) */
    UraDora = 1LL << 11,                         /* Ura Dora (裏ドラ) */
    RedDora = 1LL << 12,                         /* Red Dora (赤ドラ) */
    WhiteDragon = 1LL << 13,                     /* White Dragon (三元牌 白) */
    GreenDragon = 1LL << 14,                     /* Green Dragon (三元牌 發) */
    RedDragon = 1LL << 15,                       /* Red Dragon (三元牌 中) */
    SelfWindEast = 1LL << 16,                    /* Self Wind East (自風 東) */
    SelfWindSouth = 1LL << 17,                   /* Self Wind South (自風 南) */
    SelfWindWest = 1LL << 18,                    /* Self Wind West (自風 西) */
    SelfWindNorth = 1LL << 19,                   /* Self Wind North (自風 北) */
    RoundWindEast = 1LL << 20,                   /* Round Wind East (場風 東) */
    RoundWindSouth = 1LL << 21,                  /* Round Wind South (場風 南) */
    RoundWindWest = 1LL << 22,                   /* Round Wind West (場風 西) */
    RoundWindNorth = 1LL << 23,                  /* Round Wind North (場風 北) */
    DoubleRiichi = 1LL << 24,                    /* Double Riichi (ダブル立直) */
    SevenPairs = 1LL << 25,                      /* Seven Pairs (七対子) */
    AllTriplets = 1LL << 26,                     /* All Triplets (対々和) */
    ThreeConcealedTriplets = 1LL << 27,          /* Three Concealed Triplets (三暗刻) */
    TripleTriplets = 1LL << 28,                  /* Triple Triplets (三色同刻) */
    MixedTripleSequence = 1LL << 29,             /* Mixed Triple Sequence (三色同順) */
    AllTerminalsAndHonors = 1LL << 30,           /* All Terminals and Honors (混老頭) */
    PureStraight = 1LL << 31,                    /* Pure Straight (一気通貫) */
    HalfOutsideHand = 1LL << 32,                 /* Half Outside Hand (混全帯幺九) */
    LittleThreeDragons = 1LL << 33,              /* Little Three Dragons (小三元) */
    ThreeKongs = 1LL << 34,                      /* Three Quads (三槓子) */
    HalfFlush = 1LL << 35,                       /* Half Flush (混一色) */
    FullyOutsideHand = 1LL << 36,                /* Fully Outside Hand (純全帯幺九) */
    TwicePureDoubleSequence = 1LL << 37,         /* Twice Pure Double Sequence (二盃口) */
    NagasiMangan = 1LL << 38,                    /* Mangan at Draw (流し満貫) */
    FullFlush = 1LL << 39,                       /* Full Flush (清一色) */
    BlessingOfHeaven = 1LL << 40,                /* Blessing of Heaven (天和) */
    BlessingOfEarth = 1LL << 41,                 /* Blessing of Earth (地和) */
    HandOfMan = 1LL << 42,                       /* Hand of Man (人和) */
    AllGreen = 1LL << 43,                        /* All Green (緑一色) */
    BigThreeDragons = 1LL << 44,                 /* Big Three Dragons (大三元) */
    LittleFourWinds = 1LL << 45,                 /* Little Four Winds (小四喜) */
    AllHonors = 1LL << 46,                       /* All Honors (字一色) */
    ThirteenOrphans = 1LL << 47,                 /* Thirteen Orphans (国士無双) */
    NineGates = 1LL << 48,                       /* Nine Gates (九連宝燈) */
    FourConcealedTriplets = 1LL << 49,           /* Four Concealed Triplets (四暗刻) */
    AllTerminals = 1LL << 50,                    /* All Terminals (清老頭) */
    FourKongs = 1LL << 51,                       /* Four Kongs (四槓子) */
    SingleWaitFourConcealedTriplets = 1LL << 52, /* Single-wait Four Concealed Triplets (四暗刻単騎) */
    BigFourWinds = 1LL << 53,                    /* Big Four Winds (大四喜) */
    TrueNineGates = 1LL << 54,                   /* True Nine Gates (純正九連宝燈) */
    ThirteenWaitThirteenOrphans = 1LL << 55,     /* Thirteen-wait Thirteen Orphans (国士無双13面待ち) */
    Length = 56LL,
};
// clang-format on

static inline std::map<YakuList, std::string> Name = {
    ENTRY(Null, u8"Null", u8"役なし"),
    ENTRY(Tsumo, u8"Win by self-draw", u8"門前清自摸和"),
    ENTRY(Riichi, u8"Riichi", u8"立直"),
    ENTRY(Ippatsu, u8"One-shot Win", u8"一発"),
    ENTRY(Tanyao, u8"All Simples", u8"断幺九"),
    ENTRY(Pinfu, u8"Pinfu", u8"平和"),
    ENTRY(PureDoubleSequence, u8"Pure Double Sequence", u8"一盃口"),
    ENTRY(RobbingAKong, u8"Robbing a Kong", u8"槍槓"),
    ENTRY(AfterAKong, u8"After a Kong", u8"嶺上開花"),
    ENTRY(UnderTheSea, u8"Under the Sea", u8"海底摸月"),
    ENTRY(UnderTheRiver, u8"Under the River", u8"河底撈魚"),
    ENTRY(Dora, u8"Dora", u8"ドラ"),
    ENTRY(UraDora, u8"Ura Dora", u8"裏ドラ"),
    ENTRY(RedDora, u8"Red Dora", u8"赤ドラ"),
    ENTRY(WhiteDragon, u8"White Dragon", u8"三元牌 白"),
    ENTRY(GreenDragon, u8"Green Dragon", u8"三元牌 發"),
    ENTRY(RedDragon, u8"Red Dragon", u8"三元牌 中"),
    ENTRY(SelfWindEast, u8"Self Wind East", u8"自風 東"),
    ENTRY(SelfWindSouth, u8"Self Wind South", u8"自風 南"),
    ENTRY(SelfWindWest, u8"Self Wind West", u8"自風 西"),
    ENTRY(SelfWindNorth, u8"Self Wind North", u8"自風 北"),
    ENTRY(RoundWindEast, u8"Round Wind East", u8"場風 東"),
    ENTRY(RoundWindSouth, u8"Round Wind South", u8"場風 南"),
    ENTRY(RoundWindWest, u8"Round Wind West", u8"場風 西"),
    ENTRY(RoundWindNorth, u8"Round Wind North", u8"場風 北"),
    ENTRY(DoubleRiichi, u8"Double Riichi", u8"ダブル立直"),
    ENTRY(SevenPairs, u8"Seven Pairs", u8"七対子"),
    ENTRY(AllTriplets, u8"All Triplets", u8"対々和"),
    ENTRY(ThreeConcealedTriplets, u8"Three Concealed Triplets", u8"三暗刻"),
    ENTRY(TripleTriplets, u8"Triple Triplets", u8"三色同刻"),
    ENTRY(MixedTripleSequence, u8"Mixed Triple Sequence", u8"三色同順"),
    ENTRY(AllTerminalsAndHonors, u8"All Terminals and Honors", u8"混老頭"),
    ENTRY(PureStraight, u8"Pure Straight", u8"一気通貫"),
    ENTRY(HalfOutsideHand, u8"Half Outside Hand", u8"混全帯幺九"),
    ENTRY(LittleThreeDragons, u8"Little Three Dragons", u8"小三元"),
    ENTRY(ThreeKongs, u8"Three Kongs", u8"三槓子"),
    ENTRY(HalfFlush, u8"Half Flush", u8"混一色"),
    ENTRY(FullyOutsideHand, u8"Fully Outside Hand", u8"純全帯幺九"),
    ENTRY(TwicePureDoubleSequence, u8"Twice Pure Double Sequence", u8"二盃口"),
    ENTRY(NagasiMangan, u8"Mangan at Draw", u8"流し満貫"),
    ENTRY(FullFlush, u8"Full Flush", u8"清一色"),
    ENTRY(BlessingOfHeaven, u8"Blessing of Heaven", u8"天和"),
    ENTRY(BlessingOfEarth, u8"Blessing of Earth", u8"地和"),
    ENTRY(HandOfMan, u8"Hand of Man", u8"人和"),
    ENTRY(AllGreen, u8"All Green", u8"緑一色"),
    ENTRY(BigThreeDragons, u8"Big Three Dragons", u8"大三元"),
    ENTRY(LittleFourWinds, u8"Little Four Winds", u8"小四喜"),
    ENTRY(AllHonors, u8"All Honors", u8"字一色"),
    ENTRY(ThirteenOrphans, u8"Thirteen Orphans", u8"国士無双"),
    ENTRY(NineGates, u8"Nine Gates", u8"九連宝燈"),
    ENTRY(FourConcealedTriplets, u8"Four Concealed Triplets", u8"四暗刻"),
    ENTRY(AllTerminals, u8"All Terminals", u8"清老頭"),
    ENTRY(FourKongs, u8"Four Kongs", u8"四槓子"),
    ENTRY(SingleWaitFourConcealedTriplets, u8"Single-wait Four Concealed Triplets",
          u8"四暗刻単騎"),
    ENTRY(BigFourWinds, u8"Big Four Winds", u8"大四喜"),
    ENTRY(TrueNineGates, u8"True Nine Gates", u8"純正九連宝燈"),
    ENTRY(ThirteenWaitThirteenOrphans, u8"Thirteen-wait Thirteen Orphans",
          u8"国士無双13面待ち"),
};

/**
 * @brief 役の情報
 */
// clang-format off
static inline std::map<YakuList, std::array<int, 2>> Han = {
    {Null,                           {0, 0}},
    {Tsumo,                          {1, 0}},  // 門前限定
    {Riichi,                         {1, 0}},  // 門前限定
    {Ippatsu,                        {1, 0}},  // 門前限定
    {Tanyao,                         {1, 1}},
    {Pinfu,                          {1, 0}},  // 門前限定
    {PureDoubleSequence,             {1, 0}},  // 門前限定
    {RobbingAKong,                   {1, 1}},
    {AfterAKong,                     {1, 1}},
    {UnderTheSea,                    {1, 1}},
    {UnderTheRiver,                  {1, 1}},
    {Dora,                           {0, 0}},
    {UraDora,                        {0, 0}},
    {RedDora,                        {0, 0}},
    {WhiteDragon,                    {1, 1}},
    {GreenDragon,                    {1, 1}},
    {RedDragon,                      {1, 1}},
    {SelfWindEast,                   {1, 1}},
    {SelfWindSouth,                  {1, 1}},
    {SelfWindWest,                   {1, 1}},
    {SelfWindNorth,                  {1, 1}},
    {RoundWindEast,                  {1, 1}},
    {RoundWindSouth,                 {1, 1}},
    {RoundWindWest,                  {1, 1}},
    {RoundWindNorth,                 {1, 1}},
    {DoubleRiichi,                   {2, 0}},  // 門前限定
    {SevenPairs,                     {2, 0}},  // 門前限定
    {AllTriplets,                    {2, 2}},
    {ThreeConcealedTriplets,         {2, 2}},
    {TripleTriplets,                 {2, 2}},
    {MixedTripleSequence,            {2, 1}},  // 喰い下がり
    {AllTerminalsAndHonors,          {2, 2}},
    {PureStraight,                   {2, 1}},  // 喰い下がり
    {HalfOutsideHand,                {2, 1}},  // 喰い下がり
    {LittleThreeDragons,             {2, 2}},
    {ThreeKongs,                     {2, 2}},
    {HalfFlush,                      {3, 2}},  // 喰い下がり
    {FullyOutsideHand,               {3, 2}},  // 喰い下がり
    {TwicePureDoubleSequence,        {3, 0}},  // 門前限定
    {NagasiMangan,                   {0, 0}},  // 満貫扱い
    {FullFlush,                      {6, 5}},  // 喰い下がり
    {BlessingOfHeaven,               {1, 0}},  // 役満
    {BlessingOfEarth,                {1, 0}},  // 役満
    {HandOfMan,                      {1, 0}},  // 役満
    {AllGreen,                       {1, 0}},  // 役満
    {BigThreeDragons,                {1, 0}},  // 役満
    {LittleFourWinds,                {1, 0}},  // 役満
    {AllHonors,                      {1, 0}},  // 役満
    {ThirteenOrphans,                {1, 0}},  // 役満
    {NineGates,                      {1, 0}},  // 役満
    {FourConcealedTriplets,          {1, 0}},  // 役満
    {AllTerminals,                   {1, 0}},  // 役満
    {FourKongs,                      {1, 0}},  // 役満
    {SingleWaitFourConcealedTriplets,{2, 0}},  // ダブル役満
    {BigFourWinds,                   {2, 0}},  // ダブル役満
    {TrueNineGates,                  {2, 0}},  // ダブル役満
    {ThirteenWaitThirteenOrphans,    {2, 0}},  // ダブル役満
};
// clang-format on

/*! List of normal yaku */
static inline std::vector<YakuList> NormalYaku = {
    Tsumo,
    Riichi,
    Ippatsu,
    Tanyao,
    Pinfu,
    PureDoubleSequence,
    RobbingAKong,
    AfterAKong,
    UnderTheSea,
    UnderTheRiver,
    WhiteDragon,
    GreenDragon,
    RedDragon,
    SelfWindEast,
    SelfWindSouth,
    SelfWindWest,
    SelfWindNorth,
    RoundWindEast,
    RoundWindSouth,
    RoundWindWest,
    RoundWindNorth,
    DoubleRiichi,
    SevenPairs,
    AllTriplets,
    ThreeConcealedTriplets,
    TripleTriplets,
    MixedTripleSequence,
    AllTerminalsAndHonors,
    PureStraight,
    HalfOutsideHand,
    LittleThreeDragons,
    ThreeKongs,
    HalfFlush,
    FullyOutsideHand,
    TwicePureDoubleSequence,
    FullFlush,
};

/*! List of normal yakuman */
static inline std::vector<YakuList> Yakuman = {
    BlessingOfHeaven,
    BlessingOfEarth,
    HandOfMan,
    AllGreen,
    BigThreeDragons,
    LittleFourWinds,
    AllHonors,
    ThirteenOrphans,
    NineGates,
    FourConcealedTriplets,
    AllTerminals,
    FourKongs,
    SingleWaitFourConcealedTriplets,
    BigFourWinds,
    TrueNineGates,
    ThirteenWaitThirteenOrphans,
};
}; // namespace Yaku

/**
 * @brief Score titles
 */
namespace ScoreTitle
{
enum
{
    Null = -1,
    Mangan,           /* Mangan (満貫) */
    Haneman,          /* Haneman (跳満) */
    Baiman,           /* Baiman (倍満) */
    Sanbaiman,        /* Sanbaiman (三倍満) */
    CountedYakuman,   /* Counted Yakuman (数え役満) */
    Yakuman,          /* Yakuman (役満) */
    DoubleYakuman,    /* Double Yakuman (ダブル役満) */
    TripleYakuman,    /* Triple Yakuman (トリプル役満) */
    QuadrupleYakuman, /* Quadruple Yakuman (四倍役満) */
    QuintupleYakuman, /* Quintuple Yakuman (五倍役満) */
    SextupleYakuman,  /* Sextuple Yakuman (六倍役満) */
    Length,
};

static inline std::map<int, std::string> Name = {
    ENTRY(Null, u8"Null", u8"Null"),
    ENTRY(Mangan, u8"Mangan", u8"満貫"),
    ENTRY(Haneman, u8"Haneman", u8"跳満"),
    ENTRY(Baiman, u8"Baiman", u8"倍満"),
    ENTRY(Sanbaiman, u8"Sanbaiman", u8"三倍満"),
    ENTRY(CountedYakuman, u8"Counted Yakuman", u8"数え役満"),
    ENTRY(Yakuman, u8"Yakuman", u8"役満"),
    ENTRY(DoubleYakuman, u8"Double Yakuman", u8"ダブル役満"),
    ENTRY(TripleYakuman, u8"Triple Yakuman", u8"トリプル役満"),
    ENTRY(QuadrupleYakuman, u8"Quadruple Yakuman", u8"4倍役満"),
    ENTRY(QuintupleYakuman, u8"Quintuple Yakuman", u8"5倍役満"),
    ENTRY(SextupleYakuman, u8"Sextuple Yakuman", u8"6倍役満")};
} // namespace ScoreTitle

/**
 * @brief Fu types
 */
namespace Hu
{
enum
{
    Null = -1,
    Hu20,  /* 20 Fu */
    Hu25,  /* 25 Fu */
    Hu30,  /* 30 Fu */
    Hu40,  /* 40 Fu */
    Hu50,  /* 50 Fu */
    Hu60,  /* 60 Fu */
    Hu70,  /* 70 Fu */
    Hu80,  /* 80 Fu */
    Hu90,  /* 90 Fu */
    Hu100, /* 100 Fu */
    Hu110, /* 110 Fu */
};

static inline std::map<int, std::string> Name = {
    ENTRY(Null, u8"Null", u8"Null"),    ENTRY(Hu20, u8"20Fu", u8"20符"),
    ENTRY(Hu25, u8"25Fu", u8"25符"),    ENTRY(Hu30, u8"30Fu", u8"30符"),
    ENTRY(Hu40, u8"40Fu", u8"40符"),    ENTRY(Hu50, u8"50Fu", u8"50符"),
    ENTRY(Hu60, u8"60Fu", u8"60符"),    ENTRY(Hu70, u8"70Fu", u8"70符"),
    ENTRY(Hu80, u8"80Fu", u8"80符"),    ENTRY(Hu90, u8"90Fu", u8"90符"),
    ENTRY(Hu100, u8"100Fu", u8"100符"), ENTRY(Hu110, u8"110Fu", u8"110符")};

static inline std::map<int, int> Values = {
    {Null, -1}, {Hu20, 20}, {Hu25, 25}, {Hu30, 30}, {Hu40, 40},   {Hu50, 50},
    {Hu60, 60}, {Hu70, 70}, {Hu80, 80}, {Hu90, 90}, {Hu100, 100}, {Hu110, 110}};

static inline std::map<int, int> Keys = {
    {-1, Null}, {20, Hu20}, {25, Hu25}, {30, Hu30}, {40, Hu40},   {50, Hu50},
    {60, Hu60}, {70, Hu70}, {80, Hu80}, {90, Hu90}, {100, Hu100}, {110, Hu110}};

} // namespace Hu
} // namespace mahjong

#endif /* MAHJONG_CPP_CONST_HPP */
