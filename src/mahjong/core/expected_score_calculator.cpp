#include "expected_score_calculator.hpp"

#undef NDEBUG
#include <algorithm> // max, fill
#include <cassert>

#include <boost/dll.hpp>
#include <boost/graph/graph_utility.hpp>

#include "mahjong/core/necessary_tile_calculator.hpp"
#include "mahjong/core/score_calculator.hpp"
#include "mahjong/core/shanten_calculator.hpp"
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

ExpectedScoreCalculator::Vertex ExpectedScoreCalculator::select1(
    const Config &config, const Round &round, Player &player, Graph &graph,
    Cache &cache1, Cache &cache2, CountRed &hand_counts, CountRed &wall_counts,
    const CountRed &hand_org, const int shanten_org, const bool riichi)
{
    // If vertex exists in the cache, return it.
    CacheKey key(hand_counts);
    if (const auto itr = cache1.find(key); itr != cache1.end()) {
        return itr->second;
    }

    // Calculate necessary tiles.
    auto [type, shanten, wait] = NecessaryTileCalculator::calc(
        player.hand, player.num_melds(), config.shanten_type);

    // ToDo: 手変わりは立直していない場合のみ有効にするべき
    // bool allow_tegawari =
    //     config.enable_tegawari && !riichi &&
    //     distance(hand_counts, hand_org) + shanten < shanten_org + config.extra;

    // 元の手牌から現在の手牌に変化されるのに必要な交換枚数 + 現在の向聴数 < 元の手牌の向聴数 + 追加交換枚数
    // の場合、手変わりを許可する
    bool allow_tegawari =
        config.enable_tegawari &&
        distance(hand_counts, hand_org) + shanten < shanten_org + config.extra;
    wait |= (wait & (1LL << Tile::Manzu5)) ? (1LL << Tile::RedManzu5) : 0;
    wait |= (wait & (1LL << Tile::Pinzu5)) ? (1LL << Tile::RedPinzu5) : 0;
    wait |= (wait & (1LL << Tile::Souzu5)) ? (1LL << Tile::RedSouzu5) : 0;

    // Add vertex to graph.
    VertexData state(config.t_max + 1, shanten <= 0, shanten == -1, 0.0);
    const Vertex vertex = boost::add_vertex(state, graph);
    cache1[key] = vertex;

    for (int i = 0; i < 37; ++i) {
        bool is_wait = wait & (1LL << i);

        if (wall_counts[i] && (allow_tegawari || is_wait)) {
            const int weight = wall_counts[i];

            draw(player, hand_counts, wall_counts, i);

            // 1向聴で有効牌を自摸した場合、聴牌なので立直する
            bool call_riichi =
                config.enable_riichi && player.is_closed() && shanten == 1 && is_wait
                    ? true
                    : riichi;

            const Vertex target =
                select2(config, round, player, graph, cache1, cache2, hand_counts,
                        wall_counts, hand_org, shanten_org, call_riichi);

            if (!boost::edge(vertex, target, graph).second) {
                // 自摸前の時点で聴牌の場合、有効牌自摸後は和了形のため、点数計算を行う
                const int score = (shanten == 0 && is_wait
                                       ? calc_score(config, round, player, hand_counts,
                                                    wall_counts, type, i, riichi)
                                       : 0);
                boost::add_edge(vertex, target, {weight, score}, graph);
            }

            discard(player, hand_counts, wall_counts, i);
        }
    }

    return vertex;
}

