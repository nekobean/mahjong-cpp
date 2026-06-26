#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/writer.h>

#include "score_testcase_converter.hpp"
#include "tools/tenhou/mjlog_parser.hpp"
#include "tools/tenhou/replay_builder.hpp"

namespace tenhou = mahjong::tools::tenhou;

namespace
{

struct Options
{
    std::filesystem::path source_dir = R"(C:\work\mahjong)";
    std::filesystem::path output = "data/testcase/test_score_mjlog.json";
    std::string mode = "all";
};

struct Stats
{
    size_t files = 0;
    size_t agari = 0;
    size_t written = 0;
};

Options parse_options(const int argc, char **argv)
{
    Options options;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        auto read_value = [&](const char *name) -> std::string {
            if (i + 1 >= argc) {
                throw std::runtime_error(std::string("Missing value for ") + name);
            }
            return argv[++i];
        };

        if (arg == "--source-dir") {
            options.source_dir = read_value("--source-dir");
        }
        else if (arg == "--output") {
            options.output = read_value("--output");
        }
        else if (arg == "--mode") {
            options.mode = read_value("--mode");
            if (options.mode != "all" && options.mode != "sanma" &&
                options.mode != "yonma") {
                throw std::runtime_error("Unknown mode: " + options.mode);
            }
        }
        else {
            throw std::runtime_error("Unknown option: " + arg);
        }
    }
    return options;
}

std::vector<std::filesystem::path> collect_mjlog_files(const std::filesystem::path &dir)
{
    std::vector<std::filesystem::path> files;
    for (const auto &entry : std::filesystem::recursive_directory_iterator(dir)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        const auto path = entry.path();
        if (path.extension() == ".mjlog") {
            files.push_back(path);
        }
    }
    std::sort(files.begin(), files.end());
    return files;
}

void write_score_testcases(const std::filesystem::path &path,
                           const std::vector<tenhou::ScoreTestcase> &cases)
{
    std::filesystem::create_directories(path.parent_path());
    std::ofstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open output file: " + path.string());
    }

    rapidjson::OStreamWrapper stream(file);
    rapidjson::Writer<rapidjson::OStreamWrapper> writer(stream);
    writer.StartArray();
    for (const auto &testcase : cases) {
        tenhou::write_score_testcase(writer, testcase);
    }
    writer.EndArray();
}

void print_stats(const Stats &stats)
{
    std::cout << "Files: " << stats.files << '\n'
              << "Agari: " << stats.agari << '\n'
              << "Written: " << stats.written << '\n';
}

bool accepts_mode(const int game_mode, const std::string &filter)
{
    if (filter == "all") {
        return true;
    }
    if (filter == "sanma") {
        return game_mode == mahjong::GameMode::Sanma;
    }
    return game_mode == mahjong::GameMode::Yonma;
}

} // namespace

int main(int argc, char **argv)
{
    try {
        const Options options = parse_options(argc, argv);

        Stats stats;
        std::vector<tenhou::ScoreTestcase> cases;

        const auto files = collect_mjlog_files(options.source_dir);
        for (const auto &path : files) {
            try {
                const auto log = tenhou::parse_mjlog_file(path);
                const auto replay = tenhou::build_replay(log);

                if (!accepts_mode(replay.table.game_mode, options.mode)) {
                    continue;
                }

                ++stats.files;

                for (const auto &round : replay.rounds) {
                    int win_index = 0;
                    for (const auto &result : round.results) {
                        if (const auto *win_result =
                                std::get_if<mahjong::WinResult>(&result)) {
                            ++stats.agari;
                            cases.push_back(tenhou::convert_score_testcase(
                                replay, *win_result, win_index));
                            ++stats.written;
                            ++win_index;
                        }
                    }
                }
            }
            catch (const std::exception &e) {
                throw std::runtime_error(
                    "Failed to convert mjlog file: " + path.string() + ": " + e.what());
            }
        }

        write_score_testcases(options.output, cases);
        print_stats(stats);
        return 0;
    }
    catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
