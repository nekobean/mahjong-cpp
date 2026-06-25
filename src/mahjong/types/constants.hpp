#ifndef MAHJONG_CPP_CONSTANTS
#define MAHJONG_CPP_CONSTANTS

#include <array>
#include <cstdint>
#include <string_view>

#ifdef LANG_EN
#define MAHJONG_NAME(eng, jp) eng
#else
#define MAHJONG_NAME(eng, jp) jp
#endif

namespace mahjong
{

/**
 * @brief Mahjong modes.
 */
namespace GameMode
{

inline constexpr int Null = -1;
inline constexpr int Sanma = 0; /*! 三人麻雀 */
inline constexpr int Yonma = 1; /*! 四人麻雀 */
inline constexpr int Length = 2;

inline constexpr std::string_view name(int mode) noexcept
{
    switch (mode) {
    case Null:
        return "Null";
    case Sanma:
        return MAHJONG_NAME("Sanma", "三麻");
    case Yonma:
        return MAHJONG_NAME("Yonma", "四麻");
    default:
        return "Unknown";
    }
}

} // namespace GameMode

/**
 * @brief Game lengths.
 */
namespace GameLength
{

inline constexpr int Null = -1;
inline constexpr int Tonpu = 0;   /*! 東風戦 */
inline constexpr int Hanchan = 1; /*! 半荘戦 */
inline constexpr int Length = 2;

inline constexpr std::string_view name(int length) noexcept
{
    switch (length) {
    case Null:
        return "Null";
    case Tonpu:
        return MAHJONG_NAME("Tonpu", "東風戦");
    case Hanchan:
        return MAHJONG_NAME("Hanchan", "半荘戦");
    default:
        return "Unknown";
    }
}

} // namespace GameLength

/**
 * @brief Tile types
 */
namespace Tile
{

inline constexpr int Null = -1;
inline constexpr int Manzu1 = 0;       /*! 一萬 */
inline constexpr int Manzu2 = 1;       /*! 二萬 */
inline constexpr int Manzu3 = 2;       /*! 三萬 */
inline constexpr int Manzu4 = 3;       /*! 四萬 */
inline constexpr int Manzu5 = 4;       /*! 五萬 */
inline constexpr int Manzu6 = 5;       /*! 六萬 */
inline constexpr int Manzu7 = 6;       /*! 七萬 */
inline constexpr int Manzu8 = 7;       /*! 八萬 */
inline constexpr int Manzu9 = 8;       /*! 九萬 */
inline constexpr int Pinzu1 = 9;       /*! 一筒 */
inline constexpr int Pinzu2 = 10;      /*! 二筒 */
inline constexpr int Pinzu3 = 11;      /*! 三筒 */
inline constexpr int Pinzu4 = 12;      /*! 四筒 */
inline constexpr int Pinzu5 = 13;      /*! 五筒 */
inline constexpr int Pinzu6 = 14;      /*! 六筒 */
inline constexpr int Pinzu7 = 15;      /*! 七筒 */
inline constexpr int Pinzu8 = 16;      /*! 八筒 */
inline constexpr int Pinzu9 = 17;      /*! 九筒 */
inline constexpr int Souzu1 = 18;      /*! 一索 */
inline constexpr int Souzu2 = 19;      /*! 二索 */
inline constexpr int Souzu3 = 20;      /*! 三索 */
inline constexpr int Souzu4 = 21;      /*! 四索 */
inline constexpr int Souzu5 = 22;      /*! 五索 */
inline constexpr int Souzu6 = 23;      /*! 六索 */
inline constexpr int Souzu7 = 24;      /*! 七索 */
inline constexpr int Souzu8 = 25;      /*! 八索 */
inline constexpr int Souzu9 = 26;      /*! 九索 */
inline constexpr int East = 27;        /*! 東 */
inline constexpr int South = 28;       /*! 南 */
inline constexpr int West = 29;        /*! 西 */
inline constexpr int North = 30;       /*! 北 */
inline constexpr int WhiteDragon = 31; /*! 白 */
inline constexpr int GreenDragon = 32; /*! 發 */
inline constexpr int RedDragon = 33;   /*! 中 */
inline constexpr int RedManzu5 = 34;   /*! 赤五萬 */
inline constexpr int RedPinzu5 = 35;   /*! 赤五筒 */
inline constexpr int RedSouzu5 = 36;   /*! 赤五索 */
inline constexpr int Length = 37;

inline constexpr std::string_view name(int tile) noexcept
{
    switch (tile) {
    case Null:
        return "Null";
    case Manzu1:
        return "1m";
    case Manzu2:
        return "2m";
    case Manzu3:
        return "3m";
    case Manzu4:
        return "4m";
    case Manzu5:
        return "5m";
    case Manzu6:
        return "6m";
    case Manzu7:
        return "7m";
    case Manzu8:
        return "8m";
    case Manzu9:
        return "9m";
    case Pinzu1:
        return "1p";
    case Pinzu2:
        return "2p";
    case Pinzu3:
        return "3p";
    case Pinzu4:
        return "4p";
    case Pinzu5:
        return "5p";
    case Pinzu6:
        return "6p";
    case Pinzu7:
        return "7p";
    case Pinzu8:
        return "8p";
    case Pinzu9:
        return "9p";
    case Souzu1:
        return "1s";
    case Souzu2:
        return "2s";
    case Souzu3:
        return "3s";
    case Souzu4:
        return "4s";
    case Souzu5:
        return "5s";
    case Souzu6:
        return "6s";
    case Souzu7:
        return "7s";
    case Souzu8:
        return "8s";
    case Souzu9:
        return "9s";
    case East:
        return "1z";
    case South:
        return "2z";
    case West:
        return "3z";
    case North:
        return "4z";
    case WhiteDragon:
        return "5z";
    case GreenDragon:
        return "6z";
    case RedDragon:
        return "7z";
    case RedManzu5:
        return "0m";
    case RedPinzu5:
        return "0p";
    case RedSouzu5:
        return "0s";
    default:
        return "Unknown";
    }
}

} // namespace Tile

using Hand = std::array<int, Tile::Length>;

/**
 * @brief Player types
 */
namespace PlayerIndex
{

inline constexpr int Null = -1;
inline constexpr int Player0 = 0; /*! プレイヤー1 */
inline constexpr int Player1 = 1; /*! プレイヤー2 */
inline constexpr int Player2 = 2; /*! プレイヤー3 */
inline constexpr int Player3 = 3; /*! プレイヤー4 */
inline constexpr int Length = 4;

inline constexpr std::string_view name(int type) noexcept
{
    switch (type) {
    case Null:
        return "Null";
    case Player0:
        return MAHJONG_NAME("Player 1", "プレイヤー1");
    case Player1:
        return MAHJONG_NAME("Player 2", "プレイヤー2");
    case Player2:
        return MAHJONG_NAME("Player 3", "プレイヤー3");
    case Player3:
        return MAHJONG_NAME("Player 4", "プレイヤー4");
    default:
        return "Unknown";
    }
}

} // namespace PlayerIndex

/**
 * @brief Seat types
 */
namespace SeatType
{

inline constexpr int Null = -1;
inline constexpr int Self = 0;     /*! 自家 */
inline constexpr int Kamicha = 1;  /*! 上家 */
inline constexpr int Toimen = 2;   /*! 対面 */
inline constexpr int Shimocha = 3; /*! 下家 */
inline constexpr int Length = 4;

inline constexpr std::string_view name(int type) noexcept
{
    switch (type) {
    case Null:
        return "Null";
    case Self:
        return MAHJONG_NAME("Self", "自家");
    case Kamicha:
        return MAHJONG_NAME("Kamicha", "上家");
    case Toimen:
        return MAHJONG_NAME("Toimen", "対面");
    case Shimocha:
        return MAHJONG_NAME("Shimocha", "下家");
    default:
        return "Unknown";
    }
}

} // namespace SeatType

using BlockFlags = std::uint32_t;

/**
 * @brief Block types
 */
namespace BlockType
{

inline constexpr BlockFlags None = BlockFlags{0};
inline constexpr BlockFlags Triplet = BlockFlags{1} << 0;  /*! 刻子 */
inline constexpr BlockFlags Sequence = BlockFlags{1} << 1; /*! 順子 */
inline constexpr BlockFlags Kan = BlockFlags{1} << 2;      /*! 槓子 */
inline constexpr BlockFlags Pair = BlockFlags{1} << 3;     /*! 対子 */
inline constexpr BlockFlags Open = BlockFlags{1} << 4;     /*! 副露した牌が含まれる */

/**
 * @brief Returns the name of a single block flag or supported block flag combination.
 */
inline constexpr std::string_view name(BlockFlags type) noexcept
{
    switch (type) {
    case None:
        return "None";
    case Triplet:
        return MAHJONG_NAME("Concealed Triplet", "暗刻");
    case Triplet | Open:
        return MAHJONG_NAME("Open Triplet", "明刻");
    case Sequence:
        return MAHJONG_NAME("Sequence", "順子");
    case Sequence | Open:
        return MAHJONG_NAME("Open Sequence", "明順子");
    case Kan:
        return MAHJONG_NAME("Concealed Kan", "暗槓");
    case Kan | Open:
        return MAHJONG_NAME("Open Kan", "明槓");
    case Pair:
        return MAHJONG_NAME("Pair", "対子");
    case Pair | Open:
        return MAHJONG_NAME("Pair", "対子");
    default:
        return "Unknown";
    }
}

} // namespace BlockType

/**
 * @brief Meld types
 */
namespace MeldType
{

inline constexpr int Null = -1;
inline constexpr int Pon = 0;       /*! ポン */
inline constexpr int Chi = 1;       /*! チー */
inline constexpr int Ankan = 2;     /*! 暗槓 */
inline constexpr int Daiminkan = 3; /*! 大明槓 */
inline constexpr int Kakan = 4;     /*! 加槓 */
inline constexpr int Length = 5;

inline constexpr std::string_view name(int type) noexcept
{
    switch (type) {
    case Null:
        return "Null";
    case Pon:
        return MAHJONG_NAME("Pon", "ポン");
    case Chi:
        return MAHJONG_NAME("Chi", "チー");
    case Ankan:
        return MAHJONG_NAME("Ankan", "暗槓");
    case Daiminkan:
        return MAHJONG_NAME("Daiminkan", "大明槓");
    case Kakan:
        return MAHJONG_NAME("Kakan", "加槓");
    default:
        return "Unknown";
    }
}

} // namespace MeldType

/**
 * @brief Wait types
 */
namespace WaitType
{

inline constexpr int Null = -1;
inline constexpr int TwoSidedWait = 0;   /*! 両面待ち */
inline constexpr int EdgeWait = 1;       /*! 辺張待ち */
inline constexpr int MiddleWait = 2;     /*! 嵌張待ち */
inline constexpr int DoublePairWait = 3; /*! 双ポン待ち */
inline constexpr int SingleTileWait = 4; /*! 単騎待ち */
inline constexpr int Length = 5;

inline constexpr std::string_view name(int type) noexcept
{
    switch (type) {
    case Null:
        return "Null";
    case TwoSidedWait:
        return MAHJONG_NAME("Two-sided Wait", "両面待ち");
    case EdgeWait:
        return MAHJONG_NAME("Edge Wait", "辺張待ち");
    case MiddleWait:
        return MAHJONG_NAME("Middle Wait", "嵌張待ち");
    case DoublePairWait:
        return MAHJONG_NAME("Double-pair Wait", "双ポン待ち");
    case SingleTileWait:
        return MAHJONG_NAME("Single-tile Wait", "単騎待ち");
    default:
        return "Unknown";
    }
}

} // namespace WaitType

using RuleFlags = std::uint32_t;

/**
 * @brief Flags related to rules
 */
namespace RuleFlag
{

inline constexpr RuleFlags None = RuleFlags{0};
inline constexpr RuleFlags RedDora = RuleFlags{1} << 0;       /*! 赤ドラ有り */
inline constexpr RuleFlags UraDora = RuleFlags{1} << 1;       /*! 裏ドラ有り */
inline constexpr RuleFlags OpenTanyao = RuleFlags{1} << 2;    /*! 喰い断有り */
inline constexpr RuleFlags BankruptcyEnd = RuleFlags{1} << 3; /*! 飛び有り */
inline constexpr RuleFlags DoubleRon = RuleFlags{1} << 4;     /*! ダブロン有り */
inline constexpr RuleFlags TripleRon = RuleFlags{1} << 5;     /*! トリロン有り */
inline constexpr RuleFlags NagashiMangan = RuleFlags{1} << 6; /*! 流し満貫有り */

/**
 * @brief Returns the name of a single rule flag.
 */
inline constexpr std::string_view name(RuleFlags type) noexcept
{
    switch (type) {
    case None:
        return "None";
    case RedDora:
        return MAHJONG_NAME("Red Dora", "赤ドラ有り");
    case UraDora:
        return MAHJONG_NAME("Ura Dora", "裏ドラ有り");
    case OpenTanyao:
        return MAHJONG_NAME("Open Tanyao", "喰い断有り");
    case BankruptcyEnd:
        return MAHJONG_NAME("Bankruptcy End", "飛び有り");
    case DoubleRon:
        return MAHJONG_NAME("Double Ron", "ダブロン有り");
    case TripleRon:
        return MAHJONG_NAME("Triple Ron", "トリロン有り");
    case NagashiMangan:
        return MAHJONG_NAME("Nagashi Mangan", "流し満貫有り");
    default:
        return "Unknown";
    }
}

} // namespace RuleFlag

using ShantenFlags = std::uint32_t;

/**
 * @brief Flags related to which winning form to calculate shanten number for
 */
namespace ShantenFlag
{

inline constexpr ShantenFlags None = ShantenFlags{0};
inline constexpr ShantenFlags StandardHand = ShantenFlags{1} << 0;    /*! 一般形 */
inline constexpr ShantenFlags SevenPairs = ShantenFlags{1} << 1;      /*! 七対子 */
inline constexpr ShantenFlags ThirteenOrphans = ShantenFlags{1} << 2; /*! 国士無双 */

inline constexpr ShantenFlags All = StandardHand | SevenPairs | ThirteenOrphans;

/**
 * @brief Returns the name of a single shanten flag.
 */
inline constexpr std::string_view name(ShantenFlags flag) noexcept
{
    switch (flag) {
    case None:
        return "None";
    case StandardHand:
        return MAHJONG_NAME("Standard Hand", "一般形");
    case SevenPairs:
        return MAHJONG_NAME("Seven Pairs", "七対子");
    case ThirteenOrphans:
        return MAHJONG_NAME("Thirteen Orphans", "国士無双");
    default:
        return "Unknown";
    }
}

} // namespace ShantenFlag

using WinFlags = std::uint32_t;

/**
 * @brief Flags related to win conditions.
 *
 * For a tsumo win, specify Tsumo regardless of whether the hand is closed or open.
 * Only one of Riichi and DoubleRiichi can be specified.
 * Only one of RobbingAKan, AfterAKan, UnderTheSea, and UnderTheRiver can be specified.
 * Only one of HeavenlyHand, EarthlyHand, and HandOfMan can be specified.
 */
namespace WinFlag
{

inline constexpr WinFlags None = WinFlags{0};
inline constexpr WinFlags Tsumo = WinFlags{1} << 0;         /*! 自摸和了 */
inline constexpr WinFlags Riichi = WinFlags{1} << 1;        /*! 立直 */
inline constexpr WinFlags Ippatsu = WinFlags{1} << 2;       /*! 一発 */
inline constexpr WinFlags RobbingAKan = WinFlags{1} << 3;   /*! 槍槓 */
inline constexpr WinFlags AfterAKan = WinFlags{1} << 4;     /*! 嶺上開花 */
inline constexpr WinFlags UnderTheSea = WinFlags{1} << 5;   /*! 海底摸月 */
inline constexpr WinFlags UnderTheRiver = WinFlags{1} << 6; /*! 河底撈魚 */
inline constexpr WinFlags DoubleRiichi = WinFlags{1} << 7;  /*! ダブル立直 */
inline constexpr WinFlags NagashiMangan = WinFlags{1} << 8; /*! 流し満貫 */
inline constexpr WinFlags HeavenlyHand = WinFlags{1} << 9;  /*! 天和 */
inline constexpr WinFlags EarthlyHand = WinFlags{1} << 10;  /*! 地和 */
inline constexpr WinFlags HandOfMan = WinFlags{1} << 11;    /*! 人和 */

/**
 * @brief Returns the name of a single win flag.
 */
inline constexpr std::string_view name(WinFlags flag) noexcept
{
    switch (flag) {
    case None:
        return "None";
    case Tsumo:
        return MAHJONG_NAME("Self-draw Win", "自摸和了");
    case Riichi:
        return MAHJONG_NAME("Riichi", "立直");
    case Ippatsu:
        return MAHJONG_NAME("Ippatsu", "一発");
    case RobbingAKan:
        return MAHJONG_NAME("Robbing a Kan", "槍槓");
    case AfterAKan:
        return MAHJONG_NAME("After a Kan", "嶺上開花");
    case UnderTheSea:
        return MAHJONG_NAME("Under the Sea", "海底摸月");
    case UnderTheRiver:
        return MAHJONG_NAME("Under the River", "河底撈魚");
    case DoubleRiichi:
        return MAHJONG_NAME("Double Riichi", "ダブル立直");
    case NagashiMangan:
        return MAHJONG_NAME("Nagashi Mangan", "流し満貫");
    case HeavenlyHand:
        return MAHJONG_NAME("Heavenly Hand", "天和");
    case EarthlyHand:
        return MAHJONG_NAME("Earthly Hand", "地和");
    case HandOfMan:
        return MAHJONG_NAME("Hand of Man", "人和");
    default:
        return "Unknown";
    }
}

} // namespace WinFlag

using YakuFlags = std::uint64_t;

/**
 * @brief Yaku and bonus flags.
 */
namespace Yaku
{
// clang-format off
inline constexpr YakuFlags None = YakuFlags{0};
inline constexpr YakuFlags Tsumo = YakuFlags{1} << 0;                            /*! 門前清自摸和 */
inline constexpr YakuFlags Riichi = YakuFlags{1} << 1;                           /*! 立直 */
inline constexpr YakuFlags Ippatsu = YakuFlags{1} << 2;                          /*! 一発 */
inline constexpr YakuFlags Tanyao = YakuFlags{1} << 3;                           /*! 断幺九 */
inline constexpr YakuFlags Pinfu = YakuFlags{1} << 4;                            /*! 平和 */
inline constexpr YakuFlags PureDoubleSequence = YakuFlags{1} << 5;               /*! 一盃口 */
inline constexpr YakuFlags RobbingAKan = YakuFlags{1} << 6;                      /*! 槍槓 */
inline constexpr YakuFlags AfterAKan = YakuFlags{1} << 7;                        /*! 嶺上開花 */
inline constexpr YakuFlags UnderTheSea = YakuFlags{1} << 8;                      /*! 海底摸月 */
inline constexpr YakuFlags UnderTheRiver = YakuFlags{1} << 9;                    /*! 河底撈魚 */
inline constexpr YakuFlags Dora = YakuFlags{1} << 10;                            /*! ドラ */
inline constexpr YakuFlags UraDora = YakuFlags{1} << 11;                         /*! 裏ドラ */
inline constexpr YakuFlags RedDora = YakuFlags{1} << 12;                         /*! 赤ドラ */
inline constexpr YakuFlags WhiteDragon = YakuFlags{1} << 13;                     /*! 役牌 白 */
inline constexpr YakuFlags GreenDragon = YakuFlags{1} << 14;                     /*! 役牌 發 */
inline constexpr YakuFlags RedDragon = YakuFlags{1} << 15;                       /*! 役牌 中 */
inline constexpr YakuFlags SelfWindEast = YakuFlags{1} << 16;                    /*! 自風 東 */
inline constexpr YakuFlags SelfWindSouth = YakuFlags{1} << 17;                   /*! 自風 南 */
inline constexpr YakuFlags SelfWindWest = YakuFlags{1} << 18;                    /*! 自風 西 */
inline constexpr YakuFlags SelfWindNorth = YakuFlags{1} << 19;                   /*! 自風 北 */
inline constexpr YakuFlags RoundWindEast = YakuFlags{1} << 20;                   /*! 場風 東 */
inline constexpr YakuFlags RoundWindSouth = YakuFlags{1} << 21;                  /*! 場風 南 */
inline constexpr YakuFlags RoundWindWest = YakuFlags{1} << 22;                   /*! 場風 西 */
inline constexpr YakuFlags RoundWindNorth = YakuFlags{1} << 23;                  /*! 場風 北 */
inline constexpr YakuFlags DoubleRiichi = YakuFlags{1} << 24;                    /*! ダブル立直 */
inline constexpr YakuFlags SevenPairs = YakuFlags{1} << 25;                      /*! 七対子 */
inline constexpr YakuFlags AllTriplets = YakuFlags{1} << 26;                     /*! 対々和 */
inline constexpr YakuFlags ThreeConcealedTriplets = YakuFlags{1} << 27;          /*! 三暗刻 */
inline constexpr YakuFlags MixedTripleTriplets = YakuFlags{1} << 28;             /*! 三色同刻 */
inline constexpr YakuFlags MixedTripleSequence = YakuFlags{1} << 29;             /*! 三色同順 */
inline constexpr YakuFlags AllTerminalsAndHonors = YakuFlags{1} << 30;           /*! 混老頭 */
inline constexpr YakuFlags PureStraight = YakuFlags{1} << 31;                    /*! 一気通貫 */
inline constexpr YakuFlags HalfOutsideHand = YakuFlags{1} << 32;                 /*! 混全帯幺九 */
inline constexpr YakuFlags LittleThreeDragons = YakuFlags{1} << 33;              /*! 小三元 */
inline constexpr YakuFlags ThreeKans = YakuFlags{1} << 34;                       /*! 三槓子 */
inline constexpr YakuFlags HalfFlush = YakuFlags{1} << 35;                       /*! 混一色 */
inline constexpr YakuFlags FullyOutsideHand = YakuFlags{1} << 36;                /*! 純全帯幺九 */
inline constexpr YakuFlags TwicePureDoubleSequence = YakuFlags{1} << 37;         /*! 二盃口 */
inline constexpr YakuFlags NagashiMangan = YakuFlags{1} << 38;                   /*! 流し満貫 */
inline constexpr YakuFlags FullFlush = YakuFlags{1} << 39;                       /*! 清一色 */
inline constexpr YakuFlags HeavenlyHand = YakuFlags{1} << 40;                    /*! 天和 */
inline constexpr YakuFlags EarthlyHand = YakuFlags{1} << 41;                     /*! 地和 */
inline constexpr YakuFlags HandOfMan = YakuFlags{1} << 42;                       /*! 人和 */
inline constexpr YakuFlags AllGreen = YakuFlags{1} << 43;                        /*! 緑一色 */
inline constexpr YakuFlags BigThreeDragons = YakuFlags{1} << 44;                 /*! 大三元 */
inline constexpr YakuFlags LittleFourWinds = YakuFlags{1} << 45;                 /*! 小四喜 */
inline constexpr YakuFlags AllHonors = YakuFlags{1} << 46;                       /*! 字一色 */
inline constexpr YakuFlags ThirteenOrphans = YakuFlags{1} << 47;                 /*! 国士無双 */
inline constexpr YakuFlags NineGates = YakuFlags{1} << 48;                       /*! 九連宝燈 */
inline constexpr YakuFlags FourConcealedTriplets = YakuFlags{1} << 49;           /*! 四暗刻 */
inline constexpr YakuFlags AllTerminals = YakuFlags{1} << 50;                    /*! 清老頭 */
inline constexpr YakuFlags FourKans = YakuFlags{1} << 51;                        /*! 四槓子 */
inline constexpr YakuFlags SingleWaitFourConcealedTriplets = YakuFlags{1} << 52; /*! 四暗刻単騎 */
inline constexpr YakuFlags BigFourWinds = YakuFlags{1} << 53;                    /*! 大四喜 */
inline constexpr YakuFlags TrueNineGates = YakuFlags{1} << 54;                   /*! 純正九連宝燈 */
inline constexpr YakuFlags ThirteenWaitThirteenOrphans = YakuFlags{1} << 55;     /*! 国士無双13面待ち */
inline constexpr YakuFlags NukiDora = YakuFlags{1} << 56;                        /*! 抜きドラ */
inline constexpr int Length = 57;
// clang-format on

inline constexpr YakuFlags NormalMask =
    Tsumo | Riichi | Ippatsu | Tanyao | Pinfu | PureDoubleSequence | RobbingAKan |
    AfterAKan | UnderTheSea | UnderTheRiver | Dora | UraDora | RedDora | WhiteDragon |
    GreenDragon | RedDragon | SelfWindEast | SelfWindSouth | SelfWindWest |
    SelfWindNorth | RoundWindEast | RoundWindSouth | RoundWindWest | RoundWindNorth |
    DoubleRiichi | SevenPairs | AllTriplets | ThreeConcealedTriplets |
    MixedTripleTriplets | MixedTripleSequence | AllTerminalsAndHonors | PureStraight |
    HalfOutsideHand | LittleThreeDragons | ThreeKans | HalfFlush | FullyOutsideHand |
    TwicePureDoubleSequence | NagashiMangan | FullFlush;

inline constexpr YakuFlags YakumanMask =
    HeavenlyHand | EarthlyHand | HandOfMan | AllGreen | BigThreeDragons |
    LittleFourWinds | AllHonors | ThirteenOrphans | NineGates | FourConcealedTriplets |
    AllTerminals | FourKans | SingleWaitFourConcealedTriplets | BigFourWinds |
    TrueNineGates | ThirteenWaitThirteenOrphans;

/**
 * @brief Returns the name of a single yaku or bonus flag.
 */
inline constexpr std::string_view name(YakuFlags yaku) noexcept
{
    switch (yaku) {
    case None:
        return MAHJONG_NAME("None", "役なし");
    case Tsumo:
        return MAHJONG_NAME("Menzen Tsumo", "門前清自摸和");
    case Riichi:
        return MAHJONG_NAME("Riichi", "立直");
    case Ippatsu:
        return MAHJONG_NAME("Ippatsu", "一発");
    case Tanyao:
        return MAHJONG_NAME("All Simples", "断幺九");
    case Pinfu:
        return MAHJONG_NAME("Pinfu", "平和");
    case PureDoubleSequence:
        return MAHJONG_NAME("Pure Double Sequence", "一盃口");
    case RobbingAKan:
        return MAHJONG_NAME("Robbing a Kan", "槍槓");
    case AfterAKan:
        return MAHJONG_NAME("After a Kan", "嶺上開花");
    case UnderTheSea:
        return MAHJONG_NAME("Under the Sea", "海底摸月");
    case UnderTheRiver:
        return MAHJONG_NAME("Under the River", "河底撈魚");
    case Dora:
        return MAHJONG_NAME("Dora", "ドラ");
    case UraDora:
        return MAHJONG_NAME("Ura Dora", "裏ドラ");
    case RedDora:
        return MAHJONG_NAME("Red Dora", "赤ドラ");
    case WhiteDragon:
        return MAHJONG_NAME("White Dragon", "役牌 白");
    case GreenDragon:
        return MAHJONG_NAME("Green Dragon", "役牌 發");
    case RedDragon:
        return MAHJONG_NAME("Red Dragon", "役牌 中");
    case SelfWindEast:
        return MAHJONG_NAME("Self Wind East", "自風 東");
    case SelfWindSouth:
        return MAHJONG_NAME("Self Wind South", "自風 南");
    case SelfWindWest:
        return MAHJONG_NAME("Self Wind West", "自風 西");
    case SelfWindNorth:
        return MAHJONG_NAME("Self Wind North", "自風 北");
    case RoundWindEast:
        return MAHJONG_NAME("Round Wind East", "場風 東");
    case RoundWindSouth:
        return MAHJONG_NAME("Round Wind South", "場風 南");
    case RoundWindWest:
        return MAHJONG_NAME("Round Wind West", "場風 西");
    case RoundWindNorth:
        return MAHJONG_NAME("Round Wind North", "場風 北");
    case DoubleRiichi:
        return MAHJONG_NAME("Double Riichi", "ダブル立直");
    case SevenPairs:
        return MAHJONG_NAME("Seven Pairs", "七対子");
    case AllTriplets:
        return MAHJONG_NAME("All Triplets", "対々和");
    case ThreeConcealedTriplets:
        return MAHJONG_NAME("Three Concealed Triplets", "三暗刻");
    case MixedTripleTriplets:
        return MAHJONG_NAME("Mixed Triple Triplets", "三色同刻");
    case MixedTripleSequence:
        return MAHJONG_NAME("Mixed Triple Sequence", "三色同順");
    case AllTerminalsAndHonors:
        return MAHJONG_NAME("All Terminals and Honors", "混老頭");
    case PureStraight:
        return MAHJONG_NAME("Pure Straight", "一気通貫");
    case HalfOutsideHand:
        return MAHJONG_NAME("Half Outside Hand", "混全帯幺九");
    case LittleThreeDragons:
        return MAHJONG_NAME("Little Three Dragons", "小三元");
    case ThreeKans:
        return MAHJONG_NAME("Three Kans", "三槓子");
    case HalfFlush:
        return MAHJONG_NAME("Half Flush", "混一色");
    case FullyOutsideHand:
        return MAHJONG_NAME("Fully Outside Hand", "純全帯幺九");
    case TwicePureDoubleSequence:
        return MAHJONG_NAME("Two Pure Double Sequences", "二盃口");
    case NagashiMangan:
        return MAHJONG_NAME("Nagashi Mangan", "流し満貫");
    case FullFlush:
        return MAHJONG_NAME("Full Flush", "清一色");
    case HeavenlyHand:
        return MAHJONG_NAME("Heavenly Hand", "天和");
    case EarthlyHand:
        return MAHJONG_NAME("Earthly Hand", "地和");
    case HandOfMan:
        return MAHJONG_NAME("Hand of Man", "人和");
    case AllGreen:
        return MAHJONG_NAME("All Green", "緑一色");
    case BigThreeDragons:
        return MAHJONG_NAME("Big Three Dragons", "大三元");
    case LittleFourWinds:
        return MAHJONG_NAME("Little Four Winds", "小四喜");
    case AllHonors:
        return MAHJONG_NAME("All Honors", "字一色");
    case ThirteenOrphans:
        return MAHJONG_NAME("Thirteen Orphans", "国士無双");
    case NineGates:
        return MAHJONG_NAME("Nine Gates", "九連宝燈");
    case FourConcealedTriplets:
        return MAHJONG_NAME("Four Concealed Triplets", "四暗刻");
    case AllTerminals:
        return MAHJONG_NAME("All Terminals", "清老頭");
    case FourKans:
        return MAHJONG_NAME("Four Kans", "四槓子");
    case SingleWaitFourConcealedTriplets:
        return MAHJONG_NAME("Single-wait Four Concealed Triplets", "四暗刻単騎");
    case BigFourWinds:
        return MAHJONG_NAME("Big Four Winds", "大四喜");
    case TrueNineGates:
        return MAHJONG_NAME("True Nine Gates", "純正九連宝燈");
    case ThirteenWaitThirteenOrphans:
        return MAHJONG_NAME("Thirteen-wait Thirteen Orphans", "国士無双13面待ち");
    case NukiDora:
        return MAHJONG_NAME("Nuki Dora", "抜きドラ");
    default:
        return "Unknown";
    }
}

} // namespace Yaku

/**
 * @brief Score limit types
 */
namespace ScoreLimit
{

inline constexpr int Null = -1;
inline constexpr int Mangan = 0;           /*! 満貫 */
inline constexpr int Haneman = 1;          /*! 跳満 */
inline constexpr int Baiman = 2;           /*! 倍満 */
inline constexpr int Sanbaiman = 3;        /*! 三倍満 */
inline constexpr int CountedYakuman = 4;   /*! 数え役満 */
inline constexpr int Yakuman = 5;          /*! 役満 */
inline constexpr int DoubleYakuman = 6;    /*! ダブル役満 */
inline constexpr int TripleYakuman = 7;    /*! トリプル役満 */
inline constexpr int QuadrupleYakuman = 8; /*! 四倍役満 */
inline constexpr int QuintupleYakuman = 9; /*! 五倍役満 */
inline constexpr int SextupleYakuman = 10; /*! 六倍役満 */
inline constexpr int Length = 11;

inline constexpr std::string_view name(int type) noexcept
{
    switch (type) {
    case Null:
        return "Null";
    case Mangan:
        return MAHJONG_NAME("Mangan", "満貫");
    case Haneman:
        return MAHJONG_NAME("Haneman", "跳満");
    case Baiman:
        return MAHJONG_NAME("Baiman", "倍満");
    case Sanbaiman:
        return MAHJONG_NAME("Sanbaiman", "三倍満");
    case CountedYakuman:
        return MAHJONG_NAME("Counted Yakuman", "数え役満");
    case Yakuman:
        return MAHJONG_NAME("Yakuman", "役満");
    case DoubleYakuman:
        return MAHJONG_NAME("Double Yakuman", "ダブル役満");
    case TripleYakuman:
        return MAHJONG_NAME("Triple Yakuman", "トリプル役満");
    case QuadrupleYakuman:
        return MAHJONG_NAME("Quadruple Yakuman", "四倍役満");
    case QuintupleYakuman:
        return MAHJONG_NAME("Quintuple Yakuman", "五倍役満");
    case SextupleYakuman:
        return MAHJONG_NAME("Sextuple Yakuman", "六倍役満");
    default:
        return "Unknown";
    }
}

} // namespace ScoreLimit

/**
 * @brief Ryukyoku types
 */
namespace RyukyokuType
{

inline constexpr int Null = -1;
inline constexpr int Exhaustive = 0;    /*! 荒牌流局 */
inline constexpr int NineTerminals = 1; /*! 九種九牌 */
inline constexpr int FourWinds = 2;     /*! 四風連打 */
inline constexpr int FourRiichi = 3;    /*! 四家立直 */
inline constexpr int ThreeRon = 4;      /*! 三家和 */
inline constexpr int Length = 5;

inline constexpr std::string_view name(int type) noexcept
{
    switch (type) {
    case Null:
        return "Null";
    case Exhaustive:
        return MAHJONG_NAME("Exhaustive Draw", "荒牌流局");
    case NineTerminals:
        return MAHJONG_NAME("Nine Terminals", "九種九牌");
    case FourWinds:
        return MAHJONG_NAME("Four Winds", "四風連打");
    case FourRiichi:
        return MAHJONG_NAME("Four Riichi", "四家立直");
    case ThreeRon:
        return MAHJONG_NAME("Three Ron", "三家和");
    default:
        return "Unknown";
    }
}

} // namespace RyukyokuType

/**
 * @brief Dora conversion tables.
 */
using DoraTable = std::array<int, Tile::Length>;

inline constexpr DoraTable DoraToIndicatorYonma = {
    Tile::Manzu9, Tile::Manzu1,    Tile::Manzu2,      Tile::Manzu3,      Tile::Manzu4,
    Tile::Manzu5, Tile::Manzu6,    Tile::Manzu7,      Tile::Manzu8,      Tile::Pinzu9,
    Tile::Pinzu1, Tile::Pinzu2,    Tile::Pinzu3,      Tile::Pinzu4,      Tile::Pinzu5,
    Tile::Pinzu6, Tile::Pinzu7,    Tile::Pinzu8,      Tile::Souzu9,      Tile::Souzu1,
    Tile::Souzu2, Tile::Souzu3,    Tile::Souzu4,      Tile::Souzu5,      Tile::Souzu6,
    Tile::Souzu7, Tile::Souzu8,    Tile::North,       Tile::East,        Tile::South,
    Tile::West,   Tile::RedDragon, Tile::WhiteDragon, Tile::GreenDragon, Tile::Manzu4,
    Tile::Pinzu4, Tile::Souzu4,
};

inline constexpr DoraTable IndicatorToDoraYonma = {
    Tile::Manzu2, Tile::Manzu3,      Tile::Manzu4,    Tile::Manzu5,      Tile::Manzu6,
    Tile::Manzu7, Tile::Manzu8,      Tile::Manzu9,    Tile::Manzu1,      Tile::Pinzu2,
    Tile::Pinzu3, Tile::Pinzu4,      Tile::Pinzu5,    Tile::Pinzu6,      Tile::Pinzu7,
    Tile::Pinzu8, Tile::Pinzu9,      Tile::Pinzu1,    Tile::Souzu2,      Tile::Souzu3,
    Tile::Souzu4, Tile::Souzu5,      Tile::Souzu6,    Tile::Souzu7,      Tile::Souzu8,
    Tile::Souzu9, Tile::Souzu1,      Tile::South,     Tile::West,        Tile::North,
    Tile::East,   Tile::GreenDragon, Tile::RedDragon, Tile::WhiteDragon, Tile::Manzu6,
    Tile::Pinzu6, Tile::Souzu6,
};

// In sanma, 2m-8m and red 5m are invalid tiles. They map to Null.
// The valid manzu dora cycle is 1m <-> 9m.
inline constexpr DoraTable DoraToIndicatorSanma = {
    Tile::Manzu9, Tile::Null,      Tile::Null,        Tile::Null,        Tile::Null,
    Tile::Null,   Tile::Null,      Tile::Null,        Tile::Manzu1,      Tile::Pinzu9,
    Tile::Pinzu1, Tile::Pinzu2,    Tile::Pinzu3,      Tile::Pinzu4,      Tile::Pinzu5,
    Tile::Pinzu6, Tile::Pinzu7,    Tile::Pinzu8,      Tile::Souzu9,      Tile::Souzu1,
    Tile::Souzu2, Tile::Souzu3,    Tile::Souzu4,      Tile::Souzu5,      Tile::Souzu6,
    Tile::Souzu7, Tile::Souzu8,    Tile::North,       Tile::East,        Tile::South,
    Tile::West,   Tile::RedDragon, Tile::WhiteDragon, Tile::GreenDragon, Tile::Null,
    Tile::Pinzu4, Tile::Souzu4,
};

inline constexpr DoraTable IndicatorToDoraSanma = {
    Tile::Manzu9, Tile::Null,        Tile::Null,      Tile::Null,        Tile::Null,
    Tile::Null,   Tile::Null,        Tile::Null,      Tile::Manzu1,      Tile::Pinzu2,
    Tile::Pinzu3, Tile::Pinzu4,      Tile::Pinzu5,    Tile::Pinzu6,      Tile::Pinzu7,
    Tile::Pinzu8, Tile::Pinzu9,      Tile::Pinzu1,    Tile::Souzu2,      Tile::Souzu3,
    Tile::Souzu4, Tile::Souzu5,      Tile::Souzu6,    Tile::Souzu7,      Tile::Souzu8,
    Tile::Souzu9, Tile::Souzu1,      Tile::South,     Tile::West,        Tile::North,
    Tile::East,   Tile::GreenDragon, Tile::RedDragon, Tile::WhiteDragon, Tile::Null,
    Tile::Pinzu6, Tile::Souzu6,
};

inline constexpr int to_indicator_yonma(int dora) noexcept
{
    return DoraToIndicatorYonma[dora];
}

inline constexpr int to_indicator_sanma(int dora) noexcept
{
    return DoraToIndicatorSanma[dora];
}

inline constexpr int to_dora_yonma(int indicator) noexcept
{
    return IndicatorToDoraYonma[indicator];
}

inline constexpr int to_dora_sanma(int indicator) noexcept
{
    return IndicatorToDoraSanma[indicator];
}

inline constexpr int to_indicator(int dora, int mode) noexcept
{
    return mode == GameMode::Sanma ? DoraToIndicatorSanma[dora]
                                   : DoraToIndicatorYonma[dora];
}

inline constexpr int to_dora(int indicator, int mode) noexcept
{
    return mode == GameMode::Sanma ? IndicatorToDoraSanma[indicator]
                                   : IndicatorToDoraYonma[indicator];
}

} // namespace mahjong

#undef MAHJONG_NAME

#endif // MAHJONG_CPP_CONSTANTS
