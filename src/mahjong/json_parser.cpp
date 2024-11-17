#include "json_parser.hpp"

#include <algorithm>
#include <sstream>

#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

#include "mahjong/core/score_calculator.hpp"
#include "mahjong/core/shanten_calculator.hpp"
#include "mahjong/core/utils.hpp"

using namespace mahjong;

/**
 * @brief JSON ドキュメントを文字列にする。
 *
 * @param[in] doc ドキュメント
 * @return std::string JSON ドキュメント
 */
std::string to_json_str(rapidjson::Value &value)
{
    std::stringstream ss;
    rapidjson::OStreamWrapper osw(ss);
    rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);
    value.Accept(writer);

    return ss.str();
}

/**
 * @brief JSON ドキュメントを文字列にする。
 *
 * @param[in] doc ドキュメント
 * @return std::string JSON ドキュメント
 */
std::string to_json_str(rapidjson::Document &doc)
{
    std::stringstream ss;
    rapidjson::OStreamWrapper osw(ss);
    rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);
    doc.Accept(writer);

    return ss.str();
}

/**
 * @brief JSON データを解析する。
 *
 * @param[in] doc ドキュメント
 * @return リクエストデータ
 */
RequestData parse_request(const rapidjson::Value &doc)
{
    RequestData req;

    req.zikaze = doc["zikaze"].GetInt();
    req.bakaze = doc["bakaze"].GetInt();
    req.turn = doc["turn"].GetInt();
    req.syanten_type = doc["syanten_type"].GetInt();

    for (const auto &x : doc["dora_indicators"].GetArray())
        req.dora_indicators.push_back(x.GetInt());

    req.flag = doc["flag"].GetInt();

    std::vector<int> hand_tiles;
    for (const auto &x : doc["hand_tiles"].GetArray())
        hand_tiles.push_back(x.GetInt());

    std::vector<MeldedBlock> melds;
    for (const auto &meld : doc["melded_blocks"].GetArray()) {
        int meld_type = meld["type"].GetInt();
        std::vector<int> tiles;
        for (const auto &x : meld["tiles"].GetArray())
            tiles.push_back(x.GetInt());
        int discarded_tile = meld["discarded_tile"].GetInt();
        int from = meld["from"].GetInt();

        melds.emplace_back(meld_type, tiles, discarded_tile, from);
    }
    req.hand = Hand(hand_tiles, melds);

    if (doc.HasMember("ip"))
        req.ip = doc["ip"].GetString();

    if (doc.HasMember("version"))
        req.version = doc["version"].GetString();

    if (doc.HasMember("counts")) {
        for (const auto &x : doc["counts"].GetArray())
            req.counts.push_back(x.GetInt());
    }

    return req;
}

/**
 * @brief JSON データを解析する。
 *
 * @param[in] doc ドキュメント
 * @return リクエストデータ
 */
DiscardResponseData parse_response(const rapidjson::Value &value)
{
    DiscardResponseData res;

    res.syanten = value["syanten"]["syanten"].GetInt();
    res.normal_syanten = value["syanten"]["normal"].GetInt();
    res.tiitoi_syanten = value["syanten"]["tiitoi"].GetInt();
    res.kokusi_syanten = value["syanten"]["kokusi"].GetInt();
    res.time_us = value["time"].GetInt();
    for (const auto &x : value["candidates"].GetArray()) {
        int tile = x["tile"].GetInt();
        bool syanten_down = x["syanten_down"].GetBool();

        std::vector<std::tuple<int, int>> required_tiles;
        for (const auto &y : x["required_tiles"].GetArray()) {
            int tile = y["tile"].GetInt();
            int count = y["count"].GetInt();
            required_tiles.emplace_back(tile, count);
        }

        std::vector<double> exp_values, win_probs, tenpai_probs;
        for (const auto &y : x["tenpai_probs"].GetArray())
            tenpai_probs.push_back(y.GetDouble());
        for (const auto &y : x["win_probs"].GetArray())
            win_probs.push_back(y.GetDouble());
        for (const auto &y : x["exp_values"].GetArray())
            exp_values.push_back(y.GetDouble());

        res.candidates.emplace_back(tile, required_tiles, tenpai_probs, win_probs,
                                    exp_values, syanten_down);
    }

    return res;
}

/**
 * @brief 有効牌の一覧から json オブジェクトを作成する。
 *
 * @param[in] tiles 有効牌の一覧
 * @param[in] doc ドキュメント
 * @return rapidjson::Value 値
 */
rapidjson::Value dump_required_tiles(const std::vector<std::tuple<int, int>> &tiles,
                                     rapidjson::Document &doc)
{
    rapidjson::Value value(rapidjson::kArrayType);
    for (auto [tile, count] : tiles) {
        rapidjson::Value x(rapidjson::kObjectType);
        x.AddMember("tile", tile, doc.GetAllocator());
        x.AddMember("count", count, doc.GetAllocator());
        value.PushBack(x, doc.GetAllocator());
    }

    return value;
}

