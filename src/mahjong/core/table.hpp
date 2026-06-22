#ifndef MAHJONG_CPP_TABLE
#define MAHJONG_CPP_TABLE

#include <array>
#include <cstdint>
#include <fstream> // ifstream
#include <iterator>
#include <numeric> // accumulate
#include <string>

#include <spdlog/spdlog.h>

#include "nyanten_table.hpp"

//#define USE_NYANTEN_TABLE

namespace mahjong
{

/**
 * @brief The Table class stores data for calculating shanten number, necessary tiles, unnecessary tiles.
 *        It provides functionality to compute hash values for suits and honors tiles
 *        and access precalculated data tables based on these hash values.
 */
class Table
{
    // The table size is defined as the maximum hash value + 1.
    static constexpr size_t SuitsTableSize = 1943751;
    static constexpr size_t HonorsTableSize = 77751;
    static constexpr size_t SanmaManzuTableSize = 25;

  public:
    using TableType = std::array<int32_t, 30>;
    using HashType = int32_t;

    Table();
    template <typename ForwardIterator>
    static HashType suits_hash(ForwardIterator first, ForwardIterator last);
    static HashType sanma_manzu_hash(int manzu1_count, int manzu9_count);
    template <typename ForwardIterator>
    static HashType honors_hash(ForwardIterator first, ForwardIterator last);

  private:
    static bool initialize();
    template <size_t TableSize>
    static bool load_table(const std::string &filepath,
                           std::array<TableType, TableSize> &table);

  public:
    static std::array<TableType, SuitsTableSize> suits_table_;
    static std::array<TableType, HonorsTableSize> honors_table_;
    static std::array<TableType, SanmaManzuTableSize> sanma_manzu_table_;
};

/**
 * @brief Computes the hash value for accessing suits table.
 *
 * @tparam ForwardIterator The type of the iterator
 * @param first The beginning of the range
 * @param last The end of the range
 * @return The hash value for suits table
 */
template <typename ForwardIterator>
inline Table::HashType Table::suits_hash(ForwardIterator first, ForwardIterator last)
{
#ifdef USE_NYANTEN_TABLE
    HashType h = 0u;
    {
        std::uint_fast8_t i = 0u;
        std::uint_fast8_t n = 0u;
        while (first != last) {
            std::uint_fast8_t const c = *first++;
            n += c;
            h += nyanten_suits_table[i][n][c];
            ++i;
        }
    }
    return h;
#else
    return std::accumulate(first, last, 0, [](int x, int y) { return 5 * x + y; });
#endif
}

/**
 * @brief Computes the hash value for accessing the Sanma manzu table.
 *
 * @param manzu1_count The number of 1m tiles.
 * @param manzu9_count The number of 9m tiles.
 * @return The hash value for the Sanma manzu table.
 */
inline Table::HashType Table::sanma_manzu_hash(const int manzu1_count,
                                               const int manzu9_count)
{
    return 5 * manzu1_count + manzu9_count;
}

/**
 * @brief Computes a hash value for accessing the honors table.
 *
 * @tparam ForwardIterator The type of the iterator.
 * @param first The beginning of the range.
 * @param last The end of the range.
 * @return The hash value for the honors table.
 */
template <typename ForwardIterator>
inline Table::HashType Table::honors_hash(ForwardIterator first, ForwardIterator last)
{
#ifdef USE_NYANTEN_TABLE
    HashType h = 0u;
    {
        std::uint_fast8_t i = 0u;
        std::uint_fast8_t n = 0u;
        while (first != last) {
            std::uint_fast8_t const c = *first++;
            n += c;
            h += nyanten_honors_table[i][n][c];
            ++i;
        }
    }
    return h;
#else
    return std::accumulate(first, last, 0, [](int x, int y) { return 5 * x + y; });
#endif
}

/**
 * @brief Loads precalculated data from a file into a table.
 *
 * @tparam TableSize The size of the table.
 * @param filepath The path to the file containing the table data.
 * @param table The table to be populated with the data.
 * @return true if the table is loaded successfully; false otherwise.
 */
template <size_t TableSize>
bool Table::load_table(const std::string &filepath,
                       std::array<TableType, TableSize> &table)
{
    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        spdlog::error(u8"Failed to open table file. (path: {})", filepath);
        return false;
    }

    while (file) {
        HashType key;
        uint32_t value;
        if (!file.read(reinterpret_cast<char *>(&key), sizeof(key))) {
            break;
        }
        if (key < 0 || static_cast<size_t>(key) >= TableSize) {
            spdlog::error(u8"Invalid table key. (path: {}, key: {})", filepath, key);
            return false;
        }
        for (size_t i = 0; i < 10; ++i) {
            if (!file.read(reinterpret_cast<char *>(&value), sizeof(value))) {
                spdlog::error(u8"Failed to read table file. (path: {})", filepath);
                return false;
            }
            table[key][i] = value & 0b1111;                   // distance
            table[key][i + 10] = (value >> 4) & 0b111111111;  // wait
            table[key][i + 20] = (value >> 13) & 0b111111111; // discard
        }
    }

    spdlog::info(u8"Table file loaded. (path: {}, size: {})", filepath, table.size());

    return true;
}

} // namespace mahjong

#endif /* MAHJONG_CPP_TABLE */
