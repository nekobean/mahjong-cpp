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

    // 各牌の残り枚数を数える。
    counts_ = count_left_tiles(hand, score.dora_tiles());

    // 合計残り枚数を数える。
    int total_left_tiles = std::accumulate(counts_.begin(), counts_.end(), 0);

    // 現在の向聴数を計算する。
    auto [_, syanten] = SyantenCalculator::calc(hand, syanten_type_);
    if (syanten == -1)
        return; // 和了形

    // 現在の手牌をルートノードとする。
    Graph::vertex_descriptor root =
        boost::add_vertex(std::make_shared<NodeData>(hand, syanten), G_);

    // グラフを作成する。
    int n_tiles = hand.num_tiles() + int(hand.melded_blocks.size()) * 3;
    if (n_tiles == 13)
        build_tree_draw(G_, root, syanten + 1);
    else
        build_tree_discard(G_, root, syanten + 1);
    return;
    if (n_tiles == 14) {
        std::vector<Candidate> candidates = select(G_, root, total_left_tiles, 1);

        for (const auto &candidate : candidates) {
            std::cout << fmt::format("打 {}: 期待値: {:.4f}, 有効牌: {}枚 ",
                                     Tile::Name.at(candidate.tile), candidate.win_exp,
                                     candidate.total_required_tiles);

            for (auto [tile, num] : candidate.required_tiles)
                std::cout << fmt::format("{}: {}枚 ", Tile::Name.at(tile), num);
            std::cout << std::endl;
        }
    }
    else {
        draw(G_, root, total_left_tiles, 1);
    }
}

double ExpectedValueCalculator::draw(const Graph &G, Graph::vertex_descriptor v,
                                     int sum_required_tiles, int turn)
{
    int total_required_tiles = 0;
    for (const auto e : boost::make_iterator_range(boost::out_edges(v, G))) {
        auto data = std::static_pointer_cast<DrawData>(G_[e]);
        total_required_tiles += counts_[data->tile];
    }

    double exp = 0;
    for (const auto e : boost::make_iterator_range(boost::out_edges(v, G))) {
        auto data = std::static_pointer_cast<DrawData>(G_[e]);
        auto u    = boost::target(e, G);

        int n_required_tiles = counts_[data->tile]; // 有効牌枚数

        double prob2 = 1; // 前回の巡目までに有効牌を自摸れない確率
        for (int i = 1; i <= 18 - turn - G[v]->syanten; i++) {
            sum_required_tiles -= i; // 無駄ツモ分を残り枚数から引く

            // 次のツモで有効牌が引ける確率
            double prob1 = double(n_required_tiles) / sum_required_tiles;

            // 前回のツモまで有効牌が引けない確率
            if (i > 1) {
                prob2 *= (double(sum_required_tiles + 1 - total_required_tiles) /
                          double(sum_required_tiles + 1));
            }

            counts_[data->tile]--;
            sum_required_tiles--; // 有効牌ツモ分を残り枚数から引く

            // 向聴数を落として同様に計算
            double exp3 = discard(G_, u, sum_required_tiles, turn + i);

            sum_required_tiles++;
            counts_[data->tile]++;

            // 合計
            exp += prob1 * prob2 * exp3;

            sum_required_tiles += i;
        }
    }

    return exp;
}

double ExpectedValueCalculator::discard(const Graph &G, Graph::vertex_descriptor v,
                                        int total_left_tiles, int turn)
{
    if (G_[v]->syanten == -1) {
        // 和了形
        auto node_data = std::static_pointer_cast<LeafData>(G_[v]);
        return node_data->result.score[0];
    }

    double max_exp = 0;
    for (const auto e : boost::make_iterator_range(boost::out_edges(v, G))) {
        double exp = draw(G_, boost::target(e, G), total_left_tiles, turn);
        max_exp    = std::max(max_exp, exp);
    }

    return max_exp;
}

/**
 * @brief 各打牌の情報を計算する。
 * 
 * @param[in] G グラフ
 * @param[in] root ルートノード
 * @param[in] total_left_tiles 
 * @param[in] turn 現在の巡目
 * @return std::vector<Candidate> 
 */