/**
 * @brief 打牌候補の一覧から json オブジェクトを作成する。
 *
 * @param[in] candidate 打牌候補の一覧
 * @param[in] doc ドキュメント
 * @return rapidjson::Value 値
 */
rapidjson::Value dump_candidate(const Candidate &candidate, rapidjson::Document &doc)
{
    rapidjson::Value value(rapidjson::kObjectType);
    value.AddMember("tile", candidate.tile, doc.GetAllocator());
    value.AddMember("syanten_down", candidate.syanten_down, doc.GetAllocator());
    value.AddMember("required_tiles",
                    dump_required_tiles(candidate.required_tiles, doc),
                    doc.GetAllocator());

    if (!candidate.exp_values.empty()) {
        value.AddMember("exp_values", rapidjson::kArrayType, doc.GetAllocator());
        for (auto x : candidate.exp_values)
            value["exp_values"].PushBack(x, doc.GetAllocator());
    }

    if (!candidate.win_probs.empty()) {
        value.AddMember("win_probs", rapidjson::kArrayType, doc.GetAllocator());
        for (auto x : candidate.win_probs)
            value["win_probs"].PushBack(x, doc.GetAllocator());
    }

    if (!candidate.tenpai_probs.empty()) {
        value.AddMember("tenpai_probs", rapidjson::kArrayType, doc.GetAllocator());
        for (auto x : candidate.tenpai_probs)
            value["tenpai_probs"].PushBack(x, doc.GetAllocator());
    }

    return value;
}

/**
 * @brief リクエストデータからレスポンスデータを作成する。
 *
 * @param[in] req リクエストデータ
 * @return DrawResponseData レスポンスデータ
 */
DrawResponseData create_draw_response(const RequestData &req)
{
    ExpectedValueCalculator exp_value_calc;

    auto begin = std::chrono::steady_clock::now();

    // 点数計算の設定
    Round params;
    params.self_wind = req.zikaze;
    params.wind = req.bakaze;
    params.honba = 0;
    params.kyotaku = 0;
    std::vector<int> dora_tiles;
    for (auto x : req.dora_indicators)
        dora_tiles.push_back(ToDora.at(x));
    params.dora_tiles = dora_tiles;
    exp_value_calc.set_params(params);

    // 各打牌を分析する。
    bool success;
    std::vector<Candidate> candidates;

    if (req.counts.empty())
        std::tie(success, candidates) = exp_value_calc.calc(
            req.hand, req.dora_indicators, req.syanten_type, req.flag);
    else
        std::tie(success, candidates) = exp_value_calc.calc(
            req.hand, req.dora_indicators, req.syanten_type, req.counts, req.flag);

    auto end = std::chrono::steady_clock::now();
    auto elapsed_us =
        std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();

    DrawResponseData res;
    res.required_tiles = candidates.front().required_tiles;
    res.tenpai_probs = candidates.front().tenpai_probs;
    res.win_probs = candidates.front().win_probs;
    res.exp_values = candidates.front().exp_values;
    auto [_, syanten] = ShantenCalculator::calc(
        req.hand.counts, int(req.hand.melds.size()), req.syanten_type);
    res.syanten = syanten;
    res.normal_syanten =
        ShantenCalculator::calc_regular(req.hand.counts, int(req.hand.melds.size()));
    res.tiitoi_syanten = req.hand.melds.empty()
                             ? ShantenCalculator::calc_seven_pairs(req.hand.counts)
                             : -2;
    res.kokusi_syanten = req.hand.melds.empty()
                             ? ShantenCalculator::calc_thirteen_orphans(req.hand.counts)
                             : -2;
    res.time_us = elapsed_us;

    return res;
}

/**
 * @brief リクエストデータからレスポンスデータを作成する。
 *
 * @param[in] req リクエストデータ
 * @return DiscardResponseData レスポンスデータ
 */
