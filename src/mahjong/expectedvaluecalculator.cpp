#include "expectedvaluecalculator.hpp"

#undef NDEBUG
#include <assert.h>
#include <numeric>

namespace mahjong {

/**
 * @brief 期待値を計算する。
 * 
 * @param[in] hand 手牌
 * @param[in] score 点数計算機
 * @param[in] syanten_type 向聴数の種類
 */
void ExpectedValueCalculator::calc(const Hand &hand, const ScoreCalculator &score,
                                   int syanten_type)
{
    score_        = score;
    hand_         = hand;
    syanten_type_ = syanten_type;
    G_.clear();

    // 残り牌の枚数を数える。
    counts_ = count_left_tiles(hand, score.dora_tiles());

    int total_left_tiles = std::accumulate(counts_.begin(), counts_.end(), 0);
    std::cout << "total_left_tiles " << total_left_tiles << std::endl;

    // 現在の向聴数を計算する。
    auto [_, syanten] = SyantenCalculator::calc(hand, syanten_type_);
    if (syanten == -1)
        return; // 和了形

    int n_tiles = hand.num_tiles() + int(hand.melded_blocks.size()) * 3;

    // 現在の手牌をルートノードとする。
    Graph::vertex_descriptor root =
        boost::add_vertex(std::make_shared<NodeData>(hand, syanten), G_);

    // グラフを作成する。
    if (n_tiles == 13)
        draw(G_, root, syanten);
    else
        discard(G_, root, syanten);

    if (n_tiles == 14) {
        std::vector<Candidate> candidates = select(G_, root, total_left_tiles, 1);

        for (const auto &candidate : candidates) {
            std::cout << fmt::format("打 {}: {:.4f}", Tile::Name.at(candidate.tile),
                                     candidate.win_exp)
                      << std::endl;
        }
    }
    else {
        draw(G_, root, total_left_tiles, 1);
    }
}

double ExpectedValueCalculator::draw(const Graph &G, Graph::vertex_descriptor v,
                                     int total_left_tiles, int turn)
{
    int total_req_left_tiles = 0;
    for (const auto e : boost::make_iterator_range(boost::out_edges(v, G))) {
        auto data = std::static_pointer_cast<DrawData>(G_[e]);
        total_req_left_tiles += counts_[data->tile];
    }

    double total_exp = 0;

    for (const auto e : boost::make_iterator_range(boost::out_edges(v, G))) {
        auto data   = std::static_pointer_cast<DrawData>(G_[e]);
        auto u      = boost::target(e, G);
        double exp2 = 1; // 前回の巡目までに有効牌を自摸れない確率
        double exp3 = 1; // 期待値

        int n_req_left_tiles = counts_[data->tile]; // 有効牌枚数

        for (int m = 0; m < 18 - turn - G[v]->syanten; m++) {
            total_left_tiles -= m; // 無駄ツモ分を残り枚数から引く

            // 次のツモで有効牌が引ける確率
            double exp1 = double(n_req_left_tiles) / total_left_tiles;

            // 前回のツモまで有効牌が引けない確率
            if (m > 0) {
                exp2 *= (double(total_left_tiles + 1 - total_req_left_tiles) /
                         double(total_left_tiles + 1));
            }

            total_left_tiles--; // 有効牌ツモ分を残り枚数から引く

            // 向聴数を落として同様に計算
            double exp3 = discard(G_, u, total_left_tiles, turn + m + 1);

            total_left_tiles++;

            // 合計
            total_exp += exp1 * exp2 * exp3;

            total_left_tiles += m;
        }
    }

    return total_exp;
}

double ExpectedValueCalculator::discard(const Graph &G, Graph::vertex_descriptor v,
                                        int total_left_tiles, int turn)
{
    if (G_[v]->syanten == -1) {
        auto data = std::static_pointer_cast<LeafData>(G_[v]);
        return data->result.score[0];
    }

    double max_exp = 0;
    for (const auto e : boost::make_iterator_range(boost::out_edges(v, G))) {
        double exp = draw(G_, boost::target(e, G), total_left_tiles, turn);
        max_exp    = std::max(max_exp, exp);
    }

    return max_exp;
}

std::vector<Candidate> ExpectedValueCalculator::select(const Graph &G,
                                                       Graph::vertex_descriptor v,
                                                       int total_left_tiles, int turn)
{
    std::vector<Candidate> candidates;

    for (const auto e : boost::make_iterator_range(boost::out_edges(v, G))) {
        auto data  = std::static_pointer_cast<DrawData>(G_[e]);
        auto u     = boost::target(e, G);
        double exp = draw(G_, u, total_left_tiles, turn);

        candidates.emplace_back(data->tile, 0, 0, exp);
    }

    return candidates;
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
            node          = boost::add_vertex(
                std::make_shared<LeafData>(hand_, syanten - 1, result), G);
            vert_cache_.insert_or_assign(hand_, node);
        }
        else {
            node = boost::add_vertex(std::make_shared<NodeData>(hand_, syanten - 1), G);
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
            node = boost::add_vertex(std::make_shared<NodeData>(hand_, syanten), G);
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
