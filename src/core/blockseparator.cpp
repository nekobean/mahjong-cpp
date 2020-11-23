#include "blockseparator.hpp"

#include <boost/dll.hpp>
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/writer.h>
#include <spdlog/spdlog.h>

#include <cstdio>
#include <fstream>

namespace mahjong {

/**
 * @brief 初期化する。
 * 
 * @return 初期化に成功した場合は true、そうでない場合は false を返す。
 */
bool BlockSeparator::initialize()
{
    boost::filesystem::path s_tbl_path =
        boost::dll::program_location().parent_path() / "syupai_pattern.json";
    boost::filesystem::path z_tbl_path =
        boost::dll::program_location().parent_path() / "zihai_pattern.json";

    return make_table(s_tbl_path.string(), s_tbl_) &&
           make_table(z_tbl_path.string(), z_tbl_);
}

/**
 * @brief 初期化する。
 * 
 * @param[in] path パス
 * @param[out] table テーブル
 * @return 初期化に成功した場合は true、そうでない場合は false を返す。
 */
bool BlockSeparator::make_table(const std::string &path,
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

std::vector<Block> BlockSeparator::get_blocks(const std::string &s)
{
    std::vector<Block> blocks;

    size_t len = s.size();
    for (size_t i = 0; i < len; i += 2) {
        Block block;
        block.min_tile = s[i] - '0';
        if (s[i + 1] == 'k')
            block.type = Block::Kotu;
        else if (s[i + 1] == 's')
            block.type = Block::Syuntu;
        else if (s[i + 1] == 't')
            block.type = Block::Toitu;

        blocks.emplace_back(block);
    }

    return blocks;
}

/**
 * @brief 手牌の可能なブロック構成パターンを生成する。
 * 
 * @param[in] hand 手牌
 * @param[in] win_tile 和了牌
 * @param[in] tumo 自摸かどうか
 * @return std::vector<std::vector<Block>> 面子構成の一覧
 */
std::vector<std::vector<Block>>
BlockSeparator::create_block_patterns(const Hand &hand, int win_tile, bool tumo)
{
    std::vector<std::vector<Block>> pattern;
    std::vector<Block> blocks(5);
    int i = 0;

    // 副露ブロックをブロック一覧に追加する。
    for (const auto &melded_block : hand.melded_blocks) {
        if (melded_block.type == MeldType::Pon)
            blocks[i].type = Block::Kotu | Block::Huro;
        else if (melded_block.type == MeldType::Ti)
            blocks[i].type = Block::Syuntu | Block::Huro;
        else if (melded_block.type == MeldType::Ankan)
            blocks[i].type = Block::Kantu;
        else
            blocks[i].type = Block::Kantu | Block::Huro;
        blocks[i].min_tile = aka2normal(melded_block.tiles.front());
        blocks[i].meld     = true;

        i++;
    }

    // 手牌の切り分けパターンを列挙する。
    create_block_patterns(hand, win_tile, tumo, pattern, blocks, i);

    // for (const auto &p : pattern) {
    //     for (const auto &b : p)
    //         std::cout << b.to_string() << " ";
    //     std::cout << std::endl;
    // }

    // 和了牌と同じ牌が手牌に3枚しかない刻子は明刻子になる。
    // int n_tiles = hand.num_tiles(win_tile);
    // for (auto &blocks : pattern) {
    //     for (auto &block : blocks) {
    //         if (!(block.type & Block::Huro) && block.type == Block::Kotu &&
    //             block.min_tile == win_tile && n_tiles < 4) {
    //             block.type |= Block::Huro;
    //             break;
    //         }
    //     }
    // }

    return pattern;
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
void BlockSeparator::create_block_patterns(const Hand &hand, int win_tile, bool tumo,
                                           std::vector<std::vector<Block>> &pattern,
                                           std::vector<Block> &blocks, size_t i, int d)
{
    if (d == 4) {
        //pattern.push_back(blocks);

        if (tumo) {
            // 自摸和了の場合
            pattern.push_back(blocks);
        }
        else {
            // ロン和了の場合、ロンした牌を含むブロックを副露にする。
            bool syuntu = false; // 123123のような場合はどれか1つだけ明順子にする
            for (auto &block : blocks) {
                if (block.type & Block::Huro)
                    continue;

                if (!syuntu && block.type == Block::Syuntu &&
                    block.min_tile <= win_tile && win_tile <= block.min_tile + 2) {
                    block.type |= Block::Huro;
                    pattern.push_back(blocks);
                    block.type &= ~Block::Huro;
                    syuntu = true;
                }
                else if (block.type != Block::Syuntu && block.min_tile == win_tile) {
                    block.type |= Block::Huro;
                    pattern.push_back(blocks);
                    block.type &= ~Block::Huro;
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

std::map<int, std::vector<std::vector<Block>>> BlockSeparator::s_tbl_;
std::map<int, std::vector<std::vector<Block>>> BlockSeparator::z_tbl_;

} // namespace mahjong
