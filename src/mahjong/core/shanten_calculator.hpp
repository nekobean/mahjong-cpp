#ifndef MAHJONG_CPP_SHANTEN_CALCULATOR2
#define MAHJONG_CPP_SHANTEN_CALCULATOR2

#include <array>
#include <cstdint>
#include <tuple>

#include "mahjong/types/types.hpp"
#include "nyanten_table.hpp"

namespace mahjong
{
//#define NYANTEN

/**
 * @brief Types of Shanten numbers
 */
namespace ShantenType
{
enum
{
    Null = 0,
    Regular = 1,     /* Shanten number for regular form (一般形の向聴数) */
    Chiitoitsu = 2,  /* Shanten number for Chiitoitsu (七対子の向聴数) */
    Kokushimusou = 4 /* Shanten number for Kokushimusou (国士無双の向聴数) */
};

static inline std::map<int, std::string> Name = {
    {ShantenType::Regular, "通常"},
    {ShantenType::Chiitoitsu, "七対子"},
    {ShantenType::Kokushimusou, "国士無双"},
};
}; // namespace ShantenType

/**
 * @brief Class for calculating Shanten number
 */
class ShantenCalculator
{
    // The table size is the maximum hash value + 1
    static const size_t SuitsTableSize = 1943751;
    static const size_t HonorsTableSize = 77751;
    using ResultType = std::array<int32_t, 30>;

  public:
    using TableType = std::array<int32_t, 30>;
    using HashType = uint32_t;

    ShantenCalculator();
    static std::tuple<int, int>
    calc(const Hand &hand, int type = ShantenType::Regular | ShantenType::Chiitoitsu |
                                      ShantenType::Kokushimusou);
    static bool initialize();
    static int calc_regular(const Hand &hand);
    static int calc_chiitoitsu(const Hand &hand);
    static int calc_kokushimusou(const Hand &hand);
    template <typename ForwardIterator>
    static HashType calc_suits_hash(ForwardIterator first, ForwardIterator last);
    template <typename ForwardIterator>
    static HashType calc_honors_hash(ForwardIterator first, ForwardIterator last);

  private:
    template <size_t TableSize>
    static bool load_table(const std::string &filepath,
                           std::array<TableType, TableSize> &table);
    static void add1(ResultType &lhs, const TableType &rhs, const int m);
    static void add2(ResultType &lhs, const TableType &rhs, const int m);

  public:
    static std::array<TableType, SuitsTableSize> suits_table_;
    static std::array<TableType, HonorsTableSize> honors_table_;
};

/**
 * @brief Load precalulated data into the table.
 *
 * @tparam TableSize The size of the table
 * @param filepath The path to the file
 * @param table The table to load data into
 * @return true if loading is successful, otherwise false
 */
template <size_t TableSize>
bool ShantenCalculator::load_table(const std::string &filepath,
                                   std::array<TableType, TableSize> &table)
{
    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        spdlog::error("Failed to open table file. (path: {})", filepath);
        return false;
    }

    do {
        HashType key;
        uint32_t value;
        file.read(reinterpret_cast<char *>(&key), sizeof(key));
        for (size_t i = 0; i < 10; ++i) {
            file.read(reinterpret_cast<char *>(&value), sizeof(value));
            table[key][i] = value & 0b1111;
            table[key][i + 10] = (value >> 4) & 0b111111111;
            table[key][i + 20] = (value >> 13) & 0b111111111;
        }
    } while (!file.eof());

    spdlog::info("Table file loaded. (path: {})", filepath);

    return true;
}

/**
 * @brief Calculate the hash value for suits table.
 *
 * @tparam ForwardIterator The type of the iterator
 * @param first The beginning of the range
 * @param last The end of the range
 * @return The hash value for suits
 */
template <typename ForwardIterator>
inline ShantenCalculator::HashType
ShantenCalculator::calc_suits_hash(ForwardIterator first, ForwardIterator last)
{
#ifdef NYANTEN
    HashType h = 0u;
    {
        std::uint_fast8_t i = 0u;
        std::uint_fast8_t n = 0u;
        while (first != last) {
            std::uint_fast8_t const c = *first++;
            n += c;
            h += shupai_table[i][n][c];
            ++i;
        }
    }
    return h;
#else
    return std::accumulate(first, last, 0, [](int x, int y) { return 5 * x + y; });
#endif
}

/**
 * @brief Calculate the hash value for honors table.
 *
 * @tparam ForwardIterator The type of the iterator
 * @param first The beginning of the range
 * @param last The end of the range
 * @return The hash value for suits
 */
template <typename ForwardIterator>
inline ShantenCalculator::HashType
ShantenCalculator::calc_honors_hash(ForwardIterator first, ForwardIterator last)
{
#ifdef NYANTEN
    HashType h = 0u;
    {
        std::uint_fast8_t i = 0u;
        std::uint_fast8_t n = 0u;
        while (first != last) {
            std::uint_fast8_t const c = *first++;
            n += c;
            h += zipai_table[i][n][c];
            ++i;
        }
    }
    return h;
#else
    return std::accumulate(first, last, 0, [](int x, int y) { return 5 * x + y; });
#endif
}

} // namespace mahjong

#endif /* MAHJONG_CPP_SHANTEN_CALCULATOR */