std::vector<Candidate> ExpectedValueCalculator::select(const Graph &G,
                                                       Graph::vertex_descriptor root,
                                                       int total_left_tiles, int turn)
{
    std::vector<Candidate> candidates;

    for (const auto e1 : boost::make_iterator_range(boost::out_edges(root, G))) {
        auto data  = std::static_pointer_cast<DrawData>(G_[e1]);
        double exp = draw(G_, boost::target(e1, G), total_left_tiles, turn);

        std::vector<std::tuple<int, int>> required_tiles;
        int total_required_tiles = 0;
        for (const auto e2 :
             boost::make_iterator_range(boost::out_edges(boost::target(e1, G), G))) {
            auto data = std::static_pointer_cast<DrawData>(G_[e2]);

            required_tiles.emplace_back(data->tile, counts_[data->tile]);
            total_required_tiles += counts_[data->tile];
        }

        candidates.emplace_back(data->tile, total_required_tiles, required_tiles, 0, 0,
                                exp);
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
    std::vector<int> counts(34, 4);

    for (int i = 0; i < 34; ++i)
        counts[i] -= hand.num_tiles(i);

    for (const auto &block : hand.melded_blocks) {
        for (auto tile : block.tiles) {
            tile = aka2normal(tile);
            counts[tile]--;
        }
    }

    for (auto tile : dora_tiles)
        counts[tile]--;

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
 */
void ExpectedValueCalculator::build_tree_draw(Graph &G, Graph::vertex_descriptor parent,
                                              int n_left_tumo)
{
    //std::vector<int> tiles = RequiredTileSelector::select(hand_, syanten_type_);
    //int total_tiles = count_num_required_tiles(counts_, tiles);
    int total_tiles = 0;
    int syanten     = G[parent]->syanten;

    for (int tile = 0; tile < 34; ++tile) {
        if (counts_[tile] == 0)
            continue; // 残り枚数が0枚の場合

        // 手牌に加える
        add_tile(hand_, tile);
        counts_[tile]--;

        auto [_, syanten] = SyantenCalculator::calc(hand_, syanten_type_);

        if (syanten < G[parent]->syanten) {
            Graph::vertex_descriptor node;
            auto itr = vert_cache_.find(hand_);
            if (itr != vert_cache_.end()) {
                node = itr->second;
            }
            else if (syanten == -1) {
                Result result =
                    score_.calc(hand_, tile, HandFlag::Tumo | HandFlag::Reach);
                node = boost::add_vertex(
                    std::make_shared<LeafData>(hand_, syanten, result), G);
                vert_cache_.insert_or_assign(hand_, node);
            }
            else {
                auto node_data = std::make_shared<NodeData>(hand_, syanten);
                node           = boost::add_vertex(node_data, G);
                vert_cache_.insert_or_assign(hand_, node);
            }

            auto edge_data =
                std::make_shared<DrawData>(tile, counts_[tile], total_tiles);
            boost::add_edge(parent, node, edge_data, G);

            if (n_left_tumo >= syanten && syanten != -1 && itr == vert_cache_.end())
                build_tree_discard(G, node, n_left_tumo - 1);
        }

        // 手牌から除く
        counts_[tile]++;
        remove_tile(hand_, tile);
    }
}

/**
 * @brief グラフ (DAG) を作成する。
 * 
 * @param[in,out] G グラフ
 * @param[in] parent 親ノード
 */
void ExpectedValueCalculator::build_tree_discard(Graph &G,
                                                 Graph::vertex_descriptor parent,
                                                 int n_left_tumo)
{
    for (int tile = 0; tile < 34; ++tile) {
        if (!hand_.contains(tile))
            continue; // 牌が手牌にない場合

        // 手牌から除く
        remove_tile(hand_, tile);

        auto [_, syanten] = SyantenCalculator::calc(hand_, syanten_type_);

        if (syanten == G[parent]->syanten) {
            // 向聴数が変化しない場合は打牌候補

            Graph::vertex_descriptor node;
            auto itr = vert_cache_.find(hand_);
            if (itr != vert_cache_.end()) {
                node = itr->second;
            }
            else {
                auto node_data = std::make_shared<NodeData>(hand_, syanten);
                node           = boost::add_vertex(node_data, G);
                vert_cache_.insert_or_assign(hand_, node);
            }

            auto edge_data = std::make_shared<DiscardData>(tile);
            boost::add_edge(parent, node, edge_data, G);

            if (itr == vert_cache_.end())
                build_tree_draw(G, node, n_left_tumo);
        }

        // 手牌に戻す
        add_tile(hand_, tile);
    }
}

} // namespace mahjong
