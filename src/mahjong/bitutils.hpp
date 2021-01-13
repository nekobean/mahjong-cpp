#ifndef MAHJONG_CPP_BITUTILS
#define MAHJONG_CPP_BITUTILS

#include <bitset>
#include <iostream>
#include <string>
#include <vector>

namespace mahjong {
/**
 * @brief ビット演算関係のヘルパー関数やマスクなどを定義
 * 
 */
class Bit {
    using key_type = unsigned int;

public:
    static inline const std::vector<key_type> mask = {
        7,       7 << 3,  7 << 6,  7 << 9,  7 << 12, 7 << 15, 7 << 18, 7 << 21,
        7 << 24, 7,       7 << 3,  7 << 6,  7 << 9,  7 << 12, 7 << 15, 7 << 18,
        7 << 21, 7 << 24, 7,       7 << 3,  7 << 6,  7 << 9,  7 << 12, 7 << 15,
        7 << 18, 7 << 21, 7 << 24, 7,       7 << 3,  7 << 6,  7 << 9,  7 << 12,
        7 << 15, 7 << 18, 7 << 12, 7 << 12, 7 << 12,
    };

    static inline const std::vector<key_type> tile1 = {
        1,       1 << 3,  1 << 6,  1 << 9,  1 << 12, 1 << 15, 1 << 18, 1 << 21,
        1 << 24, 1,       1 << 3,  1 << 6,  1 << 9,  1 << 12, 1 << 15, 1 << 18,
        1 << 21, 1 << 24, 1,       1 << 3,  1 << 6,  1 << 9,  1 << 12, 1 << 15,
        1 << 18, 1 << 21, 1 << 24, 1,       1 << 3,  1 << 6,  1 << 9,  1 << 12,
        1 << 15, 1 << 18, 1 << 12, 1 << 12, 1 << 12,
    };

    static inline const std::vector<key_type> tile2 = {
        2,       2 << 3,  2 << 6,  2 << 9,  2 << 12, 2 << 15, 2 << 18, 2 << 21,
        2 << 24, 2,       2 << 3,  2 << 6,  2 << 9,  2 << 12, 2 << 15, 2 << 18,
        2 << 21, 2 << 24, 2,       2 << 3,  2 << 6,  2 << 9,  2 << 12, 2 << 15,
        2 << 18, 2 << 21, 2 << 24, 2,       2 << 3,  2 << 6,  2 << 9,  2 << 12,
        2 << 15, 2 << 18, 2 << 12, 2 << 12, 2 << 12,
    };

    static inline const std::vector<key_type> tile3 = {
        3,       3 << 3,  3 << 6,  3 << 9,  3 << 12, 3 << 15, 3 << 18, 3 << 21,
        3 << 24, 3,       3 << 3,  3 << 6,  3 << 9,  3 << 12, 3 << 15, 3 << 18,
        3 << 21, 3 << 24, 3,       3 << 3,  3 << 6,  3 << 9,  3 << 12, 3 << 15,
        3 << 18, 3 << 21, 3 << 24, 3,       3 << 3,  3 << 6,  3 << 9,  3 << 12,
        3 << 15, 3 << 18, 3 << 12, 3 << 12, 3 << 12,
    };

    static inline const std::vector<key_type> tile4 = {
        4,       4 << 3,  4 << 6,  4 << 9,  4 << 12, 4 << 15, 4 << 18, 4 << 21,
        4 << 24, 4,       4 << 3,  4 << 6,  4 << 9,  4 << 12, 4 << 15, 4 << 18,
        4 << 21, 4 << 24, 4,       4 << 3,  4 << 6,  4 << 9,  4 << 12, 4 << 15,
        4 << 18, 4 << 21, 4 << 24, 4,       4 << 3,  4 << 6,  4 << 9,  4 << 12,
        4 << 15, 4 << 18, 4 << 21, 4 << 24, 4 << 12, 4 << 12, 4 << 12,
    };

    static void print_2digits(key_type x)
    {
        std::cout << std::bitset<27>(x) << std::endl;
    }

    static std::string to_10digits(key_type x)
    {
        std::string s;
        for (int i = 0; i < 9; ++i)
            s += std::to_string(Bit::num_tiles(x, i));

        return s;
    }

    /**
     * @brief ビット列から i 要素目の牌の枚数を取得する。
     * 
     * @param[in] x ビット列
     * @param[in] i 位置
     * @return int 牌の枚数
     */
    static int num_tiles(key_type x, int i)
    {
        return (x & Bit::mask[i]) >> (i * 3);
    }

    /**
     * @brief 牌の合計数を数える。
     * 
     * @param[in] x ビット列
     * @return int 牌の合計数
     */
    static int sum(key_type x)
    {
        int cnt = (x >> 3 & 07070707) + (x & 07070707);
        cnt     = (cnt >> 6 & 0170017) + (cnt & 0170017);
        cnt     = (cnt >> 12 & 037) + (cnt & 037);
        cnt += x >> 24;

        return cnt;
    }

    static bool check_exclusive(unsigned long long x)
    {
        return !x || !(x & (x - 1));
    }

    // 老頭牌のマスク
    //                                  | 9 | 8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 |
    static const key_type RotohaiMask = 0b111'000'000'000'000'000'000'000'111;

    // 断幺九牌のマスク
    //                                 | 9 | 8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 |
    static const key_type TanyaoMask = 0b000'111'111'111'111'111'111'111'000;

    // 三元牌のマスク
    //                                    |Tyu|Hat|Hak|Pe |Sya|Nan|Ton|
    static const key_type SangenhaiMask = 0b111'111'111'000'000'000'000;

    // 風牌のマスク
    //                                  |Tyu|Hat|Hak|Pe |Sya|Nan|Ton|
    static const key_type KazehaiMask = 0b000'000'000'111'111'111'111;
};

} // namespace mahjong

#endif /* MAHJONG_CPP_BITUTILS */