ExpectedScoreCalculator::Vertex ExpectedScoreCalculator::select2(
    const Config &config, const Round &round, Player &player, Graph &graph,
    Cache &cache1, Cache &cache2, CountRed &hand_counts, CountRed &wall_counts,
    const CountRed &hand_org, const int shanten_org, const bool riichi)
{
    // If vertex exists in the cache, return it.
    CacheKey key(hand_counts);
    if (const auto itr = cache2.find(key); itr != cache2.end()) {
        return itr->second;
    }

    // Calculate unnecessary tiles.
    auto [type, shanten, disc] = UnnecessaryTileCalculator::calc(
        player.hand, player.num_melds(), config.shanten_type);

    // ToDo: 向聴戻しは立直していない場合のみ有効にするべき
    // bool allow_shanten_down =
    //     config.enable_shanten_down && !riichi &&
    //     distance(hand_counts, hand_org) + shanten < shanten_org + config.extra;

    // 元の手牌から現在の手牌に変化されるのに必要な交換枚数 + 現在の向聴数 < 元の手牌の向聴数 + 追加交換枚数
    // の場合、向聴戻しを許可する
    bool allow_shanten_down =
        config.enable_shanten_down &&
        distance(hand_counts, hand_org) + shanten < shanten_org + config.extra;
    disc |= (disc & (1LL << Tile::Manzu5)) ? (1LL << Tile::RedManzu5) : 0;
    disc |= (disc & (1LL << Tile::Pinzu5)) ? (1LL << Tile::RedPinzu5) : 0;
    disc |= (disc & (1LL << Tile::Souzu5)) ? (1LL << Tile::RedSouzu5) : 0;

    // Add vertex to graph.
    VertexData state(config.t_max + 1, shanten <= 0, shanten == -1, 0.0);
    const Vertex vertex = boost::add_vertex(state, graph);
    cache2[key] = vertex;

    for (int i = 0; i < 37; ++i) {
        bool is_disc = disc & (1LL << i);

        if (hand_counts[i] && (allow_shanten_down || is_disc)) {
            discard(player, hand_counts, wall_counts, i);

            const int weight = wall_counts[i];
            const Vertex source =
                select1(config, round, player, graph, cache1, cache2, hand_counts,
                        wall_counts, hand_org, shanten_org, riichi);

            draw(player, hand_counts, wall_counts, i);

            if (!boost::edge(source, vertex, graph).second) {
                // 打牌前の時点で向聴数が-1の場合、和了形のため、点数計算を行う
                const int score =
                    (shanten == -1 ? calc_score(config, round, player, hand_counts,
                                                wall_counts, type, i, riichi)
                                   : 0);
                boost::add_edge(source, vertex, {weight, score}, graph);
            }
        }
    }

    return vertex;
}

void ExpectedScoreCalculator::calc_values(const Config &config, Graph &graph,
                                          const Cache &cache1, const Cache &cache2)
{
    for (int t = config.t_max - 1; t >= config.t_min; --t) {
        // draw node
        for (auto &[_, vertex] : cache1) {
            VertexData &state = graph[vertex];
            for (auto [first, last] = boost::out_edges(vertex, graph); first != last;
                 ++first) {
                const auto target = boost::target(*first, graph);
                const auto [weight, score] = graph[*first];
                const VertexData &state2 = graph[target];

                state.tenpai_prob[t] +=
                    weight * (state2.tenpai_prob[t + 1] - state.tenpai_prob[t + 1]);
                state.win_prob[t] +=
                    weight * (state2.win_prob[t + 1] - state.win_prob[t + 1]);
                state.exp_score[t] += weight * (std::max(static_cast<double>(score),
                                                         state2.exp_score[t + 1]) -
                                                state.exp_score[t + 1]);
            }

            state.tenpai_prob[t] =
                state.tenpai_prob[t + 1] + state.tenpai_prob[t] / config.sum;
            state.win_prob[t] = state.win_prob[t + 1] + state.win_prob[t] / config.sum;
            state.exp_score[t] =
                state.exp_score[t + 1] + state.exp_score[t] / config.sum;
        }

        // discard node
        for (auto &[_, vertex] : cache2) {
            VertexData &state = graph[vertex];

            for (auto [first, last] = boost::in_edges(vertex, graph); first != last;
                 ++first) {
                const auto source = boost::source(*first, graph);
                const VertexData &state2 = graph[source];

                state.tenpai_prob[t] =
                    std::max(state.tenpai_prob[t], state2.tenpai_prob[t]);
                state.win_prob[t] = std::max(state.win_prob[t], state2.win_prob[t]);
                state.exp_score[t] = std::max(state.exp_score[t], state2.exp_score[t]);
            }
        }
    }
}

