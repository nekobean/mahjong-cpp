#ifndef MAHJONG_CPP_CONST_HPP
#define MAHJONG_CPP_CONST_HPP

#include <map>
#include <string>

#ifdef ENGLISH
#define ENTRY(id, eng, jp) {id, eng}
#else
#define ENTRY(id, eng, jp) {id, jp}
#endif

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
    ENTRY(Null, u8"Null", u8"Null"),
    ENTRY(Manzu1, u8"Manzu1", u8"一萬"),
    ENTRY(Manzu2, u8"Manzu2", u8"二萬"),
    ENTRY(Manzu3, u8"Manzu3", u8"三萬"),
    ENTRY(Manzu4, u8"Manzu4", u8"四萬"),
    ENTRY(Manzu5, u8"Manzu5", u8"五萬"),
    ENTRY(Manzu6, u8"Manzu6", u8"六萬"),
    ENTRY(Manzu7, u8"Manzu7", u8"七萬"),
    ENTRY(Manzu8, u8"Manzu8", u8"八萬"),
    ENTRY(Manzu9, u8"Manzu9", u8"九萬"),
    ENTRY(Pinzu1, u8"Pinzu1", u8"一筒"),
    ENTRY(Pinzu2, u8"Pinzu2", u8"二筒"),
    ENTRY(Pinzu3, u8"Pinzu3", u8"三筒"),
    ENTRY(Pinzu4, u8"Pinzu4", u8"四筒"),
    ENTRY(Pinzu5, u8"Pinzu5", u8"五筒"),
    ENTRY(Pinzu6, u8"Pinzu6", u8"六筒"),
    ENTRY(Pinzu7, u8"Pinzu7", u8"七筒"),
    ENTRY(Pinzu8, u8"Pinzu8", u8"八筒"),
    ENTRY(Pinzu9, u8"Pinzu9", u8"九筒"),
    ENTRY(Souzu1, u8"Souzu1", u8"一索"),
    ENTRY(Souzu2, u8"Souzu2", u8"二索"),
    ENTRY(Souzu3, u8"Souzu3", u8"三索"),
    ENTRY(Souzu4, u8"Souzu4", u8"四索"),
    ENTRY(Souzu5, u8"Souzu5", u8"五索"),
    ENTRY(Souzu6, u8"Souzu6", u8"六索"),
    ENTRY(Souzu7, u8"Souzu7", u8"七索"),
    ENTRY(Souzu8, u8"Souzu8", u8"八索"),
    ENTRY(Souzu9, u8"Souzu9", u8"九索"),
    ENTRY(East, u8"East", u8"東"),
    ENTRY(South, u8"South", u8"南"),
    ENTRY(West, u8"West", u8"西"),
    ENTRY(North, u8"North", u8"北"),
    ENTRY(White, u8"White", u8"白"),
    ENTRY(Green, u8"Green", u8"發"),
    ENTRY(Red, u8"Red", u8"中"),
    ENTRY(RedManzu5, u8"RedManzu5", u8"赤五萬"),
    ENTRY(RedPinzu5, u8"RedPinzu5", u8"赤五筒"),
    ENTRY(RedSouzu5, u8"RedSouzu5", u8"赤五索"),
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
    Length = 6,
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
    ENTRY(Null, "Null", "Null"), ENTRY(Myself, "Myself", "自家"),
    ENTRY(LeftPlayer, "Left Player", "上家"),
    ENTRY(OppositePlayer, "Opposite Player", "対面"),
    ENTRY(RightPlayer, "Right Player", "下家")};
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
    ENTRY(Null, "Null", "Null"),          ENTRY(Pong, "Pong", "ポン"),
    ENTRY(Chow, "Chow", "チー"),          ENTRY(ClosedKong, "Closed Kong", "暗槓"),
    ENTRY(OpenKong, "Open Kong", "明槓"), ENTRY(AddedKong, "Added Kong", "加槓")};
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
    ENTRY(Null, "Null", "Null"), ENTRY(Player0, "Player0", "プレイヤー1"),
    ENTRY(Player1, "Player1", "プレイヤー2"), ENTRY(Player2, "Player2", "プレイヤー3"),
    ENTRY(Player3, "Player3", "プレイヤー4")};
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
};

static inline const std::map<int, std::string> Name = {
    ENTRY(Null, "Null", "Null"),
    ENTRY(DoubleEdgeWait, "Double Edge Wait", "両面待ち"),
    ENTRY(EdgeWait, "Edge Wait", "辺張待ち"),
    ENTRY(ClosedWait, "Closed Wait", "嵌張待ち"),
    ENTRY(TripletWait, "Triplet Wait", "双ポン待ち"),
    ENTRY(PairWait, "Pair Wait", "単騎待ち")};

} // namespace WaitType

/**
 * @brief Rule types
 */
namespace RuleType
{
enum
{
    Null = 0,
    RedDora = 1 << 1,    /* Allow red dora (赤ドラ有り) */
    OpenTanyao = 1 << 2, /* Allow open Tanyao (喰い断有り) */
};

static inline const std::map<int, std::string> Name = {
    ENTRY(Null, "Null", "Null"), ENTRY(RedDora, "Red Dora", "赤ドラ有り"),
    ENTRY(OpenTanyao, "Open Tanyao", "喰い断有り")};
} // namespace RuleType

/**
 * @brief The winning form to calculate shanten number
 */
namespace ShantenType
{
enum
{
    Null = 0,
    Regular = 1,     /* Shanten number of regular form (一般形の向聴数) */
    Chiitoitsu = 2,  /* Shanten number of Chiitoitsu (七対子の向聴数) */
    Kokushimusou = 4 /* Shanten number of Kokushimusou (国士無双の向聴数) */
};

static inline const std::map<int, std::string> Name = {
    ENTRY(Null, "Null", "Null"),
    ENTRY(Regular, "Regular", "一般形"),
    ENTRY(Chiitoitsu, "Chiitoitsu", "七対子"),
    ENTRY(Kokushimusou, "Kokushimusou", "国士無双"),
};
}; // namespace ShantenType

#endif /* MAHJONG_CPP_CONST_HPP */