DiscardResponseData create_discard_response(const RequestData &req)
{
    ExpectedValueCalculator exp_value_calc;

    auto begin = std::chrono::steady_clock::now();

    // 点数計算の設定
    Round params;
    params.self_wind = req.zikaze;
    params.wind = req.bakaze;
    params.honba = 0;
    params.kyotaku = 0;
    std::vector<int> dora_tiles;
    for (auto x : req.dora_indicators)
        dora_tiles.push_back(ToDora.at(x));
    params.dora_tiles = dora_tiles;
    exp_value_calc.set_params(params);

    // 各打牌を分析する。
    bool success;
    std::vector<Candidate> candidates;

    if (req.counts.empty())
        std::tie(success, candidates) = exp_value_calc.calc(
            req.hand, req.dora_indicators, req.syanten_type, req.flag);
    else
        std::tie(success, candidates) = exp_value_calc.calc(
            req.hand, req.dora_indicators, req.syanten_type, req.counts, req.flag);

    auto end = std::chrono::steady_clock::now();
    auto elapsed_us =
        std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();

    DiscardResponseData res;
    auto [_, syanten] = ShantenCalculator::calc(
        req.hand.counts, int(req.hand.melds.size()), req.syanten_type);
    res.syanten = syanten;
    res.normal_syanten =
        ShantenCalculator::calc_regular(req.hand.counts, int(req.hand.melds.size()));
    res.tiitoi_syanten = req.hand.melds.empty()
                             ? ShantenCalculator::calc_seven_pairs(req.hand.counts)
                             : -2;
    res.kokusi_syanten = req.hand.melds.empty()
                             ? ShantenCalculator::calc_thirteen_orphans(req.hand.counts)
                             : -2;
    res.time_us = elapsed_us;
    res.candidates = candidates;

    return res;
}

/**
 * @brief レスポンスデータから値を作成する。
 *
 * @param[in] res レスポンスデータ
 * @param[in] doc ドキュメント
 * @return rapidjson::Value 値
 */
rapidjson::Value dump_draw_response(const DrawResponseData &res,
                                    rapidjson::Document &doc)
{
    rapidjson::Value syanten_value(rapidjson::kObjectType);
    syanten_value.AddMember("syanten", res.syanten, doc.GetAllocator());
    syanten_value.AddMember("normal", res.normal_syanten, doc.GetAllocator());
    syanten_value.AddMember("tiitoi", res.tiitoi_syanten, doc.GetAllocator());
    syanten_value.AddMember("kokusi", res.kokusi_syanten, doc.GetAllocator());

    rapidjson::Value value(rapidjson::kObjectType);
    value.AddMember("result_type", 0, doc.GetAllocator());
    value.AddMember("syanten", syanten_value, doc.GetAllocator());
    value.AddMember("time", res.time_us, doc.GetAllocator());
    value.AddMember("required_tiles", dump_required_tiles(res.required_tiles, doc),
                    doc.GetAllocator());

    if (!res.exp_values.empty()) {
        value.AddMember("exp_values", rapidjson::kArrayType, doc.GetAllocator());
        for (auto x : res.exp_values)
            value["exp_values"].PushBack(x, doc.GetAllocator());
    }

    if (!res.win_probs.empty()) {
        value.AddMember("win_probs", rapidjson::kArrayType, doc.GetAllocator());
        for (auto x : res.win_probs)
            value["win_probs"].PushBack(x, doc.GetAllocator());
    }

    if (!res.tenpai_probs.empty()) {
        value.AddMember("tenpai_probs", rapidjson::kArrayType, doc.GetAllocator());
        for (auto x : res.tenpai_probs)
            value["tenpai_probs"].PushBack(x, doc.GetAllocator());
    }

    return value;
}

/**
 * @brief レスポンスデータから値を作成する。
 *
 * @param[in] res レスポンスデータ
 * @param[in] doc ドキュメント
 * @return rapidjson::Value 値
 */
rapidjson::Value dump_discard_response(const DiscardResponseData &res,
                                       rapidjson::Document &doc)
{
    rapidjson::Value syanten_value(rapidjson::kObjectType);
    syanten_value.AddMember("syanten", res.syanten, doc.GetAllocator());
    syanten_value.AddMember("normal", res.normal_syanten, doc.GetAllocator());
    syanten_value.AddMember("tiitoi", res.tiitoi_syanten, doc.GetAllocator());
    syanten_value.AddMember("kokusi", res.kokusi_syanten, doc.GetAllocator());

    rapidjson::Value value(rapidjson::kObjectType);
    value.AddMember("result_type", 1, doc.GetAllocator());
    value.AddMember("syanten", syanten_value, doc.GetAllocator());
    value.AddMember("time", res.time_us, doc.GetAllocator());
    value.AddMember("candidates", rapidjson::kArrayType, doc.GetAllocator());
    for (const auto &candidate : res.candidates)
        value["candidates"].PushBack(dump_candidate(candidate, doc),
                                     doc.GetAllocator());

    return value;
}

/**
 * @brief レスポンスデータを作成する。
 *
 * @param[in] res レスポンスデータ
 * @param[in] doc ドキュメント
 * @return rapidjson::Value 値
 */
rapidjson::Value create_response(const RequestData &req, rapidjson::Document &doc)
{
    // 手牌の枚数を求める。
    int n_tiles = req.hand.num_tiles() + int(req.hand.melds.size()) * 3;

    if (n_tiles == 14) {
        DiscardResponseData res = create_discard_response(req);
        return dump_discard_response(res, doc);
    }
    else {
        DrawResponseData res = create_draw_response(req);
        return dump_draw_response(res, doc);
    }
}
