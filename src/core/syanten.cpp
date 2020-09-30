#include "syanten.hpp"

#include <boost/dll.hpp>
#include <spdlog/spdlog.h>

#include <chrono>
#include <fstream>

#include "bitutils.hpp"

namespace mahjong
{

std::vector<SyantenCalculator::Pattern> SyantenCalculator::s_tbl_;
std::vector<SyantenCalculator::Pattern> SyantenCalculator::z_tbl_;

/**
 * @brief 向聴数を計算する。
 *
 * @param[in] tehai 手牌
 * @param[in] n_fuuro 副露数 (0 ~ 4)
 * @param[in] type 計算対象の向聴数の種類
 * @return int 向聴数
 */
int SyantenCalculator::calc(const Tehai &tehai, int n_fuuro, int type)
{
    if (SyantenCalculator::s_tbl_.empty())
        initialize();

#ifdef CHECK_ARGUMENT
    if (type <= 0 || type >= 9) {
        spdlog::warn("Invalid type {} passed.", type);
        return -2;
    }

    if (n_fuuro < 0 || n_fuuro >= 5) {
        spdlog::warn("Invalid n_fuuro {} passed.", n_fuuro);
        return -2;
    }
#endif

    int ret = std::numeric_limits<int>::max();
    if (type & SyantenType::Normal)
        ret = std::min(ret, calc_normal(tehai, n_fuuro));
    if (type & SyantenType::Tiitoi)
        ret = std::min(ret, calc_tiitoi(tehai));
    if (type & SyantenType::Kokushi)
        ret = std::min(ret, calc_kokusi(tehai));

    return ret;
}

/**
 * @brief 初期化する。
 * 
 * @return 初期化に成功した場合は true、そうでない場合は false を返す。
 */
bool SyantenCalculator::initialize()
{
    boost::filesystem::path s_tbl_path =
        boost::dll::program_location().parent_path() / "shuupai_table.txt";
    boost::filesystem::path z_tbl_path =
        boost::dll::program_location().parent_path() / "zihai_table.txt";

    // テーブルを初期化する。
    s_tbl_.resize(ShuupaiTableSize);
    z_tbl_.resize(ZihaiTableSize);

    return make_table(s_tbl_path.string(), s_tbl_) && make_table(z_tbl_path.string(), z_tbl_);
}

/**
 * @brief 通常手の向聴数を計算する。
 *
 * @param[in] tehai 手牌
 * @param[in] n_fuuro 副露数
 * @return int 向聴数
 */
int SyantenCalculator::calc_normal(const Tehai &tehai, int n_fuuro)
{
    // 面子数 * 2 + 候補, 面子 + 候補 <= 4 の最大値を計算する。

    int n_mentsu = n_fuuro + s_tbl_[tehai.manzu].n_mentsu + s_tbl_[tehai.pinzu].n_mentsu +
                   s_tbl_[tehai.sozu].n_mentsu + z_tbl_[tehai.zihai].n_mentsu;
    int n_kouho = s_tbl_[tehai.manzu].n_kouho + s_tbl_[tehai.pinzu].n_kouho +
                  s_tbl_[tehai.sozu].n_kouho + z_tbl_[tehai.zihai].n_kouho;

    // 雀頭なしと仮定して計算
    int max = n_mentsu * 2 + std::min(4 - n_mentsu, n_kouho);

    for (size_t i = 0; i < 9; ++i) {
        if ((tehai.manzu & Bit::mask[i]) >= Bit::hai2[i]) {
            // 萬子 i を雀頭とした場合
            int manzu = tehai.manzu - Bit::hai2[i]; // 雀頭とした2枚を手牌から減らす
            // 雀頭2枚を抜いた結果、変化する部分は萬子の面子数、面子候補数だけなので、以下のように差分だけ調整する。
            // (m + p + s + z + f) + (m' - m) = m' + p + s + z + f
            int n_mentsu2 = n_mentsu + s_tbl_[manzu].n_mentsu - s_tbl_[tehai.manzu].n_mentsu;
            int n_kouho2 = n_kouho + s_tbl_[manzu].n_kouho - s_tbl_[tehai.manzu].n_kouho;
            // +1 は雀頭分
            max = std::max(max, n_mentsu2 * 2 + std::min(4 - n_mentsu2, n_kouho2) + 1);
        }

        if ((tehai.pinzu & Bit::mask[i]) >= Bit::hai2[i]) {
            // 筒子 i を雀頭とした場合
            int pinzu = tehai.pinzu - Bit::hai2[i];
            int n_mentsu2 = n_mentsu + s_tbl_[pinzu].n_mentsu - s_tbl_[tehai.pinzu].n_mentsu;
            int n_kouho2 = n_kouho + s_tbl_[pinzu].n_kouho - s_tbl_[tehai.pinzu].n_kouho;

            max = std::max(max, n_mentsu2 * 2 + std::min(4 - n_mentsu2, n_kouho2) + 1);
        }

        if ((tehai.sozu & Bit::mask[i]) >= Bit::hai2[i]) {
            // 索子 i を雀頭とした場合
            int sozu = tehai.sozu - Bit::hai2[i];
            int n_mentsu2 = n_mentsu + s_tbl_[sozu].n_mentsu - s_tbl_[tehai.sozu].n_mentsu;
            int n_kouho2 = n_kouho + s_tbl_[sozu].n_kouho - s_tbl_[tehai.sozu].n_kouho;

            max = std::max(max, n_mentsu2 * 2 + std::min(4 - n_mentsu2, n_kouho2) + 1);
        }
    }

    for (size_t i = 0; i < 7; ++i) {
        // 字牌 i を雀頭とした場合
        if ((tehai.zihai & Bit::mask[i]) >= Bit::hai2[i]) {
            int zihai = tehai.zihai - Bit::hai2[i];

            int n_mentsu2 = n_mentsu + (z_tbl_[zihai].n_mentsu - z_tbl_[tehai.zihai].n_mentsu);
            int n_kouho2 = n_kouho + (z_tbl_[zihai].n_kouho - z_tbl_[tehai.zihai].n_kouho);

            max = std::max(max, n_mentsu2 * 2 + std::min(4 - n_mentsu2, n_kouho2) + 1);
        }
    }

    return 8 - max;
}

/**
 * @brief 七対子手の向聴数を計算する。
 *
 * @param[in] tehai 手牌
 * @return int 向聴数
 */
int SyantenCalculator::calc_tiitoi(const Tehai &tehai)
{
    // 牌の種類 (1枚以上の牌) を数える。
    int n_types = s_tbl_[tehai.manzu].n_ge1 + s_tbl_[tehai.pinzu].n_ge1 + s_tbl_[tehai.sozu].n_ge1 +
                  z_tbl_[tehai.zihai].n_ge1;
    // 対子の数 (2枚以上の牌) を数える。
    int n_toitsu = s_tbl_[tehai.manzu].n_ge2 + s_tbl_[tehai.pinzu].n_ge2 +
                   s_tbl_[tehai.sozu].n_ge2 + z_tbl_[tehai.zihai].n_ge2;

    int syanten = 6 - n_toitsu;
    if (n_types < 7)
        syanten += 7 - n_types; // 4枚持ちを考慮

    return syanten;
}

/**
 * @brief 国士無双の向聴数を計算する。
 *
 * @param[in] tehai 手牌
 * @return int 向聴数
 */
int SyantenCalculator::calc_kokusi(const Tehai &tehai)
{
    // 老頭牌を抽出する。
    int manzu19 = tehai.manzu & Bit::RotohaiMask;
    int pinzu19 = tehai.pinzu & Bit::RotohaiMask;
    int sozu19 = tehai.sozu & Bit::RotohaiMask;

    // 幺九牌の種類 (1枚以上の牌) を数える。
    int n_yaochuhai = s_tbl_[manzu19].n_ge1 + s_tbl_[pinzu19].n_ge1 + s_tbl_[sozu19].n_ge1 +
                      z_tbl_[tehai.zihai].n_ge1;

    // 幺九牌の対子があるかどうか
    int toitsu_flag = ((manzu19 & 0b110'000'000'000'000'000'000'000'110) |
                       (pinzu19 & 0b110'000'000'000'000'000'000'000'110) |
                       (sozu19 & 0b110'000'000'000'000'000'000'000'110) |
                       (tehai.zihai & 0b110'110'110'110'110'110'110)) > 0;

    return 13 - toitsu_flag - n_yaochuhai;
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
        // ファイルの各行は `<キー> <面子数> <面子候補数>` (例: `000000022 0 2`)
        // キーは数牌の場合: <牌1の数><牌2の数>...<牌9の数>
        //       字牌の場合: <東の数><南の数><西の数><北の数><白の数><発の数><中の数>00

        // ビット列に変換する。
        // 例: [0, 2, 0, 2, 2, 1, 1, 1, 4] -> 69510160 (00|000|100|001|001|001|010|010|000|010|000)
        //                                                     牌9 牌8 牌7 牌6 牌5 牌4 牌3 牌2 牌1
        size_t hash = 0;
        for (size_t i = 9; i-- > 0;)
            hash = hash * 8 + (line[i] - '0');
        assert(table.size() > hash);

        // テーブルに格納する。
        table[hash].n_mentsu = line[10] - '0';
        table[hash].n_kouho = line[11] - '0';
        table[hash].n_ge1 = line[12] - '0';
        table[hash].n_ge2 = line[13] - '0';
        table[hash].n_ge3 = line[14] - '0';
        table[hash].n_ge4 = line[15] - '0';
    }

    return true;
}

} // namespace mahjong
