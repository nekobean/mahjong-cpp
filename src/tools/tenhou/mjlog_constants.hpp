#ifndef MAHJONG_CPP_TOOLS_TENHOU_MJLOG_CONSTANTS
#define MAHJONG_CPP_TOOLS_TENHOU_MJLOG_CONSTANTS

namespace mahjong::tools::tenhou
{

enum class GameSpeed
{
    Normal,
    Fast,
};

enum class TableLevel
{
    Ippan,
    Joukyu,
    Tokujou,
    Houou,
};

enum class Rank
{
    Newcomer = 0,
    Kyu9 = 1,
    Kyu8 = 2,
    Kyu7 = 3,
    Kyu6 = 4,
    Kyu5 = 5,
    Kyu4 = 6,
    Kyu3 = 7,
    Kyu2 = 8,
    Kyu1 = 9,
    Dan1 = 10,
    Dan2 = 11,
    Dan3 = 12,
    Dan4 = 13,
    Dan5 = 14,
    Dan6 = 15,
    Dan7 = 16,
    Dan8 = 17,
    Dan9 = 18,
    Dan10 = 19,
    Tenhoui = 20,
};

enum class Gender
{
    Male,
    Female,
    Computer,
    Unknown,
};

} // namespace mahjong::tools::tenhou

#endif // MAHJONG_CPP_TOOLS_TENHOU_MJLOG_CONSTANTS
