#ifndef MAHJONG_CPP_BITUTILS
#define MAHJONG_CPP_BITUTILS

#include <bitset>
#include <iostream>
#include <vector>

namespace mahjong
{
/**
 * @brief ビット演算関係のヘルパー関数やマスクなどを定義
 * 
 */
class Bit
{
public:
    static const std::vector<int> mask;
    static const std::vector<int> hai1;
    static const std::vector<int> hai2;
    static const std::vector<int> hai3;
    static const std::vector<int> hai4;
    static const std::vector<int> ge2;

    static void print(int x)
    {
        std::cout << std::bitset<27>(x) << std::endl;
    }

    /**
     * @brief ビット列から i 要素目の牌の枚数を取得する。
     * 
     * @param[in] x ビット列
     * @param[in] i 位置
     * @return int 牌の枚数
     */
    static int get_n_tile(int x, int i)
    {
        return (x & Bit::mask[i]) >> (i * 3);
    }

    /**
     * @brief 指定した牌が2枚以上あるかどうか
     * 
     * @return 指定した牌が2枚以上ある場合は true、そうでない場合は false を返す。
     */
    static bool is_ge2(int x, int i)
    {
        return x & Bit::ge2[i];
    }

    /**
     * @brief 1枚以上の牌の種類を取得する。
     * 
     * @param[in] x ビット列
     * @return int 1枚以上の牌の種類
     */
    static int count_ge1(int x)
    {
        int cnt = x >> 2 | x >> 1 | x;
        cnt = (cnt >> 3 & 01010101) + (cnt & 01010101) + (cnt >> 24 & 1);
        cnt = (cnt >> 6 & 030003) + (cnt & 030003);
        cnt = (cnt >> 12 & 7) + (cnt & 7);

        return cnt;
    }

    /**
     * @brief 2枚以上の牌の種類を取得する。
     * 
     * @param[in] x ビット列
     * @return int 2枚以上の牌の種類
     */
    static int count_ge2(int x)
    {
        int cnt = x >> 2 | x >> 1;
        cnt = (cnt >> 3 & 01010101) + (cnt & 01010101) + (cnt >> 24 & 1);
        cnt = (cnt >> 6 & 030003) + (cnt & 030003);
        cnt = (cnt >> 12 & 7) + (cnt & 7);

        return cnt;
    }

    /**
     * @brief 牌の合計数を数える。
     * 
     * @param[in] x ビット列
     * @return int 牌の合計数
     */
    static int sum(int x)
    {
        int cnt = (x >> 3 & 07070707) + (x & 07070707);
        cnt = (cnt >> 6 & 0170017) + (cnt & 0170017);
        cnt = (cnt >> 12 & 037) + (cnt & 037);
        cnt += x >> 24;

        return cnt;
    }

    /**
     * @brief 3枚の牌の種類を取得する。
     * 
     * @param[in] x ビット列
     * @return int 3枚の牌の種類
     */
    static int count_eq3(int x)
    {
        int cnt = x ^ 0555555555;
        cnt &= cnt >> 2 & cnt >> 1;
        cnt = (cnt >> 3 & 01010101) + (cnt & 01010101) + (cnt >> 24);
        cnt = (cnt >> 6 & 030003) + (cnt & 030003);
        cnt = (cnt >> 12 & 7) + (cnt & 7);

        return cnt;
    }

    // 老頭牌のマスク
    //                              | 9 | 8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 |
    static const int RotohaiMask = 0b111'000'000'000'000'000'000'000'111;

    // 断幺九牌のマスク
    //                             | 9 | 8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 |
    static const int TanyaoMask = 0b000'111'111'111'111'111'111'111'000;

    // 三元牌のマスク
    //                                |Tyu|Hat|Hak|Pe |Sya|Nan|Ton|
    static const int SangenhaiMask = 0b111'111'111'000'000'000'000;

    // 風牌のマスク
    //                              |Tyu|Hat|Hak|Pe |Sya|Nan|Ton|
    static const int KazehaiMask = 0b000'000'000'111'111'111'111;
};

} // namespace mahjong

#endif /* MAHJONG_CPP_BITUTILS */
