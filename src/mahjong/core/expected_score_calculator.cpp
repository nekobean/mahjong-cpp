#include "expected_score_calculator.hpp"

#undef NDEBUG
#include <algorithm> // max, fill
#include <cassert>

#include <boost/dll.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/graphviz.hpp>

#include "mahjong/core/necessary_tile_calculator.hpp"
#include "mahjong/core/score_calculator.hpp"
#include "mahjong/core/shanten_calculator.hpp"
#include "mahjong/core/string.hpp"
#include "mahjong/core/unnecessary_tile_calculator.hpp"
#include "mahjong/core/utils.hpp"

namespace mahjong
{

/**
 * @brief 山の残り枚数を計算する。
 *
 * @param round 場の情報
 * @param player プレイヤーの情報
 * @param enable_reddora 赤ドラを有効にするか
 * @return 山の残り枚数
 */
Count ExpectedScoreCalculator::create_wall(const Round &round, const Player &player,
                                           const bool enable_reddora)
{
    Count wall{0}, melds{0}, indicators{0};

    for (auto tile : round.dora_indicators) {
        ++indicators[to_no_reddora(tile)];
        if (is_reddora(tile)) {
            ++indicators[tile];
        }
    }

    for (const auto &meld : player.melds) {
        for (auto tile : meld.tiles) {
            ++melds[to_no_reddora(tile)];
            if (is_reddora(tile)) {
                ++melds[tile];
            }
        }
    }

    for (int i = 0; i < 34; ++i) {
        wall[i] = 4 - (player.hand[i] + melds[i] + indicators[i]);
    }
    if (enable_reddora) {
        for (int i = 34; i < 37; ++i) {
            wall[i] = 1 - (player.hand[i] + melds[i] + indicators[i]);
        }
    }

    // assert
    if (!enable_reddora) {
        for (int i = 34; i < 37; ++i) {
            assert(wall[i] == 0);
        }
    }
    else {
        if (wall[Tile::RedManzu5]) {
            assert(wall[Tile::Manzu5] >= 1);
        }
        if (wall[Tile::RedPinzu5]) {
            assert(wall[Tile::Pinzu5] >= 1);
        }
        if (wall[Tile::RedSouzu5]) {
            assert(wall[Tile::Souzu5] >= 1);
        }
        if (wall[Tile::Manzu5] == 4) {
            assert(wall[Tile::RedManzu5] == 1);
        }
        if (wall[Tile::Pinzu5] == 4) {
            assert(wall[Tile::RedPinzu5] == 1);
        }
        if (wall[Tile::Souzu5] == 4) {
            assert(wall[Tile::RedSouzu5] == 1);
        }
    }

    // for (int i = 0; i < 34; ++i) {
    //     assert(wall[i] >= 0 && wall[i] <= 4);
    // }
    // for (int i = 34; i < 37; ++i) {
    //     if (enable_reddora) {
    //         assert(wall[i] >= 0 && wall[i] <= 1);
    //     }
    //     else {
    //         assert(wall[i] == 0);
    //     }
    // }

    return wall;
}

/**
 * @brief 残り枚数で赤ドラの表現を以下のように変更する。
 *        counts[Tile::Manzu5]: 赤を含む5萬の枚数、counts[Tile::RedManzu5]: 赤5萬があるかどうか
 *        counts[Tile::Pinzu5]: 赤を含む5筒の枚数、counts[Tile::RedPinzu5]: 赤5筒があるかどうか
 *        counts[Tile::Souzu5]: 赤を含む5索の枚数、counts[Tile::RedSouzu5]: 赤5索があるかどうか
 *        ↓
 *        counts[Tile::Manzu5]: 赤ドラ以外の5萬の枚数、counts[Tile::RedManzu5]: 赤5萬の枚数
 *        counts[Tile::Pinzu5]: 赤ドラ以外の5筒の枚数、counts[Tile::RedPinzu5]: 赤5筒の枚数
 *        counts[Tile::Souzu5]: 赤ドラ以外の5索の枚数、counts[Tile::RedSouzu5]: 赤5索の枚数
 *
 * @param counts 残り枚数
 * @param enable_reddora 赤ドラを有効にするか
 * @return 赤ドラを区別した残り枚数
 */
ExpectedScoreCalculator::CountRed
ExpectedScoreCalculator::encode(const Count &counts, const bool enable_reddora)
{
    CountRed ret{0};
    for (int i = 0; i < 34; ++i) {
        ret[i] = counts[i];
    }

    if (enable_reddora) {
        if (counts[Tile::RedManzu5]) {
            --ret[Tile::Manzu5];
            ++ret[Tile::RedManzu5];
        }
        if (counts[Tile::RedPinzu5]) {
            --ret[Tile::Pinzu5];
            ++ret[Tile::RedPinzu5];
        }
        if (counts[Tile::RedSouzu5]) {
            --ret[Tile::Souzu5];
            ++ret[Tile::RedSouzu5];
        }
    }

    return ret;
}

/**
 * @brief 手牌 hand から手牌 hand_org に変化するための交換枚数を計算する。
 *
 * @param hand
 * @param hand_org
 * @return int
 */
int ExpectedScoreCalculator::distance(const CountRed &hand, const CountRed &hand_org)
{
    int dist = 0;
    for (int i = 0; i < hand.size(); ++i) {
        dist += std::max(hand[i] - hand_org[i], 0);
    }

    return dist;
}

/**
 * @brief tile を手牌に加え、山から削除する。
 *
 * @param player プレイヤー
 * @param hand_counts 赤ドラを区別する手牌の残り枚数
 * @param wall_counts 赤ドラを区別する山の残り枚数
 * @param tile 牌
 */
void ExpectedScoreCalculator::draw(Player &player, CountRed &hand_counts,
                                   CountRed &wall_counts, const int tile)
{
    ++hand_counts[tile];
    --wall_counts[tile];

    // Update hand
    player.hand[tile]++;
    if (tile == Tile::RedManzu5) {
        player.hand[Tile::Manzu5]++;
    }
    else if (tile == Tile::RedPinzu5) {
        player.hand[Tile::Pinzu5]++;
    }
    else if (tile == Tile::RedSouzu5) {
        player.hand[Tile::Souzu5]++;
    }
}

/**
 * @brief tile を手牌から削除し、山に加える。
 *
 * @param player プレイヤー
 * @param hand_counts 赤ドラを区別する手牌の残り枚数
 * @param wall_counts 赤ドラを区別する山の残り枚数
 * @param tile 牌
 */
void ExpectedScoreCalculator::discard(Player &player, CountRed &hand_counts,
                                      CountRed &wall_counts, const int tile)
{
    --hand_counts[tile];
    ++wall_counts[tile];

    // Update hand
    player.hand[tile]--;
    if (tile == Tile::RedManzu5) {
        player.hand[Tile::Manzu5]--;
    }
    else if (tile == Tile::RedPinzu5) {
        player.hand[Tile::Pinzu5]--;
    }
    else if (tile == Tile::RedSouzu5) {
        player.hand[Tile::Souzu5]--;
    }
}

int ExpectedScoreCalculator::calc_score(const Config &config, const Round &round,
                                        Player &player, CountRed &hand_counts,
                                        CountRed &wall_counts, const int shanten_type,
                                        const int win_tile, const bool riichi)
{
    // 立直している場合、立直、自摸のフラグを立てる
    int win_flag = riichi ? (WinFlag::Tsumo | WinFlag::Riichi) : WinFlag::Tsumo;

    Result result =
        ScoreCalculator::calc_fast(round, player, win_tile, win_flag, shanten_type);

    if (!result.success) {
        return 0; // 役なしの場合は0点
    }

    if (!config.enable_uradora || !(win_flag & WinFlag::Riichi) ||
        round.dora_indicators.empty()) {
        // 裏ドラ計算なし、立直していない、表ドラ表示牌なしの場合
        return result.score[0];
    }

    if (result.score_title >= ScoreTitle::CountedYakuman) {
        return result.score[0]; // 役満の場合
    }

    const int num_indicators = round.dora_indicators.size();

    if (num_indicators == 1) {
        // 裏ドラが1枚の場合、裏ドラが乗る確率を解析的に計算する。
        Count wall = wall_counts;
        wall[Tile::Manzu5] += wall[Tile::RedManzu5];
        wall[Tile::Pinzu5] += wall[Tile::RedPinzu5];
        wall[Tile::Souzu5] += wall[Tile::RedSouzu5];
        Count hand_and_melds = hand_counts;
        hand_and_melds[Tile::Manzu5] += hand_and_melds[Tile::RedManzu5];
        hand_and_melds[Tile::Pinzu5] += hand_and_melds[Tile::RedPinzu5];
        hand_and_melds[Tile::Souzu5] += hand_and_melds[Tile::RedSouzu5];
        for (const auto &meld : player.melds) {
            for (auto tile : meld.tiles) {
                ++hand_and_melds[to_no_reddora(tile)];
            }
        }

        std::vector<int> up_scores =
            ScoreCalculator::get_up_scores(round, player, result, win_flag, 4);

        std::vector<double> num_indicators(5, 0);
        for (int tile = 0; tile < 34; ++tile) {
            int num = hand_and_melds[tile];
            num_indicators[num] += wall[ToIndicator[tile]];
        }

        // 裏ドラがi枚乗った場合の点数 * 裏ドラがi枚乗る確率を i = 0~4 で足し合わせる
        double score = 0;
        for (int i = 0; i <= 4; ++i) {
            score += up_scores[i] * double(num_indicators[i]) / config.sum;
        }

        return score;
    }
    else {
        // 裏ドラ考慮ありかつ表ドラが2枚以上の場合、統計データを利用する。
        // uradora_table_[i][j] は i が表ドラ表示牌の数、
        // j は 0~10 は j 枚乗る枚数、12 は 11枚以上乗る枚数
        // 立直の1翻があるため、11枚以上乗った場合は数え役満である
        std::vector<int> up_scores =
            ScoreCalculator::get_up_scores(round, player, result, win_flag, 12);

        double score = 0;
        for (int i = 0; i <= 12; ++i) {
            score += up_scores[i] * uradora_table_[num_indicators][i];
        }

        return score;
    }
}

ExpectedScoreCalculator::Vertex ExpectedScoreCalculator::draw_node(
    const Config &config, const Round &round, Player &player, Graph &graph,
    Cache &cache1, Cache &cache2, CountRed &hand_counts, CountRed &wall_counts,
    const CountRed &hand_org, const int shanten_org, const bool riichi)
{
    // If vertex exists in the cache, return it.
    CacheKey key(hand_counts, riichi);
    if (const auto itr = cache1.find(key); itr != cache1.end()) {
        return itr->second;
    }

    // Calculate necessary tiles.
    auto [type, shanten, wait] = NecessaryTileCalculator::calc(
        player.hand, player.num_melds(), config.shanten_type);

    // 元の手牌から現在の手牌に変化されるのに必要な交換枚数 + 現在の向聴数 < 元の手牌の向聴数 + 追加交換枚数
    // の場合、手変わりを許可する
    bool allow_tegawari =
        config.enable_tegawari && !riichi &&
        distance(hand_counts, hand_org) + shanten < shanten_org + config.extra;
    wait |= (wait & (1LL << Tile::Manzu5)) ? (1LL << Tile::RedManzu5) : 0;
    wait |= (wait & (1LL << Tile::Pinzu5)) ? (1LL << Tile::RedPinzu5) : 0;
    wait |= (wait & (1LL << Tile::Souzu5)) ? (1LL << Tile::RedSouzu5) : 0;

// Add vertex to graph.
#ifdef DEBUG_GRAPH
    VertexData vertex_data(config.t_max + 1, 0.0, 0.0, 0.0, player, shanten, riichi);
#else
    VertexData vertex_data(config.t_max + 1, 0.0, 0.0, 0.0);
#endif
    vertex_data.tenpai_prob[config.t_max] = shanten == 0;
    const Vertex vertex = boost::add_vertex(vertex_data, graph);
    cache1[key] = vertex;

    const bool can_call_riichi =
        !riichi && config.enable_riichi && player.is_closed() && shanten == 1;

    for (int i = 0; i < 37; ++i) {
        bool is_wait = wait & (1LL << i);

        if (wall_counts[i] && (allow_tegawari || is_wait)) {
            const int weight = wall_counts[i];

            draw(player, hand_counts, wall_counts, i);

            // 1向聴で有効牌を自摸した場合、聴牌なので立直する
            if (can_call_riichi && is_wait) {
                const Vertex target =
                    discard_node(config, round, player, graph, cache1, cache2,
                                 hand_counts, wall_counts, hand_org, shanten_org, true);
                if (!boost::edge(vertex, target, graph).second) {
#ifdef DEBUG_GRAPH
                    boost::add_edge(vertex, target, {weight, 0, i, true}, graph);
#else
                    boost::add_edge(vertex, target, {weight, 0}, graph);
#endif
                }
            }

            const Vertex target =
                discard_node(config, round, player, graph, cache1, cache2, hand_counts,
                             wall_counts, hand_org, shanten_org, riichi);

            if (!boost::edge(vertex, target, graph).second) {
                // 自摸前の時点で聴牌の場合、有効牌自摸後は和了形のため、点数計算を行う
                const int score = (shanten == 0 && is_wait
                                       ? calc_score(config, round, player, hand_counts,
                                                    wall_counts, type, i, riichi)
                                       : 0);
#ifdef DEBUG_GRAPH
                boost::add_edge(vertex, target, {weight, score, i, false}, graph);
#else
                boost::add_edge(vertex, target, {weight, score}, graph);
#endif
            }

            discard(player, hand_counts, wall_counts, i);
        }
    }

    return vertex;
}

ExpectedScoreCalculator::Vertex ExpectedScoreCalculator::discard_node(
    const Config &config, const Round &round, Player &player, Graph &graph,
    Cache &cache1, Cache &cache2, CountRed &hand_counts, CountRed &wall_counts,
    const CountRed &hand_org, const int shanten_org, const bool riichi)
{
    // If vertex exists in the cache, return it.
    CacheKey key(hand_counts, riichi);
    if (const auto itr = cache2.find(key); itr != cache2.end()) {
        return itr->second;
    }

    // Calculate unnecessary tiles.
    auto [type, shanten, disc] = UnnecessaryTileCalculator::calc(
        player.hand, player.num_melds(), config.shanten_type);

    // 元の手牌から現在の手牌に変化されるのに必要な交換枚数 + 現在の向聴数 < 元の手牌の向聴数 + 追加交換枚数
    // の場合、向聴戻しを許可する
    bool allow_shanten_down =
        config.enable_shanten_down && !riichi &&
        distance(hand_counts, hand_org) + shanten < shanten_org + config.extra;
    disc |= (disc & (1LL << Tile::Manzu5)) ? (1LL << Tile::RedManzu5) : 0;
    disc |= (disc & (1LL << Tile::Pinzu5)) ? (1LL << Tile::RedPinzu5) : 0;
    disc |= (disc & (1LL << Tile::Souzu5)) ? (1LL << Tile::RedSouzu5) : 0;

    // Add vertex to graph.
#ifdef DEBUG_GRAPH
    const Vertex vertex =
        boost::add_vertex(VertexData(config.t_max + 1, shanten == 0, shanten == -1, 0.0,
                                     player, shanten, riichi),
                          graph);
#else
    const Vertex vertex = boost::add_vertex(
        VertexData(config.t_max + 1, shanten == 0, shanten == -1, 0.0), graph);
#endif
    cache2[key] = vertex;
    const bool can_call_riichi =
        !riichi && config.enable_riichi && player.is_closed() && shanten == 0;

    for (int i = 0; i < 37; ++i) {
        bool is_disc = disc & (1LL << i);

        if (hand_counts[i] && (allow_shanten_down || is_disc)) {
            if (can_call_riichi && is_disc) {
                // 現在の状態が聴牌の場合、立直宣言するパターンも探索する
                discard(player, hand_counts, wall_counts, i);

                const int weight = wall_counts[i];
                const Vertex source =
                    draw_node(config, round, player, graph, cache1, cache2, hand_counts,
                              wall_counts, hand_org, shanten_org, true);

                draw(player, hand_counts, wall_counts, i);

                if (!boost::edge(source, vertex, graph).second) {
#ifdef DEBUG_GRAPH
                    boost::add_edge(source, vertex, {weight, 0, i, true}, graph);
#else
                    boost::add_edge(source, vertex, {weight, 0}, graph);
#endif
                }
            }

            discard(player, hand_counts, wall_counts, i);

            const int weight = wall_counts[i];
            const Vertex source =
                draw_node(config, round, player, graph, cache1, cache2, hand_counts,
                          wall_counts, hand_org, shanten_org, riichi);

            draw(player, hand_counts, wall_counts, i);

            if (!boost::edge(source, vertex, graph).second) {
                // 打牌前の時点で向聴数が-1の場合、和了形のため、点数計算を行う
                const int score =
                    (shanten == -1 ? calc_score(config, round, player, hand_counts,
                                                wall_counts, type, i, riichi)
                                   : 0);

#ifdef DEBUG_GRAPH
                boost::add_edge(source, vertex, {weight, score, i, false}, graph);
#else
                boost::add_edge(source, vertex, {weight, score}, graph);
#endif
            }
        }
    }

    return vertex;
}

/**
 * @brief Calculate the probability of tenpai, the probability of winning, and the expected score.
 *        https://github.com/nekobean/mahjong-cpp/wiki/%E8%81%B4%E7%89%8C%E7%A2%BA%E7%8E%87%E3%80%81%E5%92%8C%E4%BA%86%E7%A2%BA%E7%8E%87%E3%80%81%E7%82%B9%E6%95%B0%E6%9C%9F%E5%BE%85%E5%80%A4
 * @param config Configulation
 * @param graph Graph
 * @param cache1 List of draw node
 * @param cache2 List of discard node
 */
void ExpectedScoreCalculator::calc_stats(const Config &config, Graph &graph,
                                         const Cache &cache1, const Cache &cache2)
{
    for (int t = config.t_max; t >= config.t_min; --t) {
        // draw node
        if (t < config.t_max) {
            for (const auto &[_, vertex] : cache1) {
                VertexData &s1 = graph[vertex];
                for (const auto &edge :
                     boost::make_iterator_range(boost::out_edges(vertex, graph))) {
#ifdef DEBUG_GRAPH
                    const auto &[weight, score, tile, riichi] = graph[edge];
#else
                    const auto &[weight, score] = graph[edge];
#endif
                    const VertexData &s2 = graph[boost::target(edge, graph)];

                    s1.tenpai_prob[t] +=
                        weight * (s2.tenpai_prob[t + 1] - s1.tenpai_prob[t + 1]);
                    s1.win_prob[t] +=
                        weight * (s2.win_prob[t + 1] - s1.win_prob[t + 1]);
                    s1.exp_score[t] += weight * (std::max(static_cast<double>(score),
                                                          s2.exp_score[t + 1]) -
                                                 s1.exp_score[t + 1]);
                }

                s1.tenpai_prob[t] =
                    s1.tenpai_prob[t + 1] + s1.tenpai_prob[t] / (config.sum - t);
                s1.win_prob[t] = s1.win_prob[t + 1] + s1.win_prob[t] / (config.sum - t);
                s1.exp_score[t] =
                    s1.exp_score[t + 1] + s1.exp_score[t] / (config.sum - t);
            }
        }

        // discard node
        for (const auto &[_, vertex] : cache2) {
            VertexData &s1 = graph[vertex];
            for (const auto &edge :
                 boost::make_iterator_range(boost::in_edges(vertex, graph))) {
                const VertexData &s2 = graph[boost::source(edge, graph)];
                s1.tenpai_prob[t] = std::max(s1.tenpai_prob[t], s2.tenpai_prob[t]);
                s1.win_prob[t] = std::max(s1.win_prob[t], s2.win_prob[t]);
                s1.exp_score[t] = std::max(s1.exp_score[t], s2.exp_score[t]);
            }
        }
    }
}

std::tuple<int, std::vector<std::tuple<int, int>>>
ExpectedScoreCalculator::get_necessary_tiles(const Config &config, const Player &player,
                                             const Count &wall)
{
    const auto [shanten_type, shanten, tiles] = NecessaryTileCalculator::select(
        player.hand, player.num_melds(), config.shanten_type);

    std::vector<std::tuple<int, int>> necessary_tiles;
    for (const auto tile : tiles) {
        necessary_tiles.emplace_back(tile, wall[tile]);
    }

    return {shanten, necessary_tiles};
}

/**
 * @brief グラフを dot 形式で出力する。
 *
 * @param filename Filename
 * @param graph Graph
 */
void ExpectedScoreCalculator::write_graph(const std::string &filename,
                                          const Graph &graph)
{
#ifdef DEBUG_GRAPH
    const auto vertex_writer = [&](std::ostream &out, const Vertex &v) {
        const VertexData &state = graph[v];
        const std::string shanten = state.shanten == -1 ? "和了"
                                    : state.shanten == 0
                                        ? "聴牌"
                                        : std::to_string(state.shanten) + "向聴";
        const std::string riichi = state.riichi ? "立直" : "ダマ";
        out << fmt::format("[label=\"{} {}\n({}, {})\"]", v, to_mpsz(state.player.hand),
                           shanten, riichi);
    };

    const auto edge_writer = [&](std::ostream &out, const Edge &e) {
        const auto &[weight, score, tile, call_riichi] = graph[e];
        out << fmt::format("[label=\"({}, {}, {}{})\"]", weight, score,
                           Tile::Name.at(tile), call_riichi ? ", 立直" : "");
    };

    std::ofstream file(filename);
    boost::write_graphviz(file, graph, vertex_writer, edge_writer);
#endif
}

std::tuple<std::vector<ExpectedScoreCalculator::Stat>, int>
ExpectedScoreCalculator::calc(const Config &config, const Round &round,
                              const Player &player)
{
    const Count wall = create_wall(round, player, config.enable_reddora);

    return calc(config, round, player, wall);
}

ExpectedScoreCalculator::VertexData
ExpectedScoreCalculator::get_stat(Graph &graph, const Cache &cache1,
                                  CountRed &hand_counts)
{
    const auto itr_dama = cache1.find(CacheKey(hand_counts, false));
    const auto itr_riichi = cache1.find(CacheKey(hand_counts, true));

    if (itr_dama != cache1.end() && itr_riichi != cache1.end()) {
        const VertexData &state1 = graph[itr_dama->second];
        const VertexData &state2 = graph[itr_riichi->second];

        if (state1.exp_score > state2.exp_score) {
            return state1;
        }
        else {
            return state2;
        }
    }
    else {
        assert(itr_dama != cache1.end());
        return graph[itr_dama->second];
    }
}

std::tuple<std::vector<ExpectedScoreCalculator::Stat>, int>
ExpectedScoreCalculator::calc(const Config &_config, const Round &round,
                              const Player &_player, const Count &wall)
{
    Graph graph;
    Cache cache1, cache2;

    Config config = _config;
    if (config.sum == 0) {
        config.sum = std::accumulate(wall.begin(), wall.begin() + 34, 0);
    }

    Player player = _player;
    // 手牌と山の各牌の枚数を作成する。
    // 赤ドラありの場合、赤なしの5と赤ありの5は別々に管理する。
    CountRed hand_counts = encode(player.hand, config.enable_reddora);
    CountRed wall_counts = encode(wall, config.enable_reddora);

    // Calculate shanten number of specified hand.
    const CountRed hand_org = hand_counts;
    const int shanten_org = std::get<1>(
        ShantenCalculator::calc(player.hand, player.num_melds(), config.shanten_type));
    std::vector<Stat> stats;

    if (player.num_tiles() % 3 == 1) {
        // 13枚の場合
        if (config.calc_stats) {
            // 期待値、確率計算を行う場合
            if (config.enable_riichi && player.is_closed() && shanten_org == 0) {
                // 聴牌の場合、すでに立直済みである場合も検索する
                draw_node(config, round, player, graph, cache1, cache2, hand_counts,
                          wall_counts, hand_org, shanten_org, true);
            }

            draw_node(config, round, player, graph, cache1, cache2, hand_counts,
                      wall_counts, hand_org, shanten_org, false);

            // 確率、期待値を計算する。
            calc_stats(config, graph, cache1, cache2);

            // 結果を取得する。
            const VertexData state = get_stat(graph, cache1, hand_counts);
            // 有効牌の一覧を計算する。
            const auto [shanten2, necessary_tiles] =
                get_necessary_tiles(config, player, wall);

            stats.emplace_back(Stat{Tile::Null, state.tenpai_prob, state.win_prob,
                                    state.exp_score, necessary_tiles, shanten2});
        }
        else {
            const auto [shanten2, necessary_tiles] =
                get_necessary_tiles(config, player, wall);
            stats.emplace_back(Stat{Tile::Null, {}, {}, {}, necessary_tiles, shanten2});
        }
    }
    else if (player.num_tiles() % 3 == 2) {
        if (config.calc_stats) {
            // 期待値、確率計算を行う場合

            // 14枚の場合は打牌を起点に手牌遷移のグラフを作成する。
            discard_node(config, round, player, graph, cache1, cache2, hand_counts,
                         wall_counts, hand_org, shanten_org, false);

            // 確率、期待値を計算する。
            calc_stats(config, graph, cache1, cache2);

            // 結果を取得する。
            for (int i = 0; i < 37; ++i) {
                if (hand_counts[i] > 0) {
                    discard(player, hand_counts, wall_counts, i);

                    // 結果を取得する。
                    const VertexData state = get_stat(graph, cache1, hand_counts);
                    // 有効牌の一覧を計算する。
                    const auto [shanten2, necessary_tiles] =
                        get_necessary_tiles(config, player, wall);

                    stats.emplace_back(Stat{i, state.tenpai_prob, state.win_prob,
                                            state.exp_score, necessary_tiles,
                                            shanten2});

                    draw(player, hand_counts, wall_counts, i);
                }
            }
        }
        else {
            for (int i = 0; i < 37; ++i) {
                if (hand_counts[i] > 0) {
                    discard(player, hand_counts, wall_counts, i);
                    const auto [shanten2, necessary_tiles] =
                        get_necessary_tiles(config, player, wall);
                    stats.emplace_back(Stat{i, {}, {}, {}, necessary_tiles, shanten2});
                    draw(player, hand_counts, wall_counts, i);
                }
            }
        }
    }

    const int searched = static_cast<int>(boost::num_vertices(graph));
#ifdef DEBUG_GRAPH
    // const std::string filename =
    //     fmt::format("{}_reddora={}_riichi={}.dot", to_mpsz(player.hand),
    //                 config.enable_reddora, config.enable_riichi);
    // write_graph(R"(E:\work\mahjong-cpp\)" + filename, graph);

    std::map<int, int> shanten_count;
    std::map<int, int> edge_count;
    for (const auto &vertex : boost::make_iterator_range(boost::vertices(graph))) {
        shanten_count[graph[vertex].shanten]++;
    }

    for (const auto &edge : boost::make_iterator_range(boost::edges(graph))) {
        edge_count[std::get<1>(graph[edge]) != 0]++;
    }

    for (const auto &[shanten, count] : shanten_count) {
        spdlog::info(u8"向聴数: {}, 数: {}", shanten, count);
    }
    for (const auto &[is_calc, count] : edge_count) {
        spdlog::info(u8"点数計算: {}, 数: {}", is_calc, count);
    }
    spdlog::info(u8"頂点数: {}", static_cast<int>(boost::num_vertices(graph)));
    spdlog::info(u8"辺数: {}", static_cast<int>(boost::num_edges(graph)));
#endif

    return {stats, searched};
}

bool ExpectedScoreCalculator::load_uradora_table()
{
    boost::filesystem::path path =
        boost::dll::program_location().parent_path() / "uradora.bin";
    std::ifstream ifs(path.string(), std::ios::binary);
    ifs.read(reinterpret_cast<char *>(&uradora_table_), sizeof(uradora_table_));

    spdlog::info(u8"Uradora table file loaded. (path: {})", path.string());

    return true;
}

ExpectedScoreCalculator::ExpectedScoreCalculator()
{
    load_uradora_table();
}

std::array<std::array<double, 13>, 6> ExpectedScoreCalculator::uradora_table_;

static ExpectedScoreCalculator inst;

} // namespace mahjong