std::tuple<std::vector<ExpectedScoreCalculator::Stat>, int>
ExpectedScoreCalculator::calc(const Config &config, const Round &round,
                              const Player &player)
{
    const Count wall = create_wall(round, player, config.enable_reddora);

    return calc(config, round, player, wall);
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

std::tuple<std::vector<ExpectedScoreCalculator::Stat>, int>
ExpectedScoreCalculator::calc(const Config &_config, const Round &round,
                              const Player &_player, const Count &wall)
{
    // Vertex に格納している (t_max + 1) * 3 の長さの1次元配列のうち、
    // 1 ~ t_max が tenpai_prob,
    // t_max + 1 ~ t_max * 2 + 1 が win_prob,
    // t_max * 2 + 2 ~ t_max * 3 + 2 が exp_score の値を格納している
    Graph graph;
    Cache cache1, cache2;

    Config config = _config;
    if (config.sum == 0) {
        config.sum = std::accumulate(wall.begin(), wall.begin() + 34, 0);
    }

    // 設定で聴牌時に立直orダマの分岐を行うように考慮する予定だが、未実装
    // 立直なしの場合、三暗刻などの役が高く評価されてしまい、実戦と乖離した結果が出る傾向があるため、
    // 現状は立直を強制的に有効にした (2024/11/25)
    config.enable_riichi = true;

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
    const int num_tiles = player.num_tiles() + player.num_melds() * 3;

    // 聴牌の場合、立直する。
    // ToDo: ダマの場合の手牌遷移も探索する必要あり
    const bool riichi = config.enable_riichi && player.is_closed() && shanten_org <= 0;

    if (num_tiles == 13) {
        // 13枚の場合
        if (config.calc_stats) {
            // 期待値、確率計算を行う場合

            // 13枚の場合は自摸を起点に手牌遷移のグラフを作成する。
            select1(config, round, player, graph, cache1, cache2, hand_counts,
                    wall_counts, hand_org, shanten_org, riichi);

            // 確率、期待値を計算する。
            calc_values(config, graph, cache1, cache2);

            // 結果を取得する。
            if (const auto itr = cache1.find(hand_counts); itr != cache1.end()) {
                const VertexData &state = graph[itr->second];

                // 有効牌の一覧を計算する。
                const auto [shanten2, necessary_tiles] =
                    get_necessary_tiles(config, player, wall);

                stats.emplace_back(Stat{Tile::Null, state.tenpai_prob, state.win_prob,
                                        state.exp_score, necessary_tiles, shanten2});
            }
        }
        else {
            const auto [shanten2, necessary_tiles] =
                get_necessary_tiles(config, player, wall);
            stats.emplace_back(Stat{Tile::Null, {}, {}, {}, necessary_tiles, shanten2});
        }
    }
    else {
        if (config.calc_stats) {
            // 期待値、確率計算を行う場合

            // 14枚の場合は打牌を起点に手牌遷移のグラフを作成する。
            select2(config, round, player, graph, cache1, cache2, hand_counts,
                    wall_counts, hand_org, shanten_org, riichi);

            // 確率、期待値を計算する。
            calc_values(config, graph, cache1, cache2);

            // 結果を取得する。
            for (int i = 0; i < 37; ++i) {
                if (hand_counts[i] > 0) {
                    discard(player, hand_counts, wall_counts, i);
                    if (const auto itr = cache1.find(hand_counts);
                        itr != cache1.end()) {
                        const VertexData &state = graph[itr->second];

                        // 有効牌の一覧を計算する。
                        const auto [shanten2, necessary_tiles] =
                            get_necessary_tiles(config, player, wall);

                        stats.emplace_back(Stat{i, state.tenpai_prob, state.win_prob,
                                                state.exp_score, necessary_tiles,
                                                shanten2});
                    }
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
