#include "syanten.hpp"

#include <boost/dll.hpp>
#include <spdlog/spdlog.h>

#include <fstream>

#include "bitutils.hpp"

namespace mahjong {

SyantenCalculator::SyantenCalculator()
{
    initialize();
}

/**
 * @brief 向聴数を計算する。
 *
 * @param[in] hand 手牌
 * @param[in] type 計算対象の向聴数の種類
 * @return std::tuple<SyantenType, int> (向聴数の種類, 向聴数)
 */
std::tuple<int, int> SyantenCalculator::calc(const Hand &hand, int type)
{
#ifdef CHECK_ARGUMENT
    if (type < 0 || type > 7) {
        spdlog::warn("Invalid type {} passed.", type);
        return -2;
    }
#endif

    std::tuple<int, int> ret = {SyantenType::Null, std::numeric_limits<int>::max()};

    if (type & SyantenType::Normal) {
        int syanten = calc_normal(hand);
        if (syanten < std::get<1>(ret))
            ret = {SyantenType::Normal, syanten};
    }
    if (type & SyantenType::Tiitoi) {
        int syanten = calc_tiitoi(hand);
        if (syanten < std::get<1>(ret))
            ret = {SyantenType::Tiitoi, syanten};
    }
    if (type & SyantenType::Kokusi) {
        int syanten = calc_kokusi(hand);
        if (syanten < std::get<1>(ret))
            ret = {SyantenType::Kokusi, syanten};
    }

    return ret;
}

/**
 * @brief 初期化する。
 * 
 * @return 初期化に成功した場合は true、そうでない場合は false を返す。
 */
bool SyantenCalculator::initialize()
{
    boost::filesystem::path exe_path = boost::dll::program_location().parent_path();

#ifdef USE_UNORDERED_MAP
    if (s_tbl_.empty()) {
        boost::filesystem::path path = exe_path / "syupai_table.bin";
        std::ifstream ifs(path.string(), std::ios::binary);
        for (size_t i = 0; i < ShuupaiPatternSize; ++i) {
            unsigned int key;
            Pattern pattern;
            ifs.read((char *)&key, sizeof(unsigned int));
            ifs.read((char *)&pattern, sizeof(Pattern));

            s_tbl_[key] = pattern;
        }
    }

    if (z_tbl_.empty()) {
        boost::filesystem::path path = exe_path / "zihai_table.bin";
        std::ifstream ifs(path.string(), std::ios::binary);

        for (size_t i = 0; i < ZihaiPatternSize; ++i) {
            unsigned int key;
            Pattern pattern;
            ifs.read((char *)&key, sizeof(unsigned int));
            ifs.read((char *)&pattern, sizeof(Pattern));

            z_tbl_[key] = pattern;
        }
    }
#else
    if (s_tbl_.empty()) {
        s_tbl_.resize(ShuupaiTableSize);

        boost::filesystem::path path = exe_path / "syupai_table.bin";
        std::ifstream ifs(path.string(), std::ios::binary);
        for (size_t i = 0; i < ShuupaiPatternSize; ++i) {
            unsigned int key;
            ifs.read((char *)&key, sizeof(unsigned int));
            ifs.read((char *)&s_tbl_[key], sizeof(Pattern));
        }
    }

    if (z_tbl_.empty()) {
        z_tbl_.resize(ZihaiTableSize);

        boost::filesystem::path path = exe_path / "zihai_table.bin";
        std::ifstream ifs(path.string(), std::ios::binary);

        for (size_t i = 0; i < ZihaiPatternSize; ++i) {
            unsigned int key;
            ifs.read((char *)&key, sizeof(unsigned int));
            ifs.read((char *)&z_tbl_[key], sizeof(Pattern));
        }
    }
#endif

    return true;
}

/**
 * @brief 通常手の向聴数を計算する。
 *
 * @param[in] hand 手牌
 * @return int 向聴数
 */
int SyantenCalculator::calc_normal(const Hand &hand)
{
    // 制約条件「面子 + 候補 <= 4」で「面子数 * 2 + 候補」の最大値を計算する。
    int n_melds      = int(hand.melded_blocks.size());
    int n_mentu_base = n_melds + s_tbl_[hand.manzu].n_mentu +
                       s_tbl_[hand.pinzu].n_mentu + s_tbl_[hand.sozu].n_mentu +
                       z_tbl_[hand.zihai].n_mentu;
    int n_kouho_base = s_tbl_[hand.manzu].n_kouho + s_tbl_[hand.pinzu].n_kouho +
                       s_tbl_[hand.sozu].n_kouho + z_tbl_[hand.zihai].n_kouho;

    // 雀頭なし
    int max = n_mentu_base * 2 + std::min(4 - n_mentu_base, n_kouho_base);

    if (s_tbl_[hand.manzu].head) {
        // 萬子の雀頭有り
        int n_mentu = n_mentu_base + s_tbl_[hand.manzu].n_mentu_diff;
        int n_kouho = n_kouho_base + s_tbl_[hand.manzu].n_kouho_diff;
        max         = std::max(max, n_mentu * 2 + std::min(4 - n_mentu, n_kouho) + 1);
    }

    if (s_tbl_[hand.pinzu].head) {
        // 筒子の雀頭有り
        int n_mentu = n_mentu_base + s_tbl_[hand.pinzu].n_mentu_diff;
        int n_kouho = n_kouho_base + s_tbl_[hand.pinzu].n_kouho_diff;
        max         = std::max(max, n_mentu * 2 + std::min(4 - n_mentu, n_kouho) + 1);
    }

    if (s_tbl_[hand.sozu].head) {
        // 索子の雀頭有り
        int n_mentu = n_mentu_base + s_tbl_[hand.sozu].n_mentu_diff;
        int n_kouho = n_kouho_base + s_tbl_[hand.sozu].n_kouho_diff;
        max         = std::max(max, n_mentu * 2 + std::min(4 - n_mentu, n_kouho) + 1);
    }

    if (z_tbl_[hand.zihai].head) {
        // 字牌の雀頭有り
        int n_mentu = n_mentu_base + z_tbl_[hand.zihai].n_mentu_diff;
        int n_kouho = n_kouho_base + z_tbl_[hand.zihai].n_kouho_diff;
        max         = std::max(max, n_mentu * 2 + std::min(4 - n_mentu, n_kouho) + 1);
    }

    return 8 - max;
}

/**
 * @brief 七対子手の向聴数を計算する。
 *
 * @param[in] hand 手牌
 * @return int 向聴数
 */
int SyantenCalculator::calc_tiitoi(const Hand &hand)
{
    // 牌の種類 (1枚以上の牌) を数える。
    int n_types = s_tbl_[hand.manzu].n_ge1 + s_tbl_[hand.pinzu].n_ge1 +
                  s_tbl_[hand.sozu].n_ge1 + z_tbl_[hand.zihai].n_ge1;
    // 対子の数 (2枚以上の牌) を数える。
    int n_toitu = s_tbl_[hand.manzu].n_ge2 + s_tbl_[hand.pinzu].n_ge2 +
                  s_tbl_[hand.sozu].n_ge2 + z_tbl_[hand.zihai].n_ge2;

    int syanten = 6 - n_toitu;
    if (n_types < 7)
        syanten += 7 - n_types; // 4枚持ちを考慮

    return syanten;
}

/**
 * @brief 国士無双の向聴数を計算する。
 *
 * @param[in] hand 手牌
 * @return int 向聴数
 */
int SyantenCalculator::calc_kokusi(const Hand &hand)
{
    // 老頭牌を抽出する。
    int manzu19 = hand.manzu & Bit::RotohaiMask;
    int pinzu19 = hand.pinzu & Bit::RotohaiMask;
    int sozu19  = hand.sozu & Bit::RotohaiMask;

    // 幺九牌の種類 (1枚以上の牌) を数える。
    int n_yaotyuhai = s_tbl_[manzu19].n_ge1 + s_tbl_[pinzu19].n_ge1 +
                      s_tbl_[sozu19].n_ge1 + z_tbl_[hand.zihai].n_ge1;

    // 幺九牌の対子があるかどうか
    int toitu = ((manzu19 & 0b110'000'000'000'000'000'000'000'110) |
                 (pinzu19 & 0b110'000'000'000'000'000'000'000'110) |
                 (sozu19 & 0b110'000'000'000'000'000'000'000'110) |
                 (hand.zihai & 0b110'110'110'110'110'110'110)) > 0;

    return 13 - toitu - n_yaotyuhai;
}

#ifdef USE_UNORDERED_MAP
std::unordered_map<unsigned int, SyantenCalculator::Pattern> SyantenCalculator::s_tbl_;
std::unordered_map<unsigned int, SyantenCalculator::Pattern> SyantenCalculator::z_tbl_;
#else
std::vector<SyantenCalculator::Pattern> SyantenCalculator::s_tbl_;
std::vector<SyantenCalculator::Pattern> SyantenCalculator::z_tbl_;
#endif

static SyantenCalculator inst;

} // namespace mahjong
