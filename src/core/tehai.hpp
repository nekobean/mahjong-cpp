#ifndef MAHJONG_CPP_TEHAI
#define MAHJONG_CPP_TEHAI

#include <spdlog/spdlog.h>

#include <string>
#include <vector>

namespace mahjong
{
/**
 * @brief 牌
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
        Souzu1,    /*! 一索 */
        Souzu2,    /*! 二索 */
        Souzu3,    /*! 三索 */
        Souzu4,    /*! 四索 */
        Souzu5,    /*! 五索 */
        Souzu6,    /*! 六索 */
        Souzu7,    /*! 七索 */
        Souzu8,    /*! 八索 */
        Souzu9,    /*! 九索 */
        Ton,       /*! 東 */
        Nan,       /*! 南 */
        Sya,       /*! 西 */
        Pei,       /*! 北 */
        Haku,      /*! 泊 */
        Hatsu,     /*! 発 */
        Tyun,      /*! 中 */
        RedManzu5, /*! 赤五萬 */
        RedPinzu5, /*! 赤五筒 */
        RedSouzu5, /*! 赤五索 */
    };

    static const int Length = 34;
    static const std::vector<std::string> KANJI_NAMES;
    static const std::vector<int> All;
    static const std::vector<int> Manzu;
    static const std::vector<int> Pinzu;
    static const std::vector<int> Souzu;
    static const std::vector<int> Routouhai;
    static const std::vector<int> Zihai;
    static const std::vector<int> Yaochuhai;
    static const std::vector<int> mask;
    static const std::vector<int> hai1;
    static const std::vector<int> hai2;
    static const std::vector<int> hai3;
    static const std::vector<int> hai4;
    static const std::vector<int> ge2;
};

/**
 * @brief 手牌
 * 
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
    /*! 手牌 */
    std::vector<int> tiles;

    /*! ビット列にした手牌
     *  例: [0, 2, 0, 2, 2, 1, 1, 1, 4] -> 69510160 (00|000|100|001|001|001|010|010|000|010|000)
     *                                                      牌9 牌8 牌7 牌6 牌5 牌4 牌3 牌2 牌1
     */
    int manzu;
    int pinzu;
    int souzu;
    int zihai;
};

} // namespace mahjong

#endif /* MAHJONG_CPP_TEHAI */
