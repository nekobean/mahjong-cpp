#ifndef MAHJONG_CPP_SCORE_TABLE
#define MAHJONG_CPP_SCORE_TABLE

#include <array>

namespace mahjong
{
/**
 * @brief Score table
 */
namespace ScoreTable
{
/**
 * @brief Table to determine Mangan.
 */
static const inline std::array<std::array<bool, 4>, 11> IsMangan = {{
    // clang-format off
    // 1han   2han   3han   4han
    {false, false, false, false},  // 20fu
    {false, false, false, false},  // 25fu
    {false, false, false, false},  // 30fu
    {false, false, false,  true},  // 40fu
    {false, false, false,  true},  // 50fu
    {false, false, false,  true},  // 60fu
    {false, false,  true,  true},  // 70fu
    {false, false,  true,  true},  // 80fu
    {false, false,  true,  true},  // 90fu
    {false, false,  true,  true},  // 100fu
    {false, false,  true,  true},  // 110fu
    // clang-format on
}};

enum
{
    RonDiscarderToDealer,
    RonDiscarderToPlayer,
    TsumoPlayerToDealer,
    TsumoDealerToPlayer,
    TsumoPlayerToPlayer,
};

/**
 * @brief Points paid by the discarder when the dealer wins by Ron.
 */
static const inline std::array<std::array<std::array<int, 4>, 11>, 5> BelowMangan = {{
    // (Ron) discarder -> dealer
    {{
        // clang-format off
        // 1han   2han   3han   4han
        {    0,     0,     0,     0}, // 20fu (20fu is Pinfu Tsumo only)
        {    0,  2400,  4800,  9600}, // 25fu (25fu is Seven Pairs)
        { 1500,  2900,  5800, 11600}, // 30fu
        { 2000,  3900,  7700,     0}, // 40fu
        { 2400,  4800,  9600,     0}, // 50fu
        { 2900,  5800, 11600,     0}, // 60fu
        { 3400,  6800,     0,     0}, // 70fu
        { 3900,  7700,     0,     0}, // 80fu
        { 4400,  8700,     0,     0}, // 90fu
        { 4800,  9600,     0,     0}, // 100fu
        { 5300, 10600,     0,     0}, // 110fu
        // clang-format on
    }},
    // (Ron) discarder -> player
    {{
        // clang-format off
        // 1han   2han   3han   4han
        {    0,     0,     0,     0}, // 20fu (20fu is Pinfu Tsumo only)
        {    0,  1600,  3200,  6400}, // 25fu (Seven Pairs)
        { 1000,  2000,  3900,  7700}, // 30fu
        { 1300,  2600,  5200,     0}, // 40fu
        { 1600,  3200,  6400,     0}, // 50fu
        { 2000,  3900,  7700,     0}, // 60fu
        { 2300,  4500,     0,     0}, // 70fu
        { 2600,  5200,     0,     0}, // 80fu
        { 2900,  5800,     0,     0}, // 90fu
        { 3200,  6400,     0,     0}, // 100fu
        { 3600,  7100,     0,     0}, // 110fu
        // clang-format on
    }},
    // (Tsumo) player -> dealer
    {{
        // clang-format off
        // 1han   2han   3han   4han
        {    0,   700,  1300,  2600}, // 20fu (Pinfu, Tsumo)
        {    0,     0,  1600,  3200}, // 25fu (Seven Pairs, Tsumo)
        {  500,  1000,  2000,  3900}, // 30fu
        {  700,  1300,  2600,     0}, // 40fu
        {  800,  1600,  3200,     0}, // 50fu
        { 1000,  2000,  3900,     0}, // 60fu
        { 1200,  2300,     0,     0}, // 70fu
        { 1300,  2600,     0,     0}, // 80fu
        { 1500,  2900,     0,     0}, // 90fu
        { 1600,  3200,     0,     0}, // 100fu
        { 1800,  3600,     0,     0}, // 110fu
        // clang-format on
    }},
    // (Tsumo) dealer -> player
    {{
        // clang-format off
        // 1han   2han   3han   4han
        {    0,   700,  1300,  2600}, // 20fu (Pinfu, Tsumo)
        {    0,     0,  1600,  3200}, // 25fu (Seven Pairs, Tsumo)
        {  500,  1000,  2000,  3900}, // 30fu
        {  700,  1300,  2600,     0}, // 40fu
        {  800,  1600,  3200,     0}, // 50fu
        { 1000,  2000,  3900,     0}, // 60fu
        { 1200,  2300,     0,     0}, // 70fu
        { 1300,  2600,     0,     0}, // 80fu
        { 1500,  2900,     0,     0}, // 90fu
        { 1600,  3200,     0,     0}, // 100fu
        { 1800,  3600,     0,     0}, // 110fu
        // clang-format on
    }},
    // (Tsumo) player -> player
    {{
        // clang-format off
        // 1han   2han   3han   4han
        {    0,   400,   700,  1300}, // 20fu (Pinfu, Tsumo)
        {    0,     0,   800,  1600}, // 25fu (Seven Pairs, Tsumo)
        {  300,   500,  1000,  2000}, // 30fu
        {  400,   700,  1300,     0}, // 40fu
        {  400,   800,  1600,     0}, // 50fu
        {  500,  1000,  2000,     0}, // 60fu
        {  600,  1200,     0,     0}, // 70fu
        {  700,  1300,     0,     0}, // 80fu
        {  800,  1500,     0,     0}, // 90fu
        {  800,  1600,     0,     0}, // 100fu
        {  900,  800,      0,     0}, // 110fu
        // clang-format on
    }},
}};

static const inline std::array<std::array<int, 11>, 5> AboveMangan = {
    {// (Ron) discarder -> dealer
     {12000, 18000, 24000, 36000, 48000, 48000, 96000, 144000, 192000, 240000, 288000},
     // (Ron) discarder -> player
     {8000, 12000, 16000, 24000, 32000, 32000, 64000, 96000, 128000, 160000, 192000},
     // (Tsumo) player -> dealer
     {4000, 6000, 8000, 12000, 16000, 16000, 32000, 48000, 64000, 80000, 96000},
     // (Tsumo) dealer -> player
     {4000, 6000, 8000, 12000, 16000, 16000, 32000, 48000, 64000, 80000, 96000},
     // (Tsumo) player -> player
     {2000, 3000, 4000, 6000, 8000, 8000, 16000, 24000, 32000, 40000, 48000}}};
}; // namespace ScoreTable
} // namespace mahjong

#endif /* MAHJONG_CPP_SCORE_TABLE */
