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
        if (syanten < std::get<1>(ret)) {
            std::get<0>(ret) = SyantenType::Normal;
            std::get<1>(ret) = syanten;
        }
    }
    if (type & SyantenType::Tiitoi) {
        int syanten = calc_tiitoi(hand);
        if (syanten < std::get<1>(ret)) {
            std::get<0>(ret) = SyantenType::Tiitoi;
            std::get<1>(ret) = syanten;
        }
    }
    if (type & SyantenType::Kokusi) {
        int syanten = calc_kokusi(hand);
        if (syanten < std::get<1>(ret)) {
            std::get<0>(ret) = SyantenType::Kokusi;
            std::get<1>(ret) = syanten;
        }
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
    if (!s_tbl_.empty())
        return true; // 初期化済み

    boost::filesystem::path s_tbl_path =
        boost::dll::program_location().parent_path() / "syupai_table.txt";
    boost::filesystem::path z_tbl_path =
        boost::dll::program_location().parent_path() / "zihai_table.txt";

    // テーブルを初期化する。
    s_tbl_.resize(ShuupaiTableSize);
    z_tbl_.resize(ZihaiTableSize);

    return make_table(s_tbl_path.string(), s_tbl_) &&
           make_table(z_tbl_path.string(), z_tbl_);
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
    int n_yaochuhai = s_tbl_[manzu19].n_ge1 + s_tbl_[pinzu19].n_ge1 +
                      s_tbl_[sozu19].n_ge1 + z_tbl_[hand.zihai].n_ge1;

    // 幺九牌の対子があるかどうか
    int toitu = ((manzu19 & 0b110'000'000'000'000'000'000'000'110) |
                 (pinzu19 & 0b110'000'000'000'000'000'000'000'110) |
                 (sozu19 & 0b110'000'000'000'000'000'000'000'110) |
                 (hand.zihai & 0b110'110'110'110'110'110'110)) > 0;

    return 13 - toitu - n_yaochuhai;
}

/**
 * @brief ハッシュテーブルを作成する。
 *
 * @param[in] path データのファイルパス
 * @param[out] table テーブル
 * @return テーブルの作成に成功した場合は true、そうでない場合は false を返す。
 */
bool SyantenCalculator::make_table(const std::string &path, std::vector<Pattern> &table)
{
    std::ifstream ifs(path);
    if (!ifs) {
        spdlog::error("Failed to open {}.", path);
        return false;
    }

    std::string line;
    while (std::getline(ifs, line)) {
        // ファイルの各行は `<キー> <面子数><面子候補数><1枚以上の数><2枚以上の数><3枚以上の数><4枚以上の数>`
        // キーは数牌の場合: <牌1の数><牌2の数>...<牌9の数>
        //       字牌の場合: <東の数><南の数><西の数><北の数><白の数><発の数><中の数>00

        // ビット列に変換する。
        // 例: [0, 2, 0, 2, 2, 1, 1, 1, 4] -> 69510160 (00|000|100|001|001|001|010|010|000|010|000)
        //      牌1 牌2 ... 牌9                                牌9 牌8 牌7 牌6 牌5 牌4 牌3 牌2 牌1
        size_t hash = 0;
        for (size_t i = 9; i-- > 0;)
            hash = hash * 8 + (line[i] - '0');
        assert(table.size() > hash);

        // テーブルに格納する。
        table[hash].n_mentu      = line[10] - '0';
        table[hash].n_kouho      = line[11] - '0';
        table[hash].head         = line[12] - '0';
        table[hash].n_mentu_diff = line[13] - '0' - table[hash].n_mentu;
        table[hash].n_kouho_diff = line[14] - '0' - table[hash].n_kouho;
        table[hash].n_ge1        = line[15] - '0';
        table[hash].n_ge2        = line[16] - '0';
        table[hash].n_ge3        = line[17] - '0';
        table[hash].n_ge4        = line[18] - '0';
        table[hash].n            = Bit::sum(hash);
    }

    return true;
}

std::vector<SyantenCalculator::Pattern> SyantenCalculator::s_tbl_;
std::vector<SyantenCalculator::Pattern> SyantenCalculator::z_tbl_;
static SyantenCalculator inst;

} // namespace mahjong
