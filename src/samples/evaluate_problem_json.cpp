#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "mahjong/mahjong.hpp"

namespace
{

using namespace mahjong;
namespace fs = std::filesystem;

constexpr double ScoreEpsilon = 1e-9;

ExpectedScoreCalculator::Objective objective_from_env()
{
    const char *value = std::getenv("EVALUATE_OBJECTIVE");
    if (value == nullptr) {
        return ExpectedScoreCalculator::Objective::ExpectedScore;
    }
    return static_cast<ExpectedScoreCalculator::Objective>(std::stoi(value));
}

struct Problem
{
    std::string id;
    std::string title;
    std::string hand_mpsz;
    std::string dora_indicators_mpsz;
    int turn = 0;
    std::string answer_mpsz;
};

struct Evaluation
{
    Problem problem;
    std::string status = "ok";
    std::string message;
    int shanten = 0;
    bool calc_stats = false;
    int searched = 0;
    long long time_us = 0;
    int best_tile = Tile::Null;
    double best_score = 0.0;
    std::vector<int> answer_tiles;
    bool answer_matches_best = false;
};

struct SnapshotSummary
{
    int total = 0;
    int ok = 0;
    int no_stats = 0;
    int error = 0;
};

struct Snapshot
{
    std::string problem_path;
    std::vector<Evaluation> entries;
    SnapshotSummary summary;
};

struct SnapshotDiff
{
    int baseline_total = 0;
    int candidate_total = 0;
    int matched_ids = 0;
    int unchanged = 0;
    int score_changed_same_best = 0;
    int rank_changed = 0;
    int status_changed = 0;
    int only_in_baseline = 0;
    int only_in_candidate = 0;
    std::vector<std::string> score_change_lines;
    std::vector<std::string> rank_change_lines;
    std::vector<std::string> status_change_lines;
    std::vector<std::string> missing_lines;
};

struct RangeCheckSummary
{
    int total = 0;
    int ok = 0;
    int no_stats = 0;
    int error = 0;
    int out_of_range = 0;
    double max_tenpai_prob = 0.0;
    double max_win_prob = 0.0;
};

std::vector<int> expand_tiles(const Hand &hand)
{
    std::vector<int> tiles;
    for (int tile = 0; tile < 34; ++tile) {
        int count = hand[tile];
        int red_tile = Tile::Null;

        if (tile == Tile::Manzu5) {
            red_tile = Tile::RedManzu5;
        }
        else if (tile == Tile::Pinzu5) {
            red_tile = Tile::RedPinzu5;
        }
        else if (tile == Tile::Souzu5) {
            red_tile = Tile::RedSouzu5;
        }

        if (red_tile != Tile::Null) {
            count -= hand[red_tile];
        }

        for (int i = 0; i < count; ++i) {
            tiles.push_back(tile);
        }
        if (red_tile != Tile::Null) {
            for (int i = 0; i < hand[red_tile]; ++i) {
                tiles.push_back(red_tile);
            }
        }
    }
    return tiles;
}

std::vector<int> parse_tile_list(const std::string &mpsz)
{
    if (mpsz.empty()) {
        return {};
    }
    const Hand hand = from_mpsz(mpsz);
    return expand_tiles(hand);
}

std::vector<Problem> load_problems(const std::string &path)
{
    std::ifstream ifs(path);
    if (!ifs.is_open()) {
        throw std::runtime_error("Failed to open problem file: " + path);
    }

    rapidjson::IStreamWrapper isw(ifs);
    rapidjson::Document doc;
    if (doc.ParseStream(isw).HasParseError() || !doc.IsArray()) {
        throw std::runtime_error("Problem file must be a JSON array.");
    }

    std::vector<Problem> problems;
    problems.reserve(doc.Size());
    for (const auto &item : doc.GetArray()) {
        if (!item.IsObject()) {
            throw std::runtime_error("Each problem entry must be an object.");
        }

        Problem problem;
        problem.id = item["id"].GetString();
        problem.title = item["title"].GetString();
        problem.hand_mpsz = item["hand"].GetString();
        problem.dora_indicators_mpsz = item["dora_indicators"].GetString();
        problem.turn = item["turn"].GetInt();
        problem.answer_mpsz = item["answer"].GetString();
        problems.push_back(problem);
    }

    return problems;
}

Round make_round(const Problem &problem)
{
    Round round;
    round.rules = RuleFlag::OpenTanyao | RuleFlag::RedDora;
    round.wind = Tile::East;
    round.kyoku = 1;
    round.honba = 0;
    round.kyotaku = 0;
    round.dora_indicators = parse_tile_list(problem.dora_indicators_mpsz);
    return round;
}

Player make_player(const Problem &problem)
{
    Player player;
    player.hand = from_mpsz(problem.hand_mpsz);
    player.wind = Tile::East;
    return player;
}

ExpectedScoreCalculator::Config make_config(const Round &round, const Player &player)
{
    ExpectedScoreCalculator::Config config;
    config.t_min = 1;
    config.t_max = 18;
    config.extra = 1;
    config.shanten_type = ShantenFlag::All;
    config.enable_reddora = true;
    config.enable_uradora = true;
    config.enable_shanten_down = true;
    config.enable_tegawari = true;
    config.objective = objective_from_env();
    const MergedCount wall = create_wall(round, player, config.enable_reddora);
    config.sum = std::accumulate(wall.begin(), wall.begin() + 34, 0);
    return config;
}

bool same_score(double lhs, double rhs)
{
    return std::fabs(lhs - rhs) <= ScoreEpsilon;
}

std::string tile_name(const int tile)
{
    return tile == Tile::Null ? "null" : Tile::name(tile);
}

Evaluation evaluate_problem(const Problem &problem)
{
    Evaluation evaluation;
    evaluation.problem = problem;

    try {
        const Round round = make_round(problem);
        const Player player = make_player(problem);
        ExpectedScoreCalculator::Config config = make_config(round, player);

        evaluation.shanten = std::get<1>(ShantenCalculator::calc(
            player.hand, player.num_melds(), config.shanten_type, round.mode));
        evaluation.answer_tiles = parse_tile_list(problem.answer_mpsz);
        evaluation.calc_stats = evaluation.shanten <= 3;

        if (evaluation.shanten == -1) {
            evaluation.status = "error";
            evaluation.message = "Winning hand is not supported.";
            return evaluation;
        }
        if (!evaluation.calc_stats) {
            evaluation.status = "no_stats";
            evaluation.message = "Shanten is greater than 3.";
            return evaluation;
        }
        const MergedCount wall = create_wall(round, player, config.enable_reddora);

        const auto start = std::chrono::steady_clock::now();
        const auto [stats, searched] =
            ExpectedScoreCalculator::calc(config, round, player, wall);
        const auto end = std::chrono::steady_clock::now();

        evaluation.searched = searched;
        evaluation.time_us =
            std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        if (stats.empty()) {
            evaluation.status = "error";
            evaluation.message = "No stats were returned.";
            return evaluation;
        }
        if (problem.turn < config.t_min ||
            problem.turn >= static_cast<int>(stats.front().exp_score.size())) {
            evaluation.status = "error";
            evaluation.message = "Problem turn is out of calculated range.";
            return evaluation;
        }

        bool best_found = false;
        for (const auto &stat : stats) {
            const double score = stat.exp_score[problem.turn];
            if (!best_found || score > evaluation.best_score + ScoreEpsilon) {
                best_found = true;
                evaluation.best_tile = stat.tile;
                evaluation.best_score = score;
            }
        }

        evaluation.answer_matches_best =
            best_found &&
            std::find(evaluation.answer_tiles.begin(), evaluation.answer_tiles.end(),
                      evaluation.best_tile) != evaluation.answer_tiles.end();
    }
    catch (const std::exception &e) {
        evaluation.status = "error";
        evaluation.message = e.what();
    }

    return evaluation;
}

Snapshot create_snapshot(const std::string &problem_path)
{
    Snapshot snapshot;
    snapshot.problem_path = problem_path;

    const std::vector<Problem> problems = load_problems(problem_path);
    snapshot.entries.reserve(problems.size());
    for (const auto &problem : problems) {
        snapshot.entries.push_back(evaluate_problem(problem));
    }

    snapshot.summary.total = static_cast<int>(snapshot.entries.size());
    for (const auto &entry : snapshot.entries) {
        if (entry.status == "ok") {
            ++snapshot.summary.ok;
        }
        else if (entry.status == "no_stats") {
            ++snapshot.summary.no_stats;
        }
        else {
            ++snapshot.summary.error;
        }
    }

    return snapshot;
}

RangeCheckSummary check_probability_ranges(const std::string &problem_path)
{
    RangeCheckSummary summary;
    const std::vector<Problem> problems = load_problems(problem_path);
    summary.total = static_cast<int>(problems.size());

    for (const auto &problem : problems) {
        try {
            const Round round = make_round(problem);
            const Player player = make_player(problem);
            ExpectedScoreCalculator::Config config = make_config(round, player);
            const int shanten = std::get<1>(ShantenCalculator::calc(
                player.hand, player.num_melds(), config.shanten_type, round.mode));

            if (shanten == -1) {
                ++summary.error;
                std::cout << problem.id << " error winning_hand\n";
                continue;
            }
            if (shanten > 3) {
                ++summary.no_stats;
                continue;
            }
            const MergedCount wall = create_wall(round, player, config.enable_reddora);
            const auto [stats, searched] =
                ExpectedScoreCalculator::calc(config, round, player, wall);
            bool problem_ok = true;

            for (const auto &stat : stats) {
                for (const double value : stat.tenpai_prob) {
                    summary.max_tenpai_prob = std::max(summary.max_tenpai_prob, value);
                    if (value < -ScoreEpsilon || value > 1.0 + ScoreEpsilon) {
                        problem_ok = false;
                        std::cout << problem.id << " tenpai_out_of_range tile="
                                  << tile_name(stat.tile) << " value=" << value << "\n";
                    }
                }
                for (const double value : stat.win_prob) {
                    summary.max_win_prob = std::max(summary.max_win_prob, value);
                    if (value < -ScoreEpsilon || value > 1.0 + ScoreEpsilon) {
                        problem_ok = false;
                        std::cout << problem.id
                                  << " win_out_of_range tile=" << tile_name(stat.tile)
                                  << " value=" << value << "\n";
                    }
                }
            }

            ++summary.ok;
            if (!problem_ok) {
                ++summary.out_of_range;
            }
        }
        catch (const std::exception &e) {
            ++summary.error;
            std::cout << problem.id << " error reason=\"" << e.what() << "\"\n";
        }
    }

    return summary;
}

rapidjson::Value to_json_array(const std::vector<int> &values, rapidjson::Document &doc)
{
    auto &allocator = doc.GetAllocator();
    rapidjson::Value array(rapidjson::kArrayType);
    for (const int value : values) {
        array.PushBack(value, allocator);
    }
    return array;
}

rapidjson::Value to_json_entry(const Evaluation &entry, rapidjson::Document &doc)
{
    auto &allocator = doc.GetAllocator();
    rapidjson::Value value(rapidjson::kObjectType);
    value.AddMember("id", rapidjson::Value(entry.problem.id.c_str(), allocator),
                    allocator);
    value.AddMember("title", rapidjson::Value(entry.problem.title.c_str(), allocator),
                    allocator);
    value.AddMember("hand",
                    rapidjson::Value(entry.problem.hand_mpsz.c_str(), allocator),
                    allocator);
    value.AddMember(
        "dora_indicators",
        rapidjson::Value(entry.problem.dora_indicators_mpsz.c_str(), allocator),
        allocator);
    value.AddMember("turn", entry.problem.turn, allocator);
    value.AddMember("answer",
                    rapidjson::Value(entry.problem.answer_mpsz.c_str(), allocator),
                    allocator);
    value.AddMember("answer_tiles", to_json_array(entry.answer_tiles, doc), allocator);
    value.AddMember("status", rapidjson::Value(entry.status.c_str(), allocator),
                    allocator);
    value.AddMember("message", rapidjson::Value(entry.message.c_str(), allocator),
                    allocator);
    value.AddMember("shanten", entry.shanten, allocator);
    value.AddMember("calc_stats", entry.calc_stats, allocator);
    value.AddMember("searched", entry.searched, allocator);
    value.AddMember("time_us", static_cast<int64_t>(entry.time_us), allocator);
    value.AddMember("best_tile", entry.best_tile, allocator);
    value.AddMember("best_tile_name",
                    rapidjson::Value(tile_name(entry.best_tile).c_str(), allocator),
                    allocator);
    value.AddMember("best_score", entry.best_score, allocator);
    value.AddMember("answer_matches_best", entry.answer_matches_best, allocator);
    return value;
}

rapidjson::Document to_json_document(const Snapshot &snapshot)
{
    rapidjson::Document doc;
    doc.SetObject();
    auto &allocator = doc.GetAllocator();

    doc.AddMember("tool", "evaluate_problem_json", allocator);
    doc.AddMember("problem_path",
                  rapidjson::Value(snapshot.problem_path.c_str(), allocator),
                  allocator);

    rapidjson::Value summary(rapidjson::kObjectType);
    summary.AddMember("total", snapshot.summary.total, allocator);
    summary.AddMember("ok", snapshot.summary.ok, allocator);
    summary.AddMember("no_stats", snapshot.summary.no_stats, allocator);
    summary.AddMember("error", snapshot.summary.error, allocator);
    doc.AddMember("summary", summary, allocator);

    rapidjson::Value entries(rapidjson::kArrayType);
    for (const auto &entry : snapshot.entries) {
        entries.PushBack(to_json_entry(entry, doc), allocator);
    }
    doc.AddMember("entries", entries, allocator);

    return doc;
}

void write_json(const rapidjson::Document &doc, const std::string &path)
{
    const fs::path output_path(path);
    if (output_path.has_parent_path()) {
        fs::create_directories(output_path.parent_path());
    }

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    writer.SetMaxDecimalPlaces(12);
    doc.Accept(writer);

    std::ofstream ofs(path, std::ios::binary);
    if (!ofs.is_open()) {
        throw std::runtime_error("Failed to open output file: " + path);
    }
    ofs << buffer.GetString();
}

Snapshot load_snapshot(const std::string &path)
{
    std::ifstream ifs(path);
    if (!ifs.is_open()) {
        throw std::runtime_error("Failed to open snapshot file: " + path);
    }

    rapidjson::IStreamWrapper isw(ifs);
    rapidjson::Document doc;
    if (doc.ParseStream(isw).HasParseError() || !doc.IsObject()) {
        throw std::runtime_error("Snapshot file must be a JSON object: " + path);
    }

    Snapshot snapshot;
    snapshot.problem_path = doc["problem_path"].GetString();
    snapshot.summary.total = doc["summary"]["total"].GetInt();
    snapshot.summary.ok = doc["summary"]["ok"].GetInt();
    snapshot.summary.no_stats = doc["summary"]["no_stats"].GetInt();
    snapshot.summary.error = doc["summary"]["error"].GetInt();

    const auto entries = doc["entries"].GetArray();
    snapshot.entries.reserve(entries.Size());
    for (const auto &item : entries) {
        Evaluation entry;
        entry.problem.id = item["id"].GetString();
        entry.problem.title = item["title"].GetString();
        entry.problem.hand_mpsz = item["hand"].GetString();
        entry.problem.dora_indicators_mpsz = item["dora_indicators"].GetString();
        entry.problem.turn = item["turn"].GetInt();
        entry.problem.answer_mpsz = item["answer"].GetString();
        for (const auto &answer_tile : item["answer_tiles"].GetArray()) {
            entry.answer_tiles.push_back(answer_tile.GetInt());
        }
        entry.status = item["status"].GetString();
        entry.message = item["message"].GetString();
        entry.shanten = item["shanten"].GetInt();
        entry.calc_stats = item["calc_stats"].GetBool();
        entry.searched = item["searched"].GetInt();
        entry.time_us = item["time_us"].GetInt64();
        entry.best_tile = item["best_tile"].GetInt();
        entry.best_score = item["best_score"].GetDouble();
        entry.answer_matches_best = item["answer_matches_best"].GetBool();
        snapshot.entries.push_back(entry);
    }

    return snapshot;
}

SnapshotDiff compare_snapshots(const Snapshot &baseline, const Snapshot &candidate)
{
    SnapshotDiff diff;
    diff.baseline_total = static_cast<int>(baseline.entries.size());
    diff.candidate_total = static_cast<int>(candidate.entries.size());

    std::unordered_map<std::string, const Evaluation *> candidate_by_id;
    candidate_by_id.reserve(candidate.entries.size());
    for (const auto &entry : candidate.entries) {
        candidate_by_id[entry.problem.id] = &entry;
    }

    std::unordered_map<std::string, const Evaluation *> baseline_by_id;
    baseline_by_id.reserve(baseline.entries.size());
    for (const auto &entry : baseline.entries) {
        baseline_by_id[entry.problem.id] = &entry;
    }

    for (const auto &base : baseline.entries) {
        const auto it = candidate_by_id.find(base.problem.id);
        if (it == candidate_by_id.end()) {
            ++diff.only_in_baseline;
            diff.missing_lines.push_back(base.problem.id + " only_in_baseline");
            continue;
        }

        ++diff.matched_ids;
        const Evaluation &cand = *it->second;

        if (base.status != cand.status) {
            ++diff.status_changed;
            diff.status_change_lines.push_back(base.problem.id + " status " +
                                               base.status + " -> " + cand.status);
            continue;
        }
        if (base.status != "ok") {
            ++diff.unchanged;
            continue;
        }

        if (base.best_tile != cand.best_tile) {
            ++diff.rank_changed;
            diff.rank_change_lines.push_back(
                base.problem.id + " best " + tile_name(base.best_tile) + " (" +
                std::to_string(base.best_score) + ") -> " + tile_name(cand.best_tile) +
                " (" + std::to_string(cand.best_score) + ")");
        }
        else if (!same_score(base.best_score, cand.best_score)) {
            ++diff.score_changed_same_best;
            diff.score_change_lines.push_back(base.problem.id + " " +
                                              tile_name(base.best_tile) + " score " +
                                              std::to_string(base.best_score) + " -> " +
                                              std::to_string(cand.best_score));
        }
        else {
            ++diff.unchanged;
        }
    }

    for (const auto &cand : candidate.entries) {
        if (baseline_by_id.find(cand.problem.id) == baseline_by_id.end()) {
            ++diff.only_in_candidate;
            diff.missing_lines.push_back(cand.problem.id + " only_in_candidate");
        }
    }

    return diff;
}

rapidjson::Document to_json_document(const SnapshotDiff &diff)
{
    rapidjson::Document doc;
    doc.SetObject();
    auto &allocator = doc.GetAllocator();

    doc.AddMember("matched_ids", diff.matched_ids, allocator);
    doc.AddMember("baseline_total", diff.baseline_total, allocator);
    doc.AddMember("candidate_total", diff.candidate_total, allocator);
    doc.AddMember("unchanged", diff.unchanged, allocator);
    doc.AddMember("score_changed_same_best", diff.score_changed_same_best, allocator);
    doc.AddMember("rank_changed", diff.rank_changed, allocator);
    doc.AddMember("status_changed", diff.status_changed, allocator);
    doc.AddMember("only_in_baseline", diff.only_in_baseline, allocator);
    doc.AddMember("only_in_candidate", diff.only_in_candidate, allocator);

    auto add_string_array = [&](const char *name,
                                const std::vector<std::string> &lines) {
        rapidjson::Value array(rapidjson::kArrayType);
        for (const auto &line : lines) {
            array.PushBack(rapidjson::Value(line.c_str(), allocator), allocator);
        }
        doc.AddMember(rapidjson::Value(name, allocator), array, allocator);
    };

    add_string_array("score_change_lines", diff.score_change_lines);
    add_string_array("rank_change_lines", diff.rank_change_lines);
    add_string_array("status_change_lines", diff.status_change_lines);
    add_string_array("missing_lines", diff.missing_lines);

    return doc;
}

void print_report(const Snapshot &snapshot, int shanten_filter)
{
    int reported = 0;
    int matched = 0;
    int no_stats = 0;
    int failed = 0;
    long long total_time_us = 0;

    for (const auto &entry : snapshot.entries) {
        if (shanten_filter >= 0 && entry.shanten != shanten_filter) {
            continue;
        }

        if (entry.status == "ok") {
            ++reported;
            ++matched;
            matched -= entry.answer_matches_best ? 0 : 1;
            total_time_us += entry.time_us;
            std::cout << entry.problem.id << " "
                      << "shanten=" << entry.shanten << " "
                      << "turn=" << entry.problem.turn << " "
                      << "answer=" << entry.problem.answer_mpsz << " "
                      << "best=" << tile_name(entry.best_tile) << " "
                      << "best_score=" << entry.best_score << " "
                      << "time_us=" << entry.time_us << " "
                      << "searched=" << entry.searched
                      << (entry.answer_matches_best ? " match" : " mismatch") << "\n";
        }
        else if (entry.status == "no_stats") {
            ++no_stats;
            std::cout << entry.problem.id << " no_stats"
                      << " shanten=" << entry.shanten << " reason=\"" << entry.message
                      << "\"\n";
        }
        else {
            ++failed;
            std::cout << entry.problem.id << " error"
                      << " shanten=" << entry.shanten << " reason=\"" << entry.message
                      << "\"\n";
        }
    }

    std::cout << "summary"
              << " reported=" << reported << " matched=" << matched
              << " no_stats=" << no_stats << " failed=" << failed
              << " match_rate=" << (reported == 0 ? 0.0 : 100.0 * matched / reported)
              << "% total_time_us=" << total_time_us
              << " avg_time_us=" << (reported == 0 ? 0 : total_time_us / reported)
              << "\n";
}

void print_diff(const SnapshotDiff &diff)
{
    std::cout << "summary"
              << " matched_ids=" << diff.matched_ids << " unchanged=" << diff.unchanged
              << " score_changed_same_best=" << diff.score_changed_same_best
              << " rank_changed=" << diff.rank_changed
              << " status_changed=" << diff.status_changed
              << " only_in_baseline=" << diff.only_in_baseline
              << " only_in_candidate=" << diff.only_in_candidate << "\n";

    for (const auto &line : diff.rank_change_lines) {
        std::cout << "rank_changed " << line << "\n";
    }
    for (const auto &line : diff.score_change_lines) {
        std::cout << "score_changed " << line << "\n";
    }
    for (const auto &line : diff.status_change_lines) {
        std::cout << "status_changed " << line << "\n";
    }
    for (const auto &line : diff.missing_lines) {
        std::cout << "missing " << line << "\n";
    }
}

void print_usage(const char *argv0)
{
    std::cout << "Usage:\n";
    std::cout << "  " << argv0 << " report <problem.json> [shanten_filter]\n";
    std::cout << "  " << argv0 << " snapshot <problem.json> <output.json>\n";
    std::cout << "  " << argv0
              << " compare <baseline.json> <candidate.json> [output.json]\n";
    std::cout << "  " << argv0 << " range <problem.json>\n";
    std::cout << "  " << argv0 << " <problem.json> [shanten_filter]\n";
}

} // namespace

