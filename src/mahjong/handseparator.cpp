#include "handseparator.hpp"

#include <boost/dll.hpp>
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/writer.h>
#include <spdlog/spdlog.h>

#include <cstdio>
#include <fstream>

namespace mahjong
{

HandSeparator::HandSeparator() { initialize(); }

/**
 * @brief 初期化する。
 *
 * @return 初期化に成功した場合は true、そうでない場合は false を返す。
 */
bool HandSeparator::initialize()
{
    if (!s_tbl_.empty())
        return true; // 初期化済み

    boost::filesystem::path s_tbl_path =
        boost::dll::program_location().parent_path() / "syupai_pattern.json";
    boost::filesystem::path z_tbl_path =
        boost::dll::program_location().parent_path() / "zihai_pattern.json";

    return make_table(s_tbl_path.string(), s_tbl_) &&
           make_table(z_tbl_path.string(), z_tbl_);
}

/**
 * @brief 手牌の可能なブロック構成パターンを生成する。
 *
 * @param[in] hand 手牌
 * @param[in] win_tile 和了牌
 * @param[in] tumo 自摸かどうか
 * @return std::vector<std::vector<Block>> 面子構成の一覧
 */
std::vector<std::tuple<std::vector<Block>, int>>
HandSeparator::separate(const Hand &hand, int win_tile, bool tumo)
{
    std::vector<std::tuple<std::vector<Block>, int>> pattern;
    std::vector<Block> blocks(5);
    int i = 0;

    // 副露ブロックをブロック一覧に追加する。
    for (const auto &melded_block : hand.melds) {
        if (melded_block.type == MeldType::Pon)
            blocks[i].type = BlockType::Kotu | BlockType::Open;
        else if (melded_block.type == MeldType::Ti)
            blocks[i].type = BlockType::Syuntu | BlockType::Open;
        else if (melded_block.type == MeldType::Ankan)
            blocks[i].type = BlockType::Kantu;
        else // 明槓、加槓
            blocks[i].type = BlockType::Kantu | BlockType::Open;
        blocks[i].min_tile = aka2normal(melded_block.tiles.front());

        i++;
    }

    // 手牌の切り分けパターンを列挙する。
    create_block_patterns(hand, win_tile, tumo, pattern, blocks, i);

    return pattern;
}

/**
 * @brief 初期化する。
 *
 * @param[in] path パス
 * @param[out] table テーブル
 * @return 初期化に成功した場合は true、そうでない場合は false を返す。
 */
bool HandSeparator::make_table(const std::string &path,
                               std::map<int, std::vector<std::vector<Block>>> &table)
{
    table.clear();

    std::FILE *fp = std::fopen(path.c_str(), "rb");
    if (!fp) {
        spdlog::error("Failed to open {}.", path);
        return false;
    }

    char *buffer = new char[1000000];
    rapidjson::FileReadStream is(fp, buffer, sizeof(buffer));
    rapidjson::Document doc;
    doc.ParseStream(is);
    if (doc.HasParseError()) {
        spdlog::error("Failed to parse {}.", path);
        return false;
    }

    for (auto &v : doc.GetArray()) {
        int key = v["key"].GetInt();

        std::vector<std::vector<Block>> pattern;
        for (auto &v2 : v["pattern"].GetArray())
            pattern.push_back(get_blocks(v2.GetString()));

        table[key] = pattern;
    }

    fclose(fp);
    delete buffer;

    return true;
}

std::vector<Block> HandSeparator::get_blocks(const std::string &s)
{
    std::vector<Block> blocks;

    size_t len = s.size();
    for (size_t i = 0; i < len; i += 2) {
        Block block;
        block.min_tile = s[i] - '0';
        if (s[i + 1] == 'k')
            block.type = BlockType::Kotu;
        else if (s[i + 1] == 's')
            block.type = BlockType::Syuntu;
        else if (s[i + 1] == 't')
            block.type = BlockType::Toitu;

        blocks.emplace_back(block);
    }

    return blocks;
}

/**
 * @brief 役満かどうかを判定する。
 *
 * @param[in] hand 手牌
 * @param[in] win_tile 和了牌
 * @param[in] flag 成立フラグ
 * @param[in] syanten_type 和了形の種類
 * @return YakuList 成立した役一覧
 */
void HandSeparator::create_block_patterns(
    const Hand &hand, int win_tile, bool tumo,
    std::vector<std::tuple<std::vector<Block>, int>> &pattern,
    std::vector<Block> &blocks, size_t i, int d)
{
    if (d == 4) {
        for (auto &block : blocks) {
            if (block.type & BlockType::Open)
                continue; // 副露ブロックは固定

            if ((block.type & BlockType::Kotu) && block.min_tile == win_tile) {
                // 双ポン待ち
                if (tumo) {
                    pattern.emplace_back(blocks, WaitType::Syanpon);
                }
                else {
                    block.type |= BlockType::Open;
                    pattern.emplace_back(blocks, WaitType::Syanpon);
                    block.type &= ~BlockType::Open;
                }
            }
            else if (block.type == BlockType::Syuntu &&
                     block.min_tile + 1 == win_tile) {
                // 嵌張待ち
                if (tumo) {
                    pattern.emplace_back(blocks, WaitType::Kantyan);
                }
                else {
                    block.type |= BlockType::Open;
                    pattern.emplace_back(blocks, WaitType::Kantyan);
                    block.type &= ~BlockType::Open;
                }
            }
            else if (block.type == BlockType::Syuntu &&
                     block.min_tile + 2 == win_tile &&
                     (block.min_tile == Tile::Manzu1 ||
                      block.min_tile == Tile::Pinzu1 ||
                      block.min_tile == Tile::Sozu1)) {
                // 辺張待ち 123
                if (tumo) {
                    pattern.emplace_back(blocks, WaitType::Pentyan);
                }
                else {
                    block.type |= BlockType::Open;
                    pattern.emplace_back(blocks, WaitType::Pentyan);
                    block.type &= ~BlockType::Open;
                }
            }
            else if (block.type == BlockType::Syuntu && block.min_tile == win_tile &&
                     (block.min_tile == Tile::Manzu7 ||
                      block.min_tile == Tile::Pinzu7 ||
                      block.min_tile == Tile::Sozu7)) {
                // 辺張待ち 789
                if (tumo) {
                    pattern.emplace_back(blocks, WaitType::Pentyan);
                }
                else {
                    block.type |= BlockType::Open;
                    pattern.emplace_back(blocks, WaitType::Pentyan);
                    block.type &= ~BlockType::Open;
                }
            }
            else if (block.type == BlockType::Syuntu &&
                     (block.min_tile == win_tile || block.min_tile + 2 == win_tile)) {
                // 両面待ち
                if (tumo) {
                    pattern.emplace_back(blocks, WaitType::Ryanmen);
                }
                else {
                    block.type |= BlockType::Open;
                    pattern.emplace_back(blocks, WaitType::Ryanmen);
                    block.type &= ~BlockType::Open;
                }
            }
            else if (block.type == BlockType::Toitu && block.min_tile == win_tile) {
                // 双ポン待ち
                if (tumo) {
                    pattern.emplace_back(blocks, WaitType::Tanki);
                }
                else {
                    block.type |= BlockType::Open;
                    pattern.emplace_back(blocks, WaitType::Tanki);
                    block.type &= ~BlockType::Open;
                }
            }
        }

        return;
    }

    if (d == 0) {
        // 萬子の面子構成
        if (s_tbl_[hand.manzu].empty())
            create_block_patterns(hand, win_tile, tumo, pattern, blocks, i, d + 1);

        for (const auto &manzu_pattern : s_tbl_[hand.manzu]) {
            for (const auto &block : manzu_pattern)
                blocks[i++] = block;
            create_block_patterns(hand, win_tile, tumo, pattern, blocks, i, d + 1);
            i -= manzu_pattern.size();
        }
    }
    else if (d == 1) {
        // 筒子の面子構成
        if (s_tbl_[hand.pinzu].empty())
            create_block_patterns(hand, win_tile, tumo, pattern, blocks, i, d + 1);

        for (const auto &pinzu_pattern : s_tbl_[hand.pinzu]) {
            for (const auto &block : pinzu_pattern)
                blocks[i++] = {block.type, block.min_tile + 9};

            create_block_patterns(hand, win_tile, tumo, pattern, blocks, i, d + 1);
            i -= pinzu_pattern.size();
        }
    }
    else if (d == 2) {
        // 索子の面子構成
        if (s_tbl_[hand.sozu].empty())
            create_block_patterns(hand, win_tile, tumo, pattern, blocks, i, d + 1);

        for (const auto &sozu_pattern : s_tbl_[hand.sozu]) {
            for (const auto &block : sozu_pattern)
                blocks[i++] = {block.type, block.min_tile + 18};
            create_block_patterns(hand, win_tile, tumo, pattern, blocks, i, d + 1);
            i -= sozu_pattern.size();
        }
    }
    else if (d == 3) {
        // 字牌の面子構成
        if (z_tbl_[hand.zihai].empty())
            create_block_patterns(hand, win_tile, tumo, pattern, blocks, i, d + 1);

        for (const auto &zihai_pattern : z_tbl_[hand.zihai]) {
            for (const auto &block : zihai_pattern)
                blocks[i++] = {block.type, block.min_tile + 27};
            create_block_patterns(hand, win_tile, tumo, pattern, blocks, i, d + 1);
            i -= zihai_pattern.size();
        }
    }
}

std::map<int, std::vector<std::vector<Block>>> HandSeparator::s_tbl_;
std::map<int, std::vector<std::vector<Block>>> HandSeparator::z_tbl_;
static HandSeparator inst;

} // namespace mahjong
