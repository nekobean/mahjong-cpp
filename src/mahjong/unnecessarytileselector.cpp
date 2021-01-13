#include "unnecessarytileselector.hpp"

#include "syanten.hpp"

namespace mahjong {

/**
 * @brief 不要牌を選択する。
 * 
 * @param[in] hand 手牌
 * @param[in] type 計算対象の向聴数の種類
 * @return std::vector<int> 牌一覧
 */
std::vector<int> UnnecessaryTileSelector::select(const Hand &hand, int type)
{
    if (type & SyantenType::Normal)
        return select_normal(hand);
    else if (type & SyantenType::Tiitoi)
        return select_tiitoi(hand);
    else
        return select_kokusi(hand);
}

std::vector<int> UnnecessaryTileSelector::select_normal(const Hand &hand)
{
    std::vector<int> tiles;
    tiles.reserve(14);
    Hand hand_after = hand; // 打牌後の手牌

    // 打牌前の向聴数を計算する。
    int syanten = SyantenCalculator::calc_normal(hand);
    if (syanten == -1)
        return {}; // 和了形

    // 打牌後に向聴数が変化しない牌を列挙する。
    for (int i = 0; i < 9; ++i) {
        if (hand.manzu & Bit::mask[i]) {
            hand_after.manzu -= Bit::tile1[i];
            int syanten_after = SyantenCalculator::calc_normal(hand_after);
            hand_after.manzu += Bit::tile1[i];

            if (syanten == syanten_after)
                tiles.push_back(i);
        }

        if (hand.pinzu & Bit::mask[i]) {
            hand_after.pinzu -= Bit::tile1[i];
            int syanten_after = SyantenCalculator::calc_normal(hand_after);
            hand_after.pinzu += Bit::tile1[i];

            if (syanten == syanten_after)
                tiles.push_back(i + Tile::Pinzu1);
        }

        if (hand.sozu & Bit::mask[i]) {
            hand_after.sozu -= Bit::tile1[i];
            int syanten_after = SyantenCalculator::calc_normal(hand_after);
            hand_after.sozu += Bit::tile1[i];

            if (syanten == syanten_after)
                tiles.push_back(i + Tile::Sozu1);
        }
    }

    for (int i = 0; i < 7; ++i) {
        if (hand.zihai & Bit::mask[i]) {
            hand_after.zihai -= Bit::tile1[i];
            int syanten_after = SyantenCalculator::calc_normal(hand_after);
            hand_after.zihai += Bit::tile1[i];

            if (syanten == syanten_after)
                tiles.push_back(i + Tile::Ton);
        }
    }

    return tiles;
}

std::vector<int> UnnecessaryTileSelector::select_tiitoi(const Hand &hand)
{
    std::vector<int> tiles;
    tiles.reserve(14);
    Hand hand_after = hand; // 打牌後の手牌

    // 打牌前の向聴数を計算する。
    int syanten = SyantenCalculator::calc_tiitoi(hand);

    // 打牌後に向聴数が変化しない牌を列挙する。
    for (int i = 0; i < 9; ++i) {
        if (hand.manzu & Bit::mask[i]) {
            hand_after.manzu -= Bit::tile1[i];
            int syanten_after = SyantenCalculator::calc_tiitoi(hand_after);
            hand_after.manzu += Bit::tile1[i];

            if (syanten == syanten_after)
                tiles.push_back(i);
        }

        if (hand.pinzu & Bit::mask[i]) {
            hand_after.pinzu -= Bit::tile1[i];
            int syanten_after = SyantenCalculator::calc_tiitoi(hand_after);
            hand_after.pinzu += Bit::tile1[i];

            if (syanten == syanten_after)
                tiles.push_back(i + Tile::Pinzu1);
        }

        if (hand.sozu & Bit::mask[i]) {
            hand_after.sozu -= Bit::tile1[i];
            int syanten_after = SyantenCalculator::calc_tiitoi(hand_after);
            hand_after.sozu += Bit::tile1[i];

            if (syanten == syanten_after)
                tiles.push_back(i + Tile::Sozu1);
        }
    }

    for (int i = 0; i < 7; ++i) {
        if (hand.zihai & Bit::mask[i]) {
            hand_after.zihai -= Bit::tile1[i];
            int syanten_after = SyantenCalculator::calc_tiitoi(hand_after);
            hand_after.zihai += Bit::tile1[i];

            if (syanten == syanten_after)
                tiles.push_back(i + Tile::Ton);
        }
    }

    return tiles;
}

std::vector<int> UnnecessaryTileSelector::select_kokusi(const Hand &hand)
{
    std::vector<int> tiles;
    tiles.reserve(14);

    auto &s_tbl = SyantenCalculator::s_tbl_;
    auto &z_tbl = SyantenCalculator::z_tbl_;

    // 幺九牌以外は不要牌
    for (int i = 1; i < 8; ++i) {
        if (hand.manzu & Bit::mask[i])
            tiles.push_back(i);

        if (hand.pinzu & Bit::mask[i])
            tiles.push_back(i + Tile::Pinzu1);

        if (hand.sozu & Bit::mask[i])
            tiles.push_back(i + Tile::Sozu1);
    }

    // 「3枚以上の幺九牌」または「対子が2個以上ある場合の幺九牌の対子」も不要牌
    int n_toitu = s_tbl[hand.manzu & Bit::RotohaiMask].n_ge2 +
                  s_tbl[hand.pinzu & Bit::RotohaiMask].n_ge2 +
                  s_tbl[hand.sozu & Bit::RotohaiMask].n_ge2 + z_tbl[hand.zihai].n_ge2;

    for (int i = 0; i < 9; i += 8) {
        Hand::key_type manzu = hand.manzu & Bit::mask[i];
        Hand::key_type pinzu = hand.pinzu & Bit::mask[i];
        Hand::key_type sozu  = hand.sozu & Bit::mask[i];

        if (manzu >= Bit::tile3[i] || (n_toitu >= 2 && manzu >= Bit::tile2[i]))
            tiles.push_back(i);

        if (pinzu >= Bit::tile3[i] || (n_toitu >= 2 && pinzu >= Bit::tile2[i]))
            tiles.push_back(i + Tile::Pinzu1);

        if (sozu >= Bit::tile3[i] || (n_toitu >= 2 && sozu >= Bit::tile2[i]))
            tiles.push_back(i + Tile::Sozu1);
    }

    for (int i = 0; i < 7; ++i) {
        Hand::key_type zihai = hand.zihai & Bit::mask[i];
        if (zihai >= Bit::tile3[i] || (n_toitu >= 2 && zihai >= Bit::tile2[i]))
            tiles.push_back(i + Tile::Ton);
    }

    return tiles;
}

} // namespace mahjong