int main(int argc, char *argv[])
{
    try {
        if (argc >= 2 && std::string(argv[1]) == "report") {
            if (argc < 3 || argc > 4) {
                print_usage(argv[0]);
                return 1;
            }
            const int shanten_filter = argc == 4 ? std::stoi(argv[3]) : 3;
            print_report(create_snapshot(argv[2]), shanten_filter);
            return 0;
        }

        if (argc >= 2 && std::string(argv[1]) == "snapshot") {
            if (argc != 4) {
                print_usage(argv[0]);
                return 1;
            }
            const Snapshot snapshot = create_snapshot(argv[2]);
            write_json(to_json_document(snapshot), argv[3]);
            std::cout << "snapshot saved: " << argv[3]
                      << " total=" << snapshot.summary.total
                      << " ok=" << snapshot.summary.ok
                      << " no_stats=" << snapshot.summary.no_stats
                      << " error=" << snapshot.summary.error << "\n";
            return 0;
        }

        if (argc >= 2 && std::string(argv[1]) == "compare") {
            if (argc != 4 && argc != 5) {
                print_usage(argv[0]);
                return 1;
            }
            const Snapshot baseline = load_snapshot(argv[2]);
            const Snapshot candidate = load_snapshot(argv[3]);
            const SnapshotDiff diff = compare_snapshots(baseline, candidate);
            print_diff(diff);
            if (argc == 5) {
                write_json(to_json_document(diff), argv[4]);
                std::cout << "diff saved: " << argv[4] << "\n";
            }
            return 0;
        }

        if (argc >= 2 && std::string(argv[1]) == "range") {
            if (argc != 3) {
                print_usage(argv[0]);
                return 1;
            }
            const RangeCheckSummary summary = check_probability_ranges(argv[2]);
            std::cout << "summary"
                      << " total=" << summary.total << " ok=" << summary.ok
                      << " no_stats=" << summary.no_stats << " error=" << summary.error
                      << " out_of_range=" << summary.out_of_range
                      << " max_tenpai_prob=" << summary.max_tenpai_prob
                      << " max_win_prob=" << summary.max_win_prob << "\n";
            return summary.out_of_range == 0 && summary.error == 0 ? 0 : 1;
        }

        if (argc < 2 || argc > 3) {
            print_usage(argv[0]);
            return 1;
        }

        const int shanten_filter = argc == 3 ? std::stoi(argv[2]) : 3;
        print_report(create_snapshot(argv[1]), shanten_filter);
        return 0;
    }
    catch (const std::exception &e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
}
