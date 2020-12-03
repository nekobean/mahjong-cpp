#include "expectedvaluecalculator.hpp"

#undef NDEBUG
#include <assert.h>

namespace mahjong {

/**
 * @brief 
 * 
 * @param hand 
 */
void ExpectedValueCalculator::calc(const Hand &hand, const ScoreCalculator &score,
                                   int syanten_type)
{
    score_        = score;
    hand_         = hand;
    syanten_type_ = syanten_type;
    G_.clear();

    auto [_, syanten] = SyantenCalculator::calc(hand, syanten_type_);
    if (syanten == -1)
        return; // 和了形

    // 残り牌の枚数を数える。
    counts_ = count_left_tiles(hand, score.dora_tiles());

    // 現在の手牌をルートノードとする。
    Graph::vertex_descriptor root =
        boost::add_vertex(std::make_shared<NodeData>(hand), G_);

    // 次のアクションが自摸かどうか
    int n_tiles = hand.num_tiles() + hand.melded_blocks.size() * 3 == 13;
    if (n_tiles == 13)
        draw(G_, root, syanten);
    else
        discard(G_, root, syanten);
}

Graph &ExpectedValueCalculator::graph()
{
    return G_;
}

/**
 * @brief すべての和了形を取得する。
 * 
 */
std::vector<std::tuple<Hand, Result>> ExpectedValueCalculator::get_win_hands()
{
    std::vector<std::tuple<Hand, Result>> hands;

    for (const auto &v : boost::make_iterator_range(boost::vertices(G_))) {
        if (auto data = std::dynamic_pointer_cast<LeafData>(G_[v]))
            hands.emplace_back(data->hand, data->result);
    }

    return hands;
}

/**
 * @brief 各牌の残り枚数を数える。
 * 
 * @param[in] hand 手牌
 * @param[in] dora_tiles ドラ牌の一覧
 * @return std::vector<int> 各牌の残り枚数
 */
std::vector<int>
ExpectedValueCalculator::count_left_tiles(const Hand &hand,
                                          const std::vector<int> &dora_tiles)
{
    std::vector<int> counts(34, 4); // 残り牌の枚数

    for (int i = 0; i < 34; ++i)
        counts[i] -= hand.num_tiles(i);

    for (const auto &block : hand.melded_blocks) {
        for (auto tile : block.tiles) {
            tile = aka2normal(tile);
            counts[tile] -= hand.num_tiles(tile);
        }
    }

    for (auto tile : dora_tiles)
        counts[tile] -= hand.num_tiles(tile);

    return counts;
}

/**
 * @brief 有効牌の合計枚数を数える。
 * 
 * @param[in] count 各牌の残り枚数
 * @param[in] tiles 有効牌の一覧
 * @return int 有効牌の合計枚数
 */
int ExpectedValueCalculator::count_num_required_tiles(const std::vector<int> &count,
                                                      const std::vector<int> &tiles)
{
    int n_tiles = 0;
    for (auto tile : tiles)
        n_tiles += count[tile];

    return n_tiles;
}

/**
 * @brief グラフ (DAG) を作成する。
 * 
 * @param[in,out] G グラフ
 * @param[in] parent 親ノード
 * @param[in] is_tumo 現在のノードが自摸かどうか
 * @param[in] syanten 現在のノードの向聴数
 */
void ExpectedValueCalculator::draw(Graph &G, Graph::vertex_descriptor parent,
                                   int syanten)
{
    std::vector<int> tiles = RequiredTileSelector::select(hand_, syanten_type_);
    int total_tiles        = count_num_required_tiles(counts_, tiles);

    for (auto tile : tiles) {
        add_tile(hand_, tile);

        Graph::vertex_descriptor node;
        auto itr = vert_cache_.find(hand_);
        if (itr != vert_cache_.end()) {
            node = itr->second;
        }
        else if (syanten == 0) {
            Result result = score_.calc(hand_, tile, HandFlag::Tumo);
            node = boost::add_vertex(std::make_shared<LeafData>(hand_, result), G);
            vert_cache_.insert_or_assign(hand_, node);
        }
        else {
            node = boost::add_vertex(std::make_shared<NodeData>(hand_), G);
            vert_cache_.insert_or_assign(hand_, node);
        }

        auto data = std::make_shared<DrawData>(tile, counts_[tile], total_tiles);
        boost::add_edge(parent, node, data, G);

        if (syanten > 0 && itr == vert_cache_.end())
            discard(G, node, syanten - 1);

        remove_tile(hand_, tile);
    }
}

/**
 * @brief グラフ (DAG) を作成する。
 * 
 * @param[in,out] G グラフ
 * @param[in] parent 親ノード
 * @param[in] is_tumo 現在のノードが自摸かどうか
 * @param[in] syanten 現在のノードの向聴数
 */
void ExpectedValueCalculator::discard(Graph &G, Graph::vertex_descriptor parent,
                                      int syanten)
{
    std::vector<int> tiles = UnnecessaryTileSelector::select(hand_, syanten_type_);

    for (auto tile : tiles) {
        remove_tile(hand_, tile);

        Graph::vertex_descriptor node;
        auto itr = vert_cache_.find(hand_);
        if (itr != vert_cache_.end()) {
            node = itr->second;
        }
        else {
            node = boost::add_vertex(std::make_shared<NodeData>(hand_), G);
            vert_cache_.insert_or_assign(hand_, node);
        }

        auto data = std::make_shared<DiscardData>(tile);
        boost::add_edge(parent, node, data, G);

        if (itr == vert_cache_.end())
            draw(G, node, syanten);

        add_tile(hand_, tile);
    }
}

} // namespace mahjong
