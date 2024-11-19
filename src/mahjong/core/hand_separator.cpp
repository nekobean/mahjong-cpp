#include "hand_separator.hpp"

#include <boost/dll.hpp>
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/writer.h>
#include <spdlog/spdlog.h>

#include <cstdio>
#include <fstream>

namespace mahjong
{

HandSeparator::HandSeparator()
{
    initialize();
}

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
        boost::dll::program_location().parent_path() / "suits_patterns.json";
    boost::filesystem::path z_tbl_path =
        boost::dll::program_location().parent_path() / "honors_patterns.json";

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
HandSeparator::separate(const Input &input)
{
    std::vector<std::tuple<std::vector<Block>, int>> pattern;
    std::vector<Block> blocks(5);
    int i = 0;

    // 副露ブロックをブロック一覧に追加する。
    for (const auto &melded_block : input.melds) {
        if (melded_block.type == MeldType::Pong)
            blocks[i].type = BlockType::Triplet | BlockType::Open;
        else if (melded_block.type == MeldType::Chow)
            blocks[i].type = BlockType::Sequence | BlockType::Open;
        else if (melded_block.type == MeldType::ClosedKong)
            blocks[i].type = BlockType::Kong;
        else // 明槓、加槓
            blocks[i].type = BlockType::Kong | BlockType::Open;
        blocks[i].min_tile = red2normal(melded_block.tiles.front());

        ++i;
    }

    // 手牌の切り分けパターンを列挙する。
    create_block_patterns(input, pattern, blocks, i);

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
            block.type = BlockType::Triplet;
        else if (s[i + 1] == 's')
            block.type = BlockType::Sequence;
        else if (s[i + 1] == 'z')
            block.type = BlockType::Pair;

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
    const Input &input, std::vector<std::tuple<std::vector<Block>, int>> &pattern,
    std::vector<Block> &blocks, size_t i, int d)
{
    int win_tile = input.win_tile;
    bool tumo = input.win_flag & WinFlag::Tsumo;

    if (d == 4) {
        for (auto &block : blocks) {
            if (block.type & BlockType::Open)
                continue; // 副露ブロックは固定

            if ((block.type & BlockType::Triplet) && block.min_tile == win_tile) {
                // 双ポン待ち
                if (tumo) {
                    pattern.emplace_back(blocks, WaitType::TripletWait);
                }
                else {
                    block.type |= BlockType::Open;
                    pattern.emplace_back(blocks, WaitType::TripletWait);
                    block.type &= ~BlockType::Open;
                }
            }
            else if (block.type == BlockType::Sequence &&
                     block.min_tile + 1 == win_tile) {
                // 嵌張待ち
                if (tumo) {
                    pattern.emplace_back(blocks, WaitType::ClosedWait);
                }
                else {
                    block.type |= BlockType::Open;
                    pattern.emplace_back(blocks, WaitType::ClosedWait);
                    block.type &= ~BlockType::Open;
                }
            }
            else if (block.type == BlockType::Sequence &&
                     block.min_tile + 2 == win_tile &&
                     (block.min_tile == Tile::Manzu1 ||
                      block.min_tile == Tile::Pinzu1 ||
                      block.min_tile == Tile::Souzu1)) {
                // 辺張待ち 123
                if (tumo) {
                    pattern.emplace_back(blocks, WaitType::EdgeWait);
                }
                else {
                    block.type |= BlockType::Open;
                    pattern.emplace_back(blocks, WaitType::EdgeWait);
                    block.type &= ~BlockType::Open;
                }
            }
            else if (block.type == BlockType::Sequence && block.min_tile == win_tile &&
                     (block.min_tile == Tile::Manzu7 ||
                      block.min_tile == Tile::Pinzu7 ||
                      block.min_tile == Tile::Souzu7)) {
                // 辺張待ち 789
                if (tumo) {
                    pattern.emplace_back(blocks, WaitType::EdgeWait);
                }
                else {
                    block.type |= BlockType::Open;
                    pattern.emplace_back(blocks, WaitType::EdgeWait);
                    block.type &= ~BlockType::Open;
                }
            }
            else if (block.type == BlockType::Sequence &&
                     (block.min_tile == win_tile || block.min_tile + 2 == win_tile)) {
                // 両面待ち
                if (tumo) {
                    pattern.emplace_back(blocks, WaitType::DoubleEdgeWait);
                }
                else {
                    block.type |= BlockType::Open;
                    pattern.emplace_back(blocks, WaitType::DoubleEdgeWait);
                    block.type &= ~BlockType::Open;
                }
            }
            else if (block.type == BlockType::Pair && block.min_tile == win_tile) {
                // 双ポン待ち
                if (tumo) {
                    pattern.emplace_back(blocks, WaitType::PairWait);
                }
                else {
                    block.type |= BlockType::Open;
                    pattern.emplace_back(blocks, WaitType::PairWait);
                    block.type &= ~BlockType::Open;
                }
            }
        }

        return;
    }

    if (d == 0) {
        // 萬子の面子構成
        if (s_tbl_[input.manzu].empty())
            create_block_patterns(input, pattern, blocks, i, d + 1);

        for (const auto &manzu_pattern : s_tbl_[input.manzu]) {
            for (const auto &block : manzu_pattern)
                blocks[i++] = block;
            create_block_patterns(input, pattern, blocks, i, d + 1);
            i -= manzu_pattern.size();
        }
    }
    else if (d == 1) {
        // 筒子の面子構成
        if (s_tbl_[input.pinzu].empty())
            create_block_patterns(input, pattern, blocks, i, d + 1);

        for (const auto &pinzu_pattern : s_tbl_[input.pinzu]) {
            for (const auto &block : pinzu_pattern)
                blocks[i++] = {block.type, block.min_tile + 9};

            create_block_patterns(input, pattern, blocks, i, d + 1);
            i -= pinzu_pattern.size();
        }
    }
    else if (d == 2) {
        // 索子の面子構成
        if (s_tbl_[input.souzu].empty())
            create_block_patterns(input, pattern, blocks, i, d + 1);

        for (const auto &sozu_pattern : s_tbl_[input.souzu]) {
            for (const auto &block : sozu_pattern)
                blocks[i++] = {block.type, block.min_tile + 18};
            create_block_patterns(input, pattern, blocks, i, d + 1);
            i -= sozu_pattern.size();
        }
    }
    else if (d == 3) {
        // 字牌の面子構成
        if (z_tbl_[input.honors].empty())
            create_block_patterns(input, pattern, blocks, i, d + 1);

        for (const auto &zihai_pattern : z_tbl_[input.honors]) {
            for (const auto &block : zihai_pattern)
                blocks[i++] = {block.type, block.min_tile + 27};
            create_block_patterns(input, pattern, blocks, i, d + 1);
            i -= zihai_pattern.size();
        }
    }
}

std::map<int, std::vector<std::vector<Block>>> HandSeparator::s_tbl_;
std::map<int, std::vector<std::vector<Block>>> HandSeparator::z_tbl_;
static HandSeparator inst;

} // namespace mahjong
