#include "types.hpp"

namespace mahjong
{

/**
 * @brief 名前
 */
const std::vector<std::string> Tile::Names = {
    "一萬", "二萬", "三萬", "四萬", "五萬", "六萬", "七萬", "八萬", "九萬", // 萬子
    "一筒", "二筒", "三筒", "四筒", "五筒", "六筒", "七筒", "八筒", "九筒", // 筒子
    "一索", "二索", "三索", "四索", "五索", "六索", "七索", "八索", "九索", // 索子
    "東",   "南",   "西",   "北",   "白",   "發",   "中",                   // 字牌
};

/**
 * @brief 萬子 (まんず)
 */
const std::vector<int> Tile::Manzu = {Tile::Manzu1, Tile::Manzu2, Tile::Manzu3,
                                      Tile::Manzu4, Tile::Manzu5, Tile::Manzu6,
                                      Tile::Manzu7, Tile::Manzu8, Tile::Manzu9};

/**
 * @brief 筒子 (ぴんず)
 */
const std::vector<int> Tile::Pinzu = {Tile::Pinzu1, Tile::Pinzu2, Tile::Pinzu3,
                                      Tile::Pinzu4, Tile::Pinzu5, Tile::Pinzu6,
                                      Tile::Pinzu7, Tile::Pinzu8, Tile::Pinzu9};

/**
 * @brief 索子 (そーず)
 */
const std::vector<int> Tile::Sozu = {Tile::Sozu1, Tile::Sozu2, Tile::Sozu3,
                                     Tile::Sozu4, Tile::Sozu5, Tile::Sozu6,
                                     Tile::Sozu7, Tile::Sozu8, Tile::Sozu9};

/**
 * @brief 字牌 (じはい)
 */
const std::vector<int> Tile::ZiHai{
    Tile::Ton, Tile::Nan, Tile::Sya, Tile::Pe, Tile::Haku, Tile::Hatsu, Tile::Tyun,
};

/**
 * @brief 老頭牌 (ろうとうはい)
 */
const std::vector<int> Tile::RotoHai = {Tile::Manzu1, Tile::Pinzu1, Tile::Sozu1,
                                        Tile::Manzu9, Tile::Pinzu9, Tile::Sozu9};

/**
 * @brief 幺九牌 (やおちゅーはい)
)
 */
const std::vector<int> Tile::YaotyuHai = {
    Tile::Manzu1, Tile::Pinzu1, Tile::Sozu1, Tile::Manzu9, Tile::Pinzu9, Tile::Sozu9, Tile::Ton,
    Tile::Nan,    Tile::Sya,    Tile::Pe,    Tile::Haku,   Tile::Hatsu,  Tile::Tyun,
};

/**
 * @brief 断幺九牌 (たんやおちゅーはい)
 */
const std::vector<int> Tile::TanyaoHai = {
    Tile::Manzu2, Tile::Manzu3, Tile::Manzu4, Tile::Manzu5, Tile::Manzu6, Tile::Manzu7,
    Tile::Manzu8, Tile::Pinzu2, Tile::Pinzu3, Tile::Pinzu4, Tile::Pinzu5, Tile::Pinzu6,
    Tile::Pinzu7, Tile::Pinzu8, Tile::Sozu2,  Tile::Sozu3,  Tile::Sozu4,  Tile::Sozu5,
    Tile::Sozu6,  Tile::Sozu7,  Tile::Sozu8};

/**
 * @brief 役牌 (やくはい)
 */
const std::vector<int> Tile::YakuHai = {Tile::Haku, Tile::Hatsu, Tile::Tyun};

/**
 * @brief 名前
 */
const std::vector<std::string> Yaku::Name = {"門前清自摸和", "立直",         "一発",
                                             "断幺九",       "平和",         "一盃口",
                                             "槍槓",         "嶺上開花",     "海底摸月",
                                             "河底撈魚",     "ドラ",         "裏ドラ",
                                             "赤ドラ",       "三元牌",       "三元牌",
                                             "三元牌",       "自風",         "自風",
                                             "自風",         "自風",         "場風",
                                             "場風",         "場風",         "場風",
                                             "W 立直",       "七対子",       "対々和",
                                             "三暗刻",       "三色同刻",     "三色同順",
                                             "混老頭",       "一気通貫",     "混全帯幺",
                                             "小三元",       "三槓子",       "混一色",
                                             "混全帯么九",   "二盃口",       "流し満貫",
                                             "清一色",       "天和",         "地和",
                                             "人和",         "緑一色",       "大三元",
                                             "小四喜和",     "字一色",       "国士無双",
                                             "九連宝燈",     "四暗刻",       "清老頭",
                                             "四槓子",       "大車輪",       "四暗刻単騎",
                                             "大四喜",       "純正九連宝燈", "国士無双13面待ち"};

} // namespace mahjong