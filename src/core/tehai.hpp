#ifndef MAHJONG_CPP_TEHAI
#define MAHJONG_CPP_TEHAI

#include <spdlog/spdlog.h>

#include <string>
#include <vector>

#include "bitutils.hpp"
#include "types.hpp"

namespace mahjong
{

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
    std::vector<int> tiles;

    /*! ビット列にした手牌
     *  例: [0, 2, 0, 2, 2, 1, 1, 1, 4] -> 69510160 (00|000|100|001|001|001|010|010|000|010|000)
     *                                                      牌9 牌8 牌7 牌6 牌5 牌4 牌3 牌2 牌1
     */
    int manzu;
    int pinzu;
    int sozu;
    int zihai;
};

} // namespace mahjong

#endif /* MAHJONG_CPP_TEHAI */